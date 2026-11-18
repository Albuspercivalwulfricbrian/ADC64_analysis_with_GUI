#ifndef WORKER_H
#define WORKER_H

#include "DataFormat.h"
#include "ProgressDialog.h"

class Worker : public QObject, public DataFormat
{
  Q_OBJECT
private:
  int64_t maxEventLimit = -1;
  double scanProgress = 0.0;

public:
  vector<size_t> eventPositions;
  std::vector<uint32_t> lastScanResult;
  bool FileIsIndexed = 0;
  double getIndexationState() { return FileIsIndexed; }
  void ReadFile();
  bool FileIndexation(int64_t maxEvents = -1);
  double getIndexationProgress();
  void ReadEvent(int64_t i, int16_t extChannel);

  bool ScanAdcIds(std::vector<uint32_t> &out);
  void setAdcMapFromIds(const std::vector<uint32_t> &ids);

  double getScanProgress() const { return scanProgress; }

  void setMaxEventLimit(int64_t limit) { maxEventLimit = limit; }
  int64_t getMaxEventLimit() const { return maxEventLimit; }
  int64_t getIndexedEventsCount() const { return eventPositions.size(); }

public slots:
  void doWork(ProgressDialog *dialog, const char *a);
  void doScan(ProgressDialog *dialog);
};

#endif // WORKER_H