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

#include "ICMP.h"
#include "bumon.h"

template<typename IP>
ICMP<IP>::ICMP(std::list<InternNet<IP>> interns, std::list<IP> selfs):
    ActiveConnections<IP>(interns, selfs) { }

std::string getTypeString(u_char type) {
    std::string result = std::to_string(type) + " (";
    switch (type) {
        case 0:
            return result + "Echo Reply)";
        case 3:
            return result + "Destination Unreachable)"; 
        case 8:
            return result + "Echo Request)";
        default:
            return result + "Unknown)";
    }
}

template<typename IP>
void ICMP<IP>::handlePacket(IP src, IP dst, uint16_t ip_len, const u_char *packet) const {
    std::string typeString = getTypeString(packet[0]);
    logfile->log(2, " %s > %s ICMP %s %d", src.toString().c_str(),
            dst.toString().c_str(), typeString.c_str(), packet[1]);
    Connection* connection = this->createSimpleConnection(src, dst, IPPROTO_ICMP);
    connection->content = typeString;
    connection->handleData(ip_len);
    connection->stop();
}

template class ICMP<Ipv4Addr>;
template class ICMP<Ipv6Addr>;
