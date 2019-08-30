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

#ifndef CONNECTIONIDENTIFIER_H
#define CONNECTIONIDENTIFIER_H

#include <string>

template<typename IP>
class ConnectionIdentifier {
    IP ip_src;
    int sport;
    IP ip_dst;
    int dport;

public:
    ConnectionIdentifier(IP ip_src, int sport, IP ip_dst, int dport);
    std::string toString() const;
    template<typename IP_> friend bool operator<(const ConnectionIdentifier<IP_> &c1, const ConnectionIdentifier<IP_> &c2);
};

template<typename IP>
bool operator<(const ConnectionIdentifier<IP> &c1, const ConnectionIdentifier<IP> &c2) {
    if (c1.ip_src < c2.ip_src) {
        return true;
    }
    if (c1.ip_src > c2.ip_src) {
        return false;
    }
    if (c1.sport < c2.sport) {
        return true;
    }
    if (c1.sport > c2.sport) {
        return false;
    }
    if (c1.ip_dst < c2.ip_dst) {
        return true;
    }
    if (c1.ip_dst > c2.ip_dst) {
        return false;
    }
    if (c1.dport < c2.dport) {
        return true;
    }
    return false;
}

#endif