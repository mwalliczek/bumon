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

#include <arpa/inet.h>

#include <cppunit/extensions/HelperMacros.h>

#include <map>

#include "netimond.h"
#include "Connection.h"
#include "FindProcess.h"
#include "ActiveUdpConnections.h"
#include "IpMock.h"

class ActiveUdpConnectionsTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( ActiveUdpConnectionsTest );
 CPPUNIT_TEST( testAddOutbound );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testAddOutbound();
};

CPPUNIT_TEST_SUITE_REGISTRATION( ActiveUdpConnectionsTest );

void ActiveUdpConnectionsTest::testAddOutbound() {
 std::list<InternNet<Ipv4Addr>> interns { InternNet<Ipv4Addr>((char*) "10.69.0.0", (char*) "255.255.0.0"),
        InternNet<Ipv4Addr>((char*) "10.133.96.0", (char*) "255.255.224.0") };
 std::list<Ipv4Addr> selfs { Ipv4Addr((char*) "10.31.1.100") };

 ActiveUdpConnections<Ipv4Addr>* activeUdpConnections = new ActiveUdpConnections<Ipv4Addr>(interns, selfs);
 ip = new IpMock(NULL, activeUdpConnections);
 
 activeUdpConnections->handlePacket(Ipv4Addr((char*) "10.31.1.100"), Ipv4Addr((char*) "10.69.1.1"), 50, (const unsigned char *) "\x05\xdc\x40\x42\x00\x55\xdc\x6b\x4e\x00\x00\x00\x00\x00\x00"); // Port 1500 -> 16450 UDP, length 77
 
 CPPUNIT_ASSERT(allConnections.size() == 1);
 Connection* con = allConnections.begin()->second;
 CPPUNIT_ASSERT(con != NULL);
 CPPUNIT_ASSERT(con->dst_port == 1500);
 
 watching->watching();
 CPPUNIT_ASSERT(allConnections.size() == 1);
 
 struct tm * timeinfo = localtime ( &con->lastAct );
 timeinfo->tm_min -= 11;
 
 con->lastAct = mktime(timeinfo);
 
 watching->watching();
 CPPUNIT_ASSERT(con->end != 0);
 
 timeinfo = localtime ( &con->end );
 timeinfo->tm_min -= 10;
 
 con->end = mktime(timeinfo);
 
 watching->watching();
 CPPUNIT_ASSERT(allConnections.size() == 0);
 delete ip;
}

