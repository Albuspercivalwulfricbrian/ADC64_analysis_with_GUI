#include "Worker.h"

void Worker::ReadFile()
{
  if ((fd = fopen(fileName, "rb")) == NULL)
  {
    printf("Achtung: Open file error or file not found!\n");
    FileIsSet = 0;
    sSizeOfFile = 0;
  }
  else
  {
    FileIsSet = 1;
    fseek(fd, 0, SEEK_END);
    sSizeOfFile = ftell(fd);
  }
  FileIsIndexed = FileIndexation(maxEventLimit);
}

bool Worker::FileIndexation(int64_t maxEvents)
{
  bool iFileIsIndexed = 0;
  FileIsIndexed = 0;

  TotalHeader.clear();
  uint32_t uiBuffer[3] = {0, 0, 0};
  uiEventWithMaxSize = 0;
  uiTotalEvents = 0;
  uint32_t uiMaxSize = 0;
  fseek(fd, 0, SEEK_SET);
  if (sSizeOfFile != 0)
  {
    while (!feof(fd))
    {
      fread(&(TotalHeader.syncword), WORD_SIZE, 1, fd);
      if ((TotalHeader.syncword) == SYNC_WORD || (TotalHeader.syncword) == SYNC_WORD_ADC64)
      {
        currPos = ftell(fd);

        if (maxEvents > 0 && uiTotalEvents >= maxEvents)
        {
          iFileIsIndexed = 1;
          break;
        }

        uiTotalEvents++;
        eventPositions.push_back(ftell(fd));
        fread(&TotalHeader.EvHeader, sizeof(TotalHeader.EvHeader), 1, fd);
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
  fseek(fd, eventPositions[i], SEEK_SET);
  fread(&TotalHeader.EvHeader, sizeof(TotalHeader.EvHeader), 1, fd);
  const uint32_t BS = (TotalHeader.EvHeader.length / 4);

  uint32_t uiBuffer[BS];
  for (int p = 0; p < BS; p++)
    uiBuffer[p] = 0;
  fread(&uiBuffer, sizeof(uiBuffer), 1, fd);
  uint32_t offset = 0;
  int32_t end = BS;
  while (end > 0)
  {
    TotalHeader.DeviceHeader.sn = uiBuffer[offset];
    offset++;
    TotalHeader.DeviceHeader.id = (uiBuffer[offset] & 0xFF000000) >> 24;
    TotalHeader.DeviceHeader.length = (uiBuffer[offset] & 0x00FFFFFF);
    offset++;
    while (end > 0)
    {
      TotalHeader.ChHeader.ch = (uiBuffer[offset] & 0xFF000000) >> 24;
      TotalHeader.ChHeader.length = (uiBuffer[offset] & 0x00FFFFFC) >> 2;
      TotalHeader.ChHeader.type = (uiBuffer[offset] & 0x3);

      if (TotalHeader.ChHeader.length > 2048 && end > 3)
        break;

      offset++;

      switch (TotalHeader.ChHeader.type)
      {
      case 0:
        TotalHeader.TimeHeader.taisec = uiBuffer[offset];
        offset++;
        TotalHeader.TimeHeader.tainsec = (uiBuffer[offset] & 0xFFFFFFFC) >> 2;
        TotalHeader.TimeHeader.taiflags = (uiBuffer[offset] & 0x3);
        offset++;
        TotalHeader.TimeHeader.chlo = uiBuffer[offset];
        offset++;
        TotalHeader.TimeHeader.chup = uiBuffer[offset];
        offset++;
        break;
      case 1:
        if (adcmap.find(TotalHeader.DeviceHeader.sn) == adcmap.end())
        {
          offset += TotalHeader.ChHeader.length;
          break;
        }

        uint16_t ch = TotalHeader.ChHeader.ch;
        if (auto adcinstance = adcmap.find(TotalHeader.DeviceHeader.sn); adcinstance != adcmap.end())
          ch += 64 * adcinstance->second;

        if (ch == extChannel)
          event_waveform.ADCID = TotalHeader.DeviceHeader.sn;
        const uint16_t SN = (TotalHeader.ChHeader.length - 2) * 2;
        TotalHeader.SubHeader.wf_tslo = uiBuffer[offset];
        offset++;
        TotalHeader.SubHeader.wf_tsup = (uiBuffer[offset] & 0xFFFFFFFC) >> 2;
        offset++;
        int16_t wave = 0;
        int32_t polarity = 1;
        int32_t iSignalOffset = 0;
        if (ch == extChannel)
        {
          event_waveform.channel = ch;
          for (int s = 0; s < (SN / 2); s++)
          {
            auto ind = offset + s;
            wave = (((uiBuffer[ind] & 0xFFFF0000) >> 16) * polarity + iSignalOffset);
            event_waveform.wf.push_back(wave);
            wave = ((uiBuffer[ind] & 0xFFFF) * polarity + iSignalOffset);
            event_waveform.wf.push_back(wave);
          }
        }
        offset += (SN / 2);
        break;
      }
      end = BS - offset;
      if (end <= 1)
        break;
    }
    if (end <= 1)
      break;
  }
}

double Worker::getIndexationProgress()
{
  double progress = 0;
  if (currPos >= 0 && currPos <= sSizeOfFile)
    progress = (double)currPos / (double)sSizeOfFile;
  else
    progress = 1.;
  return progress;
}

void Worker::doWork(ProgressDialog *dialog, const char *a)
{
  ReadFile();
  dialog->accept();
}

void Worker::doScan(ProgressDialog *dialog)
{
  // Open only to determine file size (ScanAdcIds opens its own FILE*)
  FILE *f = fopen(fileName, "rb");
  if (!f)
  {
    sSizeOfFile = 0;
    dialog->accept();
    return;
  }
  fseek(f, 0, SEEK_END);
  sSizeOfFile = ftell(f);
  fclose(f);

  lastScanResult.clear();
  ScanAdcIds(lastScanResult);
  dialog->accept();
}

bool Worker::ScanAdcIds(std::vector<uint32_t> &out)
{
  out.clear();
  scanProgress = 0.0;

  if (sSizeOfFile == 0)
    return false;

  FILE *f = fopen(fileName, "rb");
  if (!f)
    return false;

  std::vector<uint32_t> ids;
  uint32_t sync = 0;

  while (fread(&sync, WORD_SIZE, 1, f) == 1)
  {
    if (sync != SYNC_WORD && sync != SYNC_WORD_ADC64)
      continue;

    decltype(TotalHeader.EvHeader) evHdr{};
    if (fread(&evHdr, sizeof(evHdr), 1, f) != 1)
      break;

    uint32_t BS = evHdr.length / 4;
    if (BS == 0)
      continue;

    std::vector<uint32_t> buf(BS);
    if (fread(buf.data(), sizeof(uint32_t), BS, f) != BS)
      break;

    uint32_t off = 0;
    while (off + 1 < BS)
    {
      uint32_t sn = buf[off++];
      uint32_t w2 = buf[off++];
      int32_t left = w2 & 0x00FFFFFF;

      bool seen = false;
      for (uint32_t s : ids)
        if (s == sn)
        {
          seen = true;
          break;
        }
      if (!seen)
        ids.push_back(sn);

      while (left > 0 && off < BS)
      {
        uint32_t cw = buf[off++];
        uint32_t clen = (cw >> 2) & 0x3FFFFF;
        uint16_t type = cw & 0x3;

        if (clen > 2048 && (BS - off) > 3)
          break;

        if (type == 0)
        {
          off += 4;
          left -= 5;
        }
        else if (type == 1)
        {
          uint32_t payload = (clen >= 2) ? clen - 2 : 0;
          off += 2 + payload;
          left -= 3 + payload;
        }
        else
        {
          break;
        }
      }
      if (BS - off <= 1)
        break;
    }

    long pos = ftell(f);
    if (sSizeOfFile > 0)
      scanProgress = static_cast<double>(pos) / static_cast<double>(sSizeOfFile);
  }

  fclose(f);
  scanProgress = 1.0;

  out.swap(ids);
  return !out.empty();
}

void Worker::setAdcMapFromIds(const std::vector<uint32_t> &ids)
{
  adcmap.clear();
  for (size_t i = 0; i < ids.size(); ++i)
    adcmap[ids[i]] = static_cast<int16_t>(i);
}