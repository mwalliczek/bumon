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

#include "Ipv4Addr.h"

class Ipv4AddrTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( Ipv4AddrTest );
 CPPUNIT_TEST( testParse );
 CPPUNIT_TEST( testCompare );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testParse();
  void testCompare();
};

CPPUNIT_TEST_SUITE_REGISTRATION( Ipv4AddrTest );

void Ipv4AddrTest::testParse() {
 Ipv4Addr *test = new Ipv4Addr((char *) "10.69.0.0");
 CPPUNIT_ASSERT(test->empty() == false);
 delete test;
 
 test = new Ipv4Addr((char *) "10.69.0.");
 CPPUNIT_ASSERT(test->empty() == true);
 delete test;
}

void Ipv4AddrTest::testCompare() {
 Ipv4Addr test1 = Ipv4Addr((char *) "10.69.0.0");
 Ipv4Addr test2 = Ipv4Addr((char *) "10.69.0.1");
 Ipv4Addr testMask = Ipv4Addr((char *) "255.255.0.0");
 
 CPPUNIT_ASSERT(test1 != test2);
 CPPUNIT_ASSERT(!(test1 == test2));
 CPPUNIT_ASSERT(test1 < test2);
 CPPUNIT_ASSERT(test2 > test1);
 CPPUNIT_ASSERT((test2 & testMask) == test1);
}
