
#ifndef DATA_FORMAT
#define DATA_FORMAT
#include <iostream>
#include <cstdint>
#include "stdio.h"
#include <vector>
#include <array>
#include <TTree.h>
#include <TFile.h>
#include "ctime"
#include "string.h"
#include "ADCHeaderStructs.h"
#include "ChannelEntry.h"
#include <nlohmann/json.hpp>
#include "configmanager.h"

#define WORD_SIZE       4           // 4 bytes
// #define SYNC_WORD_TIME  0x72617453//0x3f60b8a8  // ADC64
#define SYNC_WORD       0x2a50d5af  // ADC64 (Old) & TQDC2
#define SYNC_WORD_ADC64 0x2a50d5af // ADC64 (New)
#define THREAD_NUMBER 8
using namespace std;

class DataFileReader
{
  private:
  static const int32_t total_channels = 128;
  char fileName[1024];
  char configName[1024];

  uint32_t uiTotalEvents;
  TOTAL_HEADER TotalHeader;
  uint32_t uiEventWithMaxSize = 0;
  FILE *fd;
  bool FileIsSet;
  ChannelEntry event_waveform;

  size_t sSizeOfFile = 0;
  size_t currPos = 0;

  std::array<short_energy_ChannelEntry*, total_channels> short_channel_info;
  time_t start_time;
  TFile* RootDataFile = nullptr;
  TTree* RootDataTree = nullptr;
  std::map<int, ConfigManager*> config_manager;
  public:
  DataFileReader()
  {
    FileIsSet = 0;
  }
  ~DataFileReader()
  {
    if (RootDataFile && RootDataTree) SaveRootFile();
  };
  uint32_t GetTotalEvents(){return uiTotalEvents;}
  uint32_t GetCurrentPosition(){return ftello(fd);}
  void setName(const char * a, const char * b = "");

  void CreateRootFile();
  void SaveRootFile()
  {
    RootDataTree->Write();
    RootDataFile->Close();    
  }

  void DisplayTimeToCalculate(int32_t EvNum, int32_t total_entries, time_t start_time);
  uint32_t ConsequentialEventsReading();
};
#endif DATA_FORMAT
