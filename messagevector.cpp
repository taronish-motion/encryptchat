#include "messagevector.h"

using namespace std;

CNetworkMessage::CNetworkMessage(){}

CNetworkMessage::CNetworkMessage(const CNetworkMessage &mess){
    DMessage = mess.DMessage;    
}

CNetworkMessage &CNetworkMessage::operator=(const CNetworkMessage &mess){
    if(this != &mess){
        DMessage = mess.DMessage;
    }
    return *this;            
}


void CNetworkMessage::Clear(){
    DMessage.clear();
}

const uint8_t *CNetworkMessage::Data() const{
    return DMessage.data();
}

const uint8_t *CNetworkMessage::Data(const uint8_t *data, size_t len){
    DMessage.resize(len);
    for(size_t Index = 0; Index < len; Index++){
        DMessage[Index] = data[Index];
    }
    return DMessage.data();
}

size_t CNetworkMessage::Length() const{
    return DMessage.size();
}

void CNetworkMessage::AppendData(const uint8_t *data, size_t len){
    size_t Index = DMessage.size();
    DMessage.resize(Index + len);
    while(Index < DMessage.size()){
        DMessage[Index] = *data;
        data++;
        Index++;
    }
}

void CNetworkMessage::AppendString(const std::string &str){
    AppendData((const uint8_t *)str.c_str(), str.length() + 1);
}

void CNetworkMessage::AppendStringWithoutNULL(const std::string &str){
    AppendData((const uint8_t *)str.c_str(), str.length());
}

void CNetworkMessage::AppendUInt16(uint16_t val){
    val = htons(val);
    AppendData((const uint8_t *)&val, sizeof(val));
}

void CNetworkMessage::AppendUInt32(uint32_t val){
    val = htonl(val);
    AppendData((const uint8_t *)&val, sizeof(val));
}

void CNetworkMessage::AppendUInt64(uint64_t val){
    uint32_t High, Low;
    // not all systems have htonll
    High = htonl(val>>32);
    Low = htonl(val);
    AppendData((const uint8_t *)&High, sizeof(High));
    AppendData((const uint8_t *)&Low, sizeof(Low));
}

bool CNetworkMessage::ExtractString(std::string &str){
    size_t Index = 0;
    
    while(Index < DMessage.size()){
        if('\0' == DMessage[Index]){
            break;   
        }
        Index++;
    }
    if(Index == DMessage.size()){
        return false;   
    }
    str = (char *)DMessage.data();
    DMessage.erase(DMessage.begin(), DMessage.begin() + Index + 1);
    return true;
}

bool CNetworkMessage::ExtractStringN(std::string &str, size_t len){
    size_t Index = 0;
    
    while(Index < len){
        if('\0' == DMessage[Index]){
            return false;
        }
        Index++;
    }
    str = std::string((char *)DMessage.data(), len);
    DMessage.erase(DMessage.begin(), DMessage.begin() + len);
    return true;
}

bool CNetworkMessage::ExtractUInt16(uint16_t &val){
    if(DMessage.size() < sizeof(val)){
        return false;
    }
    val = ntohs(*(uint16_t *)DMessage.data());
    DMessage.erase(DMessage.begin(), DMessage.begin() + sizeof(val));
    return true;
}

bool CNetworkMessage::ExtractUInt32(uint32_t &val){
    if(DMessage.size() < sizeof(val)){
        return false;
    }
    val = ntohl(*(uint32_t *)DMessage.data());
    DMessage.erase(DMessage.begin(), DMessage.begin() + sizeof(val));
    return true;
}

bool CNetworkMessage::ExtractUInt64(uint64_t &val){
    uint32_t High, Low;
    if(DMessage.size() < sizeof(val)){
        return false;
    }
    High = ntohl(*(uint32_t *)DMessage.data());
    Low = ntohl(*(uint32_t *)(DMessage.data() + sizeof(uint32_t)));
    // not all systems have ntohll
    val = (((uint64_t)High)<<32) | Low;
    DMessage.erase(DMessage.begin(), DMessage.begin() + sizeof(val));
    return true;
}
