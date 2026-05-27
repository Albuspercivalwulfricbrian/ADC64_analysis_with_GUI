#include "HistogramWindow.h"
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCoreApplication>
#include <thread>
#include <random>
#include <QApplication>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QIntValidator>
#include <QMenuBar>
#include "ProgressDialog.h"
#include "ctpl_stl.h"
#include "UltraFastHistogramReader.h"

HistogramWindow::HistogramWindow(QWidget *parent)
    : QMainWindow(parent),
      m_amplitudePlot(new HistogramPlot("Amplitude Histogram", "Amplitude (ADC Channels)", this)),
      m_chargePlot(new HistogramPlot("Charge Histogram", "Charge", this)),
      m_timePlot(new HistogramPlot("Time Histogram", "Time", this)),
      m_amplitudeVsChargePlot(new Histogram2DPlot("Amplitude vs Charge Correlation", "Amplitude (ADC Channels)", "Charge", this)),
      m_r2VsChargePlot(new Histogram2DPlot("1-R2 vs Charge Correlation", "1 - R2", "Charge", this)),
      m_mainLayout(new QVBoxLayout()),
      m_centralWidget(new QWidget(this)),
      m_eventTimeLabel(new QLabel("Current Event Time: N/A", this)),
      m_channelSpinBox(new QSpinBox(this)),
      m_updateButton(new QPushButton("Update Histograms", this)),
      m_update2DButton(new QPushButton("Update 2D Histogram", this)),
      m_histogramSelectionCombo(new QComboBox(this)),
      m_histogramListWidget(new QListWidget(this)),
      m_openRootFileAction(new QAction("Open ROOT File", this)),
      m_filePathLabel(new QLabel("No ROOT file selected", this)),
      m_r2VsChargeItem(nullptr),
      m_currentRootFile(""),
      m_currentChannel(0),
      m_dataLoaded(false),
      m_threadPool(nullptr),
      m_progressDialog(nullptr),
      m_progressTimer(nullptr),
      rootLoadedpercentage(0.0f)
{
    setupUI();

    m_amplitudePlot->setRange(10, 60000);
    m_chargePlot->setRange(1000, 10000000);
    m_timePlot->setRange(0, 10000);

    m_amplitudeVsChargePlot->setXRange(0, 60000);
    m_amplitudeVsChargePlot->setYRange(0, 10000000);
    m_amplitudeVsChargePlot->setXBins(300);
    m_amplitudeVsChargePlot->setYBins(300);
    m_amplitudeVsChargePlot->setLogZScale(false);
    m_amplitudeVsChargePlot->setAutoRangeOnDataChange(false);

    m_r2VsChargePlot->setXRange(0, 5);
    m_r2VsChargePlot->setYRange(1000, 10000000);
    m_r2VsChargePlot->setXBins(300);
    m_r2VsChargePlot->setYBins(300);
    m_r2VsChargePlot->setLogZScale(false);
    m_r2VsChargePlot->setAutoRangeOnDataChange(false);

    m_histogramEventLimit = -1;

    connect(m_updateButton, &QPushButton::clicked, this, &HistogramWindow::updateHistograms);
    connect(m_update2DButton, &QPushButton::clicked, this, &HistogramWindow::update2DHistogram);
    connect(m_histogramListWidget, &QListWidget::itemChanged, this, &HistogramWindow::onHistogramSelectionChanged);
    connect(m_openRootFileAction, &QAction::triggered, this, &HistogramWindow::onOpenRootFile);

    setWindowTitle("Histogram Analysis");
    resize(1000, 1200);
    menuBar()->addAction(m_openRootFileAction);
}

HistogramWindow::~HistogramWindow()
{
    closeProgressDialog();
    if (RootDataFile)
    {
        RootDataFile->Close();
        delete RootDataFile;
        RootDataFile = nullptr;
    }
    RootDataTree = nullptr;
    m_amplitudeData.clear();
    m_chargeData.clear();
    m_timeData.clear();
    m_oneMinusR2Data.clear();
}

