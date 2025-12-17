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
#include "Histogram2DPlot.h" // Add this include
#include "ChannelEntry.h"
#include <TTree.h>
#include <TFile.h>
#include <TROOT.h>
#include <QTimer>
#include "ProgressDialog.h"

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

    void loadRootFile(const QString &filePath);
    void setAnalysisThreadPool(ctpl::thread_pool *pool);

signals:
    void progressUpdated(int value);

public slots:
    void updateHistograms();
    void update2DHistogram(); // New slot for 2D histogram
    void onChannelChanged(int channel);
    void onHistogramSelectionChanged();
    void onOpenRootFile();
    void setEventValues(uint32_t amplitude, float charge, float time);
    void onTimeout();

private slots:
    void createProgressDialog();
    void closeProgressDialog();
    void setupProgressTimer();

private:
    void setupUI();
    void processHistogramData();
    void updateHistogramVisibility();
    void ensureProgressDialogVisible();

private:
    // Histogram plots
    HistogramPlot *m_amplitudePlot;
    HistogramPlot *m_chargePlot;
    HistogramPlot *m_timePlot;
    Histogram2DPlot *m_amplitudeVsChargePlot; // New 2D histogram

    ProgressDialog *m_progressDialog = nullptr;
    QTimer *m_progressTimer = nullptr;

    // Layout and widgets
    QVBoxLayout *m_mainLayout;
    QWidget *m_centralWidget;

    // Controls
    QLabel *m_eventTimeLabel;
    QSpinBox *m_channelSpinBox;
    QPushButton *m_updateButton;
    QPushButton *m_update2DButton; // New button for 2D histogram
    QComboBox *m_histogramSelectionCombo;
    QListWidget *m_histogramListWidget;

    // Menu action
    QAction *m_openRootFileAction;

    // Checkbox items
    QListWidgetItem *m_amplitudeItem;
    QListWidgetItem *m_chargeItem;
    QListWidgetItem *m_timeItem;
    QListWidgetItem *m_amplitudeVsChargeItem; // New item for 2D histogram

    // File path display
    QLabel *m_filePathLabel;

    QString m_currentRootFile;
    int m_currentChannel;

    // Data storage
    TFile *RootDataFile = nullptr;
    TTree *RootDataTree = nullptr;
    std::vector<uint32_t> m_amplitudeData;
    std::vector<float> m_chargeData;
    std::vector<float> m_timeData;
    uint32_t event_amplitude = 0;
    float event_charge = 0;
    float event_time = 0;
    std::atomic<bool> m_dataLoaded;
    float rootLoadedpercentage = 0;

    ctpl::thread_pool *m_threadPool;
};

#endif // HISTOGRAMWINDOW_H