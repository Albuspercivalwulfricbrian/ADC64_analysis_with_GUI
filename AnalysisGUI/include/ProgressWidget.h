#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H

#include <QWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QProgressBar>
#include <QString>
#include <QLabel>
#include <vector>
#include <Progress.h>
#include "thread"
#include "QCloseEvent"
class ProgressWidget : public QWidget {
    Q_OBJECT

public:
    enum class Mode {
        Embedded,  // For use in scroll areas
        Modal      // For standalone modal use
    };

    ProgressWidget(const std::vector<Progress*>& progressList, 
                  QWidget* parent = nullptr,
                  Mode mode = Mode::Embedded)
        : QWidget(parent, mode == Mode::Modal ? Qt::Dialog : Qt::Widget),
          currentProgressList(progressList) 
    {
        if (mode == Mode::Modal) {
            setWindowModality(Qt::ApplicationModal);
            setWindowTitle("Progress Analysis");
        }
        initializeUI();
        setWindowFlags(Qt::Window | 
            Qt::WindowTitleHint |
            Qt::WindowMinMaxButtonsHint |
            Qt::WindowCloseButtonHint);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setProgressList(const std::vector<Progress*>& progressList) {
        currentProgressList = progressList;
        rebuildUI();
    }

    // const std::vector<Progress*>& getCurrentProgressList() const { 
    //     return currentProgressList; 
    // }
    void showAsModal() {
        setWindowFlags(Qt::Dialog);
        setWindowModality(Qt::ApplicationModal);
        setWindowTitle("Progress Analysis");
        show();
    }

    void updateProgressList(const std::vector<Progress*>& newProgressList) {
        currentProgressList = newProgressList;
        rebuildUI();
    }

    void updateProgress() {
        // for (size_t index = 0; index < progressBars.size(); ++index) {
        //     progressBars[index]->setValue(static_cast<int>(100 * activeProcesses[index]->percentage));
        // }
        for (size_t i = 0; i < progressBars.size(); i++) {
            if (i < activeProcesses.size()) {
                progressBars[i]->setValue(static_cast<int>(activeProcesses[i]->percentage * 100));
            }
        }

    }

    signals:
    void requestUpdate();
    void aboutToClose();
    private:
    std::vector<QProgressBar*> progressBars;
    std::vector<Progress*> activeProcesses;
    std::vector<Progress*> currentProgressList;
    QSplitter* splitter = nullptr;
    QScrollArea* leftScrollArea = nullptr;
    QScrollArea* rightScrollArea = nullptr;
    QWidget* leftContent = nullptr;
    QWidget* rightContent = nullptr;

    void initializeUI() {
        // Main splitter
        splitter = new QSplitter(this);
        splitter->setOrientation(Qt::Horizontal);
        splitter->setHandleWidth(10);
        splitter->setChildrenCollapsible(false);

        // Create scroll areas
        leftScrollArea = new QScrollArea(this);
        rightScrollArea = new QScrollArea(this);
        
        leftScrollArea->setWidgetResizable(true);
        rightScrollArea->setWidgetResizable(true);

        splitter->addWidget(leftScrollArea);
        splitter->addWidget(rightScrollArea);

        // Set reasonable default sizes
        QList<int> sizes;
        sizes << width() / 2 << width() / 2;
        splitter->setSizes(sizes);

        // Main layout
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(splitter);
        setLayout(mainLayout);

        // Size policies
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setMinimumSize(600, 400); // Set reasonable minimum size

        rebuildUI();
    }

    void clearUI() {
        qDeleteAll(progressBars);
        progressBars.clear();
        activeProcesses.clear();

        delete leftContent;
        delete rightContent;
        leftContent = nullptr;
        rightContent = nullptr;
    }



    void rebuildUI()
    {
        clearUI();
        
        QWidget* leftContent = new QWidget;
        QVBoxLayout* leftLayout = new QVBoxLayout(leftContent);
        
        QWidget* rightContent = new QWidget;
        QVBoxLayout* rightLayout = new QVBoxLayout(rightContent);
        rightLayout->addWidget(new QLabel("Finished:", this));
    
        for (const auto& progressPtr : currentProgressList) {
            if (progressPtr->active) {
                QLabel* label = new QLabel(QString::fromStdString(progressPtr->fileName), this);
                leftLayout->addWidget(label);
                
                QProgressBar* progressBar = new QProgressBar(this);
                progressBar->setRange(0, 100);
                progressBar->setValue(static_cast<int>(progressPtr->percentage * 100));
                leftLayout->addWidget(progressBar);
                
                progressBars.push_back(progressBar);
                activeProcesses.push_back(progressPtr);
            }
            else if (progressPtr->processed) {
                QLabel* label = new QLabel(QString::fromStdString(progressPtr->fileName), this);
                rightLayout->addWidget(label);
            }
        }
        
        leftLayout->addStretch();
        rightLayout->addStretch();
        
        leftScrollArea->setWidget(leftContent);
        rightScrollArea->setWidget(rightContent);
    }

    protected:
    void closeEvent(QCloseEvent *event) override {
        emit aboutToClose();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        event->accept();
        QWidget::closeEvent(event);
    }


};

#endif // PROGRESSWIDGET_H