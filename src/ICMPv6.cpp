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

#include "ICMPv6.h"
#include "netimond.h"

ICMPv6::ICMPv6(std::list<Subnet<Ipv6Addr>> const & interns, std::list<Subnet<Ipv6Addr>> const & selfs):
    ActiveConnections<Ipv6Addr>(interns, selfs) { }

std::string ICMPv6::getTypeString(u_char type) const {
    std::string result = std::to_string(type) + " (";
    switch (type) {
        case 1:
            return result + "Destination Unreachable)"; 
        case 128:
            return result + "Echo Reply)";
        case 129:
            return result + "Echo Request)";
        default:
            return result + "Unknown)";
    }
}

void ICMPv6::handlePacket(Ipv6Addr const & src, Ipv6Addr const & dst, uint16_t ip_len, const u_char *packet) const {
    std::string typeString = getTypeString(packet[0]);
    LOG_INFO(" %s > %s ICMPv6 %s %d", src.toString().c_str(), dst.toString().c_str(), typeString.c_str(), packet[1]);
    Connection* connection = this->createSimpleConnection(src, dst, IPPROTO_ICMPV6);
    connection->content = typeString;
    connection->handleData(ip_len);
    connection->stop();
}
