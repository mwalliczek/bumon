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

#include "Ipv4Addr.h"

Ipv4Addr::Ipv4Addr() {
    this->ip.s_addr = 0;
}

Ipv4Addr::Ipv4Addr(struct in_addr ip): ip(ip) { }

Ipv4Addr::Ipv4Addr(std::string ip) {
    if (1 != inet_pton(AF_INET, ip.c_str(), &(this->ip))) {
        logfile->log(2, "ip %s could not be parsed", ip.c_str());
        this->ip.s_addr = 0;
    } 
}

bool Ipv4Addr::empty() const {
    return 0 == ip.s_addr;
}

std::string Ipv4Addr::toString() const {
    char straddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip, straddr, sizeof(straddr));
    return std::string(straddr);
}

std::string Ipv4Addr::resolve() const {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr = ip;
    socklen_t addrlen = sizeof(addr);
    char hbuf[NI_MAXHOST];

    if (getnameinfo((const sockaddr*)&addr, addrlen, hbuf, sizeof(hbuf), NULL, 0, NI_NAMEREQD)) {
        return toString();
    } else {
        return toString() + " (" + hbuf + ")";
    }
}

bool operator== (Ipv4Addr const&a, Ipv4Addr const&b) {
    return a.ip.s_addr == b.ip.s_addr;
}

bool operator!= (Ipv4Addr const&a, Ipv4Addr const&b) {
    return a.ip.s_addr != b.ip.s_addr;
}

bool operator< (Ipv4Addr const&a, Ipv4Addr const&b) {
    return a.ip.s_addr < b.ip.s_addr;
}

bool operator> (Ipv4Addr const&a, Ipv4Addr const&b) {
    return a.ip.s_addr > b.ip.s_addr;
}

Ipv4Addr operator& (Ipv4Addr const&a, Ipv4Addr const&b) {
    Ipv4Addr result;
    result.ip.s_addr = a.ip.s_addr & b.ip.s_addr;
    return result;
}
