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

-------------------------------------------------------------------------------

Inspired by sniffex.c
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
#include "ActiveTcpConnections.h"
#include "ActiveUdpConnections.h"
#include "TrafficManager.h"
#include "InternNet.h"
#include "ConfigfileParser.h"

/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14

/* IP header */
struct sniff_ip {
        u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
        u_char  ip_tos;                 /* type of service */
        u_short ip_len;                 /* total length */
        u_short ip_id;                  /* identification */
        u_short ip_off;                 /* fragment offset field */
        #define IP_RF 0x8000            /* reserved fragment flag */
        #define IP_DF 0x4000            /* dont fragment flag */
        #define IP_MF 0x2000            /* more fragments flag */
        #define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
        u_char  ip_ttl;                 /* time to live */
        u_char  ip_p;                   /* protocol */
        u_short ip_sum;                 /* checksum */
        struct  in_addr ip_src,ip_dst;  /* source and dest address */
};
#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)

std::list<InternNet*> interns;

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

FindProcess* findProcesses;
ActiveTcpConnections *activeTcpConnections;
ActiveUdpConnections *activeUdpConnections;
std::map<int, Connection*> allConnections;
std::mutex allConnections_mutex;
Watching* watching;
TrafficManager *trafficManager;
struct in_addr self_ip;
Logfile* logfile;
bool debug;

/*
 * dissect/print packet
 */
void
got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{

	/* declare pointers to packet headers */
	const struct sniff_ip *ip;              /* The IP header */

	int size_ip;
	
	/* define/compute ip header offset */
	ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
	size_ip = IP_HL(ip)*4;
	if (size_ip < 20) {
            logfile->log(9, "   * Invalid IP header length: %u bytes", size_ip);
            return;
        }

	/* determine protocol */	
	switch(ip->ip_p) {
		case IPPROTO_TCP:
			activeTcpConnections->handlePacket(ip->ip_src, ip->ip_dst, ntohs(ip->ip_len), &packet[SIZE_ETHERNET + size_ip], size_ip);
			break;
		case IPPROTO_UDP:
			activeUdpConnections->handlePacket(ip->ip_src, ip->ip_dst, ntohs(ip->ip_len), &packet[SIZE_ETHERNET + size_ip]);
			break;
		case IPPROTO_IP:
			logfile->log(9, "   Protocol: IP");
			break;
		case IPPROTO_ICMP:
		        logfile->log(9, "   Protocol: ICMP");
			break;
		default:
			logfile->log(9, "   Protocol: unknown (%d)", ip->ip_p);
			return;
	}
	
	return;
}

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
    debug = false;

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
    
    if (NULL != configPath) {
        ConfigfileParser* config = new ConfigfileParser(configPath);
    
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
        watching = new Watching((char *) mysql_host.c_str(), (char *) mysql_db.c_str(), (char *) mysql_user.c_str(), (char *) mysql_pass.c_str());
    } else {
        watching = new Watching(true);
    }
    trafficManager = new TrafficManager();
    
    activeTcpConnections = new ActiveTcpConnections();
    activeUdpConnections = new ActiveUdpConnections();
    
    signal(SIGHUP, signalHandler);
    
    /* now we can set our callback function */
    pcap_loop(handle, -1, got_packet, NULL);
    
    pcap_close(handle);
    return(0);
}
