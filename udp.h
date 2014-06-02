#ifndef UDP_H
	#define UDP_H

#include <iostream>
#include <cstdlib>
#include <csignal>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <stdint.h>
#include <string>

using namespace std;

class UDPServer{
	int serverFileDescriptor;
	struct sockaddr_in serverAddress;
	uint16_t port;
public:
	int getFD();
	uint16_t getPort();
	struct sockaddr_in getServerAddress();
	UDPServer(uint16_t udpport);
	~UDPServer();
};//class UDPServer

#endif
