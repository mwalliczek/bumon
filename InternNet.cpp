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
#include <netinet/in.h>

#include "InternNet.h"

InternNet::InternNet(const char* ip, const char* mask) {
    valid = true;
    valid &= (1 == inet_pton(AF_INET, ip, &(this->ip)));
    valid &= (1 == inet_pton(AF_INET, mask, &(this->mask)));
}

bool InternNet::match(struct in_addr ip) {
    return (ip.s_addr & this->mask.s_addr) == this->ip.s_addr;
}
