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
    statistics = new Stats(NULL);
}

void Watching::initMySQL(char* mysql_host, char* mysql_db, char* mysql_username, char* mysql_password) {
    mysql_connection = new MySql(mysql_host, mysql_db, mysql_username, mysql_password);
}

Watching::Watching(char* mysql_host, char* mysql_db, char* mysql_username, char* mysql_password) {
    if (debug) {
        logfile->log(11, "Start Watching: %s %s %s %s\n", mysql_host, mysql_db, mysql_username, mysql_password);
    }
    initMySQL(mysql_host, mysql_db, mysql_username, mysql_password);
    statistics = new Stats(mysql_connection);
    
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

class TopConnection {
    public:
        std::string src_ip, dst_ip, process;
        int dst_port, protocol;
        long long int sum;
        bool intern, inbound;
};

class TopContent {
    public:
        std::string src_ip, dst_ip, content;
        int dst_port, protocol;
        long long int sum;
        bool intern, inbound;
};

void Watching::watching() {
        activeTcpConnections->checkTimeout();
        activeUdpConnections->checkTimeout();
        
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
            std::map<std::string, TopConnection*> topConnections;
            std::map<std::string, TopContent*> topContent;
            std::map<int, long long int>::iterator iter = current->traffic->begin();
            while (iter != current->traffic->end()) {
                allConnections_mutex.lock();
                Connection* connection = allConnections[iter->first];
                allConnections_mutex.unlock();
                std::string src_ip(inet_ntoa(connection->src_ip));
                std::string dst_ip(inet_ntoa(connection->dst_ip));
                std::stringstream id;
                std::map<std::string, TopConnection*>::iterator iterTop;
                id << src_ip << ">" << dst_ip << ":" << connection->dst_port;
                if ((iterTop = topConnections.find(id.str())) != topConnections.end()) {
                    iterTop->second->sum += iter->second;
                } else {
                    TopConnection* topCon = new TopConnection();
                    topCon->src_ip = src_ip;
                    topCon->dst_ip = dst_ip;
                    topCon->dst_port = connection->dst_port;
                    topCon->process = connection->process;
                    topCon->protocol = connection->protocol;
                    topCon->intern = connection->intern;
                    topCon->sum = iter->second;
                    topCon->inbound = connection->dst_ip.s_addr == self_ip.s_addr;
                    topConnections[id.str()] = topCon;
                }
                if (!connection->content.empty()) {
                    id << "=" << connection->content;
                    std::map<std::string, TopContent*>::iterator iterContent = topContent.find(id.str());
                    if (iterContent != topContent.end()) {
                        iterContent->second->sum += iter->second;
                    } else {
                        TopContent* newContent = new TopContent();
                        newContent->content = connection->content;
                        newContent->src_ip = src_ip;
                        newContent->dst_ip = dst_ip;
                        newContent->dst_port = connection->dst_port;
                        newContent->protocol = connection->protocol;
                        newContent->intern = connection->intern;
                        newContent->sum = iter->second;
                        newContent->inbound = connection->dst_ip.s_addr == self_ip.s_addr;
                        topContent[id.str()] = newContent;
                    }
                }
                if (connection->intern) {
                    sumIntern += iter->second;
                } else {
                    sumExtern += iter->second;
                }
                iter = current->traffic->erase(iter);
            }
            delete current;
            
            std::map<std::string, TopConnection*>::const_iterator it=topConnections.begin();
            
            while (it!=topConnections.end()) {
                logfile->log(3, "%lli %s > %s:%d (%s)", it->second->sum, it->second->src_ip.c_str(), it->second->dst_ip.c_str(), it->second->dst_port, it->second->process.c_str());
                if (mysql_connection != NULL) {
                    mysql_connection->insertConnection(buff, 300, it->second->src_ip.c_str(), it->second->dst_port,
                            it->second->protocol, it->second->process.c_str(), it->second->sum, it->second->inbound ? 1 : 0, it->second->intern);
                }
                statistics->insert(statsbuff, it->second->dst_port, it->second->protocol, it->second->intern, it->second->inbound, it->second->sum);
                delete it->second;
                it = topConnections.erase(it);
            }
            
            logfile->log(3, "%s: (%lli / %lli)", buff, sumIntern, sumExtern);

            if (mysql_connection != NULL) {
                mysql_connection->insertBandwidth(buff, 300, sumIntern, 1);
                mysql_connection->insertBandwidth(buff, 300, sumExtern, 0);
            }
            std::map<std::string, TopContent*>::iterator iCon = topContent.begin();
            while (iCon != topContent.end()) {
                logfile->log(3, "%lli %s > %s:%d (%s)", iCon->second->sum, iCon->second->src_ip.c_str(), iCon->second->dst_ip.c_str(), iCon->second->dst_port, iCon->second->content.c_str());
                if (mysql_connection != NULL) {
                    mysql_connection->insertContent(buff, 300, iCon->second->src_ip.c_str(), iCon->second->dst_port,
                            iCon->second->protocol, iCon->second->content.c_str(), iCon->second->sum, iCon->second->inbound ? 1 : 0, iCon->second->intern);
                }
                delete iCon->second;
                iCon = topContent.erase(iCon);
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
