#ifndef WORKER_H
#define WORKER_H

#include "DataFormat.h"
#include "ProgressDialog.h"
class Worker : public QObject, public DataFormat
{
  Q_OBJECT
private:
  int64_t maxEventLimit;

public:
  vector<size_t> eventPositions;
  bool FileIsIndexed = 0;
  double getIndexationState() { return FileIsIndexed; }
  void ReadFile();
  bool FileIndexation(int64_t maxEvents = -1);
  double getIndexationProgress();
  void ReadEvent(int64_t i, int16_t extChannel);

  void setMaxEventLimit(int64_t limit) { maxEventLimit = limit; }
  int64_t getMaxEventLimit() const { return maxEventLimit; }
  int64_t getIndexedEventsCount() const { return eventPositions.size(); }

public slots:
  void doWork(ProgressDialog *dialog, const char *a);
};

#endif