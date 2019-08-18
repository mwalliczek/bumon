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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sstream>

#include "Statistics.h"

#include "bumon.h"

Stats::Stats(MySql* mysql_connection, char* sender, char* recipient): mysql_connection(mysql_connection), sender(sender), recipient(recipient) { }

std::string generateMessagesId(std::map<std::string, Statistics*>::iterator statsIter) {
    std::stringstream messagesId;
    messagesId << statsIter->first.substr(0, 19) << " " << statsIter->second->intern << " " << statsIter->second->inbound;
    return messagesId.str();
}

void Stats::cleanup(char *statsbuff) {
    if (lastStatsTimestamp == std::string(statsbuff)) {
        return;
    }
    std::map<std::string, Statistics*>::iterator statsIter = statistics.begin();
    if (mysql_connection != NULL) {
        std::map<std::string, std::string> messages;
        while (statsIter != statistics.end()) {
            if (statsIter->first.rfind(statsbuff, 0) != 0 && 0 == statsIter->second->dst_port && 0 == statsIter->second->protocol) {
                std::string checkSpikeResult = checkSpike(statsIter->first.substr(0, 19).c_str(), 0, 255, statsIter->second->intern, 
                        statsIter->second->inbound, statsIter->second->sum);
                messages[generateMessagesId(statsIter)] = checkSpikeResult;
            }
            statsIter++;
        }
        if (messages.size() > 0) {
            bool shouldSendMail = false;
            statsIter = statistics.begin();
            while (statsIter != statistics.end()) {
                if (statsIter->first.rfind(statsbuff, 0) != 0 && 0 != statsIter->second->dst_port && 
                        0 != statsIter->second->protocol) {
                    std::string messagesId = generateMessagesId(statsIter);
                    std::map<std::string, std::string>::iterator messagesIter = messages.find(messagesId);
                    if (messagesIter == messages.end()) {
                        messages[messagesId] = checkSpike(statsIter->first.substr(0, 19).c_str(), 0, 255, statsIter->second->intern,
                                statsIter->second->inbound, 0);
                        messagesIter = messages.find(messagesId);
                    }
                    if (!messagesIter->second.empty()) {
                        std::string result;
                        if (!(result = checkSpike(statsIter->first.substr(0, 19).c_str(), statsIter->second->dst_port, 
                                statsIter->second->protocol, statsIter->second->intern, statsIter->second->inbound, 
                                statsIter->second->sum)).empty()) {
                            messagesIter->second += result;
                            shouldSendMail = true;
                        }
                    }
                }
                statsIter++;
            }
            if (shouldSendMail) {
                sendMail(&messages);
            }
        }
        statsIter = statistics.begin();
    }
    while (statsIter != statistics.end()) {
        if (statsIter->first.rfind(statsbuff, 0) != 0) {
            logfile->log(3, "%s: %d %lli", statsIter->first.substr(0, 19).c_str(), statsIter->second->dst_port, statsIter->second->sum);
            if (mysql_connection != NULL && statsIter->second->sum >= 1024) {
                mysql_connection->insertStats((char*)statsIter->first.substr(0, 19).c_str(), statsIter->second);
            }
            delete statsIter->second;
            statsIter = statistics.erase(statsIter);
        } else {
            statsIter++;
        }
    }
}

std::string formatBandwidth(long long int sum) {
    float tb = 1099511627776;
    float gb = 1073741824;
    float mb = 1048576;
    float kb = 1024;


    char returnSize[256];


    if (sum >= tb)
        snprintf(returnSize, 256, "%.2f TB", (float)sum/tb);        
    else if (sum >= gb)
        snprintf(returnSize, 256, "%.2f GB", (float)sum/gb);
    else if (sum >= mb)
        snprintf(returnSize, 256, "%.2f MB", (float)sum/mb);   
    else if (sum >= kb)
        snprintf(returnSize, 256, "%.2f KB", (float)sum/kb);
    else
        snprintf(returnSize, 256, "%.2lli Bytes", sum);


    return std::string(returnSize);
}

std::string resolveIp(std::string ipstr) {
    struct in_addr ip;
    struct hostent *hp;
    
    if (!inet_aton(ipstr.c_str(), &ip)) {
        logfile->log(3, "Can't parse IP address %s", ipstr.c_str());
        return ipstr;
    }

    if ((hp = gethostbyaddr((const void *)&ip, sizeof ip, AF_INET)) == NULL) {
        logfile->log(10, "No name associated with %s", ipstr.c_str());
        return ipstr;
    }
    
    return ipstr + " (" + hp->h_name + ")";
}

