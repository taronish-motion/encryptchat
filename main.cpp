//Taronish Daruwalla, 997447158

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <csignal>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/poll.h>
#include <vector>
#include <sstream>
#include <fcntl.h>
#include "EncryptionLibrary.h"
#include "messagevector.h"
#include "udp.h"
#include "tcp.h"
#define BUFF_SIZE  514

using namespace std;

void parseParams(string &username, string &hostname, uint16_t &udpport,
                char *av[], uint16_t &tcpport, int &initialto, int &maxto, 
                uint16_t &hostport, int ac, uint16_t &anchorport, string &anchorhost);
void DumpData(const uint8_t *payload, int length);
char* getUsername();
char* getHostname();
void sendDatagram(UDPServer udpserver, const uint16_t &tcpport, 
                    const uint16_t &type, const string &hostname, 
                    const string &username, struct sockaddr_in clientaddr);
void sigIntHandler(int param);
void sendAuthRequest(const uint64_t &secretnumber, const string &username, UDPServer udpserver);

int interrupted = 1;


int main(int argc, char *argv[]){
    string userName = "x", hostName = "x", password, anchorHost;
    int initialTO = 5, maxTO = 60, nfds = 2, result;
    uint16_t udpPort = 50550, tcpPort = 50551, hostPort, anchorPort = 50552;
    uint8_t *buffer = new uint8_t[BUFF_SIZE];
    int currentTO;
    struct sockaddr_in clientAddress;
    socklen_t clientLength;
    TCPClient clients[16];
    int clientInd = 0;
    uint64_t secretNumber = GenerateRandomValue();

    PublicEncryptDecrypt(secretNumber, P2PI_TRUST_E, P2PI_TRUST_N);

    parseParams(userName, hostName, udpPort, argv, tcpPort, initialTO,
                maxTO, hostPort, argc, anchorPort, anchorHost);

    if(userName == "x"){
        userName = getUsername();
    }
    if(hostName == "x"){
        hostName = getHostname();
    }

    cout << "Enter password to generate private key: ";
    cin >> password;

    UDPServer udpServer(udpPort);
    TCPServer tcpServer(tcpPort);

    struct pollfd pollFDs[64];
    bzero(pollFDs, sizeof(pollFDs));
    pollFDs[0].fd = udpServer.getFD();
    pollFDs[0].events = POLLIN;
    currentTO = initialTO;

    signal(SIGINT, sigIntHandler);

    cout << "Sending Request Authenticated Key, timeout = " << currentTO << "s\n";
    sendAuthRequest(secretNumber, userName, udpServer);
    cout << "Sending discovery, timeout = " << currentTO << "s\n";
    sendDatagram(udpServer, tcpPort, 1, hostName, 
                userName, udpServer.getServerAddress());

    while(1){
        //check for ^C and service, send closing msg if occured
        if(interrupted < 0){
            sendDatagram(udpServer, tcpPort, 3, hostName, 
                userName, udpServer.getServerAddress());
            close(udpServer.getFD());
            break;
        }
        
        //Poll all the fds
        //cout << "Polling with timeout of " << currentTO << " secs" << endl;
        result = poll(pollFDs, nfds, currentTO*1000);

        if(0 > result){  //error occured
            cout << "\nExiting poll(), sending closing message." << endl;
            sendDatagram(udpServer, tcpPort, 3, hostName, 
                userName, udpServer.getServerAddress());
            exit(-1);
        }
        if(0 == result){ //nothing to read in
            if(currentTO*2 > maxTO)
                currentTO = maxTO;
            else
                currentTO *= 2;
            if(2 == nfds){
                cout << "Sending discovery, timeout = " << currentTO << "s\n";
                sendDatagram(udpServer, tcpPort, 1, hostName, 
                    userName, udpServer.getServerAddress());
                // recvfrom(udpServer.getFD(), buffer, BUFF_SIZE, 0, 
                //         (struct sockaddr *)&clientAddress, 
                //         &clientLength);

                // if(buffer[5] == 0x02){
                //     cout << "got reply" << endl;
                // }
                // if(buffer[5] == 0x01){
                //     cout << "Received self-discover." << endl;
                // }
            }//if no clients
        }
        else{   //got something on one of the fds
            if(pollFDs[0].revents & POLLIN){
                clientLength = sizeof(clientAddress);
                int recvResult = recvfrom(udpServer.getFD(), buffer, BUFF_SIZE, 0, 
                                        (struct sockaddr *)&clientAddress, 
                                        &clientLength);
                if(recvResult < 0){
                    cout << "Error reading from UDP socket" << endl;
                    close(udpServer.getFD());
                    exit(1);
                }
                else{
                    string newHost, newUser, signature;
                    uint16_t type, garbage;
                    CNetworkMessage message;
                    message.AppendData(buffer, BUFF_SIZE);
                    if(!message.ExtractStringN(signature, 4)){
                        printf("Failed to extract signature.\n");
                        if(signature != "P2PI")
                            cout << "Not the correct UDP signature." << endl;
                    }
                    message.ExtractUInt16(type);
                    message.ExtractUInt16(garbage);
                    message.ExtractUInt16(garbage);
                    message.ExtractString(newHost);
                    message.ExtractString(newUser);
                    
                    if(newUser != userName || newHost != hostName){
                        TCPClient tempClient(clientAddress, newUser, newHost);
                        clients[clientInd] = tempClient;
                        if(1 == type){
                            sendDatagram(udpServer, tcpPort, 2, hostName, 
                                        userName, clientAddress); 
                        }//if its a discover from some1 else, reply
                        pollFDs[nfds].fd = clients[clientInd].getFD();
                        pollFDs[nfds].events = POLLIN;
                        nfds++;
                        cout << "Recieved UDP datagram type " << type << endl;
                    }//if its not me, add it to the clients
                    else{
                        cout << "Received self-discover." << endl;
                    }
                }//if recvfrom called properly
                
            }//if there was an event on UDP server socket

            if(pollFDs[1].revents & POLLIN){

            }//if there was an event on TCP server socket
        }//poll got an event
        //break;
    }

    sendDatagram(udpServer, tcpPort, 3, hostName, 
                userName, udpServer.getServerAddress());

    return 0;
}//main



