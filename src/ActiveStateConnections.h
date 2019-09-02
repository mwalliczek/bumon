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

#ifndef ACTIVESTATECONNECTIONS_H
#define ACTIVESTATECONNECTIONS_H

#include <optional>

#include "ActiveConnections.h"

template<typename IP>
class ActiveStateConnections : public ActiveConnections<IP> {
    std::map<ConnectionIdentifier<IP>, int> map;
    std::mutex map_mutex;
protected:
    void lock();
    void unlock();
    Connection* createConnection(IP const & ip_src, int sport, IP const & ip_dst, int dport, u_char protocol, 
            const char *logMessage); 
    std::optional<std::pair<ConnectionIdentifier<IP>, Connection*>> findConnection(IP const & ip_src, int sport, 
            IP const & ip_dst, int dport);
    std::map<ConnectionIdentifier<IP>, int>* getMap();
public:
    ActiveStateConnections(std::list<InternNet<IP>> const & interns, std::list<IP> const & selfs);
};

#endif

