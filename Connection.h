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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <netinet/in.h>
#include <string>
#include <ctime>

#include "ConnectionIdentifier.h"

class Connection {
    bool payload;
    std::string identifier;
    
    public:
        Connection(struct in_addr, u_short, struct in_addr, u_short, u_char, std::string, std::map<ConnectionIdentifier, int>*, const char*);
        Connection(struct in_addr, struct in_addr, u_char);
        void stop();
        void handleData(int len);
        void handleData(int len, const u_char *payload, int size_payload);
        int id;
        bool intern;
        bool ack;
        time_t lastAct;
        time_t end;
        bool alreadyRunning;
        std::string content;
        struct in_addr src_ip,dst_ip;
        u_short src_port,dst_port;
        time_t begin;
        u_char protocol;
        std::string process;
        std::string getIdentifier();
};

#endif
