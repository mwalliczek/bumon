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

#include "bumon.h"

#include "Ip.h"

/* ethernet headers are always exactly 14 bytes [1] */
const int sizeEthernet = 14;

const int etherAddrLen = 6;

/* Ethernet header */
struct sniff_ethernet {
	u_char ether_dhost[etherAddrLen]; /* Destination host address */
	u_char ether_shost[etherAddrLen]; /* Source host address */
	u_short ether_type; /* IP? ARP? RARP? etc */
};

/* IP header */
struct sniff_ipv4 {
        u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
        u_char  ip_tos;                 /* type of service */
        u_short ip_len;                 /* total length */
        u_short ip_id;                  /* identification */
        u_short ip_off;                 /* fragment offset field */
        u_char  ip_ttl;                 /* time to live */
        u_char  ip_p;                   /* protocol */
        u_short ip_sum;                 /* checksum */
        struct  in_addr ip_src,ip_dst;  /* source and dest address */
};
#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)

/* IP header */
struct sniff_ipv6 {
        u_char  ip_vtc1;                /* version << 4 | traffic class >> 2 */
        u_char  ip_tc2fl1;              /* traffic class << 4 | flow control 1 */
        u_short ip_fl2;                 /* flow control 2 */
        u_short ip_payload_length;      /* payload length */
        u_char  ip_p;                   /* protocol */
        u_char  ip_ttl;                 /* time to live */
        struct  in6_addr ip_src,ip_dst; /* source and dest address */
};

const int size_ip6 = sizeof(struct sniff_ipv6);

Ip::Ip(ConfigfileParser* config) {
	std::list<InternNet<Ipv4Addr>> internsv4;
	std::list<Ipv4Addr> selfsv4;
	std::list<InternNet<Ipv6Addr>> internsv6;
	std::list<Ipv6Addr> selfsv6;
	if (NULL != config) {
		for (auto intern : config->interns) {
			InternNet<Ipv4Addr> internNetv4 = InternNet<Ipv4Addr>(intern.ip, intern.mask);
			if (internNetv4.valid) {
				internsv4.push_back(internNetv4);
			} else {
				InternNet<Ipv6Addr> internNetv6 = InternNet<Ipv6Addr>(intern.ip, intern.mask);
				if (!internNetv6.valid) {
					LOG_ERROR("Could not parse %s / %s", intern.ip.c_str(), intern.mask.c_str());
				} else {
					internsv6.push_back(internNetv6);
				}
			}
		}
		for (auto self : config->selfs) {
			Ipv4Addr selfv4 = Ipv4Addr(self);
			if (!selfv4.empty()) {
				selfsv4.push_back(selfv4);
			} else {
				Ipv6Addr selfv6 = Ipv6Addr(self);
				if (selfv6.empty()) {
					LOG_ERROR("Could not parse %s", self.c_str());
				} else {
					selfsv6.push_back(selfv6);
				}
			}
		}
	}
	activev4TcpConnections = new ActiveTcpConnections<Ipv4Addr>(internsv4, selfsv4);
	activev4UdpConnections = new ActiveUdpConnections<Ipv4Addr>(internsv4, selfsv4);
	icmpv4 = new ICMP(internsv4, selfsv4);
	otherv4 = new ActiveConnections<Ipv4Addr>(internsv4, selfsv4);
	activev6TcpConnections = new ActiveTcpConnections<Ipv6Addr>(internsv6, selfsv6);
	activev6UdpConnections = new ActiveUdpConnections<Ipv6Addr>(internsv6, selfsv6);
	icmpv6 = new ICMPv6(internsv6, selfsv6);
	otherv6 = new ActiveConnections<Ipv6Addr>(internsv6, selfsv6);
}

Ip::Ip(ActiveTcpConnections<Ipv4Addr> *activev4TcpConnections, ActiveTcpConnections<Ipv6Addr> *activev6TcpConnections, 
	    ActiveUdpConnections<Ipv4Addr> *activev4UdpConnections, 
	    ActiveUdpConnections<Ipv6Addr> *activev6UdpConnections): 
            otherv4(NULL), otherv6(NULL), icmpv4(NULL), icmpv6(NULL), activev4TcpConnections(activev4TcpConnections),
            activev6TcpConnections(activev6TcpConnections), activev4UdpConnections(activev4UdpConnections), 
            activev6UdpConnections(activev6UdpConnections) { }

