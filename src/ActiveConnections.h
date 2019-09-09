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

#include "Subnet.h"
#include "ConnectionIdentifier.h"
#include "Connection.h"
#include "SuspiciousEvents.h"

template<typename IP>
class ActiveConnections {
    std::list<Subnet<IP>> interns;
    std::list<Subnet<IP>> selfs;
    bool isIntern(IP const & ip) const;
protected:
    std::shared_ptr<SuspiciousEvents<IP>> suspiciousEvents;
    Connection* createSimpleConnection(IP const & ip_src, IP const & ip_dst, u_char protocol) const;
    bool isSelf(IP const & ip) const;
public:
    ActiveConnections(std::list<Subnet<IP>> const & interns, std::list<Subnet<IP>> const & selfs);
    void handlePacket(IP const & ip_src, IP const & ip_dst, uint16_t ip_len, u_char protocol) const;
};

#endif
