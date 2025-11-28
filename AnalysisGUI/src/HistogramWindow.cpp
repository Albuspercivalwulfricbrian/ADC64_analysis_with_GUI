#include "HistogramWindow.h"
#include <QHBoxLayout>
#include <QMessageBox>
#include <thread>
#include <random>
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
    m_chargePlot->setRange(0, 1000);
    m_timePlot->setRange(0, 1000);

    // Connect signals
    connect(m_updateButton, &QPushButton::clicked, this, &HistogramWindow::updateHistograms);
    connect(m_channelSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &HistogramWindow::onChannelChanged);
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

void HistogramWindow::setCurrentEventTime(float time)
{
    m_eventTimeLabel->setText(QString("Current Event Time: %1").arg(time, 0, 'f', 2));

    if (m_dataLoaded)
    {
        // Update event lines for all plots
        if (!m_amplitudeData.empty())
        {
            float minAmp = m_amplitudePlot->getMinRange();
            float maxAmp = m_amplitudePlot->getMaxRange();
            float normalizedTime = fmod(time, 1000.0f) / 1000.0f;
            float linePos = minAmp + normalizedTime * (maxAmp - minAmp);
            m_amplitudePlot->setEventLinePosition(linePos);
        }

        if (!m_chargeData.empty())
        {
            float minCharge = m_chargePlot->getMinRange();
            float maxCharge = m_chargePlot->getMaxRange();
            float normalizedTime = fmod(time, 1000.0f) / 1000.0f;
            float linePos = minCharge + normalizedTime * (maxCharge - minCharge);
            m_chargePlot->setEventLinePosition(linePos);
        }

        if (!m_timeData.empty())
        {
            float minTime = m_timePlot->getMinRange();
            float maxTime = m_timePlot->getMaxRange();
            float normalizedTime = fmod(time, 1000.0f) / 1000.0f;
            float linePos = minTime + normalizedTime * (maxTime - minTime);
            m_timePlot->setEventLinePosition(linePos);
        }
    }
}

void HistogramWindow::loadRootFile(const QString &filePath, int channel)
{
    m_currentRootFile = filePath;
    m_currentChannel = channel;
    m_channelSpinBox->setValue(channel);
    m_dataLoaded = false;

    // Update file path label
    if (!filePath.isEmpty())
    {
        QFileInfo fileInfo(filePath);
        m_filePathLabel->setText("File: " + fileInfo.fileName());
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
        // Clear previous data
        m_amplitudeData.clear();
        m_chargeData.clear();
        m_timeData.clear();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::random_device rd;
        std::mt19937 gen(rd());

        // Generate amplitude data
        std::normal_distribution<> ampDist(30000, 10000);
        for (int i = 0; i < 10000; ++i)
        {
            float amp = std::max(0.0f, static_cast<float>(ampDist(gen)));
            if (amp <= 60000)
                m_amplitudeData.push_back(amp);
        }

        // Generate charge data
        std::normal_distribution<> chargeDist(500, 200);
        for (int i = 0; i < 10000; ++i)
        {
            float charge = std::max(0.0f, static_cast<float>(chargeDist(gen)));
            m_chargeData.push_back(charge);
        }

        // Generate time data
        std::uniform_real_distribution<> timeDist(0, 1000);
        for (int i = 0; i < 10000; ++i)
        {
            float time = static_cast<float>(timeDist(gen));
            m_timeData.push_back(time);
        }

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

    // Update plots with their respective data
    if (m_amplitudeItem->checkState() == Qt::Checked)
    {
        m_amplitudePlot->setData(m_amplitudeData);
    }
    if (m_chargeItem->checkState() == Qt::Checked)
    {
        m_chargePlot->setData(m_chargeData);
    }
    if (m_timeItem->checkState() == Qt::Checked)
    {
        m_timePlot->setData(m_timeData);
    }
}

void HistogramWindow::onChannelChanged(int channel)
{
    m_currentChannel = channel;
    if (!m_currentRootFile.isEmpty())
    {
        loadRootFile(m_currentRootFile, channel);
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
        loadRootFile(filePath, m_currentChannel);
    }
}