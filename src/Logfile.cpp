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

#include <stdarg.h>

#include "Logfile.h"
#include "netimond.h"

Logfile::Logfile(std::string const &logfilePath, int defaultLogLevel, std::map<std::string, int> const &loglevels):
        defaultLogLevel(defaultLogLevel), loglevels(loglevels) {
    if (debug) {
        printf("Start Logfile: %s %d\n", logfilePath.c_str(), defaultLogLevel);
    }
    if (!logfilePath.empty()) {
        logfile = fopen(logfilePath.c_str(), "a");
        this->logfilePath = logfilePath;
    } else {
        logfile = stdout;
    }
}

Logfile::~Logfile() {
    if (!logfilePath.empty()) {
        log_mutex.lock();
        fclose(logfile);
    }
}

char buff[20];

void getTime() {
    time_t current;
    time(&current);
    strftime(buff, 20, "%b %d %H:%M:%S", localtime(&current));
}

const char* logLevelName(int logLevel) {
    switch (logLevel) {
        case ERROR:
            return "ERROR";
        case WARN:
            return "WARN";
        case INFO:
            return "INFO";
        case DEBUG:
            return "DEBUG";
        case TRACE:
            return "TRACE";
        default:
            return "";
    }
}

bool Logfile::checkLevel(std::string const &classname, int logLevel) const {
    auto iter = loglevels.find(classname);
    int currentLogLevel = defaultLogLevel;
    if (iter != loglevels.end()) {
        currentLogLevel = iter->second;
    }
    return (logLevel <= currentLogLevel);
}

void Logfile::log(std::string const &classname, int logLevel, std::string const &message) {
    if (checkLevel(classname, logLevel)) {
        getTime();
        log_mutex.lock();
        fprintf(logfile, "%s %s %s: %s\n", buff, logLevelName(logLevel), classname.c_str(), message.c_str());
        log_mutex.unlock();
    }
}

void Logfile::log(std::string const &classname, int logLevel, const char *format, ...) {
    if (checkLevel(classname, logLevel)) {
        getTime();
        fprintf(logfile, "%s %s %s: ", buff, logLevelName(logLevel), classname.c_str());
        va_list arg;
        va_start(arg, format);
        vfprintf(logfile, format, arg);
        va_end(arg);
        log_mutex.lock();
        fprintf(logfile, "\n");
        log_mutex.unlock();
    }
}

void Logfile::hup() {
    if (!logfilePath.empty()) {
        log_mutex.lock();
        fclose(logfile);
        logfile = fopen(logfilePath.c_str(), "a");
        log_mutex.unlock();
    }
}

void Logfile::flush() {
    if (!logfilePath.empty()) {
        fflush(logfile);
    }
}

int Logfile::parseLoglevel(std::string const &stringLogLevel) {
    if ("ERROR" == stringLogLevel) {
        return ERROR;
    } else if ("WARN" == stringLogLevel) {
        return WARN;
    } else if ("INFO" == stringLogLevel) {
        return INFO;
    } else if ("DEBUG" == stringLogLevel) {
        return DEBUG;
    } else if ("TRACE" == stringLogLevel) {
        return TRACE;
    }
    fprintf(stderr, "Can not parse LogLevel %s\n", stringLogLevel.c_str());
    return 0;
}
