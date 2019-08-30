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
 *
 *
 * ----------------------------------------------------------------------------
 *
 * Inspired by sniffex.c
 *
 * Sniffer example of TCP/IP packet capture using libpcap.
 * 
 * Version 0.1.1 (2005-07-05)
 * Copyright (c) 2005 The Tcpdump Group
 *
 * This software is intended to be used as a practical example and 
 * demonstration of the libpcap library; available at:
 * http://www.tcpdump.org/
 *
 ****************************************************************************
 *
 * This software is a modification of Tim Carstens' "sniffer.c"
 * demonstration source code, released as follows:
 * 
 * sniffer.c
 * Copyright (c) 2002 Tim Carstens
 * 2002-01-07
 * Demonstration of using libpcap
 * timcarst -at- yahoo -dot- com
 * 
 * "sniffer.c" is distributed under these terms:
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. The name "Tim Carstens" may not be used to endorse or promote
 *    products derived from this software without prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * <end of "sniffer.c" terms>
 *
 * This software, "sniffex.c", is a derivative work of "sniffer.c" and is
 * covered by the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Because this is a derivative work, you must comply with the "sniffer.c"
 *    terms reproduced above.
 * 2. Redistributions of source code must retain the Tcpdump Group copyright
 *    notice at the top of this source file, this list of conditions and the
 *    following disclaimer.
 * 3. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. The names "tcpdump" or "libpcap" may not be used to endorse or promote
 *    products derived from this software without prior written permission.
 *
 * THERE IS ABSOLUTELY NO WARRANTY FOR THIS PROGRAM.
 * BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY
 * FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN
 * OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES
 * PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED
 * OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS
 * TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE
 * PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
 * REPAIR OR CORRECTION.
 * 
 * IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
 * WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR
 * REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,
 * INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
 * OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
 * TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
 * YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER
 * PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * <end of "sniffex.c" terms>
*/

#include <unistd.h>

#include <stdio.h>
#include <pcap.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>
#include <list>

#include "bumon.h"
#include "FindProcess.h"
#include "TrafficManager.h"
#include "ConfigfileParser.h"
#include "Ip.h"

FindProcess* findProcesses;
Ip* ip;
std::map<int, Connection*> allConnections;
std::mutex allConnections_mutex;
Watching* watching;
TrafficManager *trafficManager;
Logfile* logfile;
bool debug;
std::string ssPath = "ss";
std::string sendmailPath = "/usr/lib/sendmail -t";

void signalHandler( int signum ) {
    if (SIGHUP == signum) {
        logfile->hup();
    }
}

int main(int argc, char *argv[])
{
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;				/* packet capture handle */
    
    int c = 0;
    char* configPath = NULL;
    std::string logfilePath, dev;
    int logLevel = 4;
    std::string mysql_host, mysql_user, mysql_pass, mysql_db;
    std::string warning_mail_sender, warning_mail_recipient;
    debug = false;
    int expireConnections = 7;
    int expireStats = 6;

    while ((c = getopt (argc, argv, "c:di:v:")) != -1)
    switch(c)
    {
        case 'c':
            configPath = optarg;
            break;
        case 'd':
            debug = true;
            break;
        case 'i':
            dev = optarg;
            break;
        case 'v':
            logLevel = atoi(optarg);
            break;
    }
    
    ConfigfileParser* config = NULL;
    if (NULL != configPath) {
        config = new ConfigfileParser(configPath);
    
        std::map<std::string, std::string>::iterator configIter;
        if ((configIter = config->options.find("logfile")) != config->options.end()) {
            logfilePath = configIter->second;
        }
        if ((configIter = config->options.find("loglevel")) != config->options.end()) {
            logLevel = std::stoi(configIter->second);
        }
        if ((configIter = config->options.find("mysql_host")) != config->options.end()) {
            mysql_host = configIter->second;
        }
        if ((configIter = config->options.find("mysql_username")) != config->options.end()) {
            mysql_user = configIter->second;
        }
        if ((configIter = config->options.find("mysql_password")) != config->options.end()) {
            mysql_pass = configIter->second;
        }
        if ((configIter = config->options.find("mysql_db")) != config->options.end()) {
            mysql_db = configIter->second;
        }
        if ((configIter = config->options.find("device")) != config->options.end()) {
            dev = configIter->second;
        }
        if ((configIter = config->options.find("ssPath")) != config->options.end()) {
            ssPath = configIter->second;
        }
        if ((configIter = config->options.find("sendmailPath")) != config->options.end()) {
            sendmailPath = configIter->second;
        }
        if ((configIter = config->options.find("warning_mail_sender")) != config->options.end()) {
            warning_mail_sender = configIter->second;
        }
        if ((configIter = config->options.find("warning_mail_recipient")) != config->options.end()) {
            warning_mail_recipient = configIter->second;
        }
        if ((configIter = config->options.find("expire_connections")) != config->options.end()) {
            expireConnections = std::stoi(configIter->second);
        }
        if ((configIter = config->options.find("expire_stats")) != config->options.end()) {
            expireStats = std::stoi(configIter->second);
        }
    }
    logfile = new Logfile(logfilePath, logLevel);

    if (dev.empty()) {
        char *defaultDev = pcap_lookupdev(errbuf);
        if (NULL == defaultDev) {
                fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
                return(2);
        }
        dev = std::string(defaultDev);
    }
    if (debug) {
        printf("listening to %s\n", dev.c_str());
    }
    /* Open the session in promiscuous mode */
    handle = pcap_open_live(dev.c_str(), BUFSIZ, 0, 100, errbuf);
    if (handle == NULL) {
            fprintf(stderr, "Couldn't open device %s: %s\n", dev.c_str(), errbuf);
            return(2);
    }
    
    findProcesses = new FindProcess();
    findProcesses->init();
    
    if (!mysql_host.empty() && !mysql_user.empty() && !mysql_pass.empty() && !mysql_db.empty()) {
        watching = new Watching((char *) mysql_host.c_str(), (char *) mysql_db.c_str(), (char *) mysql_user.c_str(), 
                (char *) mysql_pass.c_str(), (char *) warning_mail_sender.c_str(), 
                (char *) warning_mail_recipient.c_str(), expireConnections, expireStats);
    } else {
        watching = new Watching(true);
    }
    trafficManager = new TrafficManager();
    
    ip = new Ip(config);
    
    signal(SIGHUP, signalHandler);
    
    /* now we can set our callback function */
    pcap_loop(handle, -1, Ip::got_packet, (u_char*) ip);
    
    pcap_close(handle);
    return(0);
}
