#ifndef DATA_FORMAT
#define DATA_FORMAT
#include <iostream>
#include <cstdint>
#include "stdio.h"
#include <vector>
#include <array>
#include "ctime"
#include "string.h"
#include "ADCHeaderStructs.h"
#include "ChannelEntry.h"
#define WORD_SIZE       4           // 4 bytes
#define SYNC_WORD       0x2a50d5af  // ADC64 (Old) & TQDC2
#define SYNC_WORD_ADC64 0x2a50d5af // ADC64 (New)
#define THREAD_NUMBER 8
using namespace std;
class DataFormat
{
  protected:
  static const int32_t total_channels = 128;
  char fileName[1024];

  uint32_t uiTotalEvents;
  TOTAL_HEADER TotalHeader;
  uint32_t uiEventWithMaxSize = 0;
  FILE *fd;
  size_t sSizeOfFile = 0;
  size_t currPos = 0;

  public:
  ChannelEntry event_waveform;
  bool FileIsSet;

  DataFormat()
  {
    FileIsSet = 0;
  }
  ~DataFormat(){};
  uint32_t GetTotalEvents(){return uiTotalEvents;}
  uint32_t GetCurrentPosition(){return ftello(fd);}
  virtual void setName(const char * a);

};
#endif DATA_FORMAT
