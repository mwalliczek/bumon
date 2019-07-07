#CPPFLAGS=-g -Wall -O6 --coverage
#LDFLAGS=-g -lgcov --coverage

CPPFLAGS=-g -Wall -O6 -std=gnu++11
LDFLAGS=-g

CC = g++

bumon.o:	bumon.cpp bumon.h FindProcess.h ActiveTcpConnections.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c bumon.cpp

InternNet.o:	InternNet.cpp InternNet.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c InternNet.cpp

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

ActiveConnections.o:	ActiveConnections.cpp ActiveConnections.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ActiveConnections.cpp

ActiveTcpConnections.o:	ActiveTcpConnections.cpp ActiveTcpConnections.h ActiveConnections.h Connection.h TrafficManager.h bumon.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ActiveTcpConnections.cpp

ActiveUdpConnections.o:	ActiveUdpConnections.cpp ActiveUdpConnections.h ActiveConnections.h Connection.h TrafficManager.h bumon.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ActiveUdpConnections.cpp

ConfigfileParser.o:	ConfigfileParser.cpp ConfigfileParser.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ConfigfileParser.cpp

bumon:		bumon.o Connection.o InternNet.o FindProcess.o ActiveTcpConnections.o ActiveUdpConnections.o ActiveConnections.o Watching.o TrafficManager.o ConfigfileParser.o Logfile.o tls.o
		$(CC) $(LDFLAGS) -lpcap -lstdc++ -pthread -lmysqlclient -o bumon bumon.o Connection.o InternNet.o FindProcess.o ActiveTcpConnections.o ActiveUdpConnections.o ActiveConnections.o Watching.o TrafficManager.o ConfigfileParser.o Logfile.o tls.o
		chmod +x bumon

InternNetTest.o:	InternNetTest.cpp InternNet.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c InternNetTest.cpp

ConnectionTest.o:	ConnectionTest.cpp Connection.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ConnectionTest.cpp

FindProcessTest.o:	FindProcessTest.cpp FindProcess.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c FindProcessTest.cpp

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
		
test.o:		test.cpp
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c test.cpp

test:		InternNetTest.o ConnectionTest.o Connection.o InternNet.o FindProcess.o FindProcessTest.o ActiveTcpConnectionsTest.o ActiveTcpConnections.o test.o ActiveConnections.o TrafficManager.o WatchingMock.o Watching.o WatchingTest.o ActiveUdpConnectionsTest.o ActiveUdpConnections.o ConfigfileParserTest.o ConfigfileParser.o Logfile.o LogfileTest.o tls.o
		$(CC) $(LDFLAGS) -lcppunit -lstdc++ -pthread -lmysqlclient -o test test.o InternNetTest.o InternNet.o ConnectionTest.o Connection.o FindProcess.o FindProcessTest.o ActiveTcpConnections.o ActiveTcpConnectionsTest.o ActiveConnections.o TrafficManager.o WatchingMock.o Watching.o WatchingTest.o ActiveUdpConnectionsTest.o ActiveUdpConnections.o ConfigfileParserTest.o ConfigfileParser.o Logfile.o LogfileTest.o tls.o
		
clean:
		rm *.o bumon test
		
all:		bumon
