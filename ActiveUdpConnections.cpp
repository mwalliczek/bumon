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

#include "bumon.h"
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
ActiveUdpConnections<IP>::ActiveUdpConnections(std::list<InternNet<IP>> interns, std::list<IP> selfs) :
ActiveStateConnections<IP>(interns, selfs) { }

template<typename IP>
void ActiveUdpConnections<IP>::handlePacket(IP ip_src, IP ip_dst, uint16_t ip_len, const u_char *packet) {
	const struct sniff_udp *udp;            /* The TCP header */
	
	/*
	 *  OK, this packet is TCP.
	 */
	
	/* define/compute tcp header offset */
	udp = (struct sniff_udp*)packet;
	
	int sport = ntohs(udp->th_sport);
	int dport = ntohs(udp->th_dport);
	
	if (logfile->checkLevel(10)) {
	    logfile->log(10, "%s:%d -> %s:%d len=%d", ip_src.toString().c_str(), sport, ip_dst.toString().c_str(), dport, ip_len);
	}
	
	Connection *foundConnection = NULL;
	this->map_mutex.lock();
	allConnections_mutex.lock();
	typename std::map<ConnectionIdentifier<IP>, int>::iterator iter;
	if ((iter = this->map.find(ConnectionIdentifier<IP>(ip_src, sport, ip_dst, dport))) != this->map.end()) {
		foundConnection = allConnections[iter->second];
	} else if ((iter = this->map.find(ConnectionIdentifier<IP>(ip_dst, dport, ip_src, sport))) != this->map.end()) {
	    	foundConnection = allConnections[iter->second];
	} else {
		std::string process;
		if (this->isSelf(ip_dst) && !(process = findProcesses->findListenUdpProcess(dport)).empty()) {
	            foundConnection = this->createConnection(ip_src, sport, ip_dst, dport, IPPROTO_UDP, "new udp connection");
	            foundConnection->process = process;
		} else if (this->isSelf(ip_src) && !(process = findProcesses->findListenUdpProcess(sport)).empty()) {
	            foundConnection = this->createConnection(ip_dst, dport, ip_src, sport, IPPROTO_UDP, "new udp connection");
	            foundConnection->process = process;
		} else if (this->isSelf(ip_dst) && dport < 1024) {
	            foundConnection = this->createConnection(ip_src, sport, ip_dst, dport, IPPROTO_UDP, "new udp connection");
		} else if (this->isSelf(ip_src) && sport < 1024) {
	            foundConnection = this->createConnection(ip_dst, dport, ip_src, sport, IPPROTO_UDP, "new udp connection");
		} else {
	            foundConnection = this->createConnection(ip_src, sport, ip_dst, dport, IPPROTO_UDP, "new udp connection");
		}
	}
	this->map_mutex.unlock();
	allConnections_mutex.unlock();
	if (foundConnection) {
		time(&foundConnection->lastAct);
		trafficManager->handleTraffic(foundConnection->id, ip_len);
        }
}

template<typename IP>
void ActiveUdpConnections<IP>::checkTimeout() {
	this->map_mutex.lock();
	typename std::map<ConnectionIdentifier<IP>, int>::iterator iter = this->map.begin();
	time_t current;
	time(&current);
	while (iter != this->map.end()) {
		allConnections_mutex.lock();
		Connection* connection = allConnections[iter->second];
		allConnections_mutex.unlock();
		if (difftime(current, connection->lastAct) > 600) {
        	        iter = this->map.erase(iter);
        	        connection->stop();
        	        continue;
		}
		iter++;
	}
	this->map_mutex.unlock();
}

template class ActiveUdpConnections<Ipv4Addr>;
template class ActiveUdpConnections<Ipv6Addr>;
