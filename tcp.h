#ifndef TCP_H
	#define TCP_H

#include "tcp.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/poll.h>
#include <fcntl.h>

using namespace std;

class TCPServer{
	int serverFileDescriptor;
	struct sockaddr_in serverAddress;
	uint16_t port;

public:
	int getFD();
	TCPServer(uint16_t tcpport);
	~TCPServer();
};


class TCPClient{
	friend class TCPServer;
	struct sockaddr_in address;
	int fileDescriptor;
	string userName;
	string hostName;

public:
	int getFD();
	string getUsername();
	string getHostname();
	struct sockaddr_in getAddr();
	void operator=(const TCPClient &rhs);
	TCPClient();
	TCPClient(struct sockaddr_in clientaddr, string username, string hostname);
	~TCPClient();
};

#endif

