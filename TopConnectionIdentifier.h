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

#ifndef TOPCONNECTIONIDENTIFIER_H
#define TOPCONNECTIONIDENTIFIER_H

#include "Connection.h"

class TopConnectionIdentifier {
public:
    TopConnectionIdentifier(Connection* c);
    bool intern;
    bool inbound;
    std::string ip;
    u_short dst_port;
    u_char protocol;
    std::string text;
    
    friend bool operator<(const TopConnectionIdentifier &c1, const TopConnectionIdentifier &c2);
};

#endif
