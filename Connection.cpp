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

#include <bits/stdc++.h>
#include <string>
#include <ctime>
#include <list>
#include <regex>
#include <random>

#include "Connection.h"
#include "bumon.h"
#include "tls.h"

static std::default_random_engine generator;
static std::uniform_int_distribution<int> distribution(INT_MIN, INT_MAX);

Connection::Connection(std::shared_ptr<IpAddr> ip, u_char protocol, bool inbound, bool intern):
	ip(ip), protocol(protocol), inbound(inbound), intern(intern) {
    time(&begin);
    dst_port = 0;
    lastAct = 0;
    end = 0;
    do {
	id = distribution(generator);
    } while (allConnections.count(id) > 0);
    ack = false;
    alreadyRunning = false;
    payload = false;
    allConnections[id] = this;
}

Connection::Connection(std::shared_ptr<IpAddr> ip, u_short dst_port, u_char protocol, std::string process, bool inbound, 
	bool intern): Connection(ip, protocol, inbound, intern) {
    this->dst_port = dst_port;
    this->process = process;
}

/*
 * print data in rows of 16 bytes: offset   hex   ascii
 *
 * 00000   47 45 54 20 2f 20 48 54  54 50 2f 31 2e 31 0d 0a   GET / HTTP/1.1..
 */
void
print_hex_ascii_line(const u_char *payload, int len, int offset)
{

	int i;
	int gap;
	const u_char *ch;

	/* offset */
	printf("%05d   ", offset);
	
	/* hex */
	ch = payload;
	for(i = 0; i < len; i++) {
		printf("%02x ", *ch);
		ch++;
		/* print extra space after 8th byte for visual aid */
		if (i == 7)
			printf(" ");
	}
	/* print space to handle line less than 8 bytes */
	if (len < 8)
		printf(" ");
	
	/* fill hex gap with spaces if not full line */
	if (len < 16) {
		gap = 16 - len;
		for (i = 0; i < gap; i++) {
			printf("   ");
		}
	}
	printf("   ");
	
	/* ascii (if printable) */
	ch = payload;
	for(i = 0; i < len; i++) {
		if (isprint(*ch))
			printf("%c", *ch);
		else
			printf(".");
		ch++;
	}

	printf("\n");

return;
}

/*
 * print packet payload data (avoid printing binary data)
 */
void
print_payload(const u_char *payload, int len)
{

	int len_rem = len;
	int line_width = 16;			/* number of bytes per line */
	int line_len;
	int offset = 0;					/* zero-based offset counter */
	const u_char *ch = payload;

	if (len <= 0)
		return;

	/* data fits on one line */
	if (len <= line_width) {
		print_hex_ascii_line(ch, len, offset);
		return;
	}

	/* data spans multiple lines */
	for ( ;; ) {
		/* compute current line length */
		line_len = line_width % len_rem;
		/* print line */
		print_hex_ascii_line(ch, line_len, offset);
		/* compute total remaining */
		len_rem = len_rem - line_len;
		/* shift pointer to remaining bytes to print */
		ch = ch + line_len;
		/* add offset */
		offset = offset + line_width;
		/* check if we have line width chars or less */
		if (len_rem <= line_width) {
			/* print last line and get out */
			print_hex_ascii_line(ch, len_rem, offset);
			break;
		}
	}

return;
}

static std::regex http("\\r\\nHost: (.*)\\r\\n", std::regex_constants::ECMAScript);

void Connection::handleData(int ip_len) {
    time(&lastAct);
    trafficManager->handleTraffic(id, ip_len);
}

void Connection::handleData(int len, const u_char *payload_data, int size_payload) {
    handleData(len);
    if (!payload && !alreadyRunning && size_payload > 0) {
	if (dst_port == 80) {
            std::string contentString((const char*) payload_data, size_payload);
	    std::smatch sm;
	    if(std::regex_search(contentString, sm, http)) {
		content = sm.str(1);
		if (debug)
		    printf("content = %s\n", content.c_str());
	    }
	} else if (dst_port == 443) {
	    char *hostname;
	    int result = parse_tls_header(payload_data, size_payload, &hostname);
	    if (result >= 0) {
	    	logfile->log(9, "HTTPS host = %s", hostname);
	    	content = std::string(hostname, result);
	    	free(hostname);
		if (debug)
		    printf("content = %s\n", content.c_str());
	    } else {
	    	logfile->log(9, "Can not parse HTTPS header!");
	    }
        }
        payload = true;
    }
}

void Connection::stop() {
    time(&end);
}
