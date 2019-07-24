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

ConnectionIdentifier.o:	ConnectionIdentifier.cpp ConnectionIdentifier.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ConnectionIdentifier.cpp

ActiveTcpConnections.o:	ActiveTcpConnections.cpp ActiveTcpConnections.h ConnectionIdentifier.h Connection.h TrafficManager.h bumon.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ActiveTcpConnections.cpp

ActiveUdpConnections.o:	ActiveUdpConnections.cpp ActiveUdpConnections.h ConnectionIdentifier.h Connection.h TrafficManager.h bumon.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ActiveUdpConnections.cpp

ConfigfileParser.o:	ConfigfileParser.cpp ConfigfileParser.h
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c ConfigfileParser.cpp

bumon:		bumon.o Connection.o InternNet.o FindProcess.o ActiveTcpConnections.o ActiveUdpConnections.o ConnectionIdentifier.o Watching.o TrafficManager.o ConfigfileParser.o Logfile.o tls.o
		$(CC) $(LDFLAGS) $(LIBPATH) -o bumon bumon.o Connection.o InternNet.o FindProcess.o ActiveTcpConnections.o ActiveUdpConnections.o ConnectionIdentifier.o Watching.o TrafficManager.o ConfigfileParser.o Logfile.o tls.o $(LIBS)
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

test.o:		test.cpp
		$(CC) $(CPPFLAGS) $(LIB) $(INC) -c test.cpp

test:		InternNetTest.o ConnectionTest.o Connection.o InternNet.o FindProcess.o FindProcessTest.o ActiveTcpConnectionsTest.o ActiveTcpConnections.o test.o ConnectionIdentifier.o TrafficManager.o WatchingMock.o Watching.o WatchingTest.o ActiveUdpConnectionsTest.o ActiveUdpConnections.o ConfigfileParserTest.o ConfigfileParser.o Logfile.o LogfileTest.o tls.o ConnectionIdentifierTest.o
		$(CC) $(LDFLAGS) $(LIBPATH) -o test test.o InternNetTest.o InternNet.o ConnectionTest.o Connection.o FindProcess.o FindProcessTest.o ActiveTcpConnections.o ActiveTcpConnectionsTest.o ConnectionIdentifier.o TrafficManager.o WatchingMock.o Watching.o WatchingTest.o ActiveUdpConnectionsTest.o ActiveUdpConnections.o ConfigfileParserTest.o ConfigfileParser.o Logfile.o LogfileTest.o tls.o ConnectionIdentifierTest.o -lcppunit $(LIBS)
		
clean:
		rm -f *.o *.gcov *.gcda *.gcno bumon test
		
all:		bumon
