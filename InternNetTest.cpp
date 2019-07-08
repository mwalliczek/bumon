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

#include "InternNet.h"

class InternNetTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( InternNetTest );
 CPPUNIT_TEST( testmatches );
 CPPUNIT_TEST( testmatchesNot );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testmatches();
  void testmatchesNot();
};

CPPUNIT_TEST_SUITE_REGISTRATION( InternNetTest );

void InternNetTest::testmatches() {
 InternNet *test = new InternNet("10.69.0.0", "255.255.0.0");
 CPPUNIT_ASSERT(test->valid == true);
 struct in_addr test_addr;
 inet_pton(AF_INET, "10.69.1.1", &test_addr);
 CPPUNIT_ASSERT(test->match(test_addr) == true);
 delete test;
}

void InternNetTest::testmatchesNot() {
 InternNet *test = new InternNet("10.69.0.0", "255.255.0.0");
 struct in_addr test_addr;
 inet_pton(AF_INET, "10.70.1.1", &test_addr);
 CPPUNIT_ASSERT(test->match(test_addr) == false);
 delete test;
}

