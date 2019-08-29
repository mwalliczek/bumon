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

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

class MySqlTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( MySqlTest );
 CPPUNIT_TEST( testConnections );
 CPPUNIT_TEST( testStats );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testConnections();
  void testStats();
};

CPPUNIT_TEST_SUITE_REGISTRATION( MySqlTest );

void MySqlTest::testConnections() {
  MySql* sut = new MySql((char *)"localhost", (char *)"bumondb", (char *)"testuser", (char *)"testpwd");
  sut->insertConnection((char *)"2019-08-01 17:00:00", 300, "192.168.1.1", 80, 1, (char *)"", (char *)"", 100, 0, 0);
  
  std::string result = exec("mysql -u testuser --password=testpwd bumondb -e \"SELECT * FROM connections\"");
  CPPUNIT_ASSERT(result != "");

  sut->cleanupConnections(-1);
  delete sut;
  result = exec("mysql -u testuser --password=testpwd bumondb -e \"SELECT * FROM connections\"");
  CPPUNIT_ASSERT(result == "");
}

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
  
  sut->cleanupStats(1);
  result = sut->lookupStats((char*)"2019-07-01 14:00:00", 80, IPPROTO_TCP, true, false);
  CPPUNIT_ASSERT(result->size() == 0);
  
  delete result;
  
  delete sut;
}
