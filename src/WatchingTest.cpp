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
#include "ConnectionIdentifier.h"
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
 ConfigfileParser* config = new ConfigfileParser("testConfigMock.conf");
 ip = new Ip(config);

 Connection* con = new Connection(std::shared_ptr<IpAddr>(new Ipv4Addr("10.69.1.1")), 100, 1, "testProcess", false, false);
 
 std::shared_ptr<std::map<int, long long int>> traffic1 = 
         std::shared_ptr<std::map<int, long long int>>(new std::map<int, long long int>());
 (*traffic1)[con->id] = 100;
 con->handleData(100, (const u_char*) "\r\nHost: www.example.com\r\n", 25);
 
 time_t current;
 time(&current);
 
 watching->addHistory(current, traffic1);
 watching->watching();
 
 struct tm * timeinfo = localtime ( &current );
 timeinfo->tm_hour++;

 current = mktime(timeinfo);
 std::shared_ptr<std::map<int, long long int>> traffic2 = 
         std::shared_ptr<std::map<int, long long int>>(new std::map<int, long long int>());
 watching->addHistory(current, traffic2);
 watching->watching();

// CPPUNIT_ASSERT(allConnections.size() == 0);

 allConnections.clear();
 delete con;
 delete ip;
 delete config;
}

void WatchingTest::testIntegration() {
 ConfigfileParser* config = new ConfigfileParser("testConfigMock.conf");
 ip = new Ip(config);
 Connection* con = new Connection(std::shared_ptr<IpAddr>(new Ipv4Addr("10.69.1.1")), 80, IPPROTO_TCP, "testProcess", false, false);
 
 std::shared_ptr<std::map<int, long long int>> traffic1 = 
         std::shared_ptr<std::map<int, long long int>>(new std::map<int, long long int>());
 (*traffic1)[con->id] = 100;
 con->handleData(100, (const u_char*) "\r\nHost: www.example.com\r\n", 25);
 
 time_t current;
 time(&current);
 
 Watching* sut = new Watching("localhost", "netimonddb", "testuser", "testpwd", "root", "nobody", -1, -1);
 
 sut->addHistory(current, traffic1);
 sut->watching();
 
 struct tm * timeinfo = localtime ( &current );
 timeinfo->tm_hour++;

 current = mktime(timeinfo);
 std::shared_ptr<std::map<int, long long int>> traffic2 = 
         std::shared_ptr<std::map<int, long long int>>(new std::map<int, long long int>());
 sut->addHistory(current, traffic2);
 sut->watching();

// CPPUNIT_ASSERT(allConnections.size() == 0);

 allConnections.clear();
 delete con;
 delete sut;
 delete ip;
 delete config;
}

