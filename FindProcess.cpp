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

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <exception>
#include <functional>

#include "FindProcess.h"

void callProcessAndIterate(const char* cmd, std::function<void (char*&)> func) {
    char* line = NULL;
    size_t len = 0;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while ((getline(&line, &len, pipe.get())) != -1) {
        func(line);
    }
    if (line)
        free(line);
}

std::string parseProcess(std::string process) {
    std::size_t pos = process.find("\"");
    if (pos == std::string::npos)
        return "";
    process = process.substr(pos + 1);
    pos = process.find("\"");
    if (pos == std::string::npos)
        return "";
    return process.substr(0, pos);
}

int splitLine(char* line, std::function<void (std::string, int)> func) {
    std::string token;
    std::istringstream tokenStream(line);
    int index = 0;
    while (std::getline(tokenStream, token, ' '))
    {
        if (token.empty())
            continue;
        func(token, index++);
    }
    return index;
}

std::map<u_short, std::string> parseSS(const char* cmd) {
    std::map<u_short, std::string> newListen;
    callProcessAndIterate(cmd, [&newListen] (char *line) {
        std::string dst, process;
        int index = splitLine(line, [&dst, &process] (std::string token, int i) {
            if (i == 3) {
                dst = token;
            } else if (i == 5) {
                process = token;
            }
        });
        if (!dst.empty() && !process.empty() && index < 8) {
            int port;
            if (dst.substr(0, 9) == "127.0.0.1")
                return;
            std::size_t pos = dst.rfind(":");
            if (pos == std::string::npos)
                return;
            try {
                port = std::stoi(dst.substr(pos+1));
            } catch (std::exception& e)
            {
                std::cout << e.what() << ": " << dst << '\n';
                return;
            }
            process = parseProcess(process);
            if (process.empty())
                return;
            newListen[port] = process;
//            std::cout << port << ": " << process << std::endl;
        }
    });
    return newListen;
}

void FindProcess::init() {
    this->tcpListen = parseSS("ss -lnpt");
    this->udpListen = parseSS("ss -lnpu");
}

std::string FindProcess::findListenTcpProcess(int port) {
    std::map<u_short, std::string>::iterator it = tcpListen.find(port);
    if (it != tcpListen.end())
        return it->second;
    return "";
}

std::string FindProcess::findListenUdpProcess(int port) {
    std::map<u_short, std::string>::iterator it = udpListen.find(port);
    if (it != udpListen.end())
        return it->second;
    return "";
}

std::string FindProcess::findActiveTcpProcess(int sport, std::string dst_ip, int dport) {
    std::string result;
    std::stringstream cmd;
    cmd << "ss -anpt sport = :" << sport << " and dst " << dst_ip << ":" << dport;  
    callProcessAndIterate(cmd.str().c_str(), [&result] (char *line) {
        std::string process;
        int index = splitLine(line, [&process] (std::string token, int i) {
            if (i == 5) {
                process = token;
            }
        });
        if (!process.empty() && index < 8) {
            result = parseProcess(process);
        }
    });
    return result;
}
