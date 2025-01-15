#ifndef DATAFILEREADER_H
#define DATAFILEREADER_H

#include "DataFormat.h"
#include "ChannelEntry.h"
#include <nlohmann/json.hpp>
#include "configmanager.h"
#include <TTree.h>
#include <TFile.h>
class DataFileReader : public DataFormat
{
  private:
  char configName[1024];
  std::array<short_energy_ChannelEntry*, total_channels> short_channel_info;
  time_t start_time;
  TFile* RootDataFile = nullptr;
  TTree* RootDataTree = nullptr;
  std::map<int, ConfigManager*> config_manager;

  public:
  ChannelEntry event_waveform;
  bool FileIsSet;

  DataFileReader()
  {
    FileIsSet = 0;
  }
  ~DataFileReader()
  {
    if (RootDataFile && RootDataTree) SaveRootFile();
  };

  void CreateRootFile();
  void SaveRootFile()
  {
    RootDataTree->Write();
    RootDataFile->Close();    
  }
  void setName(const char * a) override;
  void setName(const char * a, const char * b);

  void DisplayTimeToCalculate(int32_t EvNum, int32_t total_entries, time_t start_time);
  uint32_t ConsequentialEventsReading();
};
#endif DATAFILEREADER_H
