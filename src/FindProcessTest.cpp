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

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <cppunit/extensions/HelperMacros.h>

#include "FindProcess.h"

class FindProcessTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( FindProcessTest );
 CPPUNIT_TEST( testInitAndFindTcpListen );
 CPPUNIT_TEST( testInitAndFindUdpListen );
 CPPUNIT_TEST( testFindActiveTcp );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testInitAndFindTcpListen();
  void testInitAndFindUdpListen();
  void testFindActiveTcp();
};

CPPUNIT_TEST_SUITE_REGISTRATION( FindProcessTest );

void FindProcessTest::testInitAndFindTcpListen() {
 FindProcess* findProcess = new FindProcess();
 findProcess->init();
 CPPUNIT_ASSERT(findProcess->findListenTcpProcess(25).empty() == false);
 delete findProcess;
}

void FindProcessTest::testInitAndFindUdpListen() {
 FindProcess* findProcess = new FindProcess();
 findProcess->init();
 CPPUNIT_ASSERT(findProcess->findListenUdpProcess(1500).empty() == false);
 delete findProcess;
}

void FindProcessTest::testFindActiveTcp() {
 FindProcess* findProcess = new FindProcess();
 CPPUNIT_ASSERT(findProcess->findActiveTcpProcess(35358, "127.0.0.1", 25).empty() == false);
 delete findProcess;
}
