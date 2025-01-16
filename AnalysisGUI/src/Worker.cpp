#include "Worker.h"

void Worker::ReadFile()
{
    // snprintf(fileName,sizeof(fileName),"%s",a);
    // strcat(fileName,Name);
    if ((fd=fopen(fileName, "rb")) == NULL)
    {
      printf("Achtung: Open file error or file not found!\n");
      FileIsSet = 0;
      sSizeOfFile = 0;
    }
    else
    {
      FileIsSet = 1;
      fseek(fd,0,SEEK_END);
      sSizeOfFile = ftell(fd);

    }
    FileIsIndexed = FileIndexation();  
}


bool Worker::FileIndexation() {
  bool iFileIsIndexed = 0;
  FileIsIndexed = 0;

  TotalHeader.clear();
  uint32_t uiBuffer[3] = {0,0,0};
  uiEventWithMaxSize = 0;
  uiTotalEvents=0;
  uint32_t uiMaxSize = 0;
  fseek(fd,0,SEEK_SET);
  if (sSizeOfFile!=0)
  {
    while (!feof(fd))
    {
      fread(&(TotalHeader.syncword),WORD_SIZE,1,fd);
      if ((TotalHeader.syncword) == SYNC_WORD || (TotalHeader.syncword) == SYNC_WORD_ADC64)
      {
          currPos = ftell(fd);

        // if (FileIsSet==0) FileIsSet = 1;
        uiTotalEvents++;
        eventPositions.push_back(ftell(fd));
        fread(&TotalHeader.EvHeader,sizeof(TotalHeader.EvHeader),1,fd);
        if (uiMaxSize < TotalHeader.EvHeader.length)
        {
          uiMaxSize = TotalHeader.EvHeader.length;
          uiEventWithMaxSize = uiTotalEvents;
        }
      }
      iFileIsIndexed = 1;
    }
  }
  return iFileIsIndexed;
}

void Worker::ReadEvent(int64_t i, int16_t extChannel)
{
  event_waveform.Initialize();
  fseek(fd,eventPositions[i],SEEK_SET);
  fread(&TotalHeader.EvHeader,sizeof(TotalHeader.EvHeader),1,fd);
  const uint32_t BS = (TotalHeader.EvHeader.length/4); // size of event block excluding event header

  uint32_t uiBuffer[BS];
  for (int p=0; p<BS; p++) uiBuffer[p] = 0;
  fread(&uiBuffer, sizeof(uiBuffer),1,fd);
  uint32_t offset = 0;   // in elements of event buffer
  int32_t  end    = BS;  // remain number of elements (32-bit words)
  // if (uiTotalEvents%10000==0) DisplayTimeToCalculate(ftell(fd)/1024,sSizeOfFile/1024,start_time);
  while (end > 0)
  {
    TotalHeader.DeviceHeader.sn     = uiBuffer[offset];
    offset++;
    TotalHeader.DeviceHeader.id     = (uiBuffer[offset] & 0xFF000000)>>24;
    TotalHeader.DeviceHeader.length = (uiBuffer[offset] & 0x00FFFFFF);
    offset++;
    while (end > 0)
    {
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
          // event_waveform.wf.clear();           
          if (TotalHeader.DeviceHeader.sn == 101830393) ch+=64;//If many ADCs IDs should be added +n*64
          
 
          if (ch==extChannel) event_waveform.ADCID = TotalHeader.DeviceHeader.sn;
          const uint16_t SN = (TotalHeader.ChHeader.length-2)*2;  // Number of samples
          TotalHeader.SubHeader.wf_tslo = uiBuffer[offset];
          offset++;

          TotalHeader.SubHeader.wf_tsup = (uiBuffer[offset] & 0xFFFFFFFC) >> 2;
          offset++;
          int16_t wave = 0;
          int32_t polarity = 1;
          int32_t iSignalOffset = 0;
          if (ch==extChannel) 
          {
            event_waveform.channel=ch;
            for (int s=0; s<(SN/2); s++)
            {
              auto ind = offset+s;     // dirrect cycle
              wave = (((uiBuffer[ind] & 0xFFFF0000) >> 16) * polarity + iSignalOffset);
              event_waveform.wf.push_back(wave);
              wave = ((uiBuffer[ind] & 0xFFFF) * polarity + iSignalOffset);
              event_waveform.wf.push_back(wave);
            }
            // event_waveform.wf_size = event_waveform.wf.size();
            // if (event_waveform.wf.size() > 0) break;
          }

          offset += (SN/2);
          break;
      }
      end = BS - offset;
      if (end <= 1) break;
    }
    if (end <= 1) break;
  }
}

double Worker::getIndexationProgress()
{
  double progress = 0;
  if (currPos >=0 && currPos <= sSizeOfFile) progress = (double)currPos/(double)sSizeOfFile;
  else progress = 1.;
  return progress;
}

void Worker::doWork(ProgressDialog *dialog, const char * a) 
{
    ReadFile();
    dialog->accept();
}