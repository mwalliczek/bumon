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

#ifndef IPV6ADDR_H
#define IPV6ADDR_H

#include <arpa/inet.h>
#include <netinet/in.h>

#include "IpAddr.h"

class Ipv6Addr : public IpAddr {
    struct in6_addr ip;
    Ipv6Addr();
public:
    Ipv6Addr(struct in6_addr ip);
    Ipv6Addr(std::string ip);
    std::string toString() const;
    bool empty() const;
    
    friend bool operator== (Ipv6Addr const&a, Ipv6Addr const&b);
    friend bool operator!= (Ipv6Addr const&a, Ipv6Addr const&b);
    friend bool operator< (Ipv6Addr const&a, Ipv6Addr const&b);
    friend bool operator> (Ipv6Addr const&a, Ipv6Addr const&b);
    friend Ipv6Addr operator& (Ipv6Addr const&a, Ipv6Addr const&b);
};

#endif
