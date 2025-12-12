#include "HistogramWindow.h"
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCoreApplication>
#include <thread>
#include <random>
#include "ProgressDialog.h"
#include "ctpl_stl.h"
#include "UltraFastHistogramReader.h"

HistogramWindow::HistogramWindow(QWidget *parent)
    : QMainWindow(parent),
      m_amplitudePlot(new HistogramPlot("Amplitude Histogram", "Amplitude (ADC Channels)", this)),
      m_chargePlot(new HistogramPlot("Charge Histogram", "Charge", this)),
      m_timePlot(new HistogramPlot("Time Histogram", "Time", this)),
      m_mainLayout(new QVBoxLayout()),
      m_centralWidget(new QWidget(this)),
      m_eventTimeLabel(new QLabel("Current Event Time: N/A", this)),
      m_channelSpinBox(new QSpinBox(this)),
      m_updateButton(new QPushButton("Update Histograms", this)),
      m_histogramSelectionCombo(new QComboBox(this)),
      m_histogramListWidget(new QListWidget(this)),
      m_openRootFileAction(new QAction("Open ROOT File", this)),
      m_filePathLabel(new QLabel("No ROOT file selected", this)),
      m_currentRootFile(""),
      m_currentChannel(0),
      m_dataLoaded(false),
      m_threadPool(nullptr),
      m_progressDialog(nullptr),
      m_progressTimer(nullptr),
      rootLoadedpercentage(0.0f)
{
    setupUI();

    // Set initial ranges
    m_amplitudePlot->setRange(0, 60000);
    m_chargePlot->setRange(0, 10000000);
    m_timePlot->setRange(0, 10000);

    // Connect signals
    connect(m_updateButton, &QPushButton::clicked, this, &HistogramWindow::updateHistograms);
    connect(m_histogramListWidget, &QListWidget::itemChanged, this, &HistogramWindow::onHistogramSelectionChanged);

    // Connect menu action to slot
    connect(m_openRootFileAction, &QAction::triggered, this, &HistogramWindow::onOpenRootFile);

    setWindowTitle("Histogram Analysis");
    resize(800, 900);

    // Add menu action
    menuBar()->addAction(m_openRootFileAction);
}

HistogramWindow::~HistogramWindow()
{
    // Clean up progress dialog and timer
    closeProgressDialog();
    
    if (RootDataFile) {
        RootDataFile->Close();
        delete RootDataFile;
        RootDataFile = nullptr;
    }

    RootDataTree = nullptr;
    
    // Clear data vectors
    m_amplitudeData.clear();
    m_chargeData.clear();
    m_timeData.clear();
}

void HistogramWindow::createProgressDialog()
{
    if (m_progressDialog) {
        // Clean up existing dialog first
        closeProgressDialog();
    }
    
    m_progressDialog = new ProgressDialog(this);
    m_progressDialog->setWindowTitle(QString("Processing ROOT File - Channel %1").arg(m_currentChannel));
    m_progressDialog->setWindowModality(Qt::ApplicationModal);
    
    // Connect progress updates
    connect(this, &HistogramWindow::progressUpdated, m_progressDialog, 
            &ProgressDialog::updateProgress, Qt::QueuedConnection);
    
    // Ensure it stays on top
    m_progressDialog->setWindowFlags(m_progressDialog->windowFlags() | Qt::WindowStaysOnTopHint);
}

void HistogramWindow::closeProgressDialog()
{
    if (m_progressDialog) {
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }
    
    if (m_progressTimer) {
        m_progressTimer->stop();
        m_progressTimer->deleteLater();
        m_progressTimer = nullptr;
    }
}

void HistogramWindow::setupProgressTimer()
{
    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, &HistogramWindow::onTimeout);
    m_progressTimer->start(100); // Update every 100ms
}

void HistogramWindow::ensureProgressDialogVisible()
{
    if (m_progressDialog && !m_progressDialog->isVisible()) {
        m_progressDialog->show();
        m_progressDialog->raise();
        m_progressDialog->activateWindow();
    }
}

