
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
#define WORD_SIZE       4           // 4 bytes
#define SYNC_WORD       0x2a50d5af  // ADC64 (Old) & TQDC2
#define SYNC_WORD_ADC64 0x2a50d5af // ADC64 (New)
#define THREAD_NUMBER 8
using namespace std;

class ChannelEntry {

    public:
    int32_t ADCID = 0;
    int16_t channel = 0;
    // int16_t wf_size;
    vector<int16_t> wf;
    void clear()
    {
      int32_t ADCID = 0;
      int16_t channel = 0;
      // int16_t wf_size = 0;
      wf.clear();      
    }
};

class DataFileReader
{
  protected:
  static const int32_t total_channels = 128;
  char fileName[1024];
  uint32_t uiTotalEvents;
  TOTAL_HEADER TotalHeader;
  uint32_t uiEventWithMaxSize = 0;
  FILE *fd;
  bool FileIsSet;
  ChannelEntry event_waveform;

  size_t sSizeOfFile = 0;
  size_t currPos = 0;
  public:
  DataFileReader()
  {
    FileIsSet = 0;
  }

  ~DataFileReader(){};
  uint32_t getTotalEvents(){return uiTotalEvents;}
  uint32_t getCurrentPosition(){return ftello(fd);}

  void setName(const char * a);

  vector<size_t> eventPositions;
  bool FileIsIndexed = 0;
  double getIndexationState(){return FileIsIndexed;}
  void ReadFile();
  bool FileIndexation();
  double getIndexationProgress();
  void ReadEvent(int64_t i, int16_t extChannel);
};
#endif DATA_FORMAT
