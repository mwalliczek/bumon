ifeq ($(COVERAGE), yes)
  CPPFLAGS=-g -Wall -O6 -std=gnu++11 --coverage
  LDFLAGS=-g --coverage
  LIBS=-lpcap -lstdc++ -pthread -lmysqlclient -lgcov
else
  CPPFLAGS=-g -Wall -O6 -std=gnu++11
  LDFLAGS=-g
  LIBS=-lpcap -lstdc++ -pthread -lmysqlclient
endif

CC = g++

bumon.o:	bumon.cpp bumon.h FindProcess.h ActiveTcpConnections.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c bumon.cpp

Logfile.o:	Logfile.cpp Logfile.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c Logfile.cpp

tls.o:		tls.cpp tls.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c tls.cpp

Connection.o:	Connection.cpp InternNet.h Connection.h tls.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c Connection.cpp
		
Watching.o:	Watching.cpp Watching.h Connection.h bumon.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c Watching.cpp
		
FindProcess.o:	FindProcess.cpp FindProcess.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c FindProcess.cpp

TrafficManager.o:	TrafficManager.cpp TrafficManager.h Watching.h bumon.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c TrafficManager.cpp

ConnectionIdentifier.o:	ConnectionIdentifier.cpp ConnectionIdentifier.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ConnectionIdentifier.cpp

SumConnectionIdentifier.o:	SumConnectionIdentifier.cpp SumConnectionIdentifier.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c SumConnectionIdentifier.cpp

ActiveConnections.o:	ActiveConnections.cpp ActiveConnections.h ConnectionIdentifier.h Connection.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ActiveConnections.cpp

ActiveStateConnections.o:	ActiveStateConnections.cpp ActiveStateConnections.h ActiveConnections.h ConnectionIdentifier.h Connection.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ActiveStateConnections.cpp

ActiveTcpConnections.o:	ActiveTcpConnections.cpp ActiveTcpConnections.h ActiveStateConnections.h ActiveConnections.h ConnectionIdentifier.h Connection.h TrafficManager.h bumon.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ActiveTcpConnections.cpp

ActiveUdpConnections.o:	ActiveUdpConnections.cpp ActiveUdpConnections.h ActiveStateConnections.h ActiveConnections.h ConnectionIdentifier.h Connection.h TrafficManager.h bumon.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ActiveUdpConnections.cpp

ICMP.o:		ICMP.cpp ICMP.h ActiveConnections.h bumon.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ICMP.cpp

ConfigfileParser.o:	ConfigfileParser.cpp ConfigfileParser.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ConfigfileParser.cpp
		
MySql.o:	MySql.cpp MySql.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c MySql.cpp
		
Stats.o:	Stats.cpp Stats.h Statistics.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c Stats.cpp

Ip.o:		Ip.cpp Ip.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c Ip.cpp

Ipv4Addr.o:	Ipv4Addr.cpp Ipv4Addr.h IpAddr.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c Ipv4Addr.cpp

Ipv6Addr.o:	Ipv6Addr.cpp Ipv6Addr.h IpAddr.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c Ipv6Addr.cpp

InternNet.o:	InternNet.cpp InternNet.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c InternNet.cpp

bumon:		bumon.o Connection.o FindProcess.o ActiveConnections.o ActiveStateConnections.o ActiveTcpConnections.o ActiveUdpConnections.o ConnectionIdentifier.o Watching.o TrafficManager.o ConfigfileParser.o Logfile.o tls.o MySql.o Stats.o Ip.o Ipv4Addr.o Ipv6Addr.o InternNet.o SumConnectionIdentifier.o ICMP.o
		$(CC) $(LDFLAGS) $(LIBPATH) -o bumon bumon.o Connection.o FindProcess.o ActiveConnections.o ActiveStateConnections.o ActiveTcpConnections.o ActiveUdpConnections.o ConnectionIdentifier.o Watching.o TrafficManager.o ConfigfileParser.o Logfile.o tls.o MySql.o Stats.o Ip.o Ipv4Addr.o Ipv6Addr.o InternNet.o SumConnectionIdentifier.o ICMP.o $(LIBS)
		chmod +x bumon

InternNetTest.o:	InternNetTest.cpp InternNet.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c InternNetTest.cpp

