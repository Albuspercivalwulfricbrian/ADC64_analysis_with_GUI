#ifndef WORKER_H
#define WORKER_H

#include "BinaryDataStructs.h"
#include "ProgressDialog.h"
class Worker : public QObject, public DataFileReader {
    Q_OBJECT
private:

public slots:
    void doWork(ProgressDialog *dialog, const char * a) 
    {
        ReadFile();
        dialog->accept();
    }

};

#endif