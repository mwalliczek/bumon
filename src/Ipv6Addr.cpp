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
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "Logfile.h"
#include "bumon.h"

#include "Ipv6Addr.h"

Ipv6Addr::Ipv6Addr(): ip(IN6ADDR_ANY_INIT) { }

Ipv6Addr::Ipv6Addr(struct in6_addr ip): ip(ip) { }

Ipv6Addr::Ipv6Addr(std::string ip) {
    if (1 != inet_pton(AF_INET6, ip.c_str(), &(this->ip))) {
        logfile->log(2, "ip %s could not be parsed", ip.c_str());
        this->ip = IN6ADDR_ANY_INIT;
    } 
}

bool Ipv6Addr::empty() const {
    return IN6_IS_ADDR_UNSPECIFIED(&ip);
}


std::string Ipv6Addr::toString() const {
    char straddr[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &ip, straddr, sizeof(straddr));
    return std::string(straddr);
}

std::string Ipv6Addr::resolve() const {
    struct sockaddr_in6 addr;
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = ip;
    socklen_t addrlen = sizeof(addr);
    char hbuf[NI_MAXHOST];

    if (getnameinfo((const sockaddr*)&addr, addrlen, hbuf, sizeof(hbuf), NULL, 0, NI_NAMEREQD)) {
        return toString();
    } else {
        return toString() + " (" + hbuf + ")";
    }
}

bool operator== (Ipv6Addr const&a, Ipv6Addr const&b) {
    for(size_t i=0; i<16; i++) {
        if (a.ip.s6_addr[i] != b.ip.s6_addr[i]) {
            return false;
        }
    }
    return true;
}

bool operator!= (Ipv6Addr const&a, Ipv6Addr const&b) {
    return !(a == b);
}

bool operator< (Ipv6Addr const&a, Ipv6Addr const&b) {
    for(size_t i=0; i<16; i++) {
        if (a.ip.s6_addr[i] < b.ip.s6_addr[i]) {
            return true;
        }
        if (a.ip.s6_addr[i] > b.ip.s6_addr[i]) {
            return false;
        }
    }
    return false;
}

bool operator> (Ipv6Addr const&a, Ipv6Addr const&b) {
    for(size_t i=0; i<16; i++) {
        if (a.ip.s6_addr[i] > b.ip.s6_addr[i]) {
            return true;
        }
        if (a.ip.s6_addr[i] < b.ip.s6_addr[i]) {
            return false;
        }
    }
    return false;
}

Ipv6Addr operator& (Ipv6Addr const&a, Ipv6Addr const&b) {
    Ipv6Addr result;
    for(size_t i=0; i<16; i++) {
        result.ip.s6_addr[i] = a.ip.s6_addr[i] & b.ip.s6_addr[i];
    }
    return result;
}
