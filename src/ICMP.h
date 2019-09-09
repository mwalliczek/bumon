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

#ifndef ICMP_H
#define ICMP_H

#include "ActiveConnections.h"

#include "Ipv4Addr.h"

class ICMP : public ActiveConnections<Ipv4Addr> {
    std::string getTypeString(unsigned char) const;
public:
    ICMP(std::list<Subnet<Ipv4Addr>> const & interns, std::list<Subnet<Ipv4Addr>> const & selfs);
    void handlePacket(Ipv4Addr const & ip_src, Ipv4Addr const & ip_dst, uint16_t ip_len, const u_char *packet) const;
};

#endif
