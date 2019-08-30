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

#ifndef STATS_H
#define STATS_H

#include "MySql.h"
#include "Statistics.h"
#include "SumConnectionIdentifier.h"

struct ProcessAndContent {
    std::string process;
    std::string content;
    long long int sum;
};

struct TopHosts {
    std::shared_ptr<IpAddr> ip;
    long long int sum;
    std::map<std::string, ProcessAndContent*> processAndContent;
};

bool operator<(const TopHosts &t1, const TopHosts &t2);

class Stats {
    std::map<std::string, Statistics*> statistics;
    std::map<std::string, TopHosts*> topHosts;
    std::string lastStatsTimestamp;
    MySql* mysql_connection;
    char* sender;
    char* recipient;
    int expireConnections;
    int expireStats;
    
    std::string checkSpike(const char *statsbuff, int dst_port, int protocol, bool intern, bool inbound, long long int sum);
    void sendMail(std::map<std::string, std::string>* messages);
    void insertStats(char *statsbuff, int dst_port, int protocol, bool intern, bool inbound, long long int sum);
    std::vector<TopHosts> lookupTopHosts(const char *statsbuff, int dst_port, int protocol, bool intern, bool inbound);
    
    public:
        Stats(MySql* mysql_connection, char* sender, char* recipient, int expireConnections, int expireStats);
        void cleanup(char *statsbuff);
        void insert(char *statsbuff, SumConnectionIdentifier sumConnectionIdentifier, long long int sum);
};

#endif
