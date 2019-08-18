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

#ifndef IPV4ADDR_H
#define IPV4ADDR_H

#include <arpa/inet.h>
#include <netinet/in.h>

#include "IpAddr.h"

class Ipv4Addr : public IpAddr {
    struct in_addr ip;
public:
    Ipv4Addr(struct in_addr ip);
    Ipv4Addr(std::string ip);
    std::string toString() const;
    bool empty() const;
    
    friend bool operator== (const Ipv4Addr &a, const Ipv4Addr &b);
    friend bool operator!= (const Ipv4Addr &a, const Ipv4Addr &b);
    friend bool operator< (const Ipv4Addr &a, const Ipv4Addr &b);
    friend bool operator> (const Ipv4Addr &a, const Ipv4Addr &b);
    friend Ipv4Addr& operator& (Ipv4Addr &a, const Ipv4Addr &b);
};

#endif
