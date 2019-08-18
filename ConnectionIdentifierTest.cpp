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

#include "ConnectionIdentifier.h"
#include "Ipv4Addr.h"

class ConnectionIdentifierTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( ConnectionIdentifierTest );
 CPPUNIT_TEST( test );
 CPPUNIT_TEST_SUITE_END();

 public:
  void test();
};

CPPUNIT_TEST_SUITE_REGISTRATION( ConnectionIdentifierTest );

void ConnectionIdentifierTest::test() {
    ConnectionIdentifier<Ipv4Addr> a = ConnectionIdentifier<Ipv4Addr>(Ipv4Addr((char *) "10.69.1.1"), 100, Ipv4Addr((char *) "10.31.1.100"), 200);
    ConnectionIdentifier<Ipv4Addr> b = ConnectionIdentifier<Ipv4Addr>(Ipv4Addr((char *) "10.69.1.1"), 101, Ipv4Addr((char *) "10.31.1.100"), 200);
    CPPUNIT_ASSERT(a<b);
    CPPUNIT_ASSERT(!(b<a));
    
    a = ConnectionIdentifier<Ipv4Addr>(Ipv4Addr((char *) "10.69.1.1"), 100, Ipv4Addr((char *) "10.31.1.100"), 200);
    b = ConnectionIdentifier<Ipv4Addr>(Ipv4Addr((char *) "10.31.1.100"), 100, Ipv4Addr((char *) "10.69.1.1"), 200);
    CPPUNIT_ASSERT(a<b);
    CPPUNIT_ASSERT(!(b<a));
}
