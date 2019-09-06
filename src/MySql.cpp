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
const char* insertConnectionsStatement="INSERT INTO connections (timestamp, duration, foreign_ip, dst_port, protocol, process, content, bytes, inbound, intern) \
            VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
const char* insertStatsStatement="INSERT INTO stats (timestamp, bytes, inbound, intern, dst_port, protocol, duration) VALUES(?, ?, ?, ?, ?, ?, 3600)";
const char* selectStatsStatement="SELECT bytes FROM stats WHERE inbound=? AND intern=? AND dst_port=? AND protocol=? AND HOUR(timestamp)=HOUR(?) ORDER BY bytes";
const char* numberStatsStatement="SELECT COUNT(DISTINCT timestamp) FROM stats WHERE HOUR(timestamp)=HOUR(?)";
const char* cleanupConnectionsStatement="DELETE FROM connections WHERE timestamp < DATE_SUB(CURRENT_DATE(), INTERVAL ? DAY)";
const char* cleanupStatsStatement="DELETE FROM stats WHERE timestamp < DATE_SUB(CURRENT_DATE(), INTERVAL ? MONTH)";

MySql::MySql(const char* mysql_host, const char* mysql_db, const char* mysql_username, const char* mysql_password):
        mysql_host(mysql_host), mysql_db(mysql_db), mysql_username(mysql_username), mysql_password(mysql_password) {
    init();
}

MYSQL_STMT * MySql::initStmt(const char* stmt) {
  MYSQL_STMT* mysql_stmt = mysql_stmt_init(mysql_connection);
    if (!mysql_stmt)
    {
      fprintf(stderr, " mysql_stmt_init(), out of memory\n");
      exit(0);
    }
    if (mysql_stmt_prepare(mysql_stmt, stmt, strlen(stmt))) {
      fprintf(stderr, " mysql_stmt_prepare(), INSERT failed\n");
      fprintf(stderr, " %s\n", mysql_stmt_error(mysql_stmt));
      exit(0);
    }
    return mysql_stmt;
}

void MySql::init() {
    mysql_connection = mysql_init(NULL);
    if (!mysql_real_connect(mysql_connection, mysql_host, mysql_username, mysql_password, mysql_db, 0, NULL, 0)) {
      fprintf(stderr, "Conection error : %s\n", mysql_error(mysql_connection));
      exit(0);
    }
    mysql_stmt_bandwidth = initStmt(insertBandwidthStatement);
    mysql_stmt_connections = initStmt(insertConnectionsStatement);
    mysql_stmt_insert_stats = initStmt(insertStatsStatement);
    mysql_stmt_select_stats = initStmt(selectStatsStatement);
    mysql_stmt_number_stats = initStmt(numberStatsStatement);
    mysql_stmt_cleanup_connections = initStmt(cleanupConnectionsStatement);
    mysql_stmt_cleanup_stats = initStmt(cleanupStatsStatement);
}

MySql::~MySql() {
  destroy();
}

void MySql::destroy() {
    mysql_stmt_close(mysql_stmt_bandwidth);
    mysql_stmt_close(mysql_stmt_connections);
    mysql_stmt_close(mysql_stmt_insert_stats);
    mysql_stmt_close(mysql_stmt_select_stats);
    mysql_stmt_close(mysql_stmt_number_stats);
    mysql_stmt_close(mysql_stmt_cleanup_connections);
    mysql_stmt_close(mysql_stmt_cleanup_stats);
    mysql_close(mysql_connection);
}

char name[256];

const char* MySql::protocolName(short protocol) {
    switch (protocol) {
        case IPPROTO_ICMP:
            return "icmp";
        case IPPROTO_ICMPV6:
            return "icmpv6";
        case IPPROTO_TCP:
            return "tcp";
        case IPPROTO_UDP:
            return "udp";
        case 255:
            return "";
        default:
            snprintf(name, 256, "unk (%i)", protocol);
            return name;
    }
}

void MySql::mapProtocol(short protocol, unsigned long *str_length, MYSQL_BIND* bind) {
  const char* protocolNameString = protocolName(protocol);
  bind->buffer_type= MYSQL_TYPE_STRING;
  bind->buffer = (char*) protocolNameString;
  *str_length = strlen(protocolNameString);
  bind->length = str_length;
}

