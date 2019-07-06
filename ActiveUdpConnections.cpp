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
*/

#include <arpa/inet.h>

#include "bumon.h"
#include "Connection.h"
#include "FindProcess.h"
#include "ActiveConnections.h"
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
	
	std::string ip_src_str(inet_ntoa(ip_src));
	std::string ip_dst_str(inet_ntoa(ip_dst));
	int sport = ntohs(udp->th_sport);
	int dport = ntohs(udp->th_dport);
	
	logfile->log(10, "%s:%d -> %s:%d len=%d", ip_src_str.c_str(), sport, ip_dst_str.c_str(), dport, ip_len);
	
	Connection *foundConnection = NULL;
	map_mutex.lock();
	allConnections_mutex.lock();
	std::map<std::string, int>::iterator iter;
	if ((iter = map.find(generateIdentifier(ip_src_str, sport, ip_dst_str, dport))) != map.end()) {
		foundConnection = allConnections[iter->second];
	} else if ((iter = map.find(generateIdentifier(ip_dst_str, dport, ip_src_str, sport))) != map.end()) {
	    	foundConnection = allConnections[iter->second];
	} else {
		std::string process;
		if (ip_dst.s_addr == self_ip.s_addr && !(process = findProcesses->findListenUdpProcess(dport)).empty()) {
	            foundConnection = new Connection(ip_src, sport, ip_dst, dport, IPPROTO_UDP, process);
		    logfile->log(6, "new udp connection: %s:%d > %s:%d = %d (%s)", ip_src_str.c_str(), sport, ip_dst_str.c_str(), dport, foundConnection->id, process.c_str());
	            std::string identifier = generateIdentifier(ip_src_str, sport, ip_dst_str, dport);
	            map[identifier] = foundConnection->id;
		} else if (ip_src.s_addr == self_ip.s_addr && !(process = findProcesses->findListenUdpProcess(sport)).empty()) {
	            foundConnection = new Connection(ip_dst, dport, ip_src, sport, IPPROTO_UDP, process);
		    logfile->log(6, "new udp connection: %s:%d > %s:%d = %d (%s)", ip_dst_str.c_str(), dport, ip_src_str.c_str(), sport, foundConnection->id, process.c_str());
	            std::string identifier = generateIdentifier(ip_dst_str, dport, ip_src_str, sport);
	            map[identifier] = foundConnection->id;
		} else {
	            foundConnection = new Connection(ip_src, sport, ip_dst, dport, IPPROTO_UDP, "");
		    logfile->log(6, "new udp connection: %s:%d > %s:%d = %d (%s)", ip_src_str.c_str(), sport, ip_dst_str.c_str(), dport, foundConnection->id, "");
	            std::string identifier = generateIdentifier(ip_src_str, sport, ip_dst_str, dport);
	            map[identifier] = foundConnection->id;
		}
                allConnections[foundConnection->id] = foundConnection;
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
	std::map<std::string, int>::iterator iter = map.begin();
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
