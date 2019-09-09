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

#ifndef ICMPV6_H
#define ICMPV6_H

#include "ActiveConnections.h"

#include "Ipv6Addr.h"

class ICMPv6 : public ActiveConnections<Ipv6Addr> {
    std::string getTypeString(unsigned char) const;
public:
    ICMPv6(std::list<Subnet<Ipv6Addr>> const & interns, std::list<Subnet<Ipv6Addr>> const & selfs);
    void handlePacket(Ipv6Addr const & ip_src, Ipv6Addr const & ip_dst, uint16_t ip_len, const u_char *packet) const;
};

#endif
