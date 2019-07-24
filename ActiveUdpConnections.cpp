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

void ActiveUdpConnections::handlePacket(struct in_addr ip_src, struct in_addr ip_dst, uint16_t ip_len, const u_char *packet) {
	const struct sniff_udp *udp;            /* The TCP header */
	
	/*
	 *  OK, this packet is TCP.
	 */
	
	/* define/compute tcp header offset */
	udp = (struct sniff_udp*)packet;
	
	int sport = ntohs(udp->th_sport);
	int dport = ntohs(udp->th_dport);
	
	if (logfile->checkLevel(10)) {
	    std::string ip_src_str(inet_ntoa(ip_src));
	    std::string ip_dst_str(inet_ntoa(ip_dst));

	    logfile->log(10, "%s:%d -> %s:%d len=%d", ip_src_str.c_str(), sport, ip_dst_str.c_str(), dport, ip_len);
	}
	
	Connection *foundConnection = NULL;
	map_mutex.lock();
	allConnections_mutex.lock();
	std::map<ConnectionIdentifier, int>::iterator iter;
	if ((iter = map.find(ConnectionIdentifier(ip_src, sport, ip_dst, dport))) != map.end()) {
		foundConnection = allConnections[iter->second];
	} else if ((iter = map.find(ConnectionIdentifier(ip_dst, dport, ip_src, sport))) != map.end()) {
	    	foundConnection = allConnections[iter->second];
	} else {
		std::string process;
		if (ip_dst.s_addr == self_ip.s_addr && !(process = findProcesses->findListenUdpProcess(dport)).empty()) {
	            foundConnection = new Connection(ip_src, sport, ip_dst, dport, IPPROTO_UDP, process, &map, "new udp connection");
		} else if (ip_src.s_addr == self_ip.s_addr && !(process = findProcesses->findListenUdpProcess(sport)).empty()) {
	            foundConnection = new Connection(ip_dst, dport, ip_src, sport, IPPROTO_UDP, process, &map, "new udp connection");
		} else {
	            foundConnection = new Connection(ip_src, sport, ip_dst, dport, IPPROTO_UDP, "", &map, "new udp connection");
		}
	}
	map_mutex.unlock();
	allConnections_mutex.unlock();
	if (foundConnection) {
		time(&foundConnection->lastAct);
		trafficManager->handleTraffic(foundConnection->id, ip_len);
        }
}

void ActiveUdpConnections::checkTimeout() {
	map_mutex.lock();
	std::map<ConnectionIdentifier, int>::iterator iter = map.begin();
	time_t current;
	time(&current);
	while (iter != map.end()) {
		Connection* connection = allConnections[iter->second];
		if (difftime(current, connection->lastAct) > 600) {
        	        iter = map.erase(iter);
        	        connection->stop();
        	        continue;
		}
		iter++;
	}
	map_mutex.unlock();
}