void HistogramWindow::createProgressDialog()
{
    if (m_progressDialog)
    {
        closeProgressDialog();
    }
    m_progressDialog = new ProgressDialog(this);
    m_progressDialog->setWindowTitle(QString("Processing ROOT File - Channel %1").arg(m_currentChannel));
    m_progressDialog->setWindowModality(Qt::ApplicationModal);
    connect(this, &HistogramWindow::progressUpdated, m_progressDialog, &ProgressDialog::updateProgress, Qt::QueuedConnection);
    m_progressDialog->setWindowFlags(m_progressDialog->windowFlags() | Qt::WindowStaysOnTopHint);
}

void HistogramWindow::closeProgressDialog()
{
    if (m_progressDialog)
    {
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }
    if (m_progressTimer)
    {
        m_progressTimer->stop();
        m_progressTimer->deleteLater();
        m_progressTimer = nullptr;
    }
}

void HistogramWindow::setupProgressTimer()
{
    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, &HistogramWindow::onTimeout);
    m_progressTimer->start(100);
}

void HistogramWindow::ensureProgressDialogVisible()
{
    if (m_progressDialog && !m_progressDialog->isVisible())
    {
        m_progressDialog->show();
        m_progressDialog->raise();
        m_progressDialog->activateWindow();
    }
}

void HistogramWindow::setupUI()
{
    QWidget *controlWidget = new QWidget(this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlWidget);
    controlWidget->setFixedHeight(60);

    QLabel *channelLabel = new QLabel("Channel:", this);
    m_channelSpinBox->setRange(0, 511);
    m_channelSpinBox->setValue(0);
    m_channelSpinBox->setReadOnly(true);
    m_channelSpinBox->setEnabled(false);

    m_histogramSelectionCombo->setFixedWidth(200);
    m_histogramListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_histogramSelectionCombo->setModel(m_histogramListWidget->model());
    m_histogramSelectionCombo->setView(m_histogramListWidget);

    m_amplitudeItem = new QListWidgetItem("Amplitude Histogram", m_histogramListWidget);
    m_amplitudeItem->setCheckState(Qt::Unchecked);
    m_amplitudeItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

    m_chargeItem = new QListWidgetItem("Charge Histogram", m_histogramListWidget);
    m_chargeItem->setCheckState(Qt::Unchecked);
    m_chargeItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

    m_timeItem = new QListWidgetItem("Time Histogram", m_histogramListWidget);
    m_timeItem->setCheckState(Qt::Unchecked);
    m_timeItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

    m_amplitudeVsChargeItem = new QListWidgetItem("Amplitude vs Charge (2D)", m_histogramListWidget);
    m_amplitudeVsChargeItem->setCheckState(Qt::Unchecked);
    m_amplitudeVsChargeItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

    m_r2VsChargeItem = new QListWidgetItem("1-R2 vs Charge (2D)", m_histogramListWidget);
    m_r2VsChargeItem->setCheckState(Qt::Unchecked);
    m_r2VsChargeItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

    m_histogramSelectionCombo->setEditText("No histograms selected");
    QLabel *histogramLabel = new QLabel("Show:", this);

    controlLayout->addWidget(histogramLabel);
    controlLayout->addWidget(m_histogramSelectionCombo);
    controlLayout->addWidget(channelLabel);
    controlLayout->addWidget(m_channelSpinBox);
    controlLayout->addWidget(m_updateButton);
    controlLayout->addWidget(m_update2DButton);
    controlLayout->addStretch();
    controlLayout->addWidget(m_eventTimeLabel);
    controlLayout->addWidget(m_filePathLabel);

    m_mainLayout->addWidget(controlWidget);
    m_mainLayout->addWidget(m_amplitudePlot);
    m_mainLayout->addWidget(m_chargePlot);
    m_mainLayout->addWidget(m_timePlot);
    m_mainLayout->addWidget(m_amplitudeVsChargePlot);
    m_mainLayout->addWidget(m_r2VsChargePlot);

    m_centralWidget->setLayout(m_mainLayout);
    setCentralWidget(m_centralWidget);

    updateHistogramVisibility();
}

