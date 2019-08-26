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

#include "IpMock.h"

IpMock::IpMock(ActiveTcpConnections<Ipv4Addr> *activev4TcpConnections, 
		ActiveUdpConnections<Ipv4Addr> *activev4UdpConnections) : Ip(activev4TcpConnections, NULL,
		activev4UdpConnections, NULL, NULL, NULL) { }

void IpMock::checkTimeout() {
	if (NULL != activev4TcpConnections) {
		activev4TcpConnections->checkTimeout();
	}
	if (NULL != activev4UdpConnections) {
		activev4UdpConnections->checkTimeout();
	}
}
