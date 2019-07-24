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

#include "ConnectionIdentifier.h"

ConnectionIdentifier::ConnectionIdentifier(struct in_addr ip_src, int sport, struct in_addr ip_dst, int dport) {
    this->ip_src = ip_src;
    this->sport = sport;
    this->ip_dst = ip_dst;
    this->dport = dport;
}

bool operator<(const ConnectionIdentifier &c1, const ConnectionIdentifier &c2) {
    if (c1.ip_src.s_addr < c2.ip_src.s_addr) {
        return true;
    }
    if (c1.ip_src.s_addr > c2.ip_src.s_addr) {
        return false;
    }
    if (c1.sport < c2.sport) {
        return true;
    }
    if (c1.sport > c2.sport) {
        return false;
    }
    if (c1.ip_dst.s_addr < c2.ip_dst.s_addr) {
        return true;
    }
    if (c1.ip_dst.s_addr > c2.ip_dst.s_addr) {
        return false;
    }
    if (c1.dport < c2.dport) {
        return true;
    }
    return false;
}
