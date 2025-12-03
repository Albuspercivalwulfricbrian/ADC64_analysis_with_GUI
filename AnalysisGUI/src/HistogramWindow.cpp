#include "HistogramWindow.h"
#include <QHBoxLayout>
#include <QMessageBox>
#include <thread>
#include <random>
#include "ProgressDialog.h"
#include "ctpl_stl.h"
HistogramWindow::HistogramWindow(QWidget *parent)
    : QMainWindow(parent),
      m_amplitudePlot(new HistogramPlot("Amplitude Histogram", "Amplitude (ADC Channels)", this)),
      m_chargePlot(new HistogramPlot("Charge Histogram", "Charge", this)),
      m_timePlot(new HistogramPlot("Time Histogram", "Time", this)),
      m_mainLayout(new QVBoxLayout()),
      m_centralWidget(new QWidget(this)),
      m_eventTimeLabel(new QLabel("Current Event Time: N/A", this)),
      m_filePathLabel(new QLabel("No ROOT file selected", this)),
      m_channelSpinBox(new QSpinBox(this)),
      m_updateButton(new QPushButton("Update Histograms", this)),
      m_histogramSelectionCombo(new QComboBox(this)),
      m_histogramListWidget(new QListWidget(this)),
      m_openRootFileAction(new QAction("Open ROOT File", this)),
      m_currentChannel(0),
      m_dataLoaded(false),
      m_threadPool(nullptr)
{
    setupUI();

    // Set initial ranges
    m_amplitudePlot->setRange(0, 60000);
    m_chargePlot->setRange(0, 10000000);
    m_timePlot->setRange(0, 10000);

    // Connect signals
    connect(m_updateButton, &QPushButton::clicked, this, &HistogramWindow::updateHistograms);
    // Note: No connection for m_channelSpinBox valueChanged since it's now read-only
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
    m_currentRootFile = filePath;
    // m_currentChannel = channel;
    // // Update the spinbox value directly without triggering signals
    // m_channelSpinBox->blockSignals(true);
    // m_channelSpinBox->setValue(channel);
    // m_channelSpinBox->blockSignals(false);
    m_dataLoaded = false;

    // Update file path label
    if (!filePath.isEmpty())
    {
        QFileInfo fileInfo(filePath);
        m_filePathLabel->setText("File: " + fileInfo.fileName());
        RootDataFile = TFile::Open((TString)(filePath.toUtf8().constData()));
        RootDataTree = (TTree *)RootDataFile->Get("adc64_data");
        std::map<Int_t, short_energy_ChannelEntry *> short_channel_info;

        sci = new short_energy_ChannelEntry();
        sci->Initialize();
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
    if (m_currentRootFile.isEmpty())
    {
        QMetaObject::invokeMethod(this, [this]()
                                  { QMessageBox::warning(this, "Error", "No ROOT file specified"); });
        return;
    }

    try
    {

        if (RootDataTree->GetBranch(TString::Format("channel_%i", m_currentChannel + 1))->GetClassName() == (TString) "PeaksInfo")
        {
            sciv->Initialize();
        }
        else if (RootDataTree->GetBranch(TString::Format("channel_%i", m_currentChannel + 1))->GetClassName() == (TString) "short_energy_ChannelEntry")
        {
            sci->Initialize();
        }
        // Clear previous data
        m_amplitudeData.clear();
        m_chargeData.clear();
        m_timeData.clear();

        const int totalEntries = RootDataTree->GetEntries();
        ProgressDialog *progressDialog = new ProgressDialog(this);
        progressDialog->setWindowTitle("Processing ROOT File");
        progressDialog->setWindowModality(Qt::ApplicationModal);
        progressDialog->show();

        if (RootDataTree->GetBranch(TString::Format("channel_%i", m_currentChannel + 1))->GetClassName() == (TString) "PeaksInfo")
        {
            RootDataTree->SetBranchAddress((TString::Format("channel_%i", m_currentChannel + 1)).Data(), &sciv);
        }
        else if (RootDataTree->GetBranch(TString::Format("channel_%i", m_currentChannel + 1))->GetClassName() == (TString) "short_energy_ChannelEntry")
        {
            RootDataTree->SetBranchAddress((TString::Format("channel_%i", m_currentChannel + 1)).Data(), &sci);
        }

        for (int i = 0; i < totalEntries; i++)
        {
            RootDataTree->GetEntry(i);
            if (RootDataTree->GetBranch(TString::Format("channel_%i", m_currentChannel + 1))->GetClassName() == (TString) "PeaksInfo")
            {
                m_amplitudeData.push_back(sciv->amp());
                m_chargeData.push_back(sciv->charge());
                m_timeData.push_back(sciv->time());
            }
            else if (RootDataTree->GetBranch(TString::Format("channel_%i", m_currentChannel + 1))->GetClassName() == (TString) "short_energy_ChannelEntry")
            {
                m_amplitudeData.push_back(sci->amp);
                m_chargeData.push_back(sci->charge);
                m_timeData.push_back(sci->time);
            }

            // Update progress scaled to 0-1000 range
            const int scaledProgress = static_cast<int>((i + 1) * 1000.0 / totalEntries);
            QMetaObject::invokeMethod(progressDialog, [progressDialog, scaledProgress]()
                                      { progressDialog->updateProgress(scaledProgress); }, Qt::QueuedConnection);
        }

        progressDialog->close();
        progressDialog->deleteLater();
        // std::random_device rd;
        // std::mt19937 gen(rd());

        // // Generate amplitude data
        // std::normal_distribution<> ampDist(30000, 10000);
        // for (int i = 0; i < 10000; ++i)
        // {
        //     float amp = std::max(0.0f, static_cast<float>(ampDist(gen)));
        //     if (amp <= 60000)
        //         m_amplitudeData.push_back(amp);
        // }

        // // Generate charge data
        // std::normal_distribution<> chargeDist(500, 200);
        // for (int i = 0; i < 10000; ++i)
        // {
        //     float charge = std::max(0.0f, static_cast<float>(chargeDist(gen)));
        //     m_chargeData.push_back(charge);
        // }

        // // Generate time data
        // std::uniform_real_distribution<> timeDist(0, 1000);
        // for (int i = 0; i < 10000; ++i)
        // {
        //     float time = static_cast<float>(timeDist(gen));
        //     m_timeData.push_back(time);
        // }

        m_dataLoaded = true;

        QMetaObject::invokeMethod(this, &HistogramWindow::updateHistograms, Qt::QueuedConnection);
    }
    catch (const std::exception &e)
    {
        QMetaObject::invokeMethod(this, [this, e]()
                                  { QMessageBox::critical(this, "Error", QString("Failed to process ROOT file: %1").arg(e.what())); });
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
    m_currentChannel = channel;
    // Update the spinbox value directly without triggering signals
    m_channelSpinBox->blockSignals(true);
    m_channelSpinBox->setValue(channel);
    m_channelSpinBox->blockSignals(false);

    if (!m_currentRootFile.isEmpty())
    // {
    //     loadRootFile(m_currentRootFile);
    // }
    // else
    {
        processHistogramData();
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
