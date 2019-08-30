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

#ifndef SUMCONNECTIONIDENTIFIER_H
#define SUMCONNECTIONIDENTIFIER_H

#include "Connection.h"

class SumConnectionIdentifier {
public:
    SumConnectionIdentifier(Connection* c);
    bool intern;
    bool inbound;
    std::shared_ptr<IpAddr> ipAddr;
    std::string ip;
    u_short dst_port;
    u_char protocol;
    std::string process;
    std::string content;
    
    friend bool operator<(const SumConnectionIdentifier &c1, const SumConnectionIdentifier &c2);
};

#endif
