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

#include <arpa/inet.h>
#include <string.h>

#include "InternNet.h"
#include "Ipv4Addr.h"
#include "Ipv6Addr.h"
#include "netimond.h"

template <>
Ipv4Addr InternNet<Ipv4Addr>::calcMask(std::string bits) {
    struct in_addr newMask;
    newMask.s_addr = 0;
    int cidr;
    try {
        cidr = std::stoi(bits);
    } catch (std::invalid_argument& e) {
        LOG_WARN("Can not parse mask %s (%s)", bits.c_str(), e.what());
        return Ipv4Addr(newMask);
    } 
    if (cidr < 1 || cidr > 32) {
        LOG_WARN("Invalid mask %s", bits.c_str());
        return Ipv4Addr(newMask);
    }
    int ocets = (cidr + 7) / 8;
    memset(&newMask.s_addr, 255, (size_t)ocets - 1);
    memset((unsigned char *)&newMask.s_addr + (ocets - 1),
                       (256 - (1 << (32 - cidr) % 8)), 1);
    return Ipv4Addr(newMask);
}

template <>
Ipv6Addr InternNet<Ipv6Addr>::calcMask(std::string bits) {
    struct in6_addr newMask = IN6ADDR_ANY_INIT;
    int cidr;
    try {
        cidr = std::stoi(bits);
    } catch (std::invalid_argument& e) {
        LOG_WARN("Can not parse mask %s (%s)", bits.c_str(), e.what());
        return Ipv6Addr(newMask);
    } 
    if (cidr < 1 || cidr > 128) {
        LOG_WARN("Invalid mask %s", bits.c_str());
        return Ipv6Addr(newMask);
    }
    for(size_t i=0; i<16; i++) {
        uint8_t byte_netmask = 0xff;
        if(cidr >= 8) {
            cidr -= 8;
        } else if(cidr == 0) {
            byte_netmask = 0;
        } else {    // routing_prefix is between 1 and 7, inclusive
            byte_netmask <<= (8 - cidr);
            cidr = 0;
        }
        newMask.s6_addr[i] = byte_netmask;
    }
    return Ipv6Addr(newMask);
}
