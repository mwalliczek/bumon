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

#include <cppunit/extensions/HelperMacros.h>

#include "Subnet.h"
#include "Ipv4Addr.h"
#include "Ipv6Addr.h"

class SubnetTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( SubnetTest );
 CPPUNIT_TEST( testmatchesv4 );
 CPPUNIT_TEST( testmatchesBitsv4 );
 CPPUNIT_TEST( testmatchesNotv4 );
 CPPUNIT_TEST( testParsev4 );
 CPPUNIT_TEST( testmatchesv6 );
 CPPUNIT_TEST( testmatchesBitsv6 );
 CPPUNIT_TEST( testmatchesNotv6 );
 CPPUNIT_TEST( testParsev6 );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testmatchesv4();
  void testmatchesBitsv4();
  void testmatchesNotv4();
  void testParsev4();
  void testmatchesv6();
  void testmatchesBitsv6();
  void testmatchesNotv6();
  void testParsev6();
};

CPPUNIT_TEST_SUITE_REGISTRATION( SubnetTest );

void SubnetTest::testmatchesv4() {
 Subnet<Ipv4Addr> *test = new Subnet<Ipv4Addr>((char *) "10.69.1.0", (char *) "255.255.255.0");
 CPPUNIT_ASSERT(test->valid == true);
 CPPUNIT_ASSERT(test->match(Ipv4Addr("10.69.1.1")) == true);
 delete test;
}

void SubnetTest::testmatchesBitsv4() {
 Subnet<Ipv4Addr> *test = new Subnet<Ipv4Addr>((char *) "10.69.1.0", (char *) "24");
 CPPUNIT_ASSERT(test->valid == true);
 CPPUNIT_ASSERT(test->match(Ipv4Addr("10.69.1.1")) == true);
 delete test;
}

void SubnetTest::testmatchesNotv4() {
 Subnet<Ipv4Addr> *test = new Subnet<Ipv4Addr>((char *) "10.69.0.0", (char *) "255.255.0.0");
 CPPUNIT_ASSERT(test->match(Ipv4Addr("10.70.1.1")) == false);
 CPPUNIT_ASSERT(test->toString() == "10.69.0.0/255.255.0.0");
 delete test;
}

void SubnetTest::testParsev4() {
 Subnet<Ipv4Addr> *test = new Subnet<Ipv4Addr>("10.69.1.0", "255.255.255.");
 CPPUNIT_ASSERT(test->valid == false);
 delete test;
 test = new Subnet<Ipv4Addr>("10.69.1.0", "abc");
 CPPUNIT_ASSERT(test->valid == false);
 delete test;
 test = new Subnet<Ipv4Addr>("10.69.1.0", "33");
 CPPUNIT_ASSERT(test->valid == false);
 delete test;
 test = new Subnet<Ipv4Addr>("10.69.1.0", "");
 CPPUNIT_ASSERT(test->valid == true);
 CPPUNIT_ASSERT(test->toString() == "10.69.1.0/255.255.255.255");
 delete test;
}

void SubnetTest::testmatchesv6() {
 Subnet<Ipv6Addr> *test = new Subnet<Ipv6Addr>((char *) "2001:123::", (char *) "ffff:ffff::");
 CPPUNIT_ASSERT(test->valid == true);
 CPPUNIT_ASSERT(test->match(Ipv6Addr("2001:123::1")) == true);
 delete test;
}

void SubnetTest::testmatchesBitsv6() {
 Subnet<Ipv6Addr> *test = new Subnet<Ipv6Addr>((char *) "2001:123::", (char *) "96");
 CPPUNIT_ASSERT(test->valid == true);
 CPPUNIT_ASSERT(test->match(Ipv6Addr("2001:123::1")) == true);
 delete test;
}

void SubnetTest::testmatchesNotv6() {
 Subnet<Ipv6Addr> *test = new Subnet<Ipv6Addr>((char *) "2001:123::", (char *) "ffff:ffff::");
 CPPUNIT_ASSERT(test->match(Ipv6Addr("2002:234::")) == false);
 CPPUNIT_ASSERT(test->toString() == "2001:123::/ffff:ffff::");
 delete test;
}

void SubnetTest::testParsev6() {
 Subnet<Ipv6Addr> *test = new Subnet<Ipv6Addr>("2001:123::", "ffff:ffff:");
 CPPUNIT_ASSERT(test->valid == false);
 delete test;
 test = new Subnet<Ipv6Addr>("2001:123::", "abc");
 CPPUNIT_ASSERT(test->valid == false);
 delete test;
 test = new Subnet<Ipv6Addr>("2001:123::", "129");
 CPPUNIT_ASSERT(test->valid == false);
 delete test;
 test = new Subnet<Ipv6Addr>("2001:123::", "");
 CPPUNIT_ASSERT(test->valid == true);
 CPPUNIT_ASSERT(test->toString() == "2001:123::/ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
 delete test;
}

