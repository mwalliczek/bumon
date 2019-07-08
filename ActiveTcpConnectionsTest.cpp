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

#include "bumon.h"
#include "Connection.h"
#include "FindProcess.h"
#include "ActiveTcpConnections.h"

class ActiveTcpConnectionsTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( ActiveTcpConnectionsTest );
 CPPUNIT_TEST( testAddOutbound );
 CPPUNIT_TEST( testAddInbound );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testAddOutbound();
  void testAddInbound();
};

CPPUNIT_TEST_SUITE_REGISTRATION( ActiveTcpConnectionsTest );

void ActiveTcpConnectionsTest::testAddOutbound() {
 struct in_addr test_src_addr, test_dst_addr;
 inet_pton(AF_INET, "10.31.1.100", &test_src_addr);
 inet_pton(AF_INET, "10.31.1.100", &self_ip);
 inet_pton(AF_INET, "10.69.1.1", &test_dst_addr);
 
 activeTcpConnections->handlePacket(test_src_addr, test_dst_addr, 50, (const unsigned char *) "\xab\xf6\x60\xc6\x0e\xa8\x96\x69\x76\xc0\x2e\xef\x50\x10\x01\x01\x7b\x50\x00\x00", 0); // Port 44022 -> 24774 Flags [.], ack 4294967216, win 257, length 0
 
 CPPUNIT_ASSERT(allConnections.size() == 1);
 Connection* con = allConnections.begin()->second;
 CPPUNIT_ASSERT(con != NULL);
 CPPUNIT_ASSERT(con->src_port == 44022);
 CPPUNIT_ASSERT(con->dst_port == 24774);
 
 struct tm * timeinfo = localtime ( &con->begin );
 timeinfo->tm_min--;
 
 con->begin = mktime(timeinfo);
 
 watching->watching();
 CPPUNIT_ASSERT(con->end != 0);
 
 timeinfo = localtime ( &con->end );
 timeinfo->tm_min -= 10;
 
 con->end = mktime(timeinfo);
 
 watching->watching();
 CPPUNIT_ASSERT(allConnections.size() == 0);
}

void ActiveTcpConnectionsTest::testAddInbound() {
 struct in_addr test_src_addr, test_dst_addr;
 inet_pton(AF_INET, "10.31.1.100", &test_dst_addr);
 inet_pton(AF_INET, "10.31.1.100", &self_ip);
 inet_pton(AF_INET, "10.69.1.1", &test_src_addr);
 
 activeTcpConnections->handlePacket(test_src_addr, test_dst_addr, 50, (const unsigned char *) "\xaa\xbb\x00\x35\x47\xd3\x9a\x3d\x00\x00\x00\x00\x50\x02\x72\x10\x5e\x31\x00\x00", 0); // Flags [S], seq 1205049917, win 29200, length 0
 
 CPPUNIT_ASSERT(allConnections.size() == 1);
 Connection* con = allConnections.begin()->second;
 CPPUNIT_ASSERT(con != NULL);
 CPPUNIT_ASSERT(con->src_port == 43707);
 CPPUNIT_ASSERT(con->dst_port == 53);
 
 struct tm * timeinfo = localtime ( &con->begin );
 timeinfo->tm_min--;
 
 con->begin = mktime(timeinfo);
 
 watching->watching();
 CPPUNIT_ASSERT(con->end != 0);
 
 timeinfo = localtime ( &con->end );
 timeinfo->tm_min -= 10;
 
 con->end = mktime(timeinfo);
 
 watching->watching();
 CPPUNIT_ASSERT(allConnections.size() == 0);
}

