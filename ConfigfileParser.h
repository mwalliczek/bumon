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

#ifndef CONFIGFILEPARSER_H
#define CONFIGFILEPARSER_H

#include <list>
#include <string>
#include <map>

class IpAndMask {
public:
    IpAndMask(std::string ip, std::string mask);
    std::string ip;
    std::string mask;
};

class ConfigfileParser {
private:
    bool handleIntern(std::string val);
    bool handleSelf(std::string val);
public:
    ConfigfileParser(std::string configFile);
    std::map<std::string, std::string> options;
    std::list<IpAndMask> interns;
    std::list<std::string> selfs;
};

#endif