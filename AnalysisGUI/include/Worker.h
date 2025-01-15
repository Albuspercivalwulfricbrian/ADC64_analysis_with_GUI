#ifndef WORKER_H
#define WORKER_H

#include "DataFormat.h"
#include "ProgressDialog.h"
class Worker : public QObject, public DataFormat {
    Q_OBJECT
private:
public:
  vector<size_t> eventPositions;
  bool FileIsIndexed = 0;
  double getIndexationState(){return FileIsIndexed;}
  void ReadFile();
  bool FileIndexation();
  double getIndexationProgress();
  void ReadEvent(int64_t i, int16_t extChannel);
public slots:
    void doWork(ProgressDialog *dialog, const char * a) ;
    // {
    //     ReadFile();
    //     dialog->accept();
    // }

};

#endif