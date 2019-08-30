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
#include "Ipv4Addr.h"
#include "IpMock.h"

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
 std::list<InternNet<Ipv4Addr>> interns { InternNet<Ipv4Addr>((char*) "10.69.0.0", (char*) "255.255.0.0"),
        InternNet<Ipv4Addr>((char*) "10.133.96.0", (char*) "255.255.224.0") };
 std::list<Ipv4Addr> selfs { Ipv4Addr((char*) "10.31.1.100") };

 ActiveTcpConnections<Ipv4Addr>* activeTcpConnections = new ActiveTcpConnections<Ipv4Addr>(interns, selfs);
 ip = new IpMock(activeTcpConnections, NULL);
 
 CPPUNIT_ASSERT(allConnections.size() == 0);
 activeTcpConnections->handlePacket(Ipv4Addr((char*) "10.31.1.100"), Ipv4Addr((char*) "10.69.1.1"), 50, (const unsigned char *) "\xab\xf6\x60\xc6\x0e\xa8\x96\x69\x76\xc0\x2e\xef\x50\x10\x01\x01\x7b\x50\x00\x00", 0); // Port 44022 -> 24774 Flags [.], ack 4294967216, win 257, length 0
 
 CPPUNIT_ASSERT(allConnections.size() == 1);
 Connection* con = allConnections.begin()->second;
 CPPUNIT_ASSERT(con != NULL);
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
 delete ip;
}

void ActiveTcpConnectionsTest::testAddInbound() {
 std::list<InternNet<Ipv4Addr>> interns { InternNet<Ipv4Addr>((char*) "10.69.0.0", (char*) "255.255.0.0"),
        InternNet<Ipv4Addr>((char*) "10.133.96.0", (char*) "255.255.224.0") };
 std::list<Ipv4Addr> selfs { Ipv4Addr((char*) "10.31.1.100") };
 ActiveTcpConnections<Ipv4Addr>* activeTcpConnections = new ActiveTcpConnections<Ipv4Addr>(interns, selfs);
 ip = new IpMock(activeTcpConnections, NULL);
 
 activeTcpConnections->handlePacket(Ipv4Addr((char*) "10.69.1.1"), Ipv4Addr((char*) "10.31.1.100"), 50, (const unsigned char *) "\xaa\xbb\x00\x35\x47\xd3\x9a\x3d\x00\x00\x00\x00\x50\x02\x72\x10\x5e\x31\x00\x00", 0); // Flags [S], seq 1205049917, win 29200, length 0
 
 CPPUNIT_ASSERT(allConnections.size() == 1);
 Connection* con = allConnections.begin()->second;
 CPPUNIT_ASSERT(con != NULL);
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
 delete ip;
}

