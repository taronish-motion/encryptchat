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
#include "messagevector.h"
#include "udp.h"
#define BUFF_SIZE  514

using namespace std;

void parseParams(string &username, string &hostname, uint16_t &udpport,
                char *av[], uint16_t &tcpport, int &initialto, int &maxto, 
                uint16_t &hostport, int ac);
void DumpData(const uint8_t *payload, int length);
char* getUsername();
char* getHostname();
void sendDatagram(UDPServer udpserver, const uint16_t &tcpport, 
                    const uint16_t &type, const string &hostname, 
                    const string &username, struct sockaddr_in clientaddr);



int main(int argc, char *argv[]){
    string userName = "x", hostName = "x";
    uint16_t udpPort = 50550, tcpPort = 50551, hostPort;
    int initialTO = 5, maxTO = 60, nfds = 2, result;
    int currentTO;
    parseParams(userName, hostName, udpPort, argv, tcpPort, initialTO,
                maxTO, hostPort, argc);
    if(userName == "x"){
        userName = getUsername();
    }
    if(hostName == "x"){
        hostName = getHostname();
    }
    UDPServer udpServer(udpPort);

    struct pollfd pollFDs[64];
    bzero(pollFDs, sizeof(pollFDs));
    pollFDs[0].fd = udpServer.getFD();
    pollFDs[0].events = POLLIN;
    currentTO = initialTO;

    while(1){
        if(nfds == 2){
            sendDatagram(udpServer, tcpPort, 0x0001, hostName, 
                        userName, udpServer.getServerAddress());
        }
        cout << "Polling with timeout of " << currentTO << " secs" << endl;
        result = poll(pollFDs, nfds, currentTO*1000);
        if(0 > result){
            cout << "poll() failed..." << endl;
            exit(-1);
        }
        if(0 == result){
            if(currentTO*2 > maxTO)
                currentTO = maxTO;
            else
                currentTO *= 2;
        }
        break;
    }

}//main



void parseParams(string &username, string &hostname, uint16_t &udpport,
    char *av[], uint16_t &tcpport, int &initialto, int &maxto, 
    uint16_t &hostport, int ac){
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
  }
}//parse parameters


char* getUsername(){
    int j, size;
    char *uname = new char[64];
    
    for(int i = 0; i < 64; i++){
        uname[i] = ' ';
    }
    getlogin_r(uname, 64);
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
    message.AppendStringWithoutNULL("P2PIM");
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

    
    sentMsg = sendto(udpserver.getFD(), message.Data(), 
                        message.Length(), 0, 
                        (struct sockaddr *)&clientaddr, 
                        sizeof(clientaddr));
    
    if(sentMsg < 0){
        cerr << "Unable to call sendto()" << endl;
        close(udpserver.getFD());
        exit(1);
    }
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
