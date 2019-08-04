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
 
 Stats* sut = new Stats(mysql, (char *)"root", (char *)"nobody");
 
 sut->insert((char*) "2019-06-01 17:00:00", 0, 0, false, true, 2048);
 sut->insert((char*) "2019-06-01 17:00:00", 80, IPPROTO_TCP, false, true, 2048);

 sut->cleanup((char*) "2019-06-02 17:00:00");
 sut->insert((char*) "2019-06-02 17:00:00", 0, 0, false, true, 2047);
 sut->insert((char*) "2019-06-02 17:00:00", 80, IPPROTO_TCP, false, true, 2047);

 sut->cleanup((char*) "2019-06-03 17:00:00");
 sut->insert((char*) "2019-06-03 17:00:00", 0, 0, false, true, 2046);
 sut->insert((char*) "2019-06-03 17:00:00", 80, IPPROTO_TCP, false, true, 2046);

 sut->cleanup((char*) "2019-06-04 17:00:00");
 sut->insert((char*) "2019-06-04 17:00:00", 0, 0, false, true, 2045);
 sut->insert((char*) "2019-06-04 17:00:00", 80, IPPROTO_TCP, false, true, 2045);

 sut->cleanup((char*) "2019-06-05 17:00:00");
 sut->insert((char*) "2019-06-05 17:00:00", 0, 0, false, true, 2044);
 sut->insert((char*) "2019-06-05 17:00:00", 80, IPPROTO_TCP, false, true, 2044);

 sut->cleanup((char*) "2019-06-06 17:00:00");
 sut->insert((char*) "2019-06-06 17:00:00", 0, 0, false, true, 2043);
 sut->insert((char*) "2019-06-06 17:00:00", 80, IPPROTO_TCP, false, true, 2043);

 sut->cleanup((char*) "2019-06-07 17:00:00");
 sut->insert((char*) "2019-06-07 17:00:00", 0, 0, false, true, 2049);
 sut->insert((char*) "2019-06-07 17:00:00", 80, IPPROTO_TCP, false, true, 2049);

 sut->cleanup((char*) "2019-06-08 17:00:00");
 sut->insert((char*) "2019-06-08 17:00:00", 0, 0, false, true, 2050);
 sut->insert((char*) "2019-06-08 17:00:00", 80, IPPROTO_TCP, false, true, 2050);

 sut->cleanup((char*) "2019-06-09 17:00:00");
 sut->insert((char*) "2019-06-09 17:00:00", 0, 0, false, true, 2051);
 sut->insert((char*) "2019-06-09 17:00:00", 80, IPPROTO_TCP, false, true, 2051);

 sut->cleanup((char*) "2019-06-10 17:00:00");
 sut->insert((char*) "2019-06-10 17:00:00", 0, 0, false, true, 20452);
 sut->insert((char*) "2019-06-10 17:00:00", 80, IPPROTO_TCP, false, true, 2052);
 
 sut->cleanup((char*) "2019-07-01 17:00:00");
 mysql->insertConnection((char*)"2019-07-01 17:10:00", 300, (char*)"10.1.2.3", 80, IPPROTO_TCP, (char*)"", 20480, 1, 0);
 
 sut->insert((char*) "2019-07-01 17:00:00", 0, 0, false, true, 20480);
 sut->insert((char*) "2019-07-01 17:00:00", 80, IPPROTO_TCP, false, true, 20480);
 sut->insert((char*) "2019-07-01 17:00:00", 443, IPPROTO_TCP, false, true, 2048);
 
 sut->cleanup((char*) "2019-07-01 18:00:00");

 delete sut;
 
 delete mysql;
}
