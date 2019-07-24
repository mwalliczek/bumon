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
#include "bumon.h"

Logfile::Logfile(std::string logfilePath, int logLevel) {
    if (debug) {
        printf("Start Logfile: %s %d\n", logfilePath.c_str(), logLevel);
    }
    if (!logfilePath.empty()) {
        logfile = fopen(logfilePath.c_str(), "a");
        this->logfilePath = logfilePath;
    } else {
        logfile = stdout;
    }
    this->logLevel = logLevel;
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

bool Logfile::checkLevel(int logLevel) {
    return (logLevel <= this->logLevel);
}

void Logfile::log(int logLevel, std::string message) {
    if (checkLevel(logLevel)) {
        getTime();
        log_mutex.lock();
        fprintf(logfile, "%s %s\n", buff, message.c_str());
        log_mutex.unlock();
    }
}

void Logfile::log(int logLevel, const char *format, ...) {
    if (checkLevel(logLevel)) {
        getTime();
        fprintf(logfile, "%s ", buff);
        va_list arg;
        va_start (arg, format);
        vfprintf (logfile, format, arg);
        va_end (arg);
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