void HistogramWindow::updateHistogramVisibility()
{
    m_amplitudePlot->setVisible(m_amplitudeItem->checkState() == Qt::Checked);
    m_chargePlot->setVisible(m_chargeItem->checkState() == Qt::Checked);
    m_timePlot->setVisible(m_timeItem->checkState() == Qt::Checked);
    m_amplitudeVsChargePlot->setVisible(m_amplitudeVsChargeItem->checkState() == Qt::Checked);
    m_r2VsChargePlot->setVisible(m_r2VsChargeItem->checkState() == Qt::Checked);

    QStringList selectedHistograms;
    if (m_amplitudeItem->checkState() == Qt::Checked)
        selectedHistograms << "Amplitude";
    if (m_chargeItem->checkState() == Qt::Checked)
        selectedHistograms << "Charge";
    if (m_timeItem->checkState() == Qt::Checked)
        selectedHistograms << "Time";
    if (m_amplitudeVsChargeItem->checkState() == Qt::Checked)
        selectedHistograms << "Amplitude vs Charge";
    if (m_r2VsChargeItem->checkState() == Qt::Checked)
        selectedHistograms << "1-R2 vs Charge";

    if (selectedHistograms.isEmpty())
    {
        m_histogramSelectionCombo->setEditText("No histograms selected");
    }
    else
    {
        m_histogramSelectionCombo->setEditText(selectedHistograms.join(", "));
    }
}
void HistogramWindow::setEventValues(uint32_t amplitude, float charge, float time, float r2)
{
    event_amplitude = amplitude;
    event_charge = charge;
    event_time = time;
    event_r2 = r2; // Store the R² value

    if (m_amplitudePlot->isVisible())
    {
        m_amplitudePlot->setEventLineVisible(true);
        m_amplitudePlot->setEventValue(static_cast<float>(amplitude));
    }
    if (m_chargePlot->isVisible())
    {
        m_chargePlot->setEventLineVisible(true);
        m_chargePlot->setEventValue(charge);
    }
    if (m_timePlot->isVisible())
    {
        m_timePlot->setEventLineVisible(true);
        m_timePlot->setEventValue(time);
    }
    if (m_amplitudeVsChargePlot->isVisible())
    {
        m_amplitudeVsChargePlot->setEventMarkerVisible(true);
        m_amplitudeVsChargePlot->setEventValue(static_cast<float>(amplitude), charge);
    }
    // Add this block for the 1-R² vs charge plot
    if (m_r2VsChargePlot->isVisible())
    {
        m_r2VsChargePlot->setEventMarkerVisible(true);
        // Calculate 1-R² for the plot (since it's labeled as "1-R2 vs Charge")
        float oneMinusR2 = 1.0f - r2;
        m_r2VsChargePlot->setEventValue(oneMinusR2, charge);
    }
}

void HistogramWindow::loadRootFile(const QString &filePath)
{
    if (!showHistogramEventLimitDialog())
    {
        return;
    }

    if (RootDataFile)
    {
        RootDataFile->Close();
        delete RootDataFile;
        RootDataFile = nullptr;
    }
    RootDataTree = nullptr;

    m_currentRootFile = filePath;
    m_dataLoaded = false;

    m_amplitudeData.clear();
    m_chargeData.clear();
    m_timeData.clear();
    m_oneMinusR2Data.clear();

    closeProgressDialog();

    if (!filePath.isEmpty())
    {
        QFileInfo fileInfo(filePath);
        m_filePathLabel->setText("File: " + fileInfo.fileName());
        RootDataFile = TFile::Open((TString)(filePath.toUtf8().constData()));
        if (RootDataFile && !RootDataFile->IsZombie())
        {
            RootDataTree = dynamic_cast<TTree *>(RootDataFile->Get("adc64_data"));
        }

        if (!RootDataTree)
        {
            QMessageBox::warning(this, "Error", "Failed to load ROOT tree from file");
            m_currentRootFile.clear();
            return;
        }
    }
    else
    {
        m_filePathLabel->setText("No ROOT file selected");
    }

    if (m_threadPool)
    {
        m_threadPool->push([this](int id)
                           { processHistogramData(); });
    }
    else
    {
        processHistogramData();
    }
}

