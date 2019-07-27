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

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "Logfile.h"
#include "bumon.h"

#include "MySql.h"

const char* insertBandwidthStatement="INSERT INTO bandwidth (timestamp, duration, bytes, intern) VALUES(?, ?, ?, ?)";
const char* insertTopConnectionsStatement="INSERT INTO top_connections (timestamp, duration, foreign_ip, dst_port, protocol, process, bytes, inbound, intern) \
            VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)";
const char* insertTopContentStatement="INSERT INTO top_content (timestamp, duration, foreign_ip, dst_port, protocol, content, bytes, inbound, intern) \
            VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)";
const char* insertStatsStatement="INSERT INTO stats (timestamp, bytes, inbound, intern, dst_port, protocol, duration) VALUES(?, ?, ?, ?, ?, ?, 3600)";

MySql::MySql(char* mysql_host, char* mysql_db, char* mysql_username, char* mysql_password):
        mysql_host(mysql_host), mysql_db(mysql_db), mysql_username(mysql_username), mysql_password(mysql_password) {
    init();
}

void MySql::init() {
    mysql_connection = mysql_init(NULL);
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
    if (mysql_stmt_prepare(mysql_stmt_bandwidth, insertBandwidthStatement, strlen(insertBandwidthStatement))) {
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
    if (mysql_stmt_prepare(mysql_stmt_connections, insertTopConnectionsStatement, strlen(insertTopConnectionsStatement))) {
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
    if (mysql_stmt_prepare(mysql_stmt_content, insertTopContentStatement, strlen(insertTopContentStatement))) {
      fprintf(stderr, " mysql_stmt_prepare(), INSERT failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(mysql_stmt_content));
      exit(0);
    }
    mysql_stmt_stats = mysql_stmt_init(mysql_connection);
    if (!mysql_stmt_stats)
    {
      fprintf(stderr, " mysql_stmt_init(), out of memory\n");
      exit(0);
    }
    if (mysql_stmt_prepare(mysql_stmt_stats, insertStatsStatement, strlen(insertStatsStatement))) {
      fprintf(stderr, " mysql_stmt_prepare(), INSERT failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(mysql_stmt_stats));
      exit(0);
    }
}

MySql::~MySql() {
  destroy();
}

void MySql::destroy() {
    mysql_stmt_close(mysql_stmt_bandwidth);
    mysql_stmt_close(mysql_stmt_connections);
    mysql_stmt_close(mysql_stmt_content);
    mysql_stmt_close(mysql_stmt_stats);
    mysql_close(mysql_connection);
}

void MySql::insertBandwidth(char* buff, short duration, long long int sum, short intern) {
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
    for (int tries=1; tries<5; tries++) {
                /* Bind the buffers */
                if (mysql_stmt_bind_param(mysql_stmt_bandwidth, bind))
                {
                  logfile->log(1, " mysql_stmt_bind_param() failed: %d\n", mysql_stmt_errno(mysql_stmt_bandwidth));
                  logfile->log(1, " %s\n", mysql_stmt_error(mysql_stmt_bandwidth));
                  exit(0);
                }
                /* Execute the INSERT statement - 1*/
                if (!mysql_stmt_execute(mysql_stmt_bandwidth)) break;
                logfile->log(1, " mysql_stmt_execute(), failed: %d\n", mysql_stmt_errno(mysql_stmt_bandwidth));
                logfile->log(1, " %s\n", mysql_stmt_error(mysql_stmt_bandwidth));
                  
                if (2013 == mysql_stmt_errno(mysql_stmt_bandwidth)) {
                    logfile->log(1, " trying to reconnect mysql");
                    destroy();
                    init();
                } else {
                    exit(0);
                }
    }
}

void MySql::insertConnection(char* buff, short duration, const char* foreign_ip, int dst_port, short protocol, 
        const char* text, long long int bytes, short inbound, short intern) {
    insertConnectionAndContent(mysql_stmt_connections, buff, duration, foreign_ip, dst_port, protocol, text, bytes, inbound, intern);
}

void MySql::insertContent(char* buff, short duration, const char* foreign_ip, int dst_port, short protocol, 
        const char* text, long long int bytes, short inbound, short intern) {
    insertConnectionAndContent(mysql_stmt_content, buff, duration, foreign_ip, dst_port, protocol, text, bytes, inbound, intern);
}

void MySql::insertConnectionAndContent(MYSQL_STMT* mysql_stmt, char* buff, short duration, const char* foreign_ip, 
        int dst_port, short protocol, const char* text, long long int bytes, short inbound, short intern) {
    MYSQL_BIND bind[9];
                unsigned long str_length_buff = strlen(buff);
                unsigned long str_length_ip = strlen(foreign_ip);
                unsigned long str_length_text = strlen(text);
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
                bind[5].buffer= (char *)text;
                bind[5].length = &str_length_text;
                bind[6].buffer_type= MYSQL_TYPE_LONGLONG;
                bind[6].buffer= (char *)&bytes;
                bind[7].buffer_type= MYSQL_TYPE_SHORT;
                bind[7].buffer= (char *)&inbound;
                bind[8].buffer_type= MYSQL_TYPE_SHORT;
                bind[8].buffer= (char *)&intern;
                
    for (int tries=1; tries<5; tries++) {
                /* Bind the buffers */
                if (mysql_stmt_bind_param(mysql_stmt, bind))
                {
                  logfile->log(1, " mysql_stmt_bind_param() failed: %d\n", mysql_stmt_errno(mysql_stmt));
                  logfile->log(1, " %s\n", mysql_stmt_error(mysql_stmt));
                  exit(0);
                }
                /* Execute the INSERT statement - 1*/
                if (!mysql_stmt_execute(mysql_stmt)) break;
                logfile->log(1, " mysql_stmt_execute() failed: %d\n", mysql_stmt_errno(mysql_stmt));
                logfile->log(1, " %s\n", mysql_stmt_error(mysql_stmt));

                if (2013 == mysql_stmt_errno(mysql_stmt)) {
                  logfile->log(1, " trying to reconnect mysql");
                  destroy();
                  init();
                } else {
                  exit(0);
                }
    }
}

void MySql::insertStats(std::string timestamp, Statistics* stat) {
                MYSQL_BIND bind[6];
                unsigned long str_length_buff = timestamp.size();
                unsigned long str_protocol = 0;
                short inbound = (stat->inbound ? 1 : 0);
                short intern = (stat->intern ? 1 : 0);
                memset(bind, 0, sizeof(bind));
                bind[0].buffer_type= MYSQL_TYPE_STRING;
                bind[0].buffer= (char *)timestamp.c_str();
                bind[0].length= &str_length_buff;
                bind[1].buffer_type= MYSQL_TYPE_LONGLONG;
                bind[1].buffer= (char *)&stat->sum;
                bind[2].buffer_type= MYSQL_TYPE_SHORT;
                bind[2].buffer= (char *)&inbound;
                bind[3].buffer_type= MYSQL_TYPE_SHORT;
                bind[3].buffer= (char *)&intern;
                bind[4].buffer_type= MYSQL_TYPE_LONG;
                bind[4].buffer= (char *)&stat->dst_port;
                bind[5].buffer_type= MYSQL_TYPE_STRING;
                if (stat->protocol == IPPROTO_TCP) {
                    bind[5].buffer = (char *)"tcp";
                    str_protocol = 3;
                } else if (stat->protocol == IPPROTO_UDP) {
                    bind[5].buffer = (char *)"udp";
                    str_protocol = 3;
                }
                bind[5].length = &str_protocol;
    for (int tries=1; tries<5; tries++) {
                /* Bind the buffers */
                if (mysql_stmt_bind_param(mysql_stmt_stats, bind))
                {
                  logfile->log(1, " mysql_stmt_bind_param() failed: %d\n", mysql_stmt_errno(mysql_stmt_stats));
                  logfile->log(1, " %s\n", mysql_stmt_error(mysql_stmt_stats));
                  exit(0);
                }
                /* Execute the INSERT statement - 1*/
                if (!mysql_stmt_execute(mysql_stmt_stats)) break;
                logfile->log(1, " mysql_stmt_execute(), failed: %d\n", mysql_stmt_errno(mysql_stmt_stats));
                logfile->log(1, " %s\n", mysql_stmt_error(mysql_stmt_stats));

                if (2013 == mysql_stmt_errno(mysql_stmt_stats)) {
                  logfile->log(1, " trying to reconnect mysql");
                  destroy();
                  init();
                } else {
                  exit(0);
                }
  }
}
