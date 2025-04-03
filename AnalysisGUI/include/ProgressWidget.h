#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H

#include <QDialog>
#include <QSplitter>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QString>
#include <QLabel>
#include <vector>
#include <Progress.h>

class ProgressWidget : public QDialog {
    Q_OBJECT

public:
    ProgressWidget(const std::vector<Progress*>& progressList, QWidget* parent = nullptr)
        : QDialog(parent) {
        setWindowTitle("Прогресс анализа");
        setModal(true); // Blocks the main window

        // Clear previous progress bars and labels
        qDeleteAll(progressBars);
        progressBars.clear();
        qDeleteAll(activeProcesses);
        activeProcesses.clear();

        // Create a QSplitter
        QSplitter* splitter = new QSplitter(this);
        splitter->setOrientation(Qt::Horizontal); // Set orientation to horizontal

        // Create left and right layouts
        QVBoxLayout* layoutLeft = new QVBoxLayout();
        layoutLeft->setAlignment(Qt::AlignTop);
        QVBoxLayout* layoutRight = new QVBoxLayout();
        layoutRight->setAlignment(Qt::AlignTop);
        layoutRight->addWidget(new QLabel("Finished:", this));

        // Add widgets to the layouts
        for (const auto& progressPtr : progressList) {
            if (progressPtr->active) {
                activeProcesses.push_back(progressPtr);
                QLabel* label = new QLabel(QString::fromStdString(progressPtr->fileName), this);
                layoutLeft->addWidget(label);
                
                QProgressBar* progressBar = new QProgressBar(this);
                progressBar->setRange(0, 100);
                progressBar->setValue(static_cast<int>(100 * progressPtr->percentage));
                layoutLeft->addWidget(progressBar);
                progressBars.push_back(progressBar);
            }
            if (progressPtr->processed) {
                QLabel* label = new QLabel(QString::fromStdString(progressPtr->fileName), this);
                layoutRight->addWidget(label);
            }
        }

        // Create widgets for the layouts
        QWidget* leftWidget = new QWidget();
        leftWidget->setLayout(layoutLeft);
        
        QWidget* rightWidget = new QWidget();
        rightWidget->setLayout(layoutRight);

        // Add left and right widgets to the splitter
        splitter->addWidget(leftWidget);
        splitter->addWidget(rightWidget);

        // Set the main layout for the dialog
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(splitter);
        setLayout(mainLayout);

        // Set size policy for the dialog
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    }

    void updateProgress() {
        for (size_t index = 0; index < progressBars.size(); ++index) {
            progressBars[index]->setValue(static_cast<int>(100 * activeProcesses[index]->percentage));
        }
    }

private:
    std::vector<QProgressBar*> progressBars;
    std::vector<Progress*> activeProcesses;
};

#endif // PROGRESSWIDGET_H