#include <stdio.h>
#include <vector>
#include <string>
#include <stdint.h>
#include <arpa/inet.h>

class CNetworkMessage{
    protected:
        std::vector< uint8_t > DMessage;
        
    public:
        CNetworkMessage();
        CNetworkMessage(const CNetworkMessage &mess);
        
        CNetworkMessage &operator=(const CNetworkMessage &mess);
  
        void Clear();
        const uint8_t *Data() const;
        const uint8_t *Data(const uint8_t *val, size_t len);
        size_t Length() const;
        
        void AppendData(const uint8_t *val, size_t len);
        void AppendString(const std::string &str);
        void AppendStringWithoutNULL(const std::string &str);
        void AppendUInt16(uint16_t val);
        void AppendUInt32(uint32_t val);
        void AppendUInt64(uint64_t val);
        
        bool ExtractString(std::string &str);
        bool ExtractStringN(std::string &str, size_t len);
        bool ExtractUInt16(uint16_t &val);
        bool ExtractUInt32(uint32_t &val);
        bool ExtractUInt64(uint64_t &val);
        
};
