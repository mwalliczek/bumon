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

#include "Connection.h"

#include "bumon.h"

class ConnectionTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( ConnectionTest );
 CPPUNIT_TEST( testHandleData );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testHandleData();
};

CPPUNIT_TEST_SUITE_REGISTRATION( ConnectionTest );


// taken from https://github.com/dlundquist/sniproxy/blob/master/tests/tls_test.c
const unsigned char good_data_1[] = {
    // TLS record
    0x16, // Content Type: Handshake
    0x03, 0x01, // Version: TLS 1.0
    0x00, 0x68, // Length
        // Handshake
        0x01, // Handshake Type: Client Hello
        0x00, 0x00, 0x64,  // Length
        0x03, 0x01, // Version: TLS 1.0
        // Random
        0x4e, 0x55, 0xde, 0x32, 0x80, 0x07, 0x92, 0x9f,
        0x50, 0x41, 0xe4, 0xf9, 0x58, 0x32, 0xfc, 0x4f,
        0x10, 0xb3, 0xde, 0x44, 0x4d, 0xa9, 0x67, 0x78,
        0xea, 0xd1, 0x5f, 0x29, 0x09, 0x04, 0xc1, 0x06,
        0x00, // Session ID Length
        0x00, 0x28, // Cipher Suites Length
            0x00, 0x39,
            0x00, 0x38,
            0x00, 0x35,
            0x00, 0x16,
            0x00, 0x13,
            0x00, 0x0a,
            0x00, 0x33,
            0x00, 0x32,
            0x00, 0x2f,
            0x00, 0x05,
            0x00, 0x04,
            0x00, 0x15,
            0x00, 0x12,
            0x00, 0x09,
            0x00, 0x14,
            0x00, 0x11,
            0x00, 0x08,
            0x00, 0x06,
            0x00, 0x03,
            0x00, 0xff,
        0x02, // Compression Methods
            0x01,
            0x00,
        0x00, 0x12, // Extensions Length
            0x00, 0x00, // Extension Type: Server Name
            0x00, 0x0e, // Length
            0x00, 0x0c, // Server Name Indication Length
                0x00, // Server Name Type: host_name
                0x00, 0x09, // Length
                // "localhost"
                0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f, 0x73, 0x74
};

#define TESTSTRING "GET / HTTP/1.1\r\nUser-Agent: check_http/v2.2 (monitoring-plugins 2.2)\r\nConnection: close\r\nHost: test.example.com\r\n\r\n"

void ConnectionTest::testHandleData() {
 Connection *test = new Connection("10.50.1.1", 80, 6, "", true, true);
 test->handleData(160, (const u_char*) TESTSTRING, sizeof(TESTSTRING));
 CPPUNIT_ASSERT(test->content == "test.example.com");
 delete test;
 test = new Connection("10.50.1.1", 443, 6, "", true, true);
 test->handleData(160, good_data_1, sizeof(good_data_1));
 CPPUNIT_ASSERT(test->content == "localhost");
 delete test;
 allConnections.clear();
}
