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
      m_amplitudeVsChargePlot(new Histogram2DPlot("Amplitude vs Charge Correlation",
                                                  "Amplitude (ADC Channels)",
                                                  "Charge", this)),
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
    m_amplitudePlot->setRange(10, 60000);
    m_chargePlot->setRange(1000, 10000000);
    m_timePlot->setRange(0, 10000);

    // Set initial ranges for 2D histogram
    m_amplitudeVsChargePlot->setXRange(0, 60000);
    m_amplitudeVsChargePlot->setYRange(0, 10000000);
    m_amplitudeVsChargePlot->setXBins(300);
    m_amplitudeVsChargePlot->setYBins(300);
    m_amplitudeVsChargePlot->setLogZScale(false);

    // Disable auto-ranging for 2D plot
    m_amplitudeVsChargePlot->setAutoRangeOnDataChange(false);

    // Initialize histogram event limit to unlimited by default
    m_histogramEventLimit = -1; // -1 means all events

    // Connect signals
    connect(m_updateButton, &QPushButton::clicked, this, &HistogramWindow::updateHistograms);
    connect(m_update2DButton, &QPushButton::clicked, this, &HistogramWindow::update2DHistogram);
    connect(m_histogramListWidget, &QListWidget::itemChanged, this, &HistogramWindow::onHistogramSelectionChanged);

    // Connect menu action to slot
    connect(m_openRootFileAction, &QAction::triggered, this, &HistogramWindow::onOpenRootFile);

    setWindowTitle("Histogram Analysis");
    resize(1000, 1200);

    // Add menu action
    menuBar()->addAction(m_openRootFileAction);
}

HistogramWindow::~HistogramWindow()
{
    // Clean up progress dialog and timer
    closeProgressDialog();

    if (RootDataFile)
    {
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
    if (m_progressDialog)
    {
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
    m_progressTimer->start(100); // Update every 100ms
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

    // Set initial combo box text
    m_histogramSelectionCombo->setEditText("No histograms selected");

    // Histogram selection label
    QLabel *histogramLabel = new QLabel("Show:", this);

    controlLayout->addWidget(histogramLabel);
    controlLayout->addWidget(m_histogramSelectionCombo);
    controlLayout->addWidget(channelLabel);
    controlLayout->addWidget(m_channelSpinBox);
    controlLayout->addWidget(m_updateButton);
    controlLayout->addWidget(m_update2DButton); // Add 2D update button
    controlLayout->addStretch();
    controlLayout->addWidget(m_eventTimeLabel);
    controlLayout->addWidget(m_filePathLabel);

    // Main layout - REMOVED the separate controls line before 2D histogram
    m_mainLayout->addWidget(controlWidget);
    m_mainLayout->addWidget(m_amplitudePlot);
    m_mainLayout->addWidget(m_chargePlot);
    m_mainLayout->addWidget(m_timePlot);
    // No separate controls widget for 2D histogram - controls are built into Histogram2DPlot
    m_mainLayout->addWidget(m_amplitudeVsChargePlot);

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
    m_amplitudeVsChargePlot->setVisible(m_amplitudeVsChargeItem->checkState() == Qt::Checked);

    // Update combo box text to show selected histograms
    QStringList selectedHistograms;
    if (m_amplitudeItem->checkState() == Qt::Checked)
        selectedHistograms << "Amplitude";
    if (m_chargeItem->checkState() == Qt::Checked)
        selectedHistograms << "Charge";
    if (m_timeItem->checkState() == Qt::Checked)
        selectedHistograms << "Time";
    if (m_amplitudeVsChargeItem->checkState() == Qt::Checked)
        selectedHistograms << "Amplitude vs Charge";

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
        m_amplitudePlot->setEventValue(static_cast<float>(amplitude)); // Convert to float
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
        m_amplitudeVsChargePlot->setEventValue(static_cast<float>(amplitude), charge); // Convert to float
    }
}

void HistogramWindow::loadRootFile(const QString &filePath)
{
    // Show event limit dialog first
    if (!showHistogramEventLimitDialog())
    {
        return; // User cancelled
    }

    // Clean up previous ROOT objects
    if (RootDataFile)
    {
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

    // Reset progress percentage
    rootLoadedpercentage = 0.0f;

    // Update progress dialog title with event limit info
    QString progressTitle;
    if (m_histogramEventLimit == -1)
    {
        progressTitle = QString("Processing ROOT File (All Events) - Channel %1").arg(m_currentChannel);
    }
    else
    {
        progressTitle = QString("Processing ROOT File (Max %1 Events) - Channel %2").arg(m_histogramEventLimit).arg(m_currentChannel);
    }

    // Create and show progress dialog
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

    // Setup timer and connections
    QMetaObject::invokeMethod(this, [this]()
                              {
                                  m_progressTimer = new QTimer(this);
                                  connect(m_progressTimer, &QTimer::timeout, this, &HistogramWindow::onTimeout);
                                  m_progressTimer->start(50);
                                  connect(this, &HistogramWindow::progressUpdated, m_progressDialog,
                                          &ProgressDialog::updateProgress, Qt::QueuedConnection);
                                  emit progressUpdated(0); }, Qt::BlockingQueuedConnection);

    try
    {
        // Clear data vectors
        m_amplitudeData.clear();
        m_chargeData.clear();
        m_timeData.clear();

        // Create UltraFastHistogramReader
        UltraFastHistogramReader reader(RootDataTree, m_currentChannel);

        // Store the last progress value
        float lastReportedProgress = 0.0f;

        // Calculate how many entries to read based on limit
        Long64_t totalEntries = RootDataTree->GetEntries();
        Long64_t entriesToRead = totalEntries;

        if (m_histogramEventLimit > 0 && m_histogramEventLimit < totalEntries)
        {
            entriesToRead = m_histogramEventLimit;
        }

        // Use the main readData function with limit
        reader.readData(m_amplitudeData, m_chargeData, m_timeData, [this, &lastReportedProgress](float progress)
                        {
                            if (progress - lastReportedProgress >= 0.005f || progress >= 1.0f)
                            {
                                rootLoadedpercentage = progress;
                                lastReportedProgress = progress;
                                emit progressUpdated(static_cast<int>(1000 * progress));
                            } },
                        entriesToRead); // Pass the limit to readData

        // Processing completed successfully
        m_dataLoaded = true;

        // Send final update
        emit progressUpdated(1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Clean up progress dialog
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

        // Update histograms
        QMetaObject::invokeMethod(this, &HistogramWindow::updateHistograms, Qt::QueuedConnection);

        if (m_amplitudeVsChargeItem->checkState() == Qt::Checked)
        {
            // When data is loaded for a new channel, reset to initial ranges
            QMetaObject::invokeMethod(this, [this]()
                                      { m_amplitudeVsChargePlot->resetRangesToInitial(); }, Qt::QueuedConnection);
        }
    }
    catch (const std::exception &e)
    {
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
                                          QString("Failed to process ROOT file: %1").arg(e.what())); });
    }
}

void HistogramWindow::onTimeout()
{
    // This slot runs in the main thread
    if (rootLoadedpercentage < 1.0f)
    {
        // Use the stored progress value
        int progressValue = static_cast<int>(1000 * rootLoadedpercentage);

        // Update the dialog directly (since we're in the main thread)
        if (m_progressDialog)
        {
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

    // Update all visible histograms
    update1DHistogram(m_amplitudePlot, m_amplitudeData, event_amplitude);
    update1DHistogram(m_chargePlot, m_chargeData, event_charge);
    update1DHistogram(m_timePlot, m_timeData, event_time);

    // Only update 2D plot when explicitly requested (via its update button)
    // Removed the automatic update here
}

void HistogramWindow::update2DHistogram()
{
    if (!m_dataLoaded || m_amplitudeData.empty() || m_chargeData.empty())
        return;

    m_amplitudeVsChargePlot->setAutoRangeOnDataChange(false);

    m_amplitudeVsChargePlot->setData(m_amplitudeData, m_chargeData);
    m_amplitudeVsChargePlot->updatePlot();

    // Ensure crosshair is shown if event values are set
    m_amplitudeVsChargePlot->setEventMarkerVisible(true);
    m_amplitudeVsChargePlot->setEventValue(static_cast<float>(event_amplitude), event_charge);
}

void HistogramWindow::onChannelChanged(int channel)
{
    // Only process if channel actually changed
    if (m_currentChannel == channel && m_dataLoaded)
    {
        return;
    }

    m_currentChannel = channel;
    // Update the spinbox value directly without triggering signals
    m_channelSpinBox->blockSignals(true);
    m_channelSpinBox->setValue(channel);
    m_channelSpinBox->blockSignals(false);

    // Reset 2D plot to initial ranges when channel changes
    if (m_amplitudeVsChargePlot->isVisible())
    {
        m_amplitudeVsChargePlot->resetRangesToInitial();
    }

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
    // Store previous visibility states
    bool amplitudeWasVisible = m_amplitudePlot->isVisible();
    bool chargeWasVisible = m_chargePlot->isVisible();
    bool timeWasVisible = m_timePlot->isVisible();
    bool amplitudeVsChargeWasVisible = m_amplitudeVsChargePlot->isVisible();

    updateHistogramVisibility();

    if (m_dataLoaded)
    {
        // Only update plots whose visibility actually changed
        if (amplitudeWasVisible != m_amplitudePlot->isVisible() || m_amplitudePlot->isVisible())
        {
            update1DHistogram(m_amplitudePlot, m_amplitudeData, event_amplitude);
        }
        if (chargeWasVisible != m_chargePlot->isVisible() || m_chargePlot->isVisible())
        {
            update1DHistogram(m_chargePlot, m_chargeData, event_charge);
        }
        if (timeWasVisible != m_timePlot->isVisible() || m_timePlot->isVisible())
        {
            update1DHistogram(m_timePlot, m_timeData, event_time);
        }

        // Only update 2D plot if its visibility changed to visible
        if (amplitudeVsChargeWasVisible != m_amplitudeVsChargePlot->isVisible())
        {
            if (m_amplitudeVsChargePlot->isVisible())
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
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "Select ROOT File",
                                                    QString(),
                                                    "ROOT Files (*.root);;All Files (*)");

    if (!filePath.isEmpty())
    {
        loadRootFile(filePath);
    }
}

bool HistogramWindow::showHistogramEventLimitDialog()
{
    // Create a custom dialog
    QDialog dialog(this);
    dialog.setWindowTitle("Histogram Event Limit");
    dialog.setMinimumWidth(350);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // Add title label
    QLabel *titleLabel = new QLabel("<b>Set maximum events for histogram filling:</b>", &dialog);
    layout->addWidget(titleLabel);

    // Add horizontal layout for Events: label and line edit
    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel *eventsLabel = new QLabel("Events:", &dialog);
    QLineEdit *eventLimitEdit = new QLineEdit(&dialog);

    // Set default value from binary file opening
    if (m_defaultEventLimit > 0)
    {
        eventLimitEdit->setText(QString::number(m_defaultEventLimit));
    }
    else
    {
        eventLimitEdit->setText("1000"); // Fallback default
    }

    eventLimitEdit->setPlaceholderText("Enter number or check 'All events'");
    eventLimitEdit->setValidator(new QIntValidator(1, 1000000000, this));

    hLayout->addWidget(eventsLabel);
    hLayout->addWidget(eventLimitEdit);
    layout->addLayout(hLayout);

    // Add checkbox for "All events"
    QCheckBox *allEventsCheckBox = new QCheckBox("Use all events from ROOT file", &dialog);

    // If default is -1 (unlimited), check the box by default
    if (m_defaultEventLimit == -1)
    {
        allEventsCheckBox->setChecked(true);
    }
    else
    {
        allEventsCheckBox->setChecked(false);
    }

    layout->addWidget(allEventsCheckBox);

    // Add informative label showing the default source
    QLabel *infoLabel = new QLabel("Default value is from binary file opening settings.", &dialog);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: gray; font-style: italic;");
    layout->addWidget(infoLabel);

    // Connect checkbox to enable/disable line edit
    QObject::connect(allEventsCheckBox, &QCheckBox::stateChanged,
                     [eventLimitEdit](int state)
                     {
                         eventLimitEdit->setEnabled(state == Qt::Unchecked);
                         if (state == Qt::Checked)
                         {
                             eventLimitEdit->clear();
                         }
                     });

    // Set initial state
    eventLimitEdit->setEnabled(!allEventsCheckBox->isChecked());

    // Add button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttonBox);

    // Connect buttons
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Show dialog
    if (dialog.exec() == QDialog::Accepted)
    {
        if (allEventsCheckBox->isChecked())
        {
            m_histogramEventLimit = -1; // All events
            return true;
        }
        else
        {
            QString text = eventLimitEdit->text().trimmed();
            if (text.isEmpty())
            {
                // If empty, use the default
                if (m_defaultEventLimit > 0)
                {
                    m_histogramEventLimit = m_defaultEventLimit;
                }
                else
                {
                    m_histogramEventLimit = 1000; // Fallback
                }
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
                    // Invalid input, show error
                    QMessageBox::warning(this, "Invalid Input",
                                         "Please enter a valid positive number.");
                    return false;
                }
            }
        }
    }

    return false; // User cancelled
}