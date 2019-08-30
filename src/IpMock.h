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

#ifndef IPMOCK_H
#define IPMOCK_H

#include "Ip.h"

class IpMock : public Ip {
public:
    IpMock(ActiveTcpConnections<Ipv4Addr> *activev4TcpConnections, ActiveUdpConnections<Ipv4Addr> *activev4UdpConnections);
    void checkTimeout() override;
};

#endif