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

#include <arpa/inet.h>

#include "netimond.h"
#include "Connection.h"
#include "FindProcess.h"
#include "ConnectionIdentifier.h"
#include "ActiveTcpConnections.h"

/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
        u_short th_sport;               /* source port */
        u_short th_dport;               /* destination port */
        tcp_seq th_seq;                 /* sequence number */
        tcp_seq th_ack;                 /* acknowledgement number */
        u_char  th_offx2;               /* data offset, rsvd */
        u_char  th_flags;
        u_short th_win;                 /* window */
        u_short th_sum;                 /* checksum */
        u_short th_urp;                 /* urgent pointer */
};

int calcOffset(u_char th_offx2) {
	return ((th_offx2 & 0xf0) >> 4);
}

template<typename IP>
ActiveTcpConnections<IP>::ActiveTcpConnections(std::list<InternNet<IP>> const & interns, std::list<IP> const & selfs) :
ActiveStateConnections<IP>(interns, selfs) { }

template<typename IP>
void debugLog(ConnectionIdentifier<IP> identifier, Connection* connection, const char* prefix) {
    if (LOG_CHECK_DEBUG()) {
	LOG_DEBUG("%s: %s = %d (%s)", prefix, identifier.toString().c_str(), connection->id, 
		connection->process.c_str());
    }
}

template<typename IP>
void ActiveTcpConnections<IP>::handlePacket(IP const & ip_src, IP const & ip_dst, uint16_t ip_len, 
		const u_char *packet, int size_ip) {
	const struct sniff_tcp *tcp;            /* The TCP header */
	int size_tcp;
	
	/*
	 *  OK, this packet is TCP.
	 */
	
	/* define/compute tcp header offset */
	tcp = (const struct sniff_tcp*)packet;
	size_tcp = calcOffset(tcp->th_offx2)*4;
	if (size_tcp < 20) {
		LOG_DEBUG("   * Invalid TCP header length: %u bytes", size_tcp);
		return;
	}
	
	int sport = ntohs(tcp->th_sport);
	int dport = ntohs(tcp->th_dport);
	
	if (LOG_CHECK_TRACE()) {
	    LOG_TRACE("%s:%d -> %s:%d flags=%d len=%d", ip_src.toString().c_str(), sport, 
		    ip_dst.toString().c_str(), dport, tcp->th_flags, ip_len);
	}
	
	std::optional<std::pair<ConnectionIdentifier<IP>, Connection*>> findConnectionResult;
	Connection *foundConnection = NULL;
	this->lock();
	allConnections_mutex.lock();
	if ((tcp->th_flags & 18) == 2) {
            foundConnection = this->createConnection(ip_src, sport, ip_dst, dport, IPPROTO_TCP, "new tcp connection");
            if (foundConnection->inbound) {
	        foundConnection->process = findProcesses->findListenTcpProcess(dport);
	    } else {
	    	foundConnection->process = findProcesses->findActiveTcpProcess(sport, ip_dst.toString(), dport);
	    }
        } else if (((tcp->th_flags & 18) == 18) && (findConnectionResult = this->findConnection(ip_dst, dport, ip_src, 
        	sport)).has_value()) {
	    foundConnection = findConnectionResult.value().second;
	    debugLog(findConnectionResult.value().first, foundConnection, "ack connection");
	    foundConnection->ack = true;
        } else if ((tcp->th_flags & 1) == 1) {
	    if ((findConnectionResult = this->findConnection(ip_src, sport, ip_dst, dport)).has_value()) {
		foundConnection = findConnectionResult.value().second;
	    	if (0 == foundConnection->end) {
		    debugLog(findConnectionResult.value().first, foundConnection, "stop connection");
		}
	    } else if ((findConnectionResult = this->findConnection(ip_dst, dport, ip_src, sport)).has_value()) {
		foundConnection = findConnectionResult.value().second;
	    	if (0 == foundConnection->end) {
		    debugLog(findConnectionResult.value().first, foundConnection, "stop connection");
	    	}
	    }
	    if (foundConnection) {
	    	foundConnection->stop();
	    }
        } else {
	    if ((findConnectionResult = this->findConnection(ip_src, sport, ip_dst, dport)).has_value()) {
		foundConnection = findConnectionResult.value().second;
	    } else if ((findConnectionResult = this->findConnection(ip_dst, dport, ip_src, sport)).has_value()) {
		foundConnection = findConnectionResult.value().second;
	        if (foundConnection->alreadyRunning && !foundConnection->ack) {
		  foundConnection->ack = true;
	        }
	    } else {
        	std::string process;
                if (this->isSelf(ip_dst) && !(process = findProcesses->findListenTcpProcess(dport)).empty()) {
                    foundConnection = this->createConnection(ip_src, sport, ip_dst, dport, IPPROTO_TCP, "already running connection");
                    foundConnection->process = process;
                } else if (this->isSelf(ip_src) && !(process = findProcesses->findListenTcpProcess(sport)).empty()) {
                    foundConnection = this->createConnection(ip_dst, dport, ip_src, sport, IPPROTO_TCP, "already running connection");
                    foundConnection->process = process;
                } else if (dport < 1024) {
                    foundConnection = this->createConnection(ip_src, sport, ip_dst, dport, IPPROTO_TCP, "already running connection");
                } else if (sport < 1024) {
                    foundConnection = this->createConnection(ip_dst, dport, ip_src, sport, IPPROTO_TCP, "already running connection");
                } else {
                    foundConnection = this->createConnection(ip_src, sport, ip_dst, dport, IPPROTO_TCP, "already running connection");
                }
                foundConnection->alreadyRunning = true;
	    }
        }
        allConnections_mutex.unlock();
        this->unlock();
	if (foundConnection) {
	    foundConnection->handleData(ip_len, &packet[size_tcp], ip_len - (size_ip + size_tcp));
        }
}

template<typename IP>
void ActiveTcpConnections<IP>::checkTimeout() {
	this->lock();
	typename std::map<ConnectionIdentifier<IP>, int>::iterator iter = this->getMap()->begin();
	time_t current;
	time(&current);
	while (iter != this->getMap()->end()) {
		allConnections_mutex.lock();
		Connection* connection = allConnections[iter->second];
		allConnections_mutex.unlock();
		if (connection->ack == false && connection->end == 0 && difftime(current, connection->begin) > 30) {
			if (LOG_CHECK_DEBUG()) {
			    LOG_DEBUG("timeout aborted connection: %s (%s)", iter->first.toString().c_str(), 
			    		connection->process.c_str());
			}
				
        	        if (connection->inbound && connection->alreadyRunning == false && LOG_CHECK_WARN()) {
			    LOG_WARN("aborted connection from %s (%s)", connection->ip->toString().c_str(), 
			    iter->first.toString().c_str());
		        }
        	        iter = this->getMap()->erase(iter);
        	        connection->stop();
        	        continue;
		}
		if (connection->end != 0 && difftime(current, connection->end) > 30) {
        	        iter = this->getMap()->erase(iter);
        	        continue;
		}
		if (difftime(current, connection->lastAct) > 3600) {
        	        iter = this->getMap()->erase(iter);
        	        connection->stop();
        	        continue;
		}
		iter++;
	}
	this->unlock();
	logfile->flush();
}

template class ActiveTcpConnections<Ipv4Addr>;
template class ActiveTcpConnections<Ipv6Addr>;
