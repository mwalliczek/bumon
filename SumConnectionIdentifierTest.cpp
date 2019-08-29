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

#include "SumConnectionIdentifier.h"

#include "Ipv4Addr.h"

class SumConnectionIdentifierTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( SumConnectionIdentifierTest );
 CPPUNIT_TEST( test );
 CPPUNIT_TEST_SUITE_END();

 public:
  void test();
};

CPPUNIT_TEST_SUITE_REGISTRATION( SumConnectionIdentifierTest );

void SumConnectionIdentifierTest::test() {
    Connection *aCon = new Connection(std::shared_ptr<IpAddr>(new Ipv4Addr("10.31.1.100")), 80, 6, "", true, true);
    SumConnectionIdentifier a = SumConnectionIdentifier(aCon);
    Connection *bCon = new Connection(std::shared_ptr<IpAddr>(new Ipv4Addr("10.69.1.1")), 80, 6, "", true, true);
    SumConnectionIdentifier b = SumConnectionIdentifier(bCon);
    CPPUNIT_ASSERT(a<b);
    CPPUNIT_ASSERT(!(b<a));
    delete aCon;
    delete bCon;
}