void parseParams(string &username, string &hostname, uint16_t &udpport,
    char *av[], uint16_t &tcpport, int &initialto, int &maxto, 
    uint16_t &hostport, int ac, uint16_t &anchorport, string &anchorhost){

    bool ahUsed = false;
    if(ac % 2 == 0){ 
        cerr << "Incorrect parameter format" << endl;
        exit(1);
    }

    for(int i = 1; i < ac; i+=2){


        if(strcmp(av[i], "-u") == 0){ 
            username = av[i+1];
        }   
        if(strcmp(av[i], "-up") == 0){ 
            udpport = atoi(av[i+1]);
            if(udpport < 0 || udpport > 65535){
                cerr << "Invalid UDP port." << endl;
                exit(1);
            }   
        }   
        if(strcmp(av[i], "-tp") == 0){ 
            tcpport = atoi(av[i+1]);
            if(tcpport < 0 || tcpport > 65535){
                cerr << "Invalid UDP port." << endl;
                exit(1);
            }   
        }   
        if(strcmp(av[i], "-dt") == 0){ 
          initialto = atoi(av[i+1]);
        }   
        if(strcmp(av[i], "-dm") == 0){ 
          maxto = atoi(av[i+1]);
        }   
        if(strcmp(av[i], "-pp") == 0){ 
          hostname = strtok(av[i+1], ":");
          hostport = atoi(strtok(NULL, ""));
        }   
        if(strcmp(av[i], "-ap") == 0 && !ahUsed){ 
          udpport = atoi(av[i+1]);
          if(anchorport < 0 || anchorport > 65535){
            cerr << "Invalid anchor port." << endl;
            exit(1);
          }   
        }
        if(strcmp(av[i], "-ah") == 0){
            ahUsed = true;
            if(strchr(av[i+1], ':') == NULL){
                anchorhost = av[i+1];
            }
            else{
                anchorhost = strtok(av[i+1], ":");
                anchorport = atoi(strtok(NULL, ""));
            }
        }
    }
}//parse parameters


char* getUsername(){
    int j, size;
    char *uname = new char[64];
    
    for(int i = 0; i < 64; i++){
        uname[i] = ' ';
    }
    uname = getenv("USER");
    for(j = 0; j < 64 && uname[j] != ' '; j++){}
    size = j - 1;
    char *uname2 = new char[size];
    uname2 = strncpy(uname2, uname, size);
    return uname2;
}

char* getHostname(){
    int j, size;
    char *hname = new char[64];
    for(int i = 0; i < 64; i++){
        hname[i] = ' ';
    }
    gethostname(hname, 64);
    for(j = 0; j < 64 && hname[j] != ' '; j++){}
    size = j - 1;
    char *hname2 = new char[size];
    hname2 = strncpy(hname2, hname, size);
    return hname2;
}


