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

#ifndef WATCHING_H
#define WATCHING_H

#include <stack>
#include <map>
#include <ctime>
#include <thread>
#include <mutex>

#include "Stats.h"
#include "MySql.h"

struct History {
    time_t time;
    std::map<int, long long int>* traffic;
};

class Watching {
    std::stack<History*> history;
    std::mutex history_mutex;
    std::thread* thread;
    MySql *mysql_connection;
    Stats* statistics;
    void initMySQL(char* mysql_host, char* mysql_db, char* mysql_username, char* mysql_password);
    
    public:
        explicit Watching(bool startThread);
        explicit Watching(char* mysql_host, char* mysql_db, char* mysql_username, char* mysql_password, 
                const char* warning_main_sender, const char* warning_main_recipient, int expireConnections, 
                int expireStats);
        Watching(const Watching&) = delete;
        ~Watching();
        void addHistory(time_t time, std::map<int, long long int>* traffic);
        void watching();
        bool doWatch;
};

#endif
