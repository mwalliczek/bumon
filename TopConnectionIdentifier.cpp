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

#include "TopConnectionIdentifier.h"

TopConnectionIdentifier::TopConnectionIdentifier(Connection* c):
    intern(c->intern), inbound(c->inbound), ip(c->ip), dst_port(c->dst_port), protocol(c->protocol) { }

bool operator<(const TopConnectionIdentifier &c1, const TopConnectionIdentifier &c2) {
    if (c1.intern && !c2.intern) {
        return true;
    }
    if (!c1.intern && c2.intern) {
        return false;
    }
    if (c1.inbound && !c2.inbound) {
        return true;
    }
    if (!c1.inbound && c2.inbound) {
        return false;
    }
    if (c1.dst_port < c2.dst_port) {
        return true;
    }
    if (c1.dst_port > c2.dst_port) {
        return false;
    }
    if (c1.protocol < c2.protocol) {
        return true;
    }
    if (c1.protocol > c2.protocol) {
        return false;
    }
    if (c1.ip < c2.ip) {
        return true;
    }
    if (c1.ip > c2.ip) {
        return false;
    }
    if (c1.text < c2.text) {
        return true;
    }
    return false;
    
}
