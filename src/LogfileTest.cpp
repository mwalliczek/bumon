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

#include "Logfile.h"

class LogfileTest : public CPPUNIT_NS::TestFixture
{
 CPPUNIT_TEST_SUITE( LogfileTest );
 CPPUNIT_TEST( test );
 CPPUNIT_TEST_SUITE_END();

 public:
  void test();
};

CPPUNIT_TEST_SUITE_REGISTRATION( LogfileTest );

void LogfileTest::test() {
 Logfile *test = new Logfile("", DEBUG, {{"LogfileTest", TRACE}});
 test->log("LogfileTest", TRACE, "test1");
 test->log("LogfileTest", DEBUG, "%s %d", "test2", 2);
 CPPUNIT_ASSERT(test->checkLevel("LogfileTest", TRACE) == true);
 CPPUNIT_ASSERT(test->checkLevel("LogfileTest", DEBUG) == true);
 CPPUNIT_ASSERT(test->checkLevel("LogfileTest1", TRACE) == false);
 CPPUNIT_ASSERT(test->checkLevel("LogfileTest1", DEBUG) == true);
 delete test;
}

