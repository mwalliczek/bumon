cmake_minimum_required(VERSION 3.1)

project(netimond)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -pedantic -O6")

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

link_libraries(pcap mysqlclient)

IF(DEFINED ENV{COVERAGE})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
	link_libraries(gcov)
	message("Use code coverage")
ENDIF()

include_directories(${PROJECT_SOURCE_DIR})
add_executable(netimond 
	netimond.cpp 
	Logfile.cpp 
	tls.cpp 
	Connection.cpp 
	Watching.cpp 
	FindProcess.cpp 
	TrafficManager.cpp 
	ConnectionIdentifier.cpp 
	SumConnectionIdentifier.cpp 
	ActiveConnections.cpp 
	ActiveStateConnections.cpp 
	ActiveTcpConnections.cpp 
	ActiveUdpConnections.cpp 
	ICMP.cpp 
	ICMPv6.cpp 
	ConfigfileParser.cpp 
	MySql.cpp 
	Stats.cpp 
	Ip.cpp 
	Ipv4Addr.cpp 
	Ipv6Addr.cpp 
	Subnet.cpp
	SuspiciousEvents.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(netimond Threads::Threads)

add_executable(netimond-test 
	netimond-test.cpp 
	Logfile.cpp 
	LogfileMock.cpp 
	tls.cpp 
	Connection.cpp 
	Watching.cpp 
	FindProcess.cpp 
	TrafficManager.cpp 
	ConnectionIdentifier.cpp 
	SumConnectionIdentifier.cpp 
	ActiveConnections.cpp 
	ActiveStateConnections.cpp 
	ActiveTcpConnections.cpp 
	ActiveUdpConnections.cpp 
	ICMP.cpp 
	ICMPv6.cpp 
	ConfigfileParser.cpp 
	MySql.cpp 
	Stats.cpp 
	Ip.cpp 
	Ipv4Addr.cpp 
	Ipv6Addr.cpp 
	Subnet.cpp
	SubnetTest.cpp
	SuspiciousEvents.cpp
	SuspiciousEventsTest.cpp
	ConnectionTest.cpp
	FindProcessTest.cpp
	ActiveTcpConnectionsTest.cpp
	ActiveUdpConnectionsTest.cpp
	WatchingMock.cpp
	WatchingTest.cpp
	LogfileTest.cpp
	ConfigfileParserTest.cpp
	ConnectionIdentifierTest.cpp
	SumConnectionIdentifierTest.cpp
	MySqlTest.cpp
	StatsTest.cpp
	IpMock.cpp
	IpTest.cpp
	Ipv4AddrTest.cpp
	Ipv6AddrTest.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(netimond-test Threads::Threads cppunit)

add_executable(FindProcessIntegrationTest 
	FindProcess.cpp
	FindProcessIntegrationTest.cpp
)
