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

#include <sstream>

#include "Statistics.h"

#include "bumon.h"

Stats::Stats(MySql* mysql_connection): mysql_connection(mysql_connection) { }

void Stats::cleanup(char *statsbuff) {
            if (lastStatsTimestamp != std::string(statsbuff)) {
                std::map<std::string, Statistics*>::iterator statsIter = statistics.begin();
                while (statsIter != statistics.end()) {
                    if (statsIter->first.rfind(statsbuff, 0) != 0) {
                        logfile->log(3, "%s: %d %lli", statsIter->first.substr(0, 19).c_str(), statsIter->second->dst_port, statsIter->second->sum);
                        if (mysql_connection != NULL) {
                            mysql_connection->insertStats(statsIter->first.substr(0, 19), statsIter->second);
                        }
                        delete statsIter->second;
                        statsIter = statistics.erase(statsIter);
                    } else {
                        statsIter++;
                    }
                }
            }
}

void Stats::insert(char *statsbuff, int dst_port, int protocol, bool intern, bool inbound, long long int sum) {
    std::stringstream result;
    lastStatsTimestamp = std::string(statsbuff);
    result << statsbuff << " " << dst_port << " " << protocol << " " << intern << " " << inbound;
    std::string statsId = result.str();
    std::map<std::string, Statistics*>::iterator statsIter = statistics.find(statsId);
    if (statsIter != statistics.end()) {
        statsIter->second->sum += sum;
    } else {
        Statistics* newStats = new Statistics();
        newStats->dst_port = dst_port;
        newStats->protocol = protocol;
        newStats->sum = sum;
        newStats->intern = intern;
        newStats->inbound = inbound;
        statistics[statsId] = newStats;
    }
}