Ip::~Ip() {
	delete activev4TcpConnections;
	delete activev4UdpConnections;
	delete icmpv4;
	delete otherv4;
	delete activev6TcpConnections;
	delete activev6UdpConnections;
	delete icmpv6;
	delete otherv6;
}

void Ip::handleV4(const u_char *packet) {
	const struct sniff_ipv4 *ipv4 = (const struct sniff_ipv4*)packet;
	
	int size_ip = IP_HL(ipv4)*4;
	if (size_ip < 20) {
            LOG_WARN("   * Invalid IP header length: %u bytes", size_ip);
            return;
        }

	/* determine protocol */	
	switch(ipv4->ip_p) {
		case IPPROTO_TCP:
			activev4TcpConnections->handlePacket(Ipv4Addr(ipv4->ip_src), Ipv4Addr(ipv4->ip_dst), 
				ntohs(ipv4->ip_len), &packet[size_ip], size_ip);
			break;
		case IPPROTO_UDP:
			activev4UdpConnections->handlePacket(Ipv4Addr(ipv4->ip_src), Ipv4Addr(ipv4->ip_dst), 
				ntohs(ipv4->ip_len), &packet[size_ip]);
			break;
		case IPPROTO_ICMP:
			icmpv4->handlePacket(Ipv4Addr(ipv4->ip_src), Ipv4Addr(ipv4->ip_dst), ntohs(ipv4->ip_len), 
				&packet[size_ip]);
			break;
		default:
			Ipv4Addr src = Ipv4Addr(ipv4->ip_src);
			Ipv4Addr dst = Ipv4Addr(ipv4->ip_dst);
			LOG_DEBUG(" %s > %s Protocol: unknown (%d)", src.toString().c_str(),
				dst.toString().c_str(), ipv4->ip_p);
			otherv4->handlePacket(src, dst, ntohs(ipv4->ip_len), ipv4->ip_p);
			return;
	}
}

void Ip::handleV6(const u_char *packet) {
	const struct sniff_ipv6 *ipv6 = (const struct sniff_ipv6*)packet;
	
	int ip_len = ntohs(ipv6->ip_payload_length) + size_ip6;
	
	/* determine protocol */	
	switch(ipv6->ip_p) {
		case IPPROTO_TCP:
			activev6TcpConnections->handlePacket(Ipv6Addr(ipv6->ip_src), Ipv6Addr(ipv6->ip_dst), 
				ip_len, &packet[size_ip6], size_ip6);
			break;
		case IPPROTO_UDP:
			activev6UdpConnections->handlePacket(Ipv6Addr(ipv6->ip_src), Ipv6Addr(ipv6->ip_dst), 
				ip_len, &packet[size_ip6]);
			break;
		case IPPROTO_ICMPV6:
			icmpv6->handlePacket(Ipv6Addr(ipv6->ip_src), Ipv6Addr(ipv6->ip_dst), ip_len, 
				&packet[size_ip6]);
			break;
		default:
			Ipv6Addr src = Ipv6Addr(ipv6->ip_src);
			Ipv6Addr dst = Ipv6Addr(ipv6->ip_dst);
			LOG_DEBUG(" %s > %s Protocol: unknown (%d)", src.toString().c_str(),
				dst.toString().c_str(), ipv6->ip_p);
			otherv6->handlePacket(src, dst, ip_len, ipv6->ip_p);
			return;
	}
	
}

/*
 * dissect/print packet
 */
void
Ip::got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
	Ip* self = (Ip*) args;
	
	const struct sniff_ethernet *etherPacket = (const struct sniff_ethernet*)packet; 
	
	int ethernetType = ntohs(etherPacket->ether_type);
	int version = packet[sizeEthernet] >> 4;

	if (ethernetType == 0x0800 && version == 4) {
		self->handleV4(&packet[sizeEthernet]);
	} else if (ethernetType == 0x86DD && version == 6) {
		self->handleV6(&packet[sizeEthernet]);
	} else {
		LOG_DEBUG("  unknown Protocol %d", ethernetType);
	}
}

void Ip::checkTimeout() {
	activev4TcpConnections->checkTimeout();
	activev4UdpConnections->checkTimeout();
	activev6TcpConnections->checkTimeout();
	activev6UdpConnections->checkTimeout();
}
