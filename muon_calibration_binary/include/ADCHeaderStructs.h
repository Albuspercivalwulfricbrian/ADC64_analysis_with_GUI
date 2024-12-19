
#ifndef ADC_HEADER_STRUCTS
#define ADC_HEADER_STRUCTS
#include <cstdint>
using namespace std;
struct EVENT_HDR_BLOCK {

    uint32_t length;                  // Event payload length: (S1+S2+...) bytes
    uint32_t evnum;                  // event number
    EVENT_HDR_BLOCK() { clear(); }
    
    void clear()
    {
        length = 0;
        evnum  = 0;
    }
};

struct DEVICE_HDR_BLOCK {
  uint32_t sn;                        // device sn
  uint32_t id;                        // device id
  uint32_t length;                    // device payload length
  DEVICE_HDR_BLOCK() { clear(); }
  void clear()
  {
    sn     = 0;
    id     = 0;
    length = 0;
  }
};

struct SUBTYPE_0_PAYLOAD{
  uint32_t taisec;                       // Event timestamp, TAI seconds
  uint32_t tainsec;                      // Event timestamp, TAI nanoseconds
  uint8_t  taiflags;                     // Event timestamp, TAI flags
  uint32_t  chlo;                        // readout channels 31:0 bit mask
  uint32_t  chup;                        // readout channels 63:32 bit mask
  SUBTYPE_0_PAYLOAD() { clear(); }
  void clear()
  {
    taisec   = 0;
    tainsec  = 0;
    taiflags = 0;
    chlo     = 0;
    chup     = 0;
  }
};


struct SUBTYPE_1_PAYLOAD{
  uint32_t wf_tsup;
  uint32_t wf_tslo;
  uint32_t tai_flags;
  SUBTYPE_1_PAYLOAD() { clear(); }
  void clear()
  {
    wf_tsup   = 0;
    wf_tslo   = 0;
    tai_flags = 0;
  }
};

struct CHANNEL_HDR_BLOCK{
  uint32_t ch;                      // Subtype-defined bits // just channel number
  uint32_t length;                    // MStream Payload Length: S 32-bit words
  uint32_t type;                      // MStream Subtype
  CHANNEL_HDR_BLOCK() { clear(); }
  void clear()
  {
    ch     = 0;
    length = 0;
    type   = 0;
  }
};

struct TOTAL_HEADER
{
    uint32_t syncword = 0;  
    EVENT_HDR_BLOCK EvHeader;
    DEVICE_HDR_BLOCK DeviceHeader;
    SUBTYPE_1_PAYLOAD SubHeader;
    CHANNEL_HDR_BLOCK ChHeader;
    SUBTYPE_0_PAYLOAD TimeHeader;

    TOTAL_HEADER(){ clear();}
    void clear()
    {
        syncword = 0;
        EvHeader.clear();
        DeviceHeader.clear();
        SubHeader.clear();
        ChHeader.clear();
        TimeHeader.clear();
    }
};
#endif ADC_HEADER_STRUCTS