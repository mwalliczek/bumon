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

class ActiveConnectionsTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( ActiveConnectionsTest );
 CPPUNIT_TEST( testIntern );
 CPPUNIT_TEST( testExtern );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testIntern();
  void testExtern();
};

CPPUNIT_TEST_SUITE_REGISTRATION( ActiveConnectionsTest );

std::list<Subnet<Ipv4Addr>> interns { Subnet<Ipv4Addr>("10.69.0.0", "255.255.0.0"), 
        Subnet<Ipv4Addr>("10.133.96.0", "255.255.224.0") };
std::list<Subnet<Ipv4Addr>> selfs { Subnet<Ipv4Addr>("10.31.1.100", "") };

void ActiveConnectionsTest::testIntern() {
    ActiveConnections<Ipv4Addr> underTest = new ActiveConnections<Ipv4Addr>(interns, selfs);
    Connection* connection = underTest->createConnection(Ipv4Addr("10.69.1.1"), Ipv4Addr("10.31.1.100"), 6);
    CPPUNIT_ASSERT(connection->intern == true);
    delete connection;
    
    connection = underTest->createConnection(Ipv4Addr("10.69.63.180"), Ipv4Addr("10.31.1.100"), 6);
    CPPUNIT_ASSERT(connection->intern == true);
    delete connection;

    connection = underTest->createConnection(Ipv4Addr("10.31.1.100"), Ipv4Addr("10.133.100.100"), 6);
    CPPUNIT_ASSERT(connection->intern == true);
    delete connection;
    delete underTest;
}

void ActiveConnectionsTest::testExtern() {
    ActiveConnections<Ipv4Addr> underTest = new ActiveConnections<Ipv4Addr>(interns, selfs);
    Connection* connection = underTest->createConnection(Ipv4Addr("10.50.1.1"), Ipv4Addr("10.31.1.100"), 6);
    CPPUNIT_ASSERT(connection->intern == false);
    delete connection;
    delete underTest;
}
