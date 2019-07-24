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

#ifndef ACTIVETCPCONNECTIONS_H
#define ACTIVETCPCONNECTIONS_H

#include <string>
#include <map>
#include <mutex>

#include "Connection.h"
#include "TrafficManager.h"
#include "ConnectionIdentifier.h"

class ActiveTcpConnections {
    std::map<ConnectionIdentifier, int> map;
    std::mutex map_mutex;
    
    public:
        void handlePacket(struct in_addr ip_src, struct in_addr ip_dst, uint16_t ip_len, const u_char *packet, int size_ip);
        void checkTimeout();
};

#endif
