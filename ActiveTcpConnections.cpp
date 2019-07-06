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

#include <arpa/inet.h>

#include "bumon.h"
#include "Connection.h"
#include "FindProcess.h"
#include "ActiveConnections.h"
#include "ActiveTcpConnections.h"

/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
        u_short th_sport;               /* source port */
        u_short th_dport;               /* destination port */
        tcp_seq th_seq;                 /* sequence number */
        tcp_seq th_ack;                 /* acknowledgement number */
        u_char  th_offx2;               /* data offset, rsvd */
#define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
        u_char  th_flags;
        #define TH_FIN  0x01
        #define TH_SYN  0x02
        #define TH_RST  0x04
        #define TH_PUSH 0x08
        #define TH_ACK  0x10
        #define TH_URG  0x20
        #define TH_ECE  0x40
        #define TH_CWR  0x80
        #define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
        u_short th_win;                 /* window */
        u_short th_sum;                 /* checksum */
        u_short th_urp;                 /* urgent pointer */
};

void ActiveTcpConnections::handlePacket(struct in_addr ip_src, struct in_addr ip_dst, uint16_t ip_len, const u_char *packet, int size_ip) {
	const struct sniff_tcp *tcp;            /* The TCP header */
	int size_tcp;
	
	/*
	 *  OK, this packet is TCP.
	 */
	
	/* define/compute tcp header offset */
	tcp = (struct sniff_tcp*)packet;
	size_tcp = TH_OFF(tcp)*4;
	if (size_tcp < 20) {
		fprintf(stderr, "   * Invalid TCP header length: %u bytes\n", size_tcp);
		return;
	}
	
	std::string ip_src_str(inet_ntoa(ip_src));
	std::string ip_dst_str(inet_ntoa(ip_dst));
	int sport = ntohs(tcp->th_sport);
	int dport = ntohs(tcp->th_dport);
	
	logfile->log(10, "%s:%d -> %s:%d flags=%d len=%d", ip_src_str.c_str(), sport, ip_dst_str.c_str(), dport, tcp->th_flags, ip_len);
	
	Connection *foundConnection = NULL;
	map_mutex.lock();
	allConnections_mutex.lock();
	std::map<std::string, int>::iterator iter;
	if ((tcp->th_flags & 18) == 2) {
	    std::string process;
	    if (ip_dst.s_addr == self_ip.s_addr) {
	        process = findProcesses->findListenTcpProcess(dport);
            } else if (ip_src.s_addr == self_ip.s_addr) {
                process = findProcesses->findActiveTcpProcess(sport, ip_dst_str, dport);
            }
	    foundConnection = new Connection(ip_src, sport, ip_dst, dport, IPPROTO_TCP, process);
	    std::string identifier = generateIdentifier(ip_src_str, sport, ip_dst_str, dport);
	    logfile->log(6, "new tcp connection: %s = %d (%s)", identifier.c_str(), foundConnection->id, process.c_str());
	    map[identifier] = foundConnection->id;
	    allConnections[foundConnection->id] = foundConnection;
        } else if ((tcp->th_flags & 18) == 18 && ((iter = map.find(generateIdentifier(ip_dst_str, dport, ip_src_str, sport))) != map.end())) {
	    foundConnection = allConnections[iter->second];
	    logfile->log(6, "ack connection: %s = %d (%s)", iter->first.c_str(), foundConnection->id, foundConnection->process.c_str());
	    foundConnection->ack = true;
        } else if ((tcp->th_flags & 1) == 1) {
	    if ((iter = map.find(generateIdentifier(ip_src_str, sport, ip_dst_str, dport))) != map.end()) {
	    	foundConnection = allConnections[iter->second];
	    	if (0 == foundConnection->end) {
		    logfile->log(6, "stop connection: %s = %d (%s)", iter->first.c_str(), foundConnection->id, foundConnection->process.c_str());
		}
	    } else if ((iter = map.find(generateIdentifier(ip_dst_str, dport, ip_src_str, sport))) != map.end()) {
	    	foundConnection = allConnections[iter->second];
	    	if (0 == foundConnection->end) {
	    	    logfile->log(6, "stop connection: %s = %d (%s)", iter->first.c_str(), foundConnection->id, foundConnection->process.c_str());
	    	}
	    }
	    if (foundConnection) {
	    	foundConnection->stop();
	    }
        } else {
	    if ((iter = map.find(generateIdentifier(ip_src_str, sport, ip_dst_str, dport))) != map.end()) {
	    	foundConnection = allConnections[iter->second];
	    } else if ((iter = map.find(generateIdentifier(ip_dst_str, dport, ip_src_str, sport))) != map.end()) {
	        foundConnection = allConnections[iter->second];
	        if (foundConnection->alreadyRunning && !foundConnection->ack) {
		  foundConnection->ack = true;
	        }
	    } else {
        	std::string process;
                if (ip_dst.s_addr == self_ip.s_addr && !(process = findProcesses->findListenTcpProcess(dport)).empty()) {
                    foundConnection = new Connection(ip_src, sport, ip_dst, dport, IPPROTO_TCP, process);
                    std::string identifier = generateIdentifier(ip_src_str, sport, ip_dst_str, dport);
                    logfile->log(6, "already running connection: %s = %d (%s)", identifier.c_str(), foundConnection->id, process.c_str());
                    map[identifier] = foundConnection->id;
                } else if (ip_src.s_addr == self_ip.s_addr && !(process = findProcesses->findListenTcpProcess(sport)).empty()) {
                    foundConnection = new Connection(ip_dst, dport, ip_src, sport, IPPROTO_TCP, process);
                    std::string identifier = generateIdentifier(ip_dst_str, dport, ip_src_str, sport);
		    logfile->log(6, "already running connection: %s = %d (%s)", identifier.c_str(), foundConnection->id, process.c_str());
                    map[identifier] = foundConnection->id;
                } else {
                    foundConnection = new Connection(ip_src, sport, ip_dst, dport, IPPROTO_TCP, "");
                    std::string identifier = generateIdentifier(ip_src_str, sport, ip_dst_str, dport);
		    logfile->log(6, "already running connection: %s = %d (%s)", identifier.c_str(), foundConnection->id, "");
                    map[identifier] = foundConnection->id;
                }
                foundConnection->alreadyRunning = true;
                allConnections[foundConnection->id] = foundConnection;
	    }
        }
        map_mutex.unlock();
        allConnections_mutex.unlock();
	if (foundConnection) {
		foundConnection->handleData(ip_len, &packet[size_tcp], ip_len - (size_ip + size_tcp));
        }
}