void HistogramWindow::setAnalysisThreadPool(ctpl::thread_pool *pool)
{
    m_threadPool = pool;
}

void HistogramWindow::processHistogramData()
{
    if (m_currentRootFile.isEmpty() || !RootDataTree)
    {
        QMetaObject::invokeMethod(this, [this]()
                                  { 
            closeProgressDialog();
            QMessageBox::warning(this, "Error", "No ROOT file specified"); });
        return;
    }

    rootLoadedpercentage = 0.0f;

    QString progressTitle = (m_histogramEventLimit == -1) ? QString("Processing ROOT File (All Events) - Channel %1").arg(m_currentChannel) : QString("Processing ROOT File (Max %1 Events) - Channel %2").arg(m_histogramEventLimit).arg(m_currentChannel);

    QMetaObject::invokeMethod(this, [this, progressTitle]()
                              {
        closeProgressDialog();
        m_progressDialog = new ProgressDialog(this);
        m_progressDialog->setWindowTitle(progressTitle);
        m_progressDialog->setWindowModality(Qt::ApplicationModal);
        m_progressDialog->setWindowFlags(m_progressDialog->windowFlags() | Qt::WindowStaysOnTopHint);
        m_progressDialog->show();
        m_progressDialog->raise();
        m_progressDialog->activateWindow();
        m_progressDialog->updateProgress(0);
        m_progressDialog->repaint();
        QApplication::processEvents(); }, Qt::BlockingQueuedConnection);

    QMetaObject::invokeMethod(this, [this]()
                              {
        m_progressTimer = new QTimer(this);
        connect(m_progressTimer, &QTimer::timeout, this, &HistogramWindow::onTimeout);
        m_progressTimer->start(50);
        connect(this, &HistogramWindow::progressUpdated, m_progressDialog, &ProgressDialog::updateProgress, Qt::QueuedConnection);
        emit progressUpdated(0); }, Qt::BlockingQueuedConnection);

    try
    {
        m_amplitudeData.clear();
        m_chargeData.clear();
        m_timeData.clear();
        m_oneMinusR2Data.clear();

        UltraFastHistogramReader reader(RootDataTree, m_currentChannel);
        float lastReportedProgress = 0.0f;

        Long64_t totalEntries = RootDataTree->GetEntries();
        Long64_t entriesToRead = totalEntries;

        if (m_histogramEventLimit > 0 && m_histogramEventLimit < totalEntries)
        {
            entriesToRead = m_histogramEventLimit;
        }

        reader.readData(m_amplitudeData, m_chargeData, m_timeData, m_oneMinusR2Data, [this, &lastReportedProgress](float progress)
                        {
            if (progress - lastReportedProgress >= 0.005f || progress >= 1.0f) {
                rootLoadedpercentage = progress;
                lastReportedProgress = progress;
                emit progressUpdated(static_cast<int>(1000 * progress));
            } }, entriesToRead);

        m_dataLoaded = true;
        emit progressUpdated(1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        QMetaObject::invokeMethod(this, [this]()
                                  {
            if (m_progressTimer) {
                m_progressTimer->stop();
                m_progressTimer->deleteLater();
                m_progressTimer = nullptr;
            }
            if (m_progressDialog) {
                m_progressDialog->updateProgress(1000);
                m_progressDialog->repaint();
                QApplication::processEvents();
                QTimer::singleShot(200, m_progressDialog, [this]() {
                    m_progressDialog->close();
                    m_progressDialog->deleteLater();
                    m_progressDialog = nullptr;
                });
            } }, Qt::QueuedConnection);

        QMetaObject::invokeMethod(this, &HistogramWindow::updateHistograms, Qt::QueuedConnection);

        if (m_amplitudeVsChargeItem->checkState() == Qt::Checked)
        {
            QMetaObject::invokeMethod(this, [this]()
                                      { m_amplitudeVsChargePlot->resetRangesToInitial(); }, Qt::QueuedConnection);
        }
        if (m_r2VsChargeItem->checkState() == Qt::Checked)
        {
            QMetaObject::invokeMethod(this, [this]()
                                      { m_r2VsChargePlot->resetRangesToInitial(); }, Qt::QueuedConnection);
        }
    }
    catch (const std::exception &e)
    {
        QMetaObject::invokeMethod(this, [this, e]()
                                  { 
            if (m_progressTimer) { m_progressTimer->stop(); m_progressTimer->deleteLater(); m_progressTimer = nullptr; }
            if (m_progressDialog) { m_progressDialog->close(); m_progressDialog->deleteLater(); m_progressDialog = nullptr; }
            QMessageBox::critical(this, "Error", QString("Failed to process ROOT file: %1").arg(e.what())); });
    }
}

void HistogramWindow::onTimeout()
{
    if (rootLoadedpercentage < 1.0f)
    {
        int progressValue = static_cast<int>(1000 * rootLoadedpercentage);
        if (m_progressDialog)
        {
            m_progressDialog->updateProgress(progressValue);
            m_progressDialog->repaint();
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        emit progressUpdated(progressValue);
    }
}

void HistogramWindow::updateHistograms()
{
    if (!m_dataLoaded)
        return;

    update1DHistogram(m_amplitudePlot, m_amplitudeData, event_amplitude);
    update1DHistogram(m_chargePlot, m_chargeData, event_charge);
    update1DHistogram(m_timePlot, m_timeData, event_time);
}

void HistogramWindow::update2DHistogram()
{
    if (!m_dataLoaded)
        return;

    if (!m_amplitudeData.empty() && !m_chargeData.empty())
    {
        m_amplitudeVsChargePlot->setAutoRangeOnDataChange(false);
        m_amplitudeVsChargePlot->setData(m_amplitudeData, m_chargeData);
        m_amplitudeVsChargePlot->updatePlot();
        m_amplitudeVsChargePlot->setEventMarkerVisible(true);
        m_amplitudeVsChargePlot->setEventValue(static_cast<float>(event_amplitude), event_charge);
    }

    if (!m_oneMinusR2Data.empty() && !m_chargeData.empty())
    {
        m_r2VsChargePlot->setAutoRangeOnDataChange(false);
        m_r2VsChargePlot->setData(m_oneMinusR2Data, m_chargeData);
        m_r2VsChargePlot->updatePlot();
    }
}

void HistogramWindow::onChannelChanged(int channel)
{
    if (m_currentChannel == channel && m_dataLoaded)
    {
        return;
    }

    m_currentChannel = channel;
    m_channelSpinBox->blockSignals(true);
    m_channelSpinBox->setValue(channel);
    m_channelSpinBox->blockSignals(false);

    if (m_amplitudeVsChargePlot->isVisible())
    {
        m_amplitudeVsChargePlot->resetRangesToInitial();
    }
    if (m_r2VsChargePlot->isVisible())
    {
        m_r2VsChargePlot->resetRangesToInitial();
    }

    if (!m_currentRootFile.isEmpty() && RootDataTree)
    {
        m_dataLoaded = false;
        m_amplitudeData.clear();
        m_chargeData.clear();
        m_timeData.clear();
        m_oneMinusR2Data.clear();

        if (m_threadPool)
        {
            m_threadPool->push([this](int id)
                               { processHistogramData(); });
        }
        else
        {
            processHistogramData();
        }
    }
}

void HistogramWindow::onHistogramSelectionChanged()
{
    bool amplitudeWasVisible = m_amplitudePlot->isVisible();
    bool chargeWasVisible = m_chargePlot->isVisible();
    bool timeWasVisible = m_timePlot->isVisible();
    bool amplitudeVsChargeWasVisible = m_amplitudeVsChargePlot->isVisible();
    bool r2VsChargeWasVisible = m_r2VsChargePlot->isVisible();

    updateHistogramVisibility();

    if (m_dataLoaded)
    {
        if (amplitudeWasVisible != m_amplitudePlot->isVisible() || m_amplitudePlot->isVisible())
        {
            update1DHistogram(m_amplitudePlot, m_amplitudeData, event_amplitude);
        }
        // ... (inside onHistogramSelectionChanged)
        if (chargeWasVisible != m_chargePlot->isVisible() || m_chargePlot->isVisible())
        {
            update1DHistogram(m_chargePlot, m_chargeData, event_charge);
        }
        if (timeWasVisible != m_timePlot->isVisible() || m_timePlot->isVisible())
        {
            update1DHistogram(m_timePlot, m_timeData, event_time);
        }
        if (amplitudeVsChargeWasVisible != m_amplitudeVsChargePlot->isVisible())
        {
            if (m_amplitudeVsChargePlot->isVisible())
            {
                update2DHistogram();
            }
        }
        if (r2VsChargeWasVisible != m_r2VsChargePlot->isVisible())
        {
            if (m_r2VsChargePlot->isVisible())
            {
                update2DHistogram();
            }
        }
    }
}

void HistogramWindow::update1DHistogram(HistogramPlot *plot, const std::vector<float> &data, float eventValue)
{
    if (plot->isVisible() && !data.empty())
    {
        plot->setData(data);
        plot->setEventLineVisible(true);
        plot->setEventValue(eventValue);
        plot->updatePlot();
    }
}

void HistogramWindow::onOpenRootFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select ROOT File", QString(), "ROOT Files (*.root);;All Files (*)");
    if (!filePath.isEmpty())
    {
        loadRootFile(filePath);
    }
}

bool HistogramWindow::showHistogramEventLimitDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Histogram Event Limit");
    dialog.setMinimumWidth(350);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QLabel *titleLabel = new QLabel("<b>Set maximum events for histogram filling:</b>", &dialog);
    layout->addWidget(titleLabel);

    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel *eventsLabel = new QLabel("Events:", &dialog);
    QLineEdit *eventLimitEdit = new QLineEdit(&dialog);

    if (m_defaultEventLimit > 0)
    {
        eventLimitEdit->setText(QString::number(m_defaultEventLimit));
    }
    else
    {
        eventLimitEdit->setText("1000");
    }

    eventLimitEdit->setPlaceholderText("Enter number or check 'All events'");
    eventLimitEdit->setValidator(new QIntValidator(1, 1000000000, this));

    hLayout->addWidget(eventsLabel);
    hLayout->addWidget(eventLimitEdit);
    layout->addLayout(hLayout);

    QCheckBox *allEventsCheckBox = new QCheckBox("Use all events from ROOT file", &dialog);
    allEventsCheckBox->setChecked(m_defaultEventLimit == -1);
    layout->addWidget(allEventsCheckBox);

    QLabel *infoLabel = new QLabel("Default value is from binary file opening settings.", &dialog);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: gray; font-style: italic;");
    layout->addWidget(infoLabel);

    QObject::connect(allEventsCheckBox, &QCheckBox::stateChanged, [eventLimitEdit](int state)
                     {
        eventLimitEdit->setEnabled(state == Qt::Unchecked);
        if (state == Qt::Checked) {
            eventLimitEdit->clear();
        } });

    eventLimitEdit->setEnabled(!allEventsCheckBox->isChecked());

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttonBox);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted)
    {
        if (allEventsCheckBox->isChecked())
        {
            m_histogramEventLimit = -1;
            return true;
        }
        else
        {
            QString text = eventLimitEdit->text().trimmed();
            if (text.isEmpty())
            {
                m_histogramEventLimit = (m_defaultEventLimit > 0) ? m_defaultEventLimit : 1000;
                return true;
            }
            else
            {
                bool ok;
                int64_t value = text.toLongLong(&ok);
                if (ok && value > 0)
                {
                    m_histogramEventLimit = value;
                    return true;
                }
                else
                {
                    QMessageBox::warning(this, "Invalid Input", "Please enter a valid positive number.");
                    return false;
                }
            }
        }
    }
    return false;
}
