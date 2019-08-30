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

#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#include <sstream>
#include <list>

#include "bumon.h"
#include "Connection.h"
#include "Watching.h"
#include "SumConnectionIdentifier.h"

void callWatching(Watching* watching) {
    while (watching->doWatch) {
        sleep(10);
        watching->watching();
    }
}

Watching::Watching(bool startThread) {
    if (debug) {
        logfile->log(11, "Start Watching");
    }
    mysql_connection = NULL;
    if (startThread) {
        doWatch = true;
        thread = new std::thread(callWatching, this);
        pthread_setschedprio(thread->native_handle(), 10);
    } else {
        doWatch = false;
        thread = NULL;
    }
    statistics = new Stats(NULL, NULL, NULL, -1, -1);
}

void Watching::initMySQL(char* mysql_host, char* mysql_db, char* mysql_username, char* mysql_password) {
    mysql_connection = new MySql(mysql_host, mysql_db, mysql_username, mysql_password);
}

Watching::Watching(char* mysql_host, char* mysql_db, char* mysql_username, char* mysql_password, 
        char* warning_main_sender, char* warning_main_recipient, int expireConnections, int expireStats) {
    if (debug) {
        logfile->log(11, "Start Watching: %s %s %s %s\n", mysql_host, mysql_db, mysql_username, mysql_password);
    }
    initMySQL(mysql_host, mysql_db, mysql_username, mysql_password);
    statistics = new Stats(mysql_connection, warning_main_sender, warning_main_recipient, expireConnections, 
            expireStats);
    
    doWatch = true;
    thread = new std::thread(callWatching, this);
    pthread_setschedprio(thread->native_handle(), 10);
}

Watching::~Watching() {
    if (NULL != thread) {
        doWatch = false;
        thread->join();
        delete thread;
    }
    delete statistics;
    if (NULL != mysql_connection) {
        delete mysql_connection;
    }
}

void Watching::addHistory(time_t time, std::map<int, long long int>* traffic) {
    History* newHistory = new History();
    newHistory->time = time;
    newHistory->traffic = traffic;
    history_mutex.lock();
    this->history.push(newHistory);
    history_mutex.unlock();
}

void Watching::watching() {
        ip->checkTimeout();
        
        while (!history.empty()) {
            history_mutex.lock();
            History* current = history.top();
            history.pop();
            history_mutex.unlock();

            char statsbuff[20];
            strftime(statsbuff, 20, "%Y-%m-%d %H:00:00", localtime(&current->time));
            statistics->cleanup(statsbuff);
            
            char buff[20];
            strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&current->time));

            long long int sumIntern = 0;
            long long int sumExtern = 0;
            std::map<SumConnectionIdentifier, long long int> sumConnections;
            std::map<int, long long int>::iterator iter = current->traffic->begin();
            while (iter != current->traffic->end()) {
                allConnections_mutex.lock();
                Connection* connection = allConnections[iter->first];
                allConnections_mutex.unlock();
                SumConnectionIdentifier id = SumConnectionIdentifier(connection);
                std::map<SumConnectionIdentifier, long long int>::iterator iterSum;
                if ((iterSum = sumConnections.find(id)) != sumConnections.end()) {
                    iterSum->second += iter->second;
                } else {
                    sumConnections[id] = iter->second;
                }
                if (connection->intern) {
                    sumIntern += iter->second;
                } else {
                    sumExtern += iter->second;
                }
                iter = current->traffic->erase(iter);
            }
            delete current;
            
            std::map<SumConnectionIdentifier, long long int>::const_iterator it=sumConnections.begin();
            while (it!=sumConnections.end()) {
                if (it->first.inbound) {
                    logfile->log(3, "%lli %s > %d (%s) (%s)", it->second, it->first.ip.c_str(), it->first.dst_port, 
                            it->first.process.c_str(), it->first.content.c_str());
                } else {
                    logfile->log(3, "%lli > %s:%d (%s) (%s)", it->second, it->first.ip.c_str(), it->first.dst_port, 
                            it->first.process.c_str(), it->first.content.c_str());
                }
                if (mysql_connection != NULL) {
                    mysql_connection->insertConnection(buff, 300, it->first.ip.c_str(), it->first.dst_port,
                            it->first.protocol, it->first.process.c_str(), it->first.content.c_str(), it->second, 
                            it->first.inbound ? 1 : 0, it->first.intern ? 1 : 0);
                }
                statistics->insert(statsbuff, it->first, it->second);
                it = sumConnections.erase(it);
            }
            
            logfile->log(3, "%s: (%lli / %lli)", buff, sumIntern, sumExtern);

            if (mysql_connection != NULL) {
                mysql_connection->insertBandwidth(buff, 300, sumIntern, 1);
                mysql_connection->insertBandwidth(buff, 300, sumExtern, 0);
            }
        }
        
        allConnections_mutex.lock();
        std::map<int, Connection*>::iterator iterAll = allConnections.begin();
        time_t current;
        time(&current);
        while (iterAll != allConnections.end()) {
            if (iterAll->second->end != 0 && difftime(current, iterAll->second->end) > 360) {
                delete iterAll->second;
                iterAll = allConnections.erase(iterAll);
                continue;
            }
            iterAll++;
        }
        allConnections_mutex.unlock();

}
