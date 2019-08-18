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

#ifndef ACTIVECONNECTIONS_H
#define ACTIVECONNECTIONS_H

#include <list>
#include <map>
#include <mutex>

#include "InternNet.h"
#include "ConnectionIdentifier.h"
#include "Connection.h"

template<typename IP>
class ActiveConnections {
    std::list<InternNet<IP>> interns;
    std::list<IP> selfs;
    bool isIntern(IP ip) const;
protected:
    Connection* createSimpleConnection(IP ip_src, IP ip_dst, u_char protocol) const;
    bool isSelf(IP ip) const;
public:
    ActiveConnections(std::list<InternNet<IP>> interns, std::list<IP> selfs);
    void handlePacket(IP ip_src, IP ip_dst, uint16_t ip_len, u_char protocol) const;
};

#endif