void ActiveTcpConnections::checkTimeout() {
	map_mutex.lock();
	std::map<std::string, int>::iterator iter = map.begin();
	time_t current;
	time(&current);
	while (iter != map.end()) {
		Connection* connection = allConnections[iter->second];
		if (connection->ack == false && connection->end == 0 && difftime(current, connection->begin) > 30) {
			std::string ip_src_str(inet_ntoa(connection->src_ip));
			std::string ip_dst_str(inet_ntoa(connection->dst_ip));
			
			logfile->log(5, "timeout aborted connection: %s:%d > %s:%d (%s)", ip_src_str.c_str(), connection->src_port, ip_dst_str.c_str(), connection->dst_port, connection->process.c_str());
        	        if (connection->dst_ip.s_addr == self_ip.s_addr && connection->alreadyRunning == false) {
			    logfile->log(2, "aborted connection from %s (%s:%d > %s:%d)", ip_src_str.c_str(), ip_src_str.c_str(), connection->src_port, ip_dst_str.c_str(), connection->dst_port);
		        }
        	        iter = map.erase(iter);
        	        connection->stop();
        	        continue;
		}
		if (connection->end != 0 && difftime(current, connection->end) > 30) {
        	        iter = map.erase(iter);
        	        continue;
		}
		if (difftime(current, connection->lastAct) > 3600) {
        	        iter = map.erase(iter);
        	        connection->stop();
        	        continue;
		}
		iter++;
	}
	map_mutex.unlock();
	logfile->flush();
}
