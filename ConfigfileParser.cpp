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

#include <arpa/inet.h>

#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <iostream>

#include "ConfigfileParser.h"
#include "InternNet.h"
#include "bumon.h"

extern std::list<InternNet*> interns;

inline std::string trim(std::string& str) {
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    str.erase(str.find_last_not_of(' ')+1);         //surfixing spaces
    return str;
}

bool handleIntern(std::string val) {
    for (std::list<InternNet*>::iterator i=interns.begin(); i!=interns.end(); i++)
        delete *i;
    interns.clear();

    std::stringstream ss(val);
    while( ss.good() ) {
        std::string entry;
        getline( ss, entry, ',' );
        trim(entry);
        size_t slash = entry.find('/');
        std::string ip, mask;
        if (slash == std::string::npos) {
            return true;
        }
        ip = entry.substr(0, slash);
        mask = entry.substr(slash+1, std::string::npos);
        InternNet* newInternNet = new InternNet(ip.c_str(), mask.c_str());
        if (!newInternNet->valid) {
            delete newInternNet;
            return true;
        }
        interns.push_back(newInternNet);
    }
    return false;
}

bool handleSelf(std::string val) {
    return (1 != inet_pton(AF_INET, val.c_str(), &self_ip));
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
