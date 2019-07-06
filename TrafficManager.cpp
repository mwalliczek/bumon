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

#include "bumon.h"

#include "TrafficManager.h"

TrafficManager::TrafficManager() {
    currentTraffic = new std::map<int, long long int>();
    time(&currentTime);
}

TrafficManager::~TrafficManager() {
    currentTraffic->clear();
    delete currentTraffic;
}

void TrafficManager::handleTraffic(int connectionId, int length) {
    time_t newTime;
    time(&newTime);
    if (difftime(newTime, currentTime) > 300) {
        watching->addHistory(currentTime, currentTraffic);
        currentTime = newTime;
        currentTraffic = new std::map<int, long long int>();
    }
    std::map<int, long long int>::iterator it = currentTraffic->find(connectionId);
    if (it != currentTraffic->end()) {
        it->second += length;
    } else {
        (*currentTraffic)[connectionId] = length;
    }
}