/*
   Copyright 2019 Matthias Walliczek

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <arpa/inet.h>

#include <cppunit/extensions/HelperMacros.h>

#include <map>

#include "bumon.h"
#include "Connection.h"
#include "FindProcess.h"
#include "ActiveUdpConnections.h"

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
 struct in_addr test_src_addr, test_dst_addr;
 inet_pton(AF_INET, "10.31.1.100", &test_src_addr);
 inet_pton(AF_INET, "10.31.1.100", &self_ip);
 inet_pton(AF_INET, "10.69.1.1", &test_dst_addr);
 
 activeUdpConnections->handlePacket(test_src_addr, test_dst_addr, 50, (const unsigned char *) "\x05\xdc\x40\x42\x00\x55\xdc\x6b\x4e\x00\x00\x00\x00\x00\x00"); // Port 1500 -> 16450 UDP, length 77
 
 CPPUNIT_ASSERT(allConnections.size() == 1);
 Connection* con = allConnections.begin()->second;
 CPPUNIT_ASSERT(con != NULL);
 CPPUNIT_ASSERT(con->src_port == 1500);
 CPPUNIT_ASSERT(con->dst_port == 16450);
 
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
}

