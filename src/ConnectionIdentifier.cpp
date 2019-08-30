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

#include <sstream>

#include "ConnectionIdentifier.h"
#include "Ipv4Addr.h"
#include "Ipv6Addr.h"

template<typename IP>
ConnectionIdentifier<IP>::ConnectionIdentifier(IP ip_src, int sport, IP ip_dst, int dport):
        ip_src(ip_src), sport(sport), ip_dst(ip_dst), dport(dport) { }

template<typename IP>
std::string ConnectionIdentifier<IP>::toString() const {
    std::stringstream result;
    result << ip_src.toString() << ":" << sport;
    result << " > " << ip_dst.toString() << ":" << dport;
    return result.str();
}

template class ConnectionIdentifier<Ipv4Addr>;
template class ConnectionIdentifier<Ipv6Addr>;
