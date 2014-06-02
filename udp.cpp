#include "udp.h"

using namespace std;

int UDPServer::getFD(){
	return serverFileDescriptor;
}

uint16_t UDPServer::getPort(){
	return port;
}

struct sockaddr_in UDPServer::getServerAddress(){
	return serverAddress;
}


UDPServer::UDPServer(uint16_t udpport){
	serverFileDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(serverFileDescriptor < 0){ 
		cerr << "Unable to call socket()" << endl;
		exit(1);
	}//socket call failed

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	serverAddress.sin_port = htons(udpport);
	int goodBind = bind(serverFileDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

	if(goodBind < 0){ 
		cout << "Error binding UDP socket" << endl;
		close(serverFileDescriptor);
		exit(1);
	}

	port = udpport;
}


UDPServer::~UDPServer(){

}

