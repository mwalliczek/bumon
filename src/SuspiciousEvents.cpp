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

#include "netimond.h"
#include "Ipv4Addr.h"
#include "Ipv6Addr.h"

#include "SuspiciousEvents.h"

template<typename IP>
IPandPort<IP>::IPandPort(IP const & src, int dport): ip(src), dport(dport) { }

template class IPandPort<Ipv4Addr>;
template class IPandPort<Ipv6Addr>;

template<typename IP>
void SuspiciousEvents<IP>::connectionToProcess(IP const & src, int dport) {
    time_t current;
    time(&current);
    IPandPort<IP> ipAndPort = IPandPort<IP>(src, dport);
    auto iter = synToProcesses.find(ipAndPort);
    if (iter != synToProcesses.end()) {
        int diff = difftime(current, iter->second.lastTime);
        iter->second.ratePerSecond = 1 + iter->second.ratePerSecond / (diff + 1);
        LOG_TRACE("connectionToProcess %s -> %d ratePerSecond: %d (%d)", src.toString().c_str(), dport, 
                iter->second.ratePerSecond, diff);
        iter->second.lastTime = current;
    } else {
        ProcessSourceInfo sourceInfo;
        sourceInfo.ratePerSecond = 1;
        sourceInfo.lastTime = current;
        synToProcesses[ipAndPort] = sourceInfo;
    }
}

template<typename IP>
void SuspiciousEvents<IP>::connectionToEmpty(IP const & src, int dport) {
    time_t current;
    time(&current);
    auto iter = synToScan.find(src);
    if (iter != synToScan.end()) {
        int diff = difftime(current, iter->second.lastTime);
        iter->second.ratePerHour = 1 + iter->second.ratePerHour * 3600 / (diff + 3600);
        LOG_TRACE("connectionToEmpty %s -> %d ratePerHour: %d (%d)", src.toString().c_str(), dport, 
                iter->second.ratePerHour, diff);
        iter->second.lastTime = current;
    } else {
        ScanSourceInfo sourceInfo;
        sourceInfo.lastTime = current;
        sourceInfo.ratePerHour = 1;
        sourceInfo.ports.insert(dport);
        synToScan[src] = sourceInfo;
    }
}

template<typename IP>
void SuspiciousEvents<IP>::tcpSynTimeout(IP const & src, int dport) {
}

template<typename IP>
void SuspiciousEvents<IP>::iCMPEcho(IP const & src) {
}

template<typename IP>
void SuspiciousEvents<IP>::iCMPUnreachable(IP const & src) {
}

template<typename IP>
void SuspiciousEvents<IP>::cleanup() {
}

template class SuspiciousEvents<Ipv4Addr>;
template class SuspiciousEvents<Ipv6Addr>;
