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

#include <arpa/inet.h>

#include "netimond.h"
#include "Connection.h"
#include "FindProcess.h"
#include "ConnectionIdentifier.h"
#include "ActiveUdpConnections.h"

struct sniff_udp {
        u_short th_sport;               /* source port */
        u_short th_dport;               /* destination port */
        u_short th_len;
        u_short th_sum;
};

template<typename IP>
ActiveUdpConnections<IP>::ActiveUdpConnections(std::list<Subnet<IP>> const & interns, 
	std::list<Subnet<IP>> const & selfs): ActiveStateConnections<IP>(interns, selfs) { }

template<typename IP>
void ActiveUdpConnections<IP>::handlePacket(IP const & ip_src, IP const & ip_dst, uint16_t ip_len, 
		const u_char *packet) {
	const struct sniff_udp *udp;            /* The UDP header */

	/*
	 *  OK, this packet is UDP.
	 */

	/* define/compute udp header offset */
	udp = (const struct sniff_udp*)packet;

	int sport = ntohs(udp->th_sport);
	int dport = ntohs(udp->th_dport);

	if (LOG_CHECK_TRACE()) {
	    LOG_TRACE("%s:%d -> %s:%d len=%d", ip_src.toString().c_str(), sport, ip_dst.toString().c_str(), dport, ip_len);
	}

	Connection *foundConnection = NULL;
	this->lock();
	allConnections_mutex.lock();
	std::optional<std::pair<ConnectionIdentifier<IP>, Connection*>> findConnectionResult;
	if ((findConnectionResult = this->findConnection(ip_src, sport, ip_dst, dport)).has_value()) {
	    foundConnection = findConnectionResult.value().second;
	} else if ((findConnectionResult = this->findConnection(ip_dst, dport, ip_src, sport)).has_value()) {
	    foundConnection = findConnectionResult.value().second;
	} else {
		std::string process;
		if (this->isSelf(ip_dst) && !(process = findProcesses->findListenUdpProcess(dport)).empty()) {
		    foundConnection = this->createConnection(ip_src, sport, ip_dst, dport, IPPROTO_UDP, "new udp connection");
		    foundConnection->process = process;
		    this->suspiciousEvents->connectionToProcess(ip_src, dport);
		} else if (this->isSelf(ip_src) && !(process = findProcesses->findListenUdpProcess(sport)).empty()) {
		    foundConnection = this->createConnection(ip_dst, dport, ip_src, sport, IPPROTO_UDP, "new udp connection");
		    foundConnection->process = process;
		    this->suspiciousEvents->connectionToProcess(ip_dst, sport);
		} else if (dport < 1024) {
		    foundConnection = this->createConnection(ip_src, sport, ip_dst, dport, IPPROTO_UDP, "new udp connection");
		    if (foundConnection->inbound) {
		    	this->suspiciousEvents->connectionToEmpty(ip_src, dport);
		    }
		} else if (sport < 1024) {
		    foundConnection = this->createConnection(ip_dst, dport, ip_src, sport, IPPROTO_UDP, "new udp connection");
		    if (foundConnection->inbound) {
		    	this->suspiciousEvents->connectionToEmpty(ip_dst, sport);
		    }
		} else {
		    foundConnection = this->createConnection(ip_src, sport, ip_dst, dport, IPPROTO_UDP, "new udp connection");
		    if (foundConnection->inbound) {
		    	this->suspiciousEvents->connectionToEmpty(ip_src, dport);
		    }
		}
	}
	this->unlock();
	allConnections_mutex.unlock();
	if (foundConnection) {
		time(&foundConnection->lastAct);
		trafficManager->handleTraffic(foundConnection->id, ip_len);
	}
}

template<typename IP>
void ActiveUdpConnections<IP>::checkTimeout() {
	this->lock();
	typename std::map<ConnectionIdentifier<IP>, int>::iterator iter = this->getMap()->begin();
	time_t current;
	time(&current);
	while (iter != this->getMap()->end()) {
		allConnections_mutex.lock();
		Connection* connection = allConnections[iter->second];
		allConnections_mutex.unlock();
		if (difftime(current, connection->lastAct) > 600) {
        	        iter = this->getMap()->erase(iter);
        	        connection->stop();
        	        continue;
		}
		iter++;
	}
	this->unlock();
}

template class ActiveUdpConnections<Ipv4Addr>;
template class ActiveUdpConnections<Ipv6Addr>;
