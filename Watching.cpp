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

#define INSERT_BANDWIDTH "INSERT INTO bandwidth (timestamp, duration, bytes, intern) VALUES(?, ?, ?, ?)"
#define INSERT_TOP_CONNECTIONS_STATEMENT "INSERT INTO top_connections (timestamp, duration, foreign_ip, dst_port, protocol, process, bytes, inbound, intern) \
            VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)"
#define INSERT_TOP_CONTENT_STATEMENT "INSERT INTO top_content (timestamp, duration, foreign_ip, dst_port, protocol, content, bytes, inbound, intern) \
            VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)"

Watching::Watching(bool startThread) {
    if (debug) {
        printf("Start Watching\n");
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
}

void Watching::initMySQL(char* mysql_host, char* mysql_db, char* mysql_username, char* mysql_password) {
    if (!mysql_real_connect(mysql_connection, mysql_host, mysql_username, mysql_password, mysql_db, 0, NULL, 0)) {
      fprintf(stderr, "Conection error : %s\n", mysql_error(mysql_connection));
      exit(0);
    }
    mysql_stmt_bandwidth = mysql_stmt_init(mysql_connection);
    if (!mysql_stmt_bandwidth)
    {
      fprintf(stderr, " mysql_stmt_init(), out of memory\n");
      exit(0);
    }
    if (mysql_stmt_prepare(mysql_stmt_bandwidth, INSERT_BANDWIDTH, strlen(INSERT_BANDWIDTH))) {
      fprintf(stderr, " mysql_stmt_prepare(), INSERT failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(mysql_stmt_bandwidth));
      exit(0);
    }
    mysql_stmt_connections = mysql_stmt_init(mysql_connection);
    if (!mysql_stmt_connections)
    {
      fprintf(stderr, " mysql_stmt_init(), out of memory\n");
      exit(0);
    }
    if (mysql_stmt_prepare(mysql_stmt_connections, INSERT_TOP_CONNECTIONS_STATEMENT, strlen(INSERT_TOP_CONNECTIONS_STATEMENT))) {
      fprintf(stderr, " mysql_stmt_prepare(), INSERT failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(mysql_stmt_connections));
      exit(0);
    }
    mysql_stmt_content = mysql_stmt_init(mysql_connection);
    if (!mysql_stmt_content)
    {
      fprintf(stderr, " mysql_stmt_init(), out of memory\n");
      exit(0);
    }
    if (mysql_stmt_prepare(mysql_stmt_content, INSERT_TOP_CONTENT_STATEMENT, strlen(INSERT_TOP_CONTENT_STATEMENT))) {
      fprintf(stderr, " mysql_stmt_prepare(), INSERT failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(mysql_stmt_content));
      exit(0);
    }
}

Watching::Watching(char* mysql_host, char* mysql_db, char* mysql_username, char* mysql_password) {
    if (debug) {
        printf("Start Watching: %s %s %s %s\n", mysql_host, mysql_db, mysql_username, mysql_password);
    }
    mysql_connection = mysql_init(NULL);
    initMySQL(mysql_host, mysql_db, mysql_username, mysql_password);
    
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
    if (NULL != mysql_connection) {
        mysql_stmt_close(mysql_stmt_bandwidth);
        mysql_stmt_close(mysql_stmt_connections);
        mysql_stmt_close(mysql_stmt_content);
        mysql_close(mysql_connection);
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

void insertBandwidth(MYSQL_STMT* mysql_stmt_bandwidth, char* buff, short duration, long long int sum, short intern) {
                MYSQL_BIND bind[4];
                unsigned long str_length = strlen(buff);
                memset(bind, 0, sizeof(bind));
                bind[0].buffer_type= MYSQL_TYPE_STRING;
                bind[0].buffer= buff;
                bind[0].length= &str_length;
                bind[1].buffer_type= MYSQL_TYPE_SHORT;
                bind[1].buffer= (char *)&duration;
                bind[2].buffer_type= MYSQL_TYPE_LONGLONG;
                bind[2].buffer= (char *)&sum;
                bind[3].buffer_type= MYSQL_TYPE_SHORT;
                bind[3].buffer= (char *)&intern;
                /* Bind the buffers */
                if (mysql_stmt_bind_param(mysql_stmt_bandwidth, bind))
                {
                  logfile->log(1, " mysql_stmt_bind_param() failed\n");
                  logfile->log(1, " %s\n", mysql_stmt_error(mysql_stmt_bandwidth));
                  exit(0);
                }
                /* Execute the INSERT statement - 1*/
                if (mysql_stmt_execute(mysql_stmt_bandwidth))
                {
                  logfile->log(1, " mysql_stmt_execute(), 1 failed\n");
                  logfile->log(1, " %s\n", mysql_stmt_error(mysql_stmt_bandwidth));
                  exit(0);
                }
}

void insertConnection(MYSQL_STMT* mysql_stmt_connections, char* buff, short duration, const char* foreign_ip, int dst_port, short protocol, const char* process, long long int bytes, short inbound, 
        short intern) {
                MYSQL_BIND bind[10];
                unsigned long str_length_buff = strlen(buff);
                unsigned long str_length_ip = strlen(foreign_ip);
                unsigned long str_length_process = strlen(process);
                unsigned long str_protocol = 0;
                memset(bind, 0, sizeof(bind));
                bind[0].buffer_type= MYSQL_TYPE_STRING;
                bind[0].buffer= buff;
                bind[0].length= &str_length_buff;
                bind[1].buffer_type= MYSQL_TYPE_SHORT;
                bind[1].buffer= (char *)&duration;
                bind[2].buffer_type= MYSQL_TYPE_STRING;
                bind[2].buffer= (char *) foreign_ip;
                bind[2].length = &str_length_ip;
                bind[3].buffer_type= MYSQL_TYPE_LONG;
                bind[3].buffer= (char *)&dst_port;
                bind[4].buffer_type= MYSQL_TYPE_STRING;
                if (protocol == IPPROTO_TCP) {
                    bind[4].buffer = (char *)"tcp";
                    str_protocol = 3;
                } else if (protocol == IPPROTO_UDP) {
                    bind[4].buffer = (char *)"udp";
                    str_protocol = 3;
                }
                bind[4].length = &str_protocol;
                bind[4].buffer= (char *)&protocol;
                bind[5].buffer_type= MYSQL_TYPE_STRING;
                bind[5].buffer= (char *)process;
                bind[5].length = &str_length_process;
                bind[6].buffer_type= MYSQL_TYPE_LONGLONG;
                bind[6].buffer= (char *)&bytes;
                bind[7].buffer_type= MYSQL_TYPE_SHORT;
                bind[7].buffer= (char *)&inbound;
                bind[8].buffer_type= MYSQL_TYPE_SHORT;
                bind[8].buffer= (char *)&intern;
                /* Bind the buffers */
                if (mysql_stmt_bind_param(mysql_stmt_connections, bind))
                {
                  logfile->log(1, " mysql_stmt_bind_param() failed: %d\n", mysql_stmt_errno(mysql_stmt_connections));
                  logfile->log(1, " %s\n", mysql_stmt_error(mysql_stmt_connections));
                  exit(0);
                }
                /* Execute the INSERT statement - 1*/
                if (mysql_stmt_execute(mysql_stmt_connections))
                {
                  logfile->log(1, " mysql_stmt_execute() failed: %d\n", mysql_stmt_errno(mysql_stmt_connections));
                  logfile->log(1, " %s\n", mysql_stmt_error(mysql_stmt_connections));
                  exit(0);
                }
}

void insertContent(MYSQL_STMT* mysql_stmt_content, char* buff, short duration, const char* foreign_ip, int dst_port, short protocol, const char* content, long long int bytes, 
        short inbound, short intern) {
                MYSQL_BIND bind[10];
                unsigned long str_length_buff = strlen(buff);
                unsigned long str_length_ip = strlen(foreign_ip);
                unsigned long str_length_content = strlen(content);
                unsigned long str_protocol = 0;
                memset(bind, 0, sizeof(bind));
                bind[0].buffer_type= MYSQL_TYPE_STRING;
                bind[0].buffer= buff;
                bind[0].length= &str_length_buff;
                bind[1].buffer_type= MYSQL_TYPE_SHORT;
                bind[1].buffer= (char *)&duration;
                bind[2].buffer_type= MYSQL_TYPE_STRING;
                bind[2].buffer= (char *) foreign_ip;
                bind[2].length = &str_length_ip;
                bind[3].buffer_type= MYSQL_TYPE_LONG;
                bind[3].buffer= (char *)&dst_port;
                bind[4].buffer_type= MYSQL_TYPE_STRING;
                if (protocol == IPPROTO_TCP) {
                    bind[4].buffer = (char *)"tcp";
                    str_protocol = 3;
                } else if (protocol == IPPROTO_UDP) {
                    bind[4].buffer = (char *)"udp";
                    str_protocol = 3;
                }
                bind[4].length = &str_protocol;
                bind[4].buffer= (char *)&protocol;
                bind[5].buffer_type= MYSQL_TYPE_STRING;
                bind[5].buffer= (char *)content;
                bind[5].length = &str_length_content;
                bind[6].buffer_type= MYSQL_TYPE_LONGLONG;
                bind[6].buffer= (char *)&bytes;
                bind[7].buffer_type= MYSQL_TYPE_SHORT;
                bind[7].buffer= (char *)&inbound;
                bind[8].buffer_type= MYSQL_TYPE_SHORT;
                bind[8].buffer= (char *)&intern;
                /* Bind the buffers */
                if (mysql_stmt_bind_param(mysql_stmt_content, bind))
                {
                  logfile->log(1, " mysql_stmt_bind_param() failed: %d\n", mysql_stmt_errno(mysql_stmt_content));
                  logfile->log(1, " %s\n", mysql_stmt_error(mysql_stmt_content));
                  exit(0);
                }
                /* Execute the INSERT statement - 1*/
                if (mysql_stmt_execute(mysql_stmt_content))
                {
                  logfile->log(1, " mysql_stmt_execute(), failed: %d\n", mysql_stmt_errno(mysql_stmt_content));
                  logfile->log(1, " %s\n", mysql_stmt_error(mysql_stmt_content));
                  exit(0);
                }
}

void Watching::watching() {
        activeTcpConnections->checkTimeout();
        activeUdpConnections->checkTimeout();
        
        while (!history.empty()) {
            history_mutex.lock();
            History* current = history.top();
            history.pop();
            history_mutex.unlock();

            char buff[20];
            strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&current->time));

            long long int sumIntern = 0;
            long long int sumExtern = 0;
            std::map<std::string, TopConnection*> topConnections;
            std::map<std::string, TopContent*> topContent;
            std::map<int, long long int>::iterator iter = current->traffic->begin();
            while (iter != current->traffic->end()) {
                Connection* connection = allConnections[iter->first];
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
                    if (it->second->inbound) {
                        insertConnection(mysql_stmt_connections, buff, 300, it->second->src_ip.c_str(), it->second->dst_port, it->second->protocol, it->second->process.c_str(), it->second->sum, 1, it->second->intern);
                    } else {
                        insertConnection(mysql_stmt_connections, buff, 300, it->second->dst_ip.c_str(), it->second->dst_port, it->second->protocol, it->second->process.c_str(), it->second->sum, 0, it->second->intern);
                    }
                }
                delete it->second;
                it = topConnections.erase(it);
            }
            
            logfile->log(3, "%s: (%lli / %lli)", buff, sumIntern, sumExtern);

            if (mysql_connection != NULL) {
                insertBandwidth(mysql_stmt_bandwidth, buff, 300, sumIntern, 1);
                insertBandwidth(mysql_stmt_bandwidth, buff, 300, sumExtern, 0);
            }
            std::map<std::string, TopContent*>::iterator iCon = topContent.begin();
            while (iCon != topContent.end()) {
                logfile->log(3, "%lli %s > %s:%d (%s)", iCon->second->sum, iCon->second->src_ip.c_str(), iCon->second->dst_ip.c_str(), iCon->second->dst_port, iCon->second->content.c_str());
                if (mysql_connection != NULL) {
                    if (iCon->second->inbound) {
                        insertContent(mysql_stmt_content, buff, 300, iCon->second->src_ip.c_str(), iCon->second->dst_port, iCon->second->protocol, iCon->second->content.c_str(), iCon->second->sum, 
                                1, iCon->second->intern);
                    } else {
                        insertContent(mysql_stmt_content, buff, 300, iCon->second->dst_ip.c_str(), iCon->second->dst_port, iCon->second->protocol, iCon->second->content.c_str(), iCon->second->sum, 
                                0, iCon->second->intern);
                    }
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