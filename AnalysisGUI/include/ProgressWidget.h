#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H
#include <QDialog>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QString>
#include <vector>
#include <QLabel>
#include <Progress.h>
#include <iostream>
class ProgressWidget : public QDialog {
    Q_OBJECT

public:
    ProgressWidget(const std::vector<Progress*>& progressList, bool* changelayout, QWidget* parent = nullptr)
        : QDialog(parent), fchangelayout(changelayout), progressPointers(progressList) {
        setWindowTitle("Прогресс анализа");
        setModal(true); // Блокирует основное окно
        layout = new QVBoxLayout(this);
        fillLayoutActive();
    }


    void updateProgress() 
    {
        if (*fchangelayout == true)
        {
            clearLayout();
            fillLayoutActive();
            *fchangelayout = false;
        }        
        for (int index = 0; index < progressBars.size(); index++)
        {
            progressBars[index]->setValue(static_cast<int>(100*progressPointers[index]->percentage ));
        }
    }

private:
    std::vector<QProgressBar*> progressBars;
    std::vector<Progress*> progressPointers; // Store pointers to Progress objects
    bool *fchangelayout;
    QVBoxLayout* layout;
    void fillLayoutActive()
    {
        this->close();
        for (const auto& progressPtr : progressPointers) 
        {
            std::cout << progressPtr->fileName << std::endl;

            if (progressPtr->active)
            {
                QLabel* label = new QLabel(QString::fromStdString(progressPtr->fileName), this);
                layout->addWidget(label);

                QProgressBar* progressBar = new QProgressBar(this);
                progressBar->setRange(0, 100);
                progressBar->setValue(static_cast<int>(100*progressPtr->percentage));
                layout->addWidget(progressBar);

            //     // Store the progress bar for later updates
                progressBars.push_back(progressBar);

            //     progressPointers.push_back(progressPtr); // Store the pointer to the Progress object 

            }  
        }

        setLayout(layout);
        this->show();
    }

    // void clearLayout() 
    // {
    //     progressBars.clear();
    //     delete layout;
    //     layout = new QVBoxLayout(this);
    // }

    void clearLayout() {
        // Clear the QVBoxLayout
        for (auto pb: progressBars) delete pb;
        progressBars.clear();

        QLayoutItem *item;
        while ((item = layout->takeAt(0)) != nullptr) {
            QWidget *widget = item->widget();
            delete item; // Delete the layout item
            if (widget) {
                if (QLabel *label = qobject_cast<QLabel*>(widget)) label->setText("");
                widget->deleteLater(); // Schedule widget for deletion
            }
        }

    }
};



#endif PROGRESSWIDGET_H