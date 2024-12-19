#include "BinaryDataStructs.h"

void DataFileReader::DisplayTimeToCalculate(int32_t EvNum, int32_t total_entries, time_t start_time)
{
  std::cout<< u8"\033[2J\033[1;1H"; 
  std::cout << (float)EvNum/(float)total_entries*100 << "%" << std::endl;
  time_t time_left = (time(NULL)-start_time)*(float)(total_entries-EvNum)/(float)(EvNum);
  std::cout << "time left: ";
  if (time_left/3600 > 0) cout << time_left/3600 <<"h ";
  if ((time_left%3600)/60 > 0 || time_left/3600 == 0) cout << (time_left%3600)/60 << "m ";
  cout << (time_left%3600)%60<< "s " <<std::endl;
  std::cout << "time spent: ";
  time_left = (time(NULL)-start_time);

  if (time_left/3600 > 0) cout << time_left/3600 <<"h ";
  if ((time_left%3600)/60 > 0 || time_left/3600 == 0) cout << (time_left%3600)/60 << "m ";
  cout << (time_left%3600)%60<< "s " <<std::endl;
}

uint32_t DataFileReader::ConsequentialEventsReading()
{ 
  TotalHeader.clear();
  uint32_t uiBuffer[3] = {0,0,0};
  uiEventWithMaxSize = 0;
  uiTotalEvents=0;

  uint32_t uiMaxSize = 0; 
  fseek(fd,0,SEEK_END);
  sSizeOfFile = ftell(fd);
  fseek(fd,0,SEEK_SET);
  while (!feof(fd))
  {

    fread(&(TotalHeader.syncword),WORD_SIZE,1,fd);
    if ((TotalHeader.syncword) == SYNC_WORD || (TotalHeader.syncword) == SYNC_WORD_ADC64)
    {
      for(int ch = 0; ch < total_channels; ch++) short_channel_info[ch]->Initialize();

      uiTotalEvents++;
      fread(&TotalHeader.EvHeader,sizeof(TotalHeader.EvHeader),1,fd);
      if (uiMaxSize < TotalHeader.EvHeader.length)
      {
        uiMaxSize = TotalHeader.EvHeader.length;
        uiEventWithMaxSize = uiTotalEvents;
      }

      if (TotalHeader.syncword != SYNC_WORD)
          return -1;
      const uint32_t BS = (TotalHeader.EvHeader.length/4); // size of event block excluding event header

      uint32_t uiBuffer[BS];
      for (int p=0; p<BS; p++) uiBuffer[p] = 0;
      fread(&uiBuffer, sizeof(uiBuffer),1,fd);
      uint32_t offset = 0;   // in elements of event buffer
      int32_t  end    = BS;  // remain number of elements (32-bit words)
      if (uiTotalEvents%10000==0) DisplayTimeToCalculate(ftell(fd)/1024,sSizeOfFile/1024,start_time);
      while (end > 0)
      {
        TotalHeader.DeviceHeader.sn     = uiBuffer[offset];
        offset++;
        TotalHeader.DeviceHeader.id     = (uiBuffer[offset] & 0xFF000000)>>24;
        TotalHeader.DeviceHeader.length = (uiBuffer[offset] & 0x00FFFFFF);
        offset++;


        while (end > 0)
        {
        //   cout << "OS: " <<offset << endl;

          TotalHeader.ChHeader.ch     = (uiBuffer[offset] & 0xFF000000) >> 24;
          TotalHeader.ChHeader.length = (uiBuffer[offset] & 0x00FFFFFC)  >> 2;
          TotalHeader.ChHeader.type   = (uiBuffer[offset] & 0x3);

          if(TotalHeader.ChHeader.length > 2048 && end > 3) break;

          offset++; // skip mstream header

          switch (TotalHeader.ChHeader.type)  
          {
            case 0:
              TotalHeader.TimeHeader.taisec   = uiBuffer[offset];
              offset++;
              TotalHeader.TimeHeader.tainsec  = (uiBuffer[offset] & 0xFFFFFFFC) >> 2;
              TotalHeader.TimeHeader.taiflags = (uiBuffer[offset] & 0x3);
              offset++;
              TotalHeader.TimeHeader.chlo     = uiBuffer[offset];
              offset++;
              TotalHeader.TimeHeader.chup     = uiBuffer[offset];
              offset++;

              break;
            case 1:
              uint16_t ch = TotalHeader.ChHeader.ch;
              if (TotalHeader.DeviceHeader.sn == 101830393) ch+=64;
              event_waveform.wf.clear(); event_waveform.ADCID = TotalHeader.DeviceHeader.sn;
              const uint16_t SN = (TotalHeader.ChHeader.length-2)*2;  // Number of samples
              TotalHeader.SubHeader.wf_tslo = uiBuffer[offset];
              offset++;

              TotalHeader.SubHeader.wf_tsup = (uiBuffer[offset] & 0xFFFFFFFC) >> 2;
              offset++;
              int16_t wave = 0;
              int32_t polarity = 1;
              int32_t iSignalOffset = 0;
              short_channel_info[ch]->ADCTimeStamp = (float)TotalHeader.SubHeader.wf_tslo+(float)(TotalHeader.SubHeader.wf_tsup/1000000000.0);

              event_waveform.channel=ch;

              for (int s=0; s<(SN/2); s++)
              {
                auto ind = offset+s;     // dirrect cycle
                wave = (((uiBuffer[ind] & 0xFFFF0000) >> 16) * polarity + iSignalOffset);
                event_waveform.wf.push_back(wave);
                wave = ((uiBuffer[ind] & 0xFFFF) * polarity + iSignalOffset);
                event_waveform.wf.push_back(wave);
              }
              event_waveform.wf_size = event_waveform.wf.size();
              ////////////////
              // event_waveform.InvertSignal();

              event_waveform.Set_Zero_Level_Area(60);
              // event_waveform.SplineWf();
              // event_waveform.SplineWf();
              short_channel_info[ch]->zl = event_waveform.CalculateZlwithNoisePeaks(130);
              // short_channel_info[ch]->zl = event_waveform.Get_Zero_Level();
              short_channel_info[ch]->zl_rms = event_waveform.Get_Zero_Level_RMS();

              event_waveform.SetBoarders(50,100);//Sanya smotry syuda. Zdes yobannye granitsy tvoyego signala dlya poiska polozhemiya pika
              int pp = event_waveform.Get_time();
              // event_waveform.SetBoarders(pp-12,pp+25);
              short_channel_info[ch]->amp = event_waveform.Get_Amplitude();
              event_waveform.AssumeSmartScope();// Sanya!!!! Zdes granitsy dlya umnogo integrirovaniye. Dlya bolshih signalov otklyuchai blyat
              
              short_channel_info[ch]->time = event_waveform.Get_time_gauss();
              short_channel_info[ch]->charge = event_waveform.Get_Charge();
              short_channel_info[ch]->ADC_ID = event_waveform.ADCID;
              short_channel_info[ch]->II = event_waveform.GetIntegralInfo();
              

              offset += (SN/2);
              break;
          }
          end = BS - offset;
          if (end <= 1) break;
        }
        if (end <= 1) break;
      }
      RootDataTree->Fill();

    }
  }
  return uiTotalEvents;
}

void DataFileReader::setName(const char * a)
{
    snprintf(fileName,sizeof(fileName),"%s",a);
}

void DataFileReader::CreateRootFile()
{
  const char * dirName = std::filesystem::path(fileName).parent_path().c_str();
  const char * Name = std::filesystem::path(fileName).stem().c_str();
  char rootName[1024];

  if ((fd=fopen(fileName, "rb")) == NULL)
  {
      printf("Achtung: Open file error or file not found!\n");
      return;
  }
  cout << "Starting" << endl;
  // RootDataFile = new TFile ((TString)(rootName)+ ".root", "RECREATE");
  RootDataFile = new TFile ((TString)(dirName)+"/calibrated_files/"+(TString)(Name)+ ".root", "RECREATE");
  RootDataTree = new TTree ("adc64_data","adc64_data");
  for(int ch = 0; ch < total_channels; ch++)
  {
    short_channel_info[ch] = new short_energy_ChannelEntry();
    short_channel_info[ch]->Initialize();
    RootDataTree->Branch((TString::Format("channel_%i", ch+1)).Data(), &short_channel_info[ch]);
  }
  start_time = std::time(nullptr);
}