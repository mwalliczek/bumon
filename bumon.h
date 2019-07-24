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

#ifndef NETMONITOR_H
#define NETMONITOR_H

#include <mutex>

#include "FindProcess.h"
#include "ActiveTcpConnections.h"
#include "ActiveUdpConnections.h"
#include "TrafficManager.h"
#include "Logfile.h"

extern FindProcess* findProcesses;
extern ActiveTcpConnections *activeTcpConnections;
extern ActiveUdpConnections *activeUdpConnections;
extern std::map<int, Connection*> allConnections;
extern std::mutex allConnections_mutex;
extern Watching* watching;
extern TrafficManager *trafficManager;
extern Logfile* logfile;
extern std::string ssPath;
extern bool debug;

extern struct in_addr self_ip;

#endif