ConnectionTest.o:	ConnectionTest.cpp Connection.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ConnectionTest.cpp

FindProcessTest.o:	FindProcessTest.cpp FindProcess.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c FindProcessTest.cpp

FindProcessIntegrationTest:	FindProcessIntegrationTest.cpp FindProcess.h FindProcess.o
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c FindProcessIntegrationTest.cpp -o FindProcessIntegrationTest.o
		$(CC) $(LDFLAGS) $(LIBPATH) -o FindProcessIntegrationTest FindProcessIntegrationTest.o FindProcess.o $(LIBS)

ActiveTcpConnectionsTest.o:	ActiveTcpConnectionsTest.cpp ActiveTcpConnections.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ActiveTcpConnectionsTest.cpp

ActiveUdpConnectionsTest.o:	ActiveUdpConnectionsTest.cpp ActiveUdpConnections.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ActiveUdpConnectionsTest.cpp

WatchingMock.o:	WatchingMock.cpp WatchingMock.h Watching.h Connection.h bumon.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c WatchingMock.cpp
		
WatchingTest.o:	WatchingTest.cpp WatchingMock.cpp Watching.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c WatchingTest.cpp
		
LogfileTest.o:	LogfileTest.cpp Logfile.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c LogfileTest.cpp
		
ConfigfileParserTest.o:	ConfigfileParserTest.cpp ConfigfileParser.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ConfigfileParserTest.cpp
		
ConnectionIdentifierTest.o: ConnectionIdentifierTest.cpp ConnectionIdentifier.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ConnectionIdentifierTest.cpp

SumConnectionIdentifierTest.o: SumConnectionIdentifierTest.cpp SumConnectionIdentifier.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c SumConnectionIdentifierTest.cpp

MySqlTest.o:	MySqlTest.cpp MySql.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c MySqlTest.cpp

StatsTest.o:	StatsTest.cpp Stats.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c StatsTest.cpp

IpMock.o:	IpMock.cpp IpMock.h Ip.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c IpMock.cpp

IpTest.o:	IpTest.cpp Ip.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c IpTest.cpp

Ipv4AddrTest.o:	Ipv4AddrTest.cpp Ipv4Addr.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c Ipv4AddrTest.cpp

Ipv6AddrTest.o:	Ipv6AddrTest.cpp Ipv6Addr.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c Ipv6AddrTest.cpp

test.o:		test.cpp
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c test.cpp

test:		InternNet.o InternNetTest.o ConnectionTest.o Connection.o FindProcess.o FindProcessTest.o ActiveConnections.o ActiveStateConnections.o ActiveTcpConnectionsTest.o ActiveTcpConnections.o test.o ConnectionIdentifier.o TrafficManager.o WatchingMock.o Watching.o WatchingTest.o ActiveUdpConnectionsTest.o ActiveUdpConnections.o ConfigfileParserTest.o ConfigfileParser.o Logfile.o LogfileTest.o tls.o ConnectionIdentifierTest.o MySql.o Stats.o MySqlTest.o StatsTest.o IpTest.o Ip.o IpMock.o SumConnectionIdentifierTest.o SumConnectionIdentifier.o Ipv4Addr.o Ipv4AddrTest.o Ipv6Addr.o Ipv6AddrTest.o ICMP.o
		$(CC) $(LDFLAGS) $(LIBPATH) -o test test.o InternNet.o InternNetTest.o ConnectionTest.o Connection.o FindProcess.o FindProcessTest.o ActiveConnections.o ActiveStateConnections.o ActiveTcpConnections.o ActiveTcpConnectionsTest.o ConnectionIdentifier.o TrafficManager.o WatchingMock.o Watching.o WatchingTest.o ActiveUdpConnectionsTest.o ActiveUdpConnections.o ConfigfileParserTest.o ConfigfileParser.o Logfile.o LogfileTest.o tls.o ConnectionIdentifierTest.o MySql.o Stats.o MySqlTest.o StatsTest.o IpTest.o Ip.o IpMock.o SumConnectionIdentifierTest.o SumConnectionIdentifier.o Ipv4Addr.o Ipv4AddrTest.o Ipv6Addr.o Ipv6AddrTest.o ICMP.o -lcppunit $(LIBS)
		
clean:
		rm -f *.o *.gcov *.gcda *.gcno bumon test
		
all:		bumon
