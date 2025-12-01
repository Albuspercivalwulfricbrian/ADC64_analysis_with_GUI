#ifndef HISTOGRAMWINDOW_H
#define HISTOGRAMWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QCheckBox>
#include <QThread>
#include <atomic>
#include <QFileDialog>
#include <QFileInfo>
#include "HistogramPlot.h"

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
    void updateHistograms();
    void onChannelChanged(int channel);
    void onHistogramSelectionChanged();
    void onOpenRootFile();
    void updateHistogramValues(uint32_t amplitude, float charge, float time_index);

private:
    void setupUI();
    void processHistogramData();
    void updateHistogramVisibility();

private:
    // Histogram plots
    HistogramPlot *m_amplitudePlot;
    HistogramPlot *m_chargePlot;
    HistogramPlot *m_timePlot;

    // Layout and widgets
    QVBoxLayout *m_mainLayout;
    QWidget *m_centralWidget;

    // Controls
    QLabel *m_eventTimeLabel;
    QSpinBox *m_channelSpinBox;
    QPushButton *m_updateButton;
    QComboBox *m_histogramSelectionCombo;
    QListWidget *m_histogramListWidget;

    // Menu action
    QAction *m_openRootFileAction;

    // Checkbox items
    QListWidgetItem *m_amplitudeItem;
    QListWidgetItem *m_chargeItem;
    QListWidgetItem *m_timeItem;

    // File path display
    QLabel *m_filePathLabel;

    QString m_currentRootFile;
    int m_currentChannel;

    // Data storage
    std::vector<uint32_t> m_amplitudeData;
    std::vector<float> m_chargeData;
    std::vector<float> m_timeData;

    std::atomic<bool> m_dataLoaded;
    ctpl::thread_pool *m_threadPool;
};

#endif // HISTOGRAMWINDOW_H