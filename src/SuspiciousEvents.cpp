/*
 * Copyright (c) 2019 Matthias Walliczek
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *  http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctime>
#include <sstream>

#include "netimond.h"
#include "Ipv4Addr.h"
#include "Ipv6Addr.h"

#include "SuspiciousEvents.h"

template<typename IP>
IPandPort<IP>::IPandPort(IP const & src, int dport): ip(src), dport(dport) { }

template class IPandPort<Ipv4Addr>;
template class IPandPort<Ipv6Addr>;

template<typename IP>
void SuspiciousEvents<IP>::connectionToProcess(IP const &src, int dport) {
    time_t current;
    time(&current);
    IPandPort<IP> ipAndPort = IPandPort<IP>(src, dport);
    auto iter = synToProcesses.find(ipAndPort);
    if (iter != synToProcesses.end()) {
        int diff = difftime(current, iter->second.lastTime);
        iter->second.ratePerSecond = 1 + iter->second.ratePerSecond / (diff + 1);
        if (LOG_CHECK_TRACE()) {
            LOG_TRACE("connectionToProcess %s -> %d ratePerSecond: %d (%d)", src.toString().c_str(), dport, 
                    iter->second.ratePerSecond, diff);
        }
        iter->second.lastTime = current;
    } else {
        ProcessSourceInfo sourceInfo;
        sourceInfo.ratePerSecond = 1;
        sourceInfo.lastTime = current;
        synToProcesses[ipAndPort] = sourceInfo;
    }
}

template<typename IP>
void SuspiciousEvents<IP>::connectionToEmpty(IP const &src, int dport) {
    time_t current;
    time(&current);
    auto iter = synToScan.find(src);
    if (iter != synToScan.end()) {
        int diff = difftime(current, iter->second.lastTime);
        iter->second.ratePer15m = 1 + iter->second.ratePer15m * 900 / (diff + 900);
        iter->second.ports.insert(dport);
        if (LOG_CHECK_TRACE()) {
            std::stringstream ports_stream;
            for (auto it : iter->second.ports) {
                ports_stream << it << ", ";
            }
            std::string ports_string = ports_stream.str();
            LOG_TRACE("connectionToEmpty %s -> %d [%s] ratePer15m: %d (%d)", src.toString().c_str(), dport, 
                    ports_string.substr(0, ports_string.length()-2).c_str(), iter->second.ratePer15m, diff);
        }
        if (iter->second.ratePer15m >= 10) {
            addScore(src, iter->second.ports.size() * iter->second.ratePer15m / 10);
            iter->second.ratePer15m %= 10;
        }
        iter->second.lastTime = current;
    } else {
        ScanSourceInfo sourceInfo;
        sourceInfo.lastTime = current;
        sourceInfo.ratePer15m = 1;
        sourceInfo.ports.insert(dport);
        synToScan[src] = sourceInfo;
    }
}

template<typename IP>
void SuspiciousEvents<IP>::tcpSynTimeout(IP const &src, int dport) {
    if (LOG_CHECK_TRACE()) {
        LOG_TRACE("tcpSynTimeout %s -> %d", src.toString().c_str(), dport);
    }
}

template<typename IP>
void SuspiciousEvents<IP>::iCMPEcho(IP const &src) {
    if (LOG_CHECK_TRACE()) {
        LOG_TRACE("ICMPEcho %s", src.toString().c_str());
    }
}

template<typename IP>
void SuspiciousEvents<IP>::iCMPUnreachable(IP const &src) {
    if (LOG_CHECK_TRACE()) {
        LOG_TRACE("ICMPUnreachable %s", src.toString().c_str());
    }
}

template<typename IP>
void SuspiciousEvents<IP>::connectionWithData(IP const & src, int dport, unsigned int duration, long long unsigned int data) {
    if (LOG_CHECK_TRACE()) {
        LOG_TRACE("connectionWithData %s (%d), %d %llu", src.toString().c_str(), dport, duration, data);
    }
    long long unsigned int dataScore = data/1000;
    long long signed int dataSumScore = (duration+dataScore)/10;
    addScore(src, -dataSumScore);
}

template<typename IP>
void checkScore(IP const & src, int score) {
    if (score >= suspiciousEventsAlarm) {
        LOG_INFO("score %s = %d", src.toString().c_str(), score);
    }
}

template<typename IP>
void SuspiciousEvents<IP>::addScore(IP const & src, int addScore) {
    if (addScore == 0) {
        return;
    }
    if (addScore > 100) {
        addScore = 100;
    }
    if (addScore < -1000) {
        addScore = -1000;
    }
    time_t current;
    time(&current);
    auto iter = score.find(src);
    if (iter != score.end()) {
        int minutesDiff = difftime(current, iter->second.first) / 300 + 1;
        iter->second.second = iter->second.second / minutesDiff / minutesDiff + addScore;
        iter->second.first = current;
        if (addScore > 0) {
            if (LOG_CHECK_DEBUG()) {
                LOG_DEBUG("addScore %s +%d = %d", src.toString().c_str(), addScore, iter->second.second);
            }
            checkScore(src, iter->second.second);
        } else if (LOG_CHECK_DEBUG()) {
            LOG_DEBUG("addScore %s %d = %d", src.toString().c_str(), addScore, iter->second.second);
            if (iter->second.second < -100000) {
                iter->second.second = -100000;
            }
        }
    } else {
        score[src] = std::pair(current, addScore);
        if (addScore > 0) {
            if (LOG_CHECK_DEBUG()) {
                LOG_DEBUG("addScore %s +%d", src.toString().c_str(), addScore);
            }
            checkScore(src, addScore);
        } else if (LOG_CHECK_DEBUG()) {
            LOG_DEBUG("addScore %s %d", src.toString().c_str(), addScore);
        }
    }
    
}

template<typename IP>
void SuspiciousEvents<IP>::cleanup() {
    time_t current;
    time(&current);
    auto iterProcess = synToProcesses.begin();
    while (iterProcess != synToProcesses.end())  {
        int diff = difftime(current, iterProcess->second.lastTime);
        if (diff > 30) {
            if (LOG_CHECK_TRACE()) {
                LOG_TRACE("connectionToProcess %s -> %d cleanup after %d", iterProcess->first.ip.toString().c_str(),
                        iterProcess->first.dport, diff);
            }
            iterProcess = synToProcesses.erase(iterProcess);
            continue;
        }
        iterProcess++;
    }
    auto iterScan = synToScan.begin();
    while (iterScan != synToScan.end())  {
        int diff = difftime(current, iterScan->second.lastTime);
        if (diff > 7200) {
            if (LOG_CHECK_TRACE()) {
                LOG_TRACE("connectionToEmpty %s cleanup after %d", iterScan->first.toString().c_str(), diff);
            }
            iterScan = synToScan.erase(iterScan);
            continue;
        }
        iterScan++;
    }
    auto iterScore = score.begin();
    while (iterScore != score.end())  {
        int diff = difftime(current, iterScore->second.first);
        int minutesDiff = diff / 300 + 1;
        if (minutesDiff > 1 && iterScore->second.second < minutesDiff * minutesDiff) {
            if (LOG_CHECK_TRACE()) {
                LOG_TRACE("score %s cleanup after %d", iterScore->first.toString().c_str(), diff);
            }
            iterScore = score.erase(iterScore);
            continue;
        }
        iterScore++;
    }
}

template class SuspiciousEvents<Ipv4Addr>;
template class SuspiciousEvents<Ipv6Addr>;
