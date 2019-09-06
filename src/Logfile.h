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

#ifndef LOGFILE_H
#define LOGFILE_H

#include <libgen.h>

#include <string>
#include <mutex>
#include <map>

#define ERROR 1
#define WARN 2
#define INFO 3
#define DEBUG 4
#define TRACE 5

#define LOG_INFO(...) logfile->log(basename((char*) __FILE__), INFO, __VA_ARGS__)
#define LOG_WARN(...) logfile->log(basename((char*) __FILE__), WARN, __VA_ARGS__)
#define LOG_ERROR(...) logfile->log(basename((char*) __FILE__), ERROR, __VA_ARGS__)
#define LOG_DEBUG(...) logfile->log(basename((char*) __FILE__), DEBUG, __VA_ARGS__)
#define LOG_TRACE(...) logfile->log(basename((char*) __FILE__), TRACE, __VA_ARGS__)

#define LOG_CHECK_INFO() logfile->checkLevel(basename((char*) __FILE__), INFO)
#define LOG_CHECK_WARN() logfile->checkLevel(basename((char*) __FILE__), WARN)
#define LOG_CHECK_ERROR() logfile->checkLevel(basename((char*) __FILE__), ERROR)
#define LOG_CHECK_DEBUG() logfile->checkLevel(basename((char*) __FILE__), DEBUG)
#define LOG_CHECK_TRACE() logfile->checkLevel(basename((char*) __FILE__), TRACE)

class Logfile {
    FILE* logfile;
    std::string logfilePath;
    int defaultLogLevel;
    std::map<std::string, int> loglevels;
    std::mutex log_mutex;
    
    public:
        Logfile(std::string logfilePath, int defaultLogLevel, std::map<std::string, int> loglevels);
        ~Logfile();
        void log(std::string classname, int logLevel, std::string message);
        void log(std::string classname, int logLevel, const char *format, ...);
        bool checkLevel(std::string classname, int logLevel) const;
        void hup();
        void flush();
        static int parseLoglevel(std::string stringLogLevel);
};

#endif