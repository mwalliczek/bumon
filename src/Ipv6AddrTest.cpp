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

#include "Ipv6Addr.h"

class Ipv6AddrTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( Ipv6AddrTest );
 CPPUNIT_TEST( testParse );
 CPPUNIT_TEST( testCompare );
 CPPUNIT_TEST( testResolve );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testParse();
  void testCompare();
  void testResolve();
};

CPPUNIT_TEST_SUITE_REGISTRATION( Ipv6AddrTest );

void Ipv6AddrTest::testParse() {
 Ipv6Addr *test = new Ipv6Addr((char *) "2001:DB8:ABCD:12::");
 CPPUNIT_ASSERT(test->empty() == false);
 delete test;
 
 test = new Ipv6Addr((char *) "2001:DB8:ABCD:12:");
 CPPUNIT_ASSERT(test->empty() == true);
 delete test;
}

void Ipv6AddrTest::testCompare() {
 Ipv6Addr test1 = Ipv6Addr((char *) "2001:DB8:ABCD:12::");
 Ipv6Addr test2 = Ipv6Addr((char *) "2001:DB8:ABCD:12::1");
 Ipv6Addr test3 = Ipv6Addr((char *) "2000:DB8:ABCD:12::2");
 Ipv6Addr testMask = Ipv6Addr((char *) "FFFF:FFFF:FFFF:FFFF::");
 
 CPPUNIT_ASSERT(test1 != test2);
 CPPUNIT_ASSERT(!(test1 == test2));
 CPPUNIT_ASSERT(test1 < test2);
 CPPUNIT_ASSERT(test2 > test1);
 CPPUNIT_ASSERT(test2 > test3);
 CPPUNIT_ASSERT(test3 < test2);
 CPPUNIT_ASSERT((test2 & testMask) == test1);
 CPPUNIT_ASSERT(test2.toString() == "2001:db8:abcd:12::1");
}

void Ipv6AddrTest::testResolve() {
 Ipv6Addr testlo = Ipv6Addr((char *) "::2");
 CPPUNIT_ASSERT(testlo.resolve() == "::2");
}
