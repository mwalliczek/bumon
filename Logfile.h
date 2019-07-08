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

#include <string>
#include <mutex>

class Logfile {
    FILE* logfile;
    std::string logfilePath;
    int logLevel;
    std::mutex log_mutex;
    
    public:
        Logfile(std::string logfilePath, int logLevel);
        ~Logfile();
        void log(int logLevel, std::string message);
        void log(int logLevel, const char *format, ...);
        void hup();
        void flush();
};

#endif