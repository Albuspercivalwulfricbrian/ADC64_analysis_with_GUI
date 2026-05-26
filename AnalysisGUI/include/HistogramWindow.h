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
#include "Histogram2DPlot.h"
#include "ChannelEntry.h"
#include <TTree.h>
#include <TFile.h>
#include <TROOT.h>
#include <QTimer>
#include "ProgressDialog.h"

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
    void setDefaultEventLimit(int64_t limit) { m_defaultEventLimit = limit; }

signals:
    void progressUpdated(int value);

public slots:
    void updateHistograms();
    void update2DHistogram();
    void onChannelChanged(int channel);
    void onHistogramSelectionChanged();
    void onOpenRootFile();
    void setEventValues(uint32_t amplitude, float charge, float time, float r2 = 0.0f);
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
    void update1DHistogram(HistogramPlot *plot, const std::vector<float> &data, float eventValue);
    bool showHistogramEventLimitDialog();

private:
    HistogramPlot *m_amplitudePlot;
    HistogramPlot *m_chargePlot;
    HistogramPlot *m_timePlot;
    Histogram2DPlot *m_amplitudeVsChargePlot;
    Histogram2DPlot *m_r2VsChargePlot; // Добавлен новый 2D график

    ProgressDialog *m_progressDialog = nullptr;
    QTimer *m_progressTimer = nullptr;

    QVBoxLayout *m_mainLayout;
    QWidget *m_centralWidget;

    QLabel *m_eventTimeLabel;
    QSpinBox *m_channelSpinBox;
    QPushButton *m_updateButton;
    QPushButton *m_update2DButton;
    QComboBox *m_histogramSelectionCombo;
    QListWidget *m_histogramListWidget;

    QAction *m_openRootFileAction;

    QListWidgetItem *m_amplitudeItem;
    QListWidgetItem *m_chargeItem;
    QListWidgetItem *m_timeItem;
    QListWidgetItem *m_amplitudeVsChargeItem;
    QListWidgetItem *m_r2VsChargeItem; // Добавлен новый чекбокс

    QLabel *m_filePathLabel;

    QString m_currentRootFile;
    int m_currentChannel;

    TFile *RootDataFile = nullptr;
    TTree *RootDataTree = nullptr;
    std::vector<float> m_amplitudeData;
    std::vector<float> m_chargeData;
    std::vector<float> m_timeData;
    std::vector<float> m_oneMinusR2Data; // Добавлен вектор данных для 1-R2

    uint32_t event_amplitude = 0;
    float event_charge = 0;
    float event_time = 0;
    float event_r2 = 0.0f; // Add this line to store R² value

    std::atomic<bool> m_dataLoaded;
    float rootLoadedpercentage = 0;
    int64_t m_histogramEventLimit;
    int64_t m_defaultEventLimit;
    ctpl::thread_pool *m_threadPool;
};

#endif // HISTOGRAMWINDOW_H
