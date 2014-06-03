//Taronish Daruwalla
#include "tcp.h"


using namespace std;

//********** TCP Server Implementations ******************

int TCPServer::getFD(){
	return serverFileDescriptor;
}


TCPServer::TCPServer(uint16_t tcpport){
	serverFileDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(serverFileDescriptor < 0){
		cerr << "Unable to call socket() for tcp." << endl;
		exit(1);
	}
	bzero((char *) &serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(tcpport);
	
	int goodBind = bind(serverFileDescriptor, 
		(struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if(goodBind < 0){
		cout << "Error binding TCP socket" << endl;
		close(serverFileDescriptor);
		exit(1);
	}
	port = tcpport;
}


TCPServer::~TCPServer(){

}


//********** TCP Client Implementations *********************

string TCPClient::getUsername(){
	return userName;
}

string TCPClient::getHostname(){
	return hostName;
}

struct sockaddr_in TCPClient::getAddr(){
	return address;
}

void TCPClient::operator=(const TCPClient &rhs){
	address = rhs.address;
	userName = rhs.userName;
	hostName = rhs.hostName;
}

TCPClient::TCPClient(){

}

TCPClient::TCPClient(struct sockaddr_in clientaddr, string username, string hostname){
	address = clientaddr;
	userName = username;
	hostName = hostname;
}


TCPClient::~TCPClient(){

}



