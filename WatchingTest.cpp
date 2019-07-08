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

class WatchingTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( WatchingTest );
 CPPUNIT_TEST( testAll );
 CPPUNIT_TEST( testIntegration );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testAll();
  void testIntegration();
};

CPPUNIT_TEST_SUITE_REGISTRATION( WatchingTest );

void WatchingTest::testAll() {
 struct in_addr test_src_addr, test_dst_addr;
 inet_pton(AF_INET, "10.31.1.100", &test_src_addr);
 inet_pton(AF_INET, "10.31.1.100", &self_ip);
 inet_pton(AF_INET, "10.69.1.1", &test_dst_addr);
 Connection* con = new Connection(test_src_addr, 50, test_dst_addr, 100, 1, "testProcess");
 
 allConnections[con->id] = con;
 std::map<int, long long int> traffic;
 traffic[con->id] = 100;
 
 time_t current;
 time(&current);
 
 watching->addHistory(current, &traffic);
 watching->watching();
 
// CPPUNIT_ASSERT(allConnections.size() == 0);

 allConnections.clear();
 delete con;
}

void WatchingTest::testIntegration() {
 struct in_addr test_src_addr, test_dst_addr;
 inet_pton(AF_INET, "10.31.1.100", &test_src_addr);
 inet_pton(AF_INET, "10.31.1.100", &self_ip);
 inet_pton(AF_INET, "10.69.1.1", &test_dst_addr);
 Connection* con = new Connection(test_src_addr, 50, test_dst_addr, 100, 1, "testProcess");
 
 allConnections[con->id] = con;
 std::map<int, long long int> traffic;
 traffic[con->id] = 100;
 
 time_t current;
 time(&current);
 
 Watching* sut = new Watching((char *)"localhost", (char *)"bumondb", (char *)"testuser", (char *)"testpwd");
 
 sut->addHistory(current, &traffic);
 sut->watching();
 
// CPPUNIT_ASSERT(allConnections.size() == 0);

 allConnections.clear();
 delete con;
 delete sut;
}

