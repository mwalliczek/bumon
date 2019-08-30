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

#include "bumon.h"
#include "Stats.h"

class StatsTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( StatsTest );
 CPPUNIT_TEST( testCheckSpike );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testCheckSpike();
};

CPPUNIT_TEST_SUITE_REGISTRATION( StatsTest );

void StatsTest::testCheckSpike() {
 MySql* mysql = new MySql((char *)"localhost", (char *)"bumondb", (char *)"testuser", (char *)"testpwd");
 
 Stats* sut = new Stats(mysql, (char *)"root", (char *)"nobody", -1, -1);
 
 Connection* con1 = new Connection(std::shared_ptr<IpAddr>(new Ipv4Addr("192.168.1.1")), 80, IPPROTO_TCP, "process", false, false);
 SumConnectionIdentifier id = SumConnectionIdentifier(con1);
 
 sut->insert((char*) "2019-06-01 17:00:00", id, 2048);

 sut->cleanup((char*) "2019-06-02 17:00:00");
 sut->insert((char*) "2019-06-02 17:00:00", id, 2047);

 sut->cleanup((char*) "2019-06-03 17:00:00");
 sut->insert((char*) "2019-06-03 17:00:00", id, 2046);

 sut->cleanup((char*) "2019-06-04 17:00:00");
 sut->insert((char*) "2019-06-04 17:00:00", id, 2045);

 sut->cleanup((char*) "2019-06-05 17:00:00");
 sut->insert((char*) "2019-06-05 17:00:00", id, 2044);

 sut->cleanup((char*) "2019-06-06 17:00:00");
 sut->insert((char*) "2019-06-06 17:00:00", id, 2043);

 sut->cleanup((char*) "2019-06-07 17:00:00");
 sut->insert((char*) "2019-06-07 17:00:00", id, 2049);

 sut->cleanup((char*) "2019-06-08 17:00:00");
 sut->insert((char*) "2019-06-08 17:00:00", id, 2050);

 sut->cleanup((char*) "2019-06-09 17:00:00");
 sut->insert((char*) "2019-06-09 17:00:00", id, 2051);

 sut->cleanup((char*) "2019-06-10 17:00:00");
 sut->insert((char*) "2019-06-10 17:00:00", id, 2052);
 
 sut->cleanup((char*) "2019-07-01 17:00:00");
 sut->insert((char*) "2019-07-01 17:00:00", id, 20480);
 delete con1;

 con1 = new Connection(std::shared_ptr<IpAddr>(new Ipv4Addr("192.168.1.2")), 80, IPPROTO_TCP, "process2", false, false);
 id = SumConnectionIdentifier(con1);
 sut->insert((char*) "2019-07-01 17:00:00", id, 10240);
 delete con1;

 Connection* con2 = new Connection(std::shared_ptr<IpAddr>(new Ipv4Addr("192.168.1.1")), 443, IPPROTO_TCP, "process3", false, false);
 sut->insert((char*) "2019-07-01 17:00:00", SumConnectionIdentifier(con2), 2048);
 
 delete con2;
 
 sut->cleanup((char*) "2019-07-01 18:00:00");

 delete sut;
 allConnections.clear();
 
 delete mysql;
}
