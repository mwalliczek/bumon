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

#include "SuspiciousEvents.h"
#include "Ipv4Addr.h"
#include "netimond-test.h"

class SuspiciousEventsTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( SuspiciousEventsTest );
 CPPUNIT_TEST( testEmpty );
 CPPUNIT_TEST( testWithData );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testEmpty();
  void testWithData();
};

CPPUNIT_TEST_SUITE_REGISTRATION( SuspiciousEventsTest );

void SuspiciousEventsTest::testEmpty() {
    suspiciousEventsAlarm = 2;
    logfile->setLevel("SuspiciousEvents.cpp", TRACE);
    std::shared_ptr<SuspiciousEvents<Ipv4Addr>> test = SuspiciousEvents<Ipv4Addr>::getInstance();
    for (int i=0; i<9; i++) {
        test->connectionToEmpty(Ipv4Addr("192.168.0.0"), 5000);
    }
    logfile->content.str("");
    test->connectionToEmpty(Ipv4Addr("192.168.0.1"), 5001);
    test->connectionToEmpty(Ipv4Addr("192.168.0.0"), 5001);
    CPPUNIT_ASSERT(logfile->content.str() == "TRACE SuspiciousEvents.cpp: connectionToEmpty 192.168.0.0 -> 5001 [5000, 5001] ratePer15m: 10 (0)\n"
        "DEBUG SuspiciousEvents.cpp: addScore 192.168.0.0 +2\n"
        "INFO SuspiciousEvents.cpp: score 192.168.0.0 = 2\n");
}

void SuspiciousEventsTest::testWithData() {
    logfile->setLevel("SuspiciousEvents.cpp", TRACE);
    logfile->content.str("");
    std::shared_ptr<SuspiciousEvents<Ipv4Addr>> test = SuspiciousEvents<Ipv4Addr>::getInstance();
    test->connectionWithData(Ipv4Addr("192.168.1.0"), 5000, 10, 100000);
    CPPUNIT_ASSERT(logfile->content.str() == "TRACE SuspiciousEvents.cpp: connectionWithData 192.168.1.0 (5000), 10 100000\nDEBUG SuspiciousEvents.cpp: addScore 192.168.1.0 -11\n");
}

