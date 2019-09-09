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

#ifndef SUSPICIOUSEVENTS_H
#define SUSPICIOUSEVENTS_H

#include <map>
#include <memory>
#include <set>

class ProcessSourceInfo {
public:    
    time_t lastTime;
    int ratePerSecond;
};

class ScanSourceInfo {
public:
    time_t lastTime;
    int ratePerHour;
    std::set<int> ports;
};

template<typename IP>
class IPandPort {
public:
    IPandPort(const IP &ip, int dport);
    IP ip;
    int dport;
    template<typename IP_> friend bool operator<(const IPandPort<IP_> &c1, const IPandPort<IP_> &c2);
};

template<typename IP>
bool operator<(const IPandPort<IP> &c1, const IPandPort<IP> &c2) {
    if (c1.ip < c2.ip) {
        return true;
    }
    if (c1.ip > c2.ip) {
        return false;
    }
    if (c1.dport < c2.dport) {
        return true;
    }
    return false;
}

template<typename IP>
class SuspiciousEvents {
    std::map<IPandPort<IP>, ProcessSourceInfo> synToProcesses;
    std::map<IP, ScanSourceInfo> synToScan;
    std::map<IP, std::pair<time_t, int>> score;
    SuspiciousEvents() { }
    
public:
    static std::shared_ptr<SuspiciousEvents> getInstance() {
        static std::shared_ptr<SuspiciousEvents> instance{ new SuspiciousEvents };
        return instance;
    }
    SuspiciousEvents(SuspiciousEvents const&) = delete;
    void operator=(SuspiciousEvents const&) = delete;
    
    void connectionToProcess(IP const & src, int dport);
    void connectionToEmpty(IP const & src, int dport);
    void connectionWithData(IP const & src, int dport, int duration, long long int data);
    void tcpSynTimeout(IP const & src, int dport);
    void iCMPEcho(IP const & src);
    void iCMPUnreachable(IP const & src);
    void cleanup();
};

#endif
