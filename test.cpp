/*
   Copyright 2019 Matthias Walliczek

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <list>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>

#include "WatchingMock.h"
#include "ActiveTcpConnections.h"
#include "ActiveUdpConnections.h"
#include "FindProcess.h"
#include "InternNet.h"
#include "Logfile.h"

Watching* watching;
ActiveTcpConnections *activeTcpConnections;
ActiveUdpConnections *activeUdpConnections;
FindProcess* findProcesses;
TrafficManager *trafficManager;
struct in_addr self_ip;
std::map<int, Connection*> allConnections;
std::mutex allConnections_mutex;
std::list<InternNet*> interns { new InternNet("10.69.0.0", "255.255.0.0"), new InternNet("10.133.96.0", "255.255.224.0") };
Logfile* logfile;
bool debug = true;

int main (int argc, char* argv[])
{
    logfile = new Logfile("", 11);
    watching = new WatchingMock();
    activeTcpConnections = new ActiveTcpConnections();
    activeUdpConnections = new ActiveUdpConnections();
    findProcesses = new FindProcess();
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
    
    delete watching;
    delete activeTcpConnections;
    delete activeUdpConnections;
    delete findProcesses;
    delete trafficManager;
    delete logfile;
    
    for (std::list<InternNet*>::iterator i=interns.begin(); i!=interns.end(); i++)
        delete *i;
    interns.clear();
    
    mysql_library_end();

    // Rueckmeldung, ob Tests erfolgreich waren
    return collectedresults.wasSuccessful () ? 0 : 1;
}