void MySql::bindAndExecute(MYSQL_STMT* stmt, MYSQL_BIND* bind) {
    for (int tries=1; tries<5; tries++) {
        /* Bind the buffers */
        if (mysql_stmt_bind_param(stmt, bind))
        {
          LOG_ERROR(" mysql_stmt_bind_param() failed: %d\n", mysql_stmt_errno(stmt));
          LOG_ERROR(" %s\n", mysql_stmt_error(stmt));
          exit(0);
        }
        /* Execute the INSERT statement - 1*/
        if (!mysql_stmt_execute(stmt)) break;
        LOG_ERROR(" mysql_stmt_execute(), failed: %d\n", mysql_stmt_errno(stmt));
        LOG_ERROR(" %s\n", mysql_stmt_error(stmt));
          
        if (2013 == mysql_stmt_errno(stmt)) {
            LOG_ERROR(" trying to reconnect mysql");
            destroy();
            init();
        } else {
            exit(0);
        }
    }
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
    
    bindAndExecute(mysql_stmt_bandwidth, bind);
}

void MySql::insertConnection(char* buff, short duration, const char* foreign_ip, int dst_port, short protocol, 
        const char* process, const char* content, long long int bytes, short inbound, short intern) {
    MYSQL_BIND bind[10];
    unsigned long str_length_buff = strlen(buff);
    unsigned long str_length_ip = strlen(foreign_ip);
    unsigned long str_length_process = strlen(process);
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
    mapProtocol(protocol, &str_protocol, &bind[4]);
    bind[5].buffer_type= MYSQL_TYPE_STRING;
    bind[5].buffer= (char *)process;
    bind[5].length = &str_length_process;
    bind[6].buffer_type= MYSQL_TYPE_STRING;
    bind[6].buffer= (char *)content;
    bind[6].length = &str_length_content;
    bind[7].buffer_type= MYSQL_TYPE_LONGLONG;
    bind[7].buffer= (char *)&bytes;
    bind[8].buffer_type= MYSQL_TYPE_SHORT;
    bind[8].buffer= (char *)&inbound;
    bind[9].buffer_type= MYSQL_TYPE_SHORT;
    bind[9].buffer= (char *)&intern;
    
    bindAndExecute(mysql_stmt_connections, bind);
}

void MySql::insertStats(char* buff, Statistics* stat) {
    MYSQL_BIND bind[6];
    unsigned long str_length_buff = strlen(buff);
    unsigned long str_protocol = 0;
    short inbound = (stat->inbound ? 1 : 0);
    short intern = (stat->intern ? 1 : 0);
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type= MYSQL_TYPE_STRING;
    bind[0].buffer= buff;
    bind[0].length= &str_length_buff;
    bind[1].buffer_type= MYSQL_TYPE_LONGLONG;
    bind[1].buffer= (char *)&stat->sum;
    bind[2].buffer_type= MYSQL_TYPE_SHORT;
    bind[2].buffer= (char *)&inbound;
    bind[3].buffer_type= MYSQL_TYPE_SHORT;
    bind[3].buffer= (char *)&intern;
    bind[4].buffer_type= MYSQL_TYPE_LONG;
    bind[4].buffer= (char *)&stat->dst_port;
    mapProtocol(stat->protocol, &str_protocol, &bind[5]);
    
    bindAndExecute(mysql_stmt_insert_stats, bind);
}

