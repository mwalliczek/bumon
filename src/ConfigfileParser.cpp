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

#include <arpa/inet.h>

#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <iostream>
#include <functional>

#include "ConfigfileParser.h"

#include "Logfile.h"

IpAndMask::IpAndMask(std::string ip, std::string mask):
    ip(ip), mask(mask) { }

inline std::string trim(std::string& str) {
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    str.erase(str.find_last_not_of(' ')+1);         //surfixing spaces
    return str;
}

bool parseComma(std::string val, std::function<bool (std::string&)> func) {
    std::stringstream ss(val);
    while( ss.good() ) {
        std::string entry;
        getline( ss, entry, ',' );
        trim(entry);
        if (func(entry)) {
            return true;
        }
    }
    return false;
}


bool ConfigfileParser::handleIntern(std::string val) {
    interns.clear();
    return parseComma(val, [this] (std::string entry) {
        size_t slash = entry.find('/');
        std::string ip, mask;
        if (slash == std::string::npos) {
            return true;
        }
        ip = entry.substr(0, slash);
        mask = entry.substr(slash+1, std::string::npos);
        this->interns.push_back(IpAndMask(ip, mask));
        return false;
    });
}

bool ConfigfileParser::handleSelf(std::string val) {
    selfs.clear();
    return parseComma(val, [this] (std::string entry) {
        size_t slash = entry.find('/');
        std::string ip, mask;
        if (slash == std::string::npos) {
            this->selfs.push_back(IpAndMask(entry, std::string()));
            return false;
        }
        ip = entry.substr(0, slash);
        mask = entry.substr(slash+1, std::string::npos);
        this->selfs.push_back(IpAndMask(ip, mask));
        return false;
    });
}

ConfigfileParser::ConfigfileParser(std::string configFile)
{
    std::ifstream cfgfile;
    cfgfile.open (configFile, std::fstream::in);

    int lineNr = 0;
    for (std::string line; std::getline(cfgfile, line); ) {
        std::istringstream is_line(line);
        std::string id, val;
        lineNr++;

        bool error = false;

        if( !std::getline(is_line, id, '=') ) {
            error = true;
        } else if (id[0] == '#') {
            continue; 
        } else if( !std::getline(is_line, val) ) {
            error = true;
        }
        if (!error) {
            trim(id);
            trim(val);
            if (id == "intern") {
                error = handleIntern(val);
            } else if (id == "self") {
                error = handleSelf(val);
            } else if (id.rfind("loglevel.", 0) == 0) {
                loglevels[id.substr(9)] = Logfile::parseLoglevel(val);
            } else {
                options[id] = val;
            }
        }
        if (error)
        {
            std::cerr << "Can not parse line " << lineNr << " in " << configFile << std::endl;
        }
    }
    cfgfile.close();
}

std::string ConfigfileParser::findOption(std::string key, std::string defaultValue) const {
    auto iter = options.find(key);
    if (iter != options.end()) {
        return iter->second;
    }
    return defaultValue;
}
