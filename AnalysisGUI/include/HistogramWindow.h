#ifndef HISTOGRAMWINDOW_H
#define HISTOGRAMWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include "qcustomplot.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QThread>
#include <atomic>

// Forward declaration
namespace ctpl
{
    class thread_pool;
}

class HistogramWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit HistogramWindow(QWidget *parent = nullptr);
    ~HistogramWindow();

    void setCurrentEventTime(float time);
    void loadRootFile(const QString &filePath, int channel);
    void setAnalysisThreadPool(ctpl::thread_pool *pool);

public slots:
    void updateHistogram();
    void onChannelChanged(int channel);
    void onRangeChanged();
    void onLogScaleToggled(bool checked);

private:
    void setupUI();
    void setupHistogram();
    void processHistogramData();

private:
    QCustomPlot *m_customPlot;
    QVBoxLayout *m_mainLayout;
    QWidget *m_centralWidget;

    QCPItemStraightLine *m_eventLine;
    QLabel *m_eventTimeLabel;
    QSpinBox *m_channelSpinBox;
    QLineEdit *m_minAmpEdit;
    QLineEdit *m_maxAmpEdit;
    QLineEdit *m_binsEdit;
    QPushButton *m_updateButton;
    QCheckBox *m_logScaleCheck;

    QString m_currentRootFile;
    int m_currentChannel;
    std::vector<float> m_amplitudeData;
    std::vector<float> m_eventTimes;

    std::atomic<bool> m_dataLoaded;
    ctpl::thread_pool *m_threadPool;

    QPen m_eventLinePen;
};

#endif // HISTOGRAMWINDOW_H