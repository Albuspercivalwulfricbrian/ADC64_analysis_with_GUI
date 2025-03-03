#ifndef DATAFILEREADER_H
#define DATAFILEREADER_H

#include "DataFormat.h"
#include "ChannelEntry.h"
// #include <json.hpp>
#include "configmanager.h"
#include <TTree.h>
#include <TFile.h>
#include <TROOT.h>
#include <Progress.h>
#include <mutex>

class DataFileReader : public DataFormat
{
  private:
  char configName[1024];
  std::array<short_energy_ChannelEntry*, total_channels> short_channel_info;
  time_t start_time;
  TFile* RootDataFile = nullptr;
  TTree* RootDataTree = nullptr;
  std::map<int, ConfigManager*> config_manager;
  std::mutex write_lock;
  public:
  ChannelEntry event_waveform;
  bool FileIsSet;

  DataFileReader()
  {
    FileIsSet = 0;
    ROOT::EnableThreadSafety();
  }
  ~DataFileReader()
  {
    // SaveRootFile();
    delete fd;
  };

  void CreateRootFile();
  void SaveRootFile()
  {
    if (RootDataFile!=nullptr)
    {
      // if (RootDataTree!=nullptr) {RootDataFile->cd(); RootDataTree->Write();}
      RootDataFile->Write(0,TObject::kOverwrite);
      RootDataFile->Close();    
     } 
  }
  void setName(const char * a) override;
  void setName(const char * a, const char * b);
  void setName(const char * a, std::map<int, ConfigManager*> ext_config);
  void DisplayTimeToCalculate(int32_t EvNum, int32_t total_entries, time_t start_time);
  uint32_t ConsequentialEventsReading(Progress *progress);
};
#endif DATAFILEREADER_H