void HistogramWindow::setupUI()
{
    // Create control panel
    QWidget *controlWidget = new QWidget(this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlWidget);
    controlWidget->setFixedHeight(60);

    // Channel selection
    QLabel *channelLabel = new QLabel("Channel:", this);
    m_channelSpinBox->setRange(0, 511);
    m_channelSpinBox->setValue(0);
    // Make the channel spinbox read-only - it should only be controlled by the main window
    m_channelSpinBox->setReadOnly(true);
    m_channelSpinBox->setEnabled(false);

    // Histogram selection combo box
    m_histogramSelectionCombo->setFixedWidth(200);
    m_histogramSelectionCombo->setToolTip("Select which histograms to display");

    // Setup the list widget as the combo box's view
    m_histogramListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_histogramSelectionCombo->setModel(m_histogramListWidget->model());
    m_histogramSelectionCombo->setView(m_histogramListWidget);

    // Add checkable items to the list widget
    m_amplitudeItem = new QListWidgetItem("Amplitude Histogram", m_histogramListWidget);
    m_amplitudeItem->setCheckState(Qt::Checked);
    m_amplitudeItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

    m_chargeItem = new QListWidgetItem("Charge Histogram", m_histogramListWidget);
    m_chargeItem->setCheckState(Qt::Unchecked);
    m_chargeItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

    m_timeItem = new QListWidgetItem("Time Histogram", m_histogramListWidget);
    m_timeItem->setCheckState(Qt::Unchecked);
    m_timeItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

    // Set initial combo box text
    m_histogramSelectionCombo->setEditText("Amplitude Histogram");

    // Histogram selection label
    QLabel *histogramLabel = new QLabel("Show:", this);

    controlLayout->addWidget(histogramLabel);
    controlLayout->addWidget(m_histogramSelectionCombo);
    controlLayout->addWidget(channelLabel);
    controlLayout->addWidget(m_channelSpinBox);
    controlLayout->addWidget(m_updateButton);
    controlLayout->addStretch();
    controlLayout->addWidget(m_eventTimeLabel);
    controlLayout->addWidget(m_filePathLabel);

    // Main layout
    m_mainLayout->addWidget(controlWidget);
    m_mainLayout->addWidget(m_amplitudePlot);
    m_mainLayout->addWidget(m_chargePlot);
    m_mainLayout->addWidget(m_timePlot);

    m_centralWidget->setLayout(m_mainLayout);
    setCentralWidget(m_centralWidget);

    // Initially update visibility based on selections
    updateHistogramVisibility();
}

void HistogramWindow::updateHistogramVisibility()
{
    // Update plot visibility based on checkbox states
    m_amplitudePlot->setVisible(m_amplitudeItem->checkState() == Qt::Checked);
    m_chargePlot->setVisible(m_chargeItem->checkState() == Qt::Checked);
    m_timePlot->setVisible(m_timeItem->checkState() == Qt::Checked);

    // Update combo box text to show selected histograms
    QStringList selectedHistograms;
    if (m_amplitudeItem->checkState() == Qt::Checked)
        selectedHistograms << "Amplitude";
    if (m_chargeItem->checkState() == Qt::Checked)
        selectedHistograms << "Charge";
    if (m_timeItem->checkState() == Qt::Checked)
        selectedHistograms << "Time";

    if (selectedHistograms.isEmpty())
    {
        m_histogramSelectionCombo->setEditText("No histograms selected");
    }
    else
    {
        m_histogramSelectionCombo->setEditText(selectedHistograms.join(", "));
    }
}