std::string Stats::checkSpike(const char* statsbuff, int dst_port, int protocol, bool intern, bool inbound, long long int sum) {
    std::vector<long long int>* stats = mysql_connection->lookupStats((char*) statsbuff, dst_port, protocol, intern, inbound);
    int length = stats->size();
    int number = mysql_connection->lookupNumberStats((char*) statsbuff);
    while (length < number) {
        stats->insert(stats->begin(), (long long int) 0);
        length = stats->size();
    }
    if (length < 10) {
        delete stats;
        return "";
    }
    long long int q25 = (*stats)[length / 4];
    long long int q75 = (*stats)[length * 3 / 4];
    delete stats;
    long long int qdiff = q75 - q25;
    std::string message;
    if (sum < q25) {
        long long int diff = q25 - sum;
        if ((double) diff / (double) qdiff > 1.5) {
            message = " is lower: " + formatBandwidth(sum) + " (" + formatBandwidth(q25) + ")\n";
        }
    } else if (sum > q75) {
        long long int diff = sum - q75;
        if (qdiff == 0 || (double) diff / (double) qdiff > 1.5) {
            message = " is higher: " + formatBandwidth(sum) + " (" + formatBandwidth(q75) + ")\n";
        }
    }
    if (!message.empty()) {
        if (protocol == 255) {
            message = std::string(inbound ? "Inbound" : "Outbound") + std::string(intern ? " intern" : " extern") + message;
        } else {
            message = "  Port " + std::to_string(dst_port) + " (" + std::string(protocolName(protocol)) + ")" + message;
            if (!intern) {
                std::vector<HostWithBandwidth>* topHosts = mysql_connection->lookupTopHostsWithBandwidth((char*) statsbuff, dst_port, protocol, inbound);
                for (auto& topHostsIter : (*topHosts)) {
                    message += "    " + resolveIp(topHostsIter.host) + ": " + formatBandwidth(topHostsIter.bytes) + "\n";
                }
                delete topHosts;
            }
        }
    }
    return message;
}

void Stats::insert(char *statsbuff, int dst_port, int protocol, bool intern, bool inbound, long long int sum) {
    std::stringstream result;
    lastStatsTimestamp = std::string(statsbuff);
    result << statsbuff << " " << dst_port << " " << protocol << " " << intern << " " << inbound;
    std::string statsId = result.str();
    std::map<std::string, Statistics*>::iterator statsIter = statistics.find(statsId);
    if (statsIter != statistics.end()) {
        statsIter->second->sum += sum;
    } else {
        Statistics* newStats = new Statistics();
        newStats->dst_port = dst_port;
        newStats->protocol = protocol;
        newStats->sum = sum;
        newStats->intern = intern;
        newStats->inbound = inbound;
        statistics[statsId] = newStats;
    }
}

void Stats::sendMail(std::map<std::string, std::string>* messages) {
    if (NULL != sender && NULL != recipient) {
        FILE *mailpipe = popen(sendmailPath.c_str(), "w");
        if (mailpipe != NULL) {
            fprintf(mailpipe, "To: %s\n", recipient);
            fprintf(mailpipe, "From: %s\n", sender);
            fprintf(mailpipe, "Subject: Traffic-Warnung\n\n");
            for (auto& messagesIter : (*messages)) {
                if (!messagesIter.second.empty()) {
                    logfile->log(2, "%s: %s", messagesIter.first.substr(0, 19).c_str(), messagesIter.second.c_str());
                    fwrite(messagesIter.first.substr(0, 19).c_str(), 1, 19, mailpipe);
                    fwrite("\n", 1, 1, mailpipe);
                    fwrite(messagesIter.second.c_str(), 1, messagesIter.second.length(), mailpipe);
                    fwrite("\n", 1, 1, mailpipe);
                }
            }
            pclose(mailpipe);
        } else {
            logfile->log(3, "Failed to invoke sendmail");
        }
     } else {
        for (auto& messagesIter : (*messages)) {
            logfile->log(2, "%s: %s", messagesIter.first.substr(0, 19).c_str(), messagesIter.second.c_str());
        }
     }
}
