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

#ifndef FINDPROCESS_H
#define FINDPROCESS_H

#include <map>
#include <string>

class FindProcess {
    std::map<u_short, std::string> tcpListen;
    std::map<u_short, std::string> udpListen;
    
    public:
        void init();
        std::string findListenTcpProcess(int port) const;
        std::string findListenUdpProcess(int port) const;
        std::string findActiveTcpProcess(int sport, std::string dst_ip, int dport) const;
};

#endif