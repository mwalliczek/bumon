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

#include <map>
#include <list>

#include "netimond.h"
#include "ConfigfileParser.h"
#include "InternNet.h"

class ConfigfileParserTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( ConfigfileParserTest );
 CPPUNIT_TEST( testAll );
 CPPUNIT_TEST_SUITE_END();

 public:
  void testAll();
};

CPPUNIT_TEST_SUITE_REGISTRATION( ConfigfileParserTest );

void ConfigfileParserTest::testAll() {

 ConfigfileParser* underTest = new ConfigfileParser("testConfig.conf");
 CPPUNIT_ASSERT(underTest->options["device"] == "eth0");
 CPPUNIT_ASSERT(underTest->findOption("device") == "eth0");
 CPPUNIT_ASSERT(underTest->findOption("foo", "bar") == "bar");
 CPPUNIT_ASSERT(underTest->interns.size() == 2);
 CPPUNIT_ASSERT(underTest->selfs.size() == 1);
 CPPUNIT_ASSERT(underTest->selfs.front() == "10.31.1.100");
 delete underTest;
}

