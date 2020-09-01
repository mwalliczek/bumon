/*
 * Copyright (c) 2020 Matthias Walliczek
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

#ifndef LOGFILEMOCK_H
#define LOGFILEMOCK_H

#include <sstream>

#include "Logfile.h"

class LogfileMock : public Logfile {
    public:
        LogfileMock();
        void log(std::string const &classname, int logLevel, const char *format, ...) override;
        void setLevel(std::string, int);
        std::stringstream content;
};

#endif