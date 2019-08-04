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

#include "bumon.h"

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
		default:
			logfile->log(2, "   Protocol: unknown (%d)", ip->ip_p);
			Connection *connection = new Connection(ip->ip_src, ip->ip_dst, ip->ip_p);
			connection->handleData(ntohs(ip->ip_len));
			connection->stop();
			return;
	}
	
	return;
}