void HistogramWindow::setEventValues(uint32_t amplitude, float charge, float time)
{
    event_amplitude = amplitude;
    event_charge = charge;
    event_time = time;

    if (m_amplitudePlot->isVisible())
    {
        m_amplitudePlot->setEventLineVisible(true);
        m_amplitudePlot->setEventValue(amplitude);
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
}

void HistogramWindow::loadRootFile(const QString &filePath)
{
    // Clean up previous ROOT objects
    if (RootDataFile) {
        RootDataFile->Close();
        delete RootDataFile;
        RootDataFile = nullptr;
    }

    RootDataTree = nullptr;

    m_currentRootFile = filePath;
    m_dataLoaded = false;

    // Clear existing data
    m_amplitudeData.clear();
    m_chargeData.clear();
    m_timeData.clear();

    // Clean up any existing progress dialog/timer
    closeProgressDialog();

    // Update file path label
    if (!filePath.isEmpty())
    {
        QFileInfo fileInfo(filePath);
        m_filePathLabel->setText("File: " + fileInfo.fileName());
        RootDataFile = TFile::Open((TString)(filePath.toUtf8().constData()));
        if (RootDataFile && !RootDataFile->IsZombie()) {
            RootDataTree = dynamic_cast<TTree*>(RootDataFile->Get("adc64_data"));
        }

        if (!RootDataTree) {
            QMessageBox::warning(this, "Error", "Failed to load ROOT tree from file");
            // Reset file path since load failed
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
                                      QMessageBox::warning(this, "Error", "No ROOT file specified"); 
                                  });
        return;
    }

    // Reset progress percentage
    rootLoadedpercentage = 0.0f;

    // Create and show progress dialog FIRST, before starting the worker
    QMetaObject::invokeMethod(this, [this]() {
        // Clean up any existing dialog first
        closeProgressDialog();
        
        // Create and show new progress dialog
        m_progressDialog = new ProgressDialog(this);
        m_progressDialog->setWindowTitle(QString("Processing ROOT File - Channel %1").arg(m_currentChannel));
        m_progressDialog->setWindowModality(Qt::ApplicationModal);
        m_progressDialog->setWindowFlags(m_progressDialog->windowFlags() | Qt::WindowStaysOnTopHint);
        
        // Show dialog
        m_progressDialog->show();
        m_progressDialog->raise();
        m_progressDialog->activateWindow();
        
        // Force immediate paint
        m_progressDialog->updateProgress(0);
        m_progressDialog->repaint();
        
        // Process events to ensure dialog is visible
        QApplication::processEvents();
        
    }, Qt::BlockingQueuedConnection);

    // Now setup the timer and connections in main thread
    QMetaObject::invokeMethod(this, [this]() {
        m_progressTimer = new QTimer(this);
        connect(m_progressTimer, &QTimer::timeout, this, &HistogramWindow::onTimeout);
        m_progressTimer->start(50); // Update every 50ms for smoother progress
        
        // Connect progress signal AFTER dialog is created
        connect(this, &HistogramWindow::progressUpdated, m_progressDialog, 
                &ProgressDialog::updateProgress, Qt::QueuedConnection);
        
        // Send initial update
        emit progressUpdated(0);
        
    }, Qt::BlockingQueuedConnection);

    try
    {
        // Clear data vectors
        m_amplitudeData.clear();
        m_chargeData.clear();
        m_timeData.clear();

        // Create UltraFastHistogramReader
        UltraFastHistogramReader reader(RootDataTree, m_currentChannel);
        
        // Store the last progress value to avoid too many updates
        float lastReportedProgress = 0.0f;
        
        // Use the mixed version that handles uint32_t for amplitude
        reader.readDataAllAtOnceMixed(m_amplitudeData, m_chargeData, m_timeData,
            [this, &lastReportedProgress](float progress) {
                // Only update if progress changed by at least 0.5%
                if (progress - lastReportedProgress >= 0.005f || progress >= 1.0f) {
                    rootLoadedpercentage = progress;
                    lastReportedProgress = progress;
                    
                    // Emit signal to update progress dialog
                    emit progressUpdated(static_cast<int>(1000 * progress));
                }
            });

        // Processing completed successfully
        m_dataLoaded = true;

        // Send final update to ensure 100% is shown
        emit progressUpdated(1000);
        
        // Small delay to ensure final update is shown
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Clean up progress dialog in main thread
        QMetaObject::invokeMethod(this, [this]() {
            if (m_progressTimer) {
                m_progressTimer->stop();
                m_progressTimer->deleteLater();
                m_progressTimer = nullptr;
            }
            
            if (m_progressDialog) {
                // Update to 100% one more time
                m_progressDialog->updateProgress(1000);
                m_progressDialog->repaint();
                QApplication::processEvents();
                
                // Close after a brief delay so user can see 100%
                QTimer::singleShot(200, m_progressDialog, [this]() {
                    m_progressDialog->close();
                    m_progressDialog->deleteLater();
                    m_progressDialog = nullptr;
                });
            }
        }, Qt::QueuedConnection);

        // Update histograms in main thread
        QMetaObject::invokeMethod(this, &HistogramWindow::updateHistograms, Qt::QueuedConnection);
    }
    catch (const std::exception &e)
    {
        // Clean up on error
        QMetaObject::invokeMethod(this, [this, e]()
                                  { 
                                      if (m_progressTimer) {
                                          m_progressTimer->stop();
                                          m_progressTimer->deleteLater();
                                          m_progressTimer = nullptr;
                                      }
                                      
                                      if (m_progressDialog) {
                                          m_progressDialog->close();
                                          m_progressDialog->deleteLater();
                                          m_progressDialog = nullptr;
                                      }
                                      
                                      QMessageBox::critical(this, "Error", 
                                          QString("Failed to process ROOT file: %1").arg(e.what())); 
                                  });
    }
}

