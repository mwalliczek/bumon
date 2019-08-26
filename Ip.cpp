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

/* IP header */
struct sniff_ip {
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
            ActiveConnections<Ipv4Addr> *other): other(other), activev4TcpConnections(activev4TcpConnections),
            activev4UdpConnections(activev4UdpConnections) { }

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
	const struct sniff_ip *ipv4;              /* The IP header */

	int size_ip;
	
	/* define/compute ip header offset */
	ipv4 = (struct sniff_ip*)&packet[sizeEthernet];
	size_ip = IP_HL(ipv4)*4;
	if (size_ip < 20) {
            logfile->log(9, "   * Invalid IP header length: %u bytes", size_ip);
            return;
        }

	/* determine protocol */	
	switch(ipv4->ip_p) {
		case IPPROTO_TCP:
			self->activev4TcpConnections->handlePacket(ipv4->ip_src, ipv4->ip_dst, ntohs(ipv4->ip_len), &packet[sizeEthernet + size_ip], size_ip);
			break;
		case IPPROTO_UDP:
			self->activev4UdpConnections->handlePacket(ipv4->ip_src, ipv4->ip_dst, ntohs(ipv4->ip_len), &packet[sizeEthernet + size_ip]);
			break;
		default:
			logfile->log(2, " %s > %s Protocol: unknown (%d)", Ipv4Addr(ipv4->ip_src).toString().c_str(), Ipv4Addr(ipv4->ip_dst).toString().c_str(), ipv4->ip_p);
			self->other->handlePacket(ipv4->ip_src, ipv4->ip_dst, ntohs(ipv4->ip_len), ipv4->ip_p);
			return;
	}
	
	return;
}

void Ip::checkTimeout() {
	activev4TcpConnections->checkTimeout();
	activev4UdpConnections->checkTimeout();
}