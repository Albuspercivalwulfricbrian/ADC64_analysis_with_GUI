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
  std::vector<short_energy_ChannelEntry *> chInfo;
  std::vector<PeaksInfo *> chInfo_peaks;
  time_t start_time;
  TFile *RootDataFile = nullptr;
  TTree *RootDataTree = nullptr;
  std::map<int, ConfigManager *> config_manager;
  std::mutex write_lock;
  bool StopAnalysis = 0;
  Int_t peak_candidate = 500;
  string WriteMode = "Single";

public:

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

  ChannelEntry event_waveform;
  bool FileIsSet;

  void SetStopAnalysis(bool);
  void SetWriteMode(string);
  void CreateRootFile();
  void SaveRootFile();
  void setName(const char *a) override;
  void setName(const char *a, const char *b);
  void setName(const char *a, std::map<int, ConfigManager *> ext_config);
  void DisplayTimeToCalculate(int32_t EvNum, int32_t total_entries, time_t start_time);
  uint32_t ConsequentialEventsReading(Progress *progress);
};
#endif DATAFILEREADER_H