void HistogramWindow::onTimeout()
{
    // This slot runs in the main thread
    if (rootLoadedpercentage < 1.0f) {
        // Use the stored progress value
        int progressValue = static_cast<int>(1000 * rootLoadedpercentage);
        
        // Update the dialog directly (since we're in the main thread)
        if (m_progressDialog) {
            m_progressDialog->updateProgress(progressValue);
            
            // Force repaint for immediate update
            m_progressDialog->repaint();
            
            // Process a few events to keep GUI responsive
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        
        // Also emit the signal for any other connections
        emit progressUpdated(progressValue);
    }
}

void HistogramWindow::updateHistograms()
{
    if (!m_dataLoaded)
        return;

    // Update amplitude histogram if visible
    if (m_amplitudePlot->isVisible() && !m_amplitudeData.empty())
    {
        std::vector<float> ampData(m_amplitudeData.begin(), m_amplitudeData.end());
        m_amplitudePlot->setData(ampData);
        m_amplitudePlot->setEventLineVisible(true);
        m_amplitudePlot->setEventValue(event_amplitude);
        m_amplitudePlot->updatePlot();
    }

    // Update charge histogram if visible
    if (m_chargePlot->isVisible() && !m_chargeData.empty())
    {
        std::vector<float> chargeData(m_chargeData.begin(), m_chargeData.end());
        m_chargePlot->setData(chargeData);
        m_chargePlot->setEventLineVisible(true);
        m_chargePlot->setEventValue(event_charge);
        m_chargePlot->updatePlot();
    }

    // Update time histogram if visible
    if (m_timePlot->isVisible() && !m_timeData.empty())
    {
        m_timePlot->setData(m_timeData);
        m_timePlot->setEventLineVisible(true);
        m_timePlot->setEventValue(event_time);
        m_timePlot->updatePlot();
    }
}

void HistogramWindow::onChannelChanged(int channel)
{
    // Only process if channel actually changed
    if (m_currentChannel == channel && m_dataLoaded) {
        return;
    }
    
    m_currentChannel = channel;
    // Update the spinbox value directly without triggering signals
    m_channelSpinBox->blockSignals(true);
    m_channelSpinBox->setValue(channel);
    m_channelSpinBox->blockSignals(false);

    // Reset data when changing channels
    if (!m_currentRootFile.isEmpty() && RootDataTree)
    {     
        // Reset data loaded flag to force reprocessing
        m_dataLoaded = false;
        
        // Clear existing data
        m_amplitudeData.clear();
        m_chargeData.clear();
        m_timeData.clear();
        
        // Reprocess for new channel
        if (m_threadPool)
        {
            m_threadPool->push([this](int id) { processHistogramData(); });
        }
        else
        {
            processHistogramData();
        }
    }
}

void HistogramWindow::onHistogramSelectionChanged()
{
    updateHistogramVisibility();

    if (m_dataLoaded)
    {
        updateHistograms();
    }
}

void HistogramWindow::onOpenRootFile()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "Select ROOT File",
                                                    QString(),
                                                    "ROOT Files (*.root);;All Files (*)");

    if (!filePath.isEmpty())
    {
        loadRootFile(filePath);
    }
}
