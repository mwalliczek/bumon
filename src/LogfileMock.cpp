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

#include <stdarg.h>

#include "LogfileMock.h"

LogfileMock::LogfileMock() : Logfile("", DEBUG, std::map<std::string, int>()) {
}

void LogfileMock::setLevel(std::string className, int level) {
    loglevels[className] = level;
}

char logBuffer[4096];

void LogfileMock::log(std::string const &classname, int logLevel, const char *format, ...) {
    snprintf(logBuffer, 255, "%s %s: ", logLevelName(logLevel), classname.c_str());
    content << logBuffer;
    if (checkLevel(classname, logLevel)) {
        printf("%s", logBuffer);
    }
    va_list arg;
    va_start(arg, format);
    vsprintf(logBuffer, format, arg);
    va_end(arg);
    content << logBuffer << std::endl;
    if (checkLevel(classname, logLevel)) {
        printf("%s\n", logBuffer);
    }
}