void sendDatagram(UDPServer udpserver, const uint16_t &tcpport, 
                    const uint16_t &type, const string &hostname, 
                    const string &username, struct sockaddr_in clientaddr){
    int broadcastEnable = 1, setOptions, sentMsg;
    CNetworkMessage message;
    message.AppendStringWithoutNULL("P2PI");
    message.AppendUInt16(type);
    message.AppendUInt16(udpserver.getPort());
    message.AppendUInt16(tcpport);
    message.AppendString(hostname);
    message.AppendString(username);

    if(type == 0x0001){ 
        setOptions = setsockopt(udpserver.getFD(), SOL_SOCKET, SO_BROADCAST,
                                &broadcastEnable, sizeof(broadcastEnable));
        if(setOptions < 0){
            cerr << "Unable to set socket options." << endl;
            close(udpserver.getFD());
            exit(1);
        }
    }//if broadcast, set options

    
    sentMsg = sendto(udpserver.getFD(), message.Data(), message.Length(), 0, 
                        (struct sockaddr *)&clientaddr, sizeof(clientaddr));
    //CNetworkMessage Message = message;
    if(sentMsg < 0){
        cerr << "Unable to call sendto()" << endl;
        close(udpserver.getFD());
        exit(1);
    }


    // string Signature, Hostname, Username;
    // uint16_t Type, UDPPort, TCPPort;
    // printf("Messag is %zu bytes long.\n",Message.Length());
    // DumpData(Message.Data(), Message.Length());
                              
    // if(!Message.ExtractStringN(Signature,4)){
    //     printf("Failed to extract signature.\n");
    // }
    // if(!Message.ExtractUInt16(Type)){
    //     printf("Failed to extract type.\n");
    // }
    // if(!Message.ExtractUInt16(UDPPort)){
    //     printf("Failed to extract UDP port.\n");
    // }
    // if(!Message.ExtractUInt16(TCPPort)){
    //     printf("Failed to extract TCP port.\n");
    // }
    // if(!Message.ExtractString(Hostname)){
    //     printf("Failed to extract hostname.\n");
    // }
    // if(!Message.ExtractString(Username)){
    //     printf("Failed to extract username.\n");
    // }
    // printf("\nSignature: %s\n", Signature.c_str());
    // printf("Type     : %hu\n", Type);
    // printf("UDP Port : %hu\n", UDPPort);
    // printf("TCP Port : %hu\n", TCPPort);
    // printf("Hostname : %s\n", Hostname.c_str());
    // printf("Username : %s\n", Username.c_str());
}


void sigIntHandler(int param){
    if(param == SIGINT){
        interrupted = -1;  
    }
    else{
        interrupted = 1;
    }
}

void sendAuthRequest(const uint64_t &secretnumber, const string &username, UDPServer udpserver){
    CNetworkMessage message;
    int setOptions, broadcastEnable = 1;//, sentMsg;
    message.AppendStringWithoutNULL("P2PI");
    message.AppendUInt16(10);
    message.AppendUInt64(secretnumber);
    message.AppendString(username);

    setOptions = setsockopt(udpserver.getFD(), SOL_SOCKET, SO_BROADCAST,
                                &broadcastEnable, sizeof(broadcastEnable));
    if(setOptions < 0){
        cerr << "Unable to set socket options." << endl;
        close(udpserver.getFD());
        exit(1);
    }
    // sentMsg = sendto(udpserver.getFD(), message.Data(), message.Length(), 0, 
    //                     (struct sockaddr *)&(udpserver.getServerAddress()), sizeof(udpserver.getServerAddress()));
    // if(sentMsg < 0){
    //     cerr << "Unable to call sendto()" << endl;
    //     close(udpserver.getFD());
    //     exit(1);
    // }
}


void DumpData(const uint8_t *payload, int length){
    int Offset;
    
    Offset = 0;
    while(Offset < length){
        for(int Index = 0; Index < 16; Index++){
            if(8 == Index){
                printf(" ");   
            }
            if(Offset + Index < length){
                printf("%02X ",payload[Offset + Index]);
            }
            else{
                printf("   ");
            }
        }
        printf("   ");
        for(int Index = 0; Index < 16; Index++){
            if(8 == Index){
                printf(" ");   
            }
            if(Offset + Index < length){
                if((' ' <= payload[Offset + Index])&&('~' >= payload[Offset + Index])){
                    printf("%c",payload[Offset + Index]);
                }
                else{
                    printf(".");
                }
            }
            else{
                printf(" ");
            }
        }
        printf("\n");
        Offset += 16;
    }
    
}
