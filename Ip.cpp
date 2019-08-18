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

#include "Ip.h"

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

Ip::Ip(ConfigfileParser* config) {
	std::list<InternNet<Ipv4Addr>> interns;
	std::list<Ipv4Addr> selfs;
	if (NULL != config) {
		for (auto intern : config->interns) {
			interns.push_back(InternNet<Ipv4Addr>(intern.ip, intern.mask));
		}
		for (auto self : config->selfs) {
			selfs.push_back(Ipv4Addr(self));
		}
	}
	activev4TcpConnections = new ActiveTcpConnections<Ipv4Addr>(interns, selfs);
	activev4UdpConnections = new ActiveUdpConnections<Ipv4Addr>(interns, selfs);
	other = new ActiveConnections<Ipv4Addr>(interns, selfs);
}

Ip::Ip(ActiveTcpConnections<Ipv4Addr> *activev4TcpConnections, ActiveUdpConnections<Ipv4Addr> *activev4UdpConnections,
            ActiveConnections<Ipv4Addr> *other): activev4TcpConnections(activev4TcpConnections),
            activev4UdpConnections(activev4UdpConnections), other(other) { }

Ip::~Ip() {
	delete activev4TcpConnections;
	delete activev4UdpConnections;
	delete other;
}

/*
 * dissect/print packet
 */
void
Ip::got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
	Ip* self = (Ip*) args;

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
			self->activev4TcpConnections->handlePacket(ip->ip_src, ip->ip_dst, ntohs(ip->ip_len), &packet[SIZE_ETHERNET + size_ip], size_ip);
			break;
		case IPPROTO_UDP:
			self->activev4UdpConnections->handlePacket(ip->ip_src, ip->ip_dst, ntohs(ip->ip_len), &packet[SIZE_ETHERNET + size_ip]);
			break;
		default:
			logfile->log(2, "   Protocol: unknown (%d)", ip->ip_p);
			self->other->handlePacket(ip->ip_src, ip->ip_dst, ntohs(ip->ip_len), ip->ip_p);
			return;
	}
	
	return;
}

void Ip::checkTimeout() {
	activev4TcpConnections->checkTimeout();
	activev4UdpConnections->checkTimeout();
}