std::vector<long long int>* MySql::lookupStats(char* buff, int dst_port, int protocol, bool intern, bool inbound) {
    MYSQL_BIND bind[5];
    unsigned long str_length_buff = strlen(buff);
    unsigned long str_protocol = 0;
    short inboundMapped = (inbound ? 1 : 0);
    short internMapped = (intern ? 1 : 0);
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type= MYSQL_TYPE_SHORT;
    bind[0].buffer= (char *)&inboundMapped;
    bind[1].buffer_type= MYSQL_TYPE_SHORT;
    bind[1].buffer= (char *)&internMapped;
    bind[2].buffer_type= MYSQL_TYPE_LONG;
    bind[2].buffer= (char *)&dst_port;
    mapProtocol(protocol, &str_protocol, &bind[3]);
    bind[4].buffer_type= MYSQL_TYPE_STRING;
    bind[4].buffer= buff;
    bind[4].length= &str_length_buff;
    
    MYSQL_BIND resultBind[1];
    long long int bytes;
    memset(resultBind, 0, sizeof(resultBind));
    resultBind[0].buffer_type= MYSQL_TYPE_LONGLONG;
    resultBind[0].buffer= (char *)&bytes;

    /* Bind the buffers */
    if (mysql_stmt_bind_param(mysql_stmt_select_stats, bind)) {
      LOG_ERROR(" mysql_stmt_bind_param() failed: %d\n", mysql_stmt_errno(mysql_stmt_select_stats));
      LOG_ERROR(" %s\n", mysql_stmt_error(mysql_stmt_select_stats));
      exit(0);
    }
    if (mysql_stmt_execute(mysql_stmt_select_stats)) {
      LOG_ERROR(" mysql_stmt_execute() failed: %d\n", mysql_stmt_errno(mysql_stmt_select_stats));
      LOG_ERROR(" %s\n", mysql_stmt_error(mysql_stmt_select_stats));
      exit(0);
    }
    if (mysql_stmt_bind_result(mysql_stmt_select_stats, resultBind)) {
      LOG_ERROR(" mysql_stmt_bind_result() failed: %d\n", mysql_stmt_errno(mysql_stmt_select_stats));
      LOG_ERROR(" %s\n", mysql_stmt_error(mysql_stmt_select_stats));
      exit(0);
    }
    if (mysql_stmt_store_result(mysql_stmt_select_stats)) {
      LOG_ERROR(" mysql_stmt_bind_result() failed: %d\n", mysql_stmt_errno(mysql_stmt_select_stats));
      LOG_ERROR(" %s\n", mysql_stmt_error(mysql_stmt_select_stats));
      exit(0);
    }
    my_ulonglong resultCount = mysql_stmt_num_rows(mysql_stmt_select_stats);
    LOG_DEBUG("lookupStats: found %d rows", resultCount);
    std::vector<long long int>* resultVector = new std::vector<long long int>();
    resultVector->reserve(resultCount);
    for (my_ulonglong i=0; i<resultCount; i++) {
      if (mysql_stmt_fetch(mysql_stmt_select_stats)) {
        LOG_ERROR(" mysql_stmt_fetch() failed: %d\n", mysql_stmt_errno(mysql_stmt_select_stats));
        LOG_ERROR(" %s\n", mysql_stmt_error(mysql_stmt_select_stats));
        exit(0);
      }
      resultVector->push_back(bytes);
    }
    return resultVector;
}

int MySql::lookupNumberStats(char* buff) {
    MYSQL_BIND bind[1];
    unsigned long str_length_buff = strlen(buff);
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type= MYSQL_TYPE_STRING;
    bind[0].buffer= buff;
    bind[0].length= &str_length_buff;

    MYSQL_BIND resultBind[1];
    int number;
    memset(resultBind, 0, sizeof(resultBind));
    resultBind[0].buffer_type= MYSQL_TYPE_LONG;
    resultBind[0].buffer= (char *)&number;

    /* Bind the buffers */
    if (mysql_stmt_bind_param(mysql_stmt_number_stats, bind)) {
      LOG_ERROR(" mysql_stmt_bind_param() failed: %d\n", mysql_stmt_errno(mysql_stmt_number_stats));
      LOG_ERROR(" %s\n", mysql_stmt_error(mysql_stmt_number_stats));
      exit(0);
    }
    if (mysql_stmt_execute(mysql_stmt_number_stats)) {
      LOG_ERROR(" mysql_stmt_execute() failed: %d\n", mysql_stmt_errno(mysql_stmt_number_stats));
      LOG_ERROR(" %s\n", mysql_stmt_error(mysql_stmt_number_stats));
      exit(0);
    }
    if (mysql_stmt_bind_result(mysql_stmt_number_stats, resultBind)) {
      LOG_ERROR(" mysql_stmt_bind_result() failed: %d\n", mysql_stmt_errno(mysql_stmt_number_stats));
      LOG_ERROR(" %s\n", mysql_stmt_error(mysql_stmt_number_stats));
      exit(0);
    }
    if (mysql_stmt_store_result(mysql_stmt_number_stats)) {
      LOG_ERROR(" mysql_stmt_bind_result() failed: %d\n", mysql_stmt_errno(mysql_stmt_number_stats));
      LOG_ERROR(" %s\n", mysql_stmt_error(mysql_stmt_number_stats));
      exit(0);
    }
    my_ulonglong resultCount = mysql_stmt_num_rows(mysql_stmt_number_stats);
    if (resultCount == 0) {
      return 0;
    }
    if (mysql_stmt_fetch(mysql_stmt_number_stats)) {
      LOG_ERROR(" mysql_stmt_fetch() failed: %d\n", mysql_stmt_errno(mysql_stmt_number_stats));
      LOG_ERROR(" %s\n", mysql_stmt_error(mysql_stmt_number_stats));
      exit(0);
    }
    return number;
}

void MySql::cleanupConnections(int days) {
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type= MYSQL_TYPE_LONG;
    bind[0].buffer= (char *)&days;
    
    bindAndExecute(mysql_stmt_cleanup_connections, bind);
}

void MySql::cleanupStats(int month) {
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type= MYSQL_TYPE_LONG;
    bind[0].buffer= (char *)&month;
    
    bindAndExecute(mysql_stmt_cleanup_stats, bind);
}
