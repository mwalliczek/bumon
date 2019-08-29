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

#include <sstream>
#include <algorithm>	// std::sort

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
    logfile->log(3, "cleanup: %s", statsbuff);
    std::map<std::string, Statistics*>::iterator statsIter = statistics.begin();
    if (mysql_connection != NULL) {
        std::map<std::string, std::string> messages;
        while (statsIter != statistics.end()) {
            if (statsIter->first.rfind(statsbuff, 0) != 0 && 0 == statsIter->second->dst_port && 
                    255 == statsIter->second->protocol && statsIter->second->sum >= 1024) {
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
                        255 != statsIter->second->protocol && statsIter->second->sum >= 1024) {
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
    auto topHostsIter = topHosts.begin();
    while (topHostsIter != topHosts.end()) {
        if (topHostsIter->first.rfind(statsbuff, 0) != 0) {
            auto pacIter = topHostsIter->second->processAndContent.begin();
            while (pacIter != topHostsIter->second->processAndContent.end()) {
                delete pacIter->second;
                pacIter = topHostsIter->second->processAndContent.erase(pacIter);
            }
            delete topHostsIter->second;
            topHostsIter = topHosts.erase(topHostsIter);
        } else {
            topHostsIter++;
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
        snprintf(returnSize, 256, "%lli Bytes", sum);

    return std::string(returnSize);
}

ProcessAndContent* findProcessAndContent(std::map<std::string, ProcessAndContent*>* map) {
    if (map->empty()) {
        return NULL;
    }
    std::vector<ProcessAndContent*> result; 
    for (auto& iter : (*map)) {
        result.push_back(iter.second);
    }
    std::sort(result.begin(), result.end(), [](const ProcessAndContent* a, const ProcessAndContent* b) -> bool { 
        return a->sum > b->sum; 
    });
    return *(result.begin());
}

std::string Stats::checkSpike(const char* statsbuff, int dst_port, int protocol, bool intern, bool inbound, long long int sum) {
    logfile->log(3, "Check spike: %s, %d, %d, %d, %d, %lli", statsbuff, dst_port, protocol, intern, inbound, sum);
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
    logfile->log(3, "Q25: %lli, Q75: %lli", q25, q75);
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
                std::vector<TopHosts> currentTopHosts = lookupTopHosts(statsbuff, dst_port, protocol, intern, inbound);
                int topHostCount = 0;
                for (auto& topHostsIter : currentTopHosts) {
                    message += "    " + topHostsIter.ip->resolve() + ": " + formatBandwidth(topHostsIter.sum) + "\n";
                    ProcessAndContent* pac = findProcessAndContent(&topHostsIter.processAndContent);
                    if (pac != NULL && (!pac->process.empty() || !pac->content.empty())) {
                        message += "      " + pac->process;
                        if (!pac->content.empty()) {
                            message += " (" + pac->content + ")";
                        }
                        message += "\n";
                    }
                    if (topHostCount++ == 3) {
                        break;
                    }
                }
            }
        }
    }
    logfile->log(3, "Message: %s", message.c_str());
    return message;
}

std::stringstream generateId(const char *statsbuff, int dst_port, int protocol, bool intern, bool inbound) {
    std::stringstream result;
    result << statsbuff << " " << dst_port << " " << protocol << " " << intern << " " << inbound;
    return result;
}

std::string generateTopHostsId(char *statsbuff, SumConnectionIdentifier sumConnectionIdentifier) {
    std::stringstream result = generateId(statsbuff, sumConnectionIdentifier.dst_port, 
        sumConnectionIdentifier.protocol, sumConnectionIdentifier.intern, sumConnectionIdentifier.inbound);
    result << " " << sumConnectionIdentifier.ip;
    return result.str();
}

bool operator<(const TopHosts &t1, const TopHosts &t2) {
    return t1.sum > t2.sum;
}

std::vector<TopHosts> Stats::lookupTopHosts(const char *statsbuff, int dst_port, int protocol, bool intern, bool inbound) {
    std::vector<TopHosts> result;
    std::string id = generateId(statsbuff, dst_port, protocol, intern, inbound).str();
    for (auto& topHostsIter : topHosts) {
        if (topHostsIter.first.rfind(id, 0) == 0) {
            result.push_back(*(topHostsIter.second));
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

void Stats::insertStats(char *statsbuff, int dst_port, int protocol, bool intern, bool inbound, long long int sum) {
    std::string statsId = generateId(statsbuff, dst_port, protocol, intern, inbound).str();
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


void Stats::insert(char *statsbuff, SumConnectionIdentifier sumConnectionIdentifier, long long int sum) {
    lastStatsTimestamp = std::string(statsbuff);
    insertStats(statsbuff, 0, 255, sumConnectionIdentifier.intern, sumConnectionIdentifier.inbound, sum);
    insertStats(statsbuff, sumConnectionIdentifier.dst_port, sumConnectionIdentifier.protocol, 
        sumConnectionIdentifier.intern, sumConnectionIdentifier.inbound, sum);
    std::string topHostsId = generateTopHostsId(statsbuff, sumConnectionIdentifier);
    auto topHostsIter = topHosts.find(topHostsId);
    TopHosts* topHost;
    if (topHostsIter != topHosts.end()) {
        topHostsIter->second->sum += sum;
        topHost = topHostsIter->second;
    } else {
        topHost = new TopHosts();
        topHost->ip = sumConnectionIdentifier.ipAddr;
        topHost->sum = sum;
        topHosts[topHostsId] = topHost;
    }
    std::string processAndContent = sumConnectionIdentifier.process + "\\" + sumConnectionIdentifier.content;
    auto processAndContentIter = topHost->processAndContent.find(processAndContent);
    if (processAndContentIter != topHost->processAndContent.end()) {
        processAndContentIter->second->sum += sum;
    } else {
        ProcessAndContent* pac = new ProcessAndContent();
        pac->process = sumConnectionIdentifier.process;
        pac->content = sumConnectionIdentifier.content;
        pac->sum = sum;
        topHost->processAndContent[processAndContent] = pac;
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
