#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H
#include <QWidget>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QString>
#include <vector>
#include <QLabel>
#include <Progress.h>


// Custom QWidget for displaying progress
class ProgressWidget : public QWidget {
    Q_OBJECT

public:
    ProgressWidget(const std::vector<Progress*>& progressList, QWidget* parent = nullptr)
        : QWidget(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);

        for (const auto& progressPtr : progressList) {
            QLabel* label = new QLabel(QString::fromStdString(progressPtr->fileName), this);
            layout->addWidget(label);

            QProgressBar* progressBar = new QProgressBar(this);
            progressBar->setRange(0, 100);
            progressBar->setValue(static_cast<int>(100*progressPtr->percentage));
            layout->addWidget(progressBar);

            // Store the progress bar for later updates
            progressBars.push_back(progressBar);
            progressPointers.push_back(progressPtr); // Store the pointer to the Progress object
        }

        setLayout(layout);
    }

    void updateProgress(int index, float value) {
        if (index >= 0 && index < progressBars.size()) {
            progressBars[index]->setValue(static_cast<int>(value));
            if (index >= 0 && index < progressPointers.size()) {
                progressPointers[index]->percentage = value; // Update the actual Progress object
            }
        }
    }

private:
    std::vector<QProgressBar*> progressBars;
    std::vector<Progress*> progressPointers; // Store pointers to Progress objects
};
#endif PROGRESSWIDGET_H