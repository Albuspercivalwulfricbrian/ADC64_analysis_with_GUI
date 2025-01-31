#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H
#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QString>
#include <qalgorithms.h>
#include <vector>
#include <QLabel>
#include <Progress.h>
class ProgressWidget : public QDialog {
    Q_OBJECT

public:
    ProgressWidget(const std::vector<Progress*>& progressList, QWidget* parent = nullptr)
        : QDialog(parent){
        setWindowTitle("Прогресс анализа");
        setModal(true); // Блокирует основное окно
        qDeleteAll(progressBars); progressBars.clear(); 
        // qDeleteAll(labels); labels.clear();   
        qDeleteAll(activeProcesses); activeProcesses.clear();


        QHBoxLayout* layout  = new QHBoxLayout(this);
        QVBoxLayout* layoutleft  = new QVBoxLayout(this);
        QVBoxLayout* layoutright  = new QVBoxLayout(this);
        layoutright->addWidget(new QLabel("Finished:", this));

        for (const auto& progressPtr : progressList) 
        {
            if (progressPtr->active)
            {
                activeProcesses.push_back(progressPtr);
                QLabel* label = new QLabel(QString::fromStdString(progressPtr->fileName), this);
                layoutleft->addWidget(label);
                progressBars.push_back(new QProgressBar(this));
                progressBars.back()->setRange(0, 100);
                layoutleft->addWidget(progressBars.back());
                progressBars.back()->setValue(static_cast<int>(100*progressPtr->percentage));
            }  
            if (progressPtr->processed)
            {
                QLabel* label = new QLabel(QString::fromStdString(progressPtr->fileName), this);
                layoutright->addWidget(label);
            }
        }
        layout->addLayout(layoutleft); layout->addLayout(layoutright);
        // setLayout(layoutleft);setLayout(layoutright);
        setLayout(layout);
    }

    void updateProgress() 
    { 
        for (int index = 0; index < progressBars.size(); index++)
        {
            progressBars[index]->setValue(static_cast<int>(100*activeProcesses[index]->percentage ));
        }
    }

private:
    std::vector<QProgressBar*> progressBars;
    std::vector<Progress*> activeProcesses;
};
#endif PROGRESSWIDGET_H