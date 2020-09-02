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

#include <list>
#include <iostream>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>

#include "netimond.h"
#include "WatchingMock.h"
#include "FindProcess.h"
#include "LogfileMock.h"

Watching* watching;
FindProcess* findProcesses;
Ip* ip;
TrafficManager *trafficManager;
std::map<int, Connection*> allConnections;
std::mutex allConnections_mutex;
Logfile* logfile;
bool debug = true;
std::string ssPath = "./ssMock";
std::string sendmailPath = "cat > sendmail.txt";
int suspiciousEventsAlarm = 3;

int main (int, char**)
{
    logfile = new LogfileMock();
    watching = new WatchingMock();
    findProcesses = new FindProcess();
    findProcesses->init();
    trafficManager = new TrafficManager();
    
    // Informiert Test-Listener ueber Testresultate
    CPPUNIT_NS :: TestResult testresult;

    // Listener zum Sammeln der Testergebnisse registrieren
    CPPUNIT_NS :: TestResultCollector collectedresults;
    testresult.addListener (&collectedresults);

    // Listener zur Ausgabe der Ergebnisse einzelner Tests
    CPPUNIT_NS :: BriefTestProgressListener progress;
    testresult.addListener (&progress);

    // Test-Suite ueber die Registry im Test-Runner einfuegen
    CPPUNIT_NS :: TestRunner testrunner;
    testrunner.addTest (CPPUNIT_NS :: TestFactoryRegistry :: getRegistry ().makeTest ());
    testrunner.run (testresult);

    // Resultate im Compiler-Format ausgeben
    CPPUNIT_NS :: CompilerOutputter compileroutputter (&collectedresults, std::cerr);
    compileroutputter.write ();
    
    // Output XML for Jenkins CPPunit plugin
    std::ofstream xmlFileOut("cppTestBasicResults.xml");
    CPPUNIT_NS :: XmlOutputter xmlOut(&collectedresults, xmlFileOut);
    xmlOut.write();

    delete watching;
    delete findProcesses;
    delete trafficManager;
    delete logfile;
    
    mysql_library_end();

    // Rueckmeldung, ob Tests erfolgreich waren
    return collectedresults.wasSuccessful () ? 0 : 1;
}
