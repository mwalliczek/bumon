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

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <iostream>

#include "FindProcess.h"

std::string ssPath = "ss";

int main(int argc, char *argv[])
{
 FindProcess* findProcess = new FindProcess();
 
 int port = 40000 + rand() % 100;
 int sock; //defines the sockets TO SEND
 struct sockaddr_in serv_addr, client_addr; 

 memset(&serv_addr, 0, sizeof(serv_addr));
 serv_addr.sin_family = AF_INET;//family of the socket, for internet it's AF_INET
 serv_addr.sin_port = htons(25);
 if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  { 
    std::cout << "Invalid address/ Address not supported" << std::endl; 
    exit(-1); 
 }   
 sock = socket(AF_INET, SOCK_STREAM, 0);
 if (sock < 0) {
    std::cout << "INVALID SOCKET" << std::endl;
    exit(-1); 
 }

 memset(&client_addr, 0, sizeof(client_addr));
 client_addr.sin_family = AF_INET;//family of the socket, for internet it's AF_INET
 client_addr.sin_port = htons(port);
 bind(sock, (struct sockaddr *)&client_addr, sizeof(client_addr)); //binds the socket to the port and the adress above

 if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cout << "Connection failed with error" << std::endl;
    exit(-1); 
 }

 usleep(100000);
 std::cout << "findActiveTcpProcess result: " << findProcess->findActiveTcpProcess(port, "127.0.0.1", 25) << std::endl;
 close(sock);
 delete findProcess;
}
