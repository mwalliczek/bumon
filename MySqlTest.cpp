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
#include "MySql.h"

class MySqlTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( MySqlTest );
 CPPUNIT_TEST( testStats );
 CPPUNIT_TEST( testLookupTopHostsWithBandwidth );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testStats();
  void testLookupTopHostsWithBandwidth();
};

CPPUNIT_TEST_SUITE_REGISTRATION( MySqlTest );

void MySqlTest::testStats() {
  MySql* sut = new MySql((char *)"localhost", (char *)"bumondb", (char *)"testuser", (char *)"testpwd");
  
  Statistics stat;
  stat.dst_port = 80;
  stat.protocol = IPPROTO_TCP;
  stat.sum = 2048;
  stat.intern = true;
  stat.inbound = false;
  
  sut->insertStats((char*)"2019-07-01 14:00:00", &stat);
  std::vector<long long int>* result = sut->lookupStats((char*)"2019-07-01 14:00:00", 80, IPPROTO_TCP, true, false);
  CPPUNIT_ASSERT(result->size() > 0);
  std::cout << "result[0] = " << (*result)[0] << std::endl;
  
  delete result;
  
  delete sut;
}

void MySqlTest::testLookupTopHostsWithBandwidth() {
  MySql* sut = new MySql((char *)"localhost", (char *)"bumondb", (char *)"testuser", (char *)"testpwd");
  
  sut->insertConnection((char*)"2019-07-01 15:10:00", 300, (char*)"10.1.2.3", 80, IPPROTO_TCP, (char*)"", 2048, 1, 0);
  
  std::vector<HostWithBandwidth>* result = sut->lookupTopHostsWithBandwidth((char*)"2019-07-01 15:00:00", 80, IPPROTO_TCP, 1);
  CPPUNIT_ASSERT(result->size() > 0);
  std::cout << "result[0] = " << (*result)[0].host << std::endl;
  
  delete result;
  
  delete sut;
}