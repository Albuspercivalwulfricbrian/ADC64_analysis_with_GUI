#include "HistogramWindow.h"
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <thread>
#include <cmath>
#include <random>
#include "ctpl_stl.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"

HistogramWindow::HistogramWindow(QWidget *parent)
    : QMainWindow(parent), m_customPlot(new QCustomPlot(this)), m_mainLayout(new QVBoxLayout()), m_centralWidget(new QWidget(this)), m_eventLine(new QCPItemStraightLine(m_customPlot)), m_eventTimeLabel(new QLabel("Current Event Time: N/A", this)), m_channelSpinBox(new QSpinBox(this)), m_minAmpEdit(new QLineEdit("0", this)), m_maxAmpEdit(new QLineEdit("60000", this)), m_binsEdit(new QLineEdit("100", this)), m_updateButton(new QPushButton("Update Histogram", this)), m_logScaleCheck(new QCheckBox("Log Scale", this)), m_currentChannel(0), m_dataLoaded(false), m_threadPool(nullptr)
{
    setupUI();
    setupHistogram();

    // FIXED: Use lambda to resolve overloaded signals
    connect(m_updateButton, &QPushButton::clicked, this, &HistogramWindow::updateHistogram);
    connect(m_channelSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &HistogramWindow::onChannelChanged);
    connect(m_logScaleCheck, &QCheckBox::toggled, this, &HistogramWindow::onLogScaleToggled);
    connect(m_minAmpEdit, &QLineEdit::editingFinished, this, &HistogramWindow::onRangeChanged);
    connect(m_maxAmpEdit, &QLineEdit::editingFinished, this, &HistogramWindow::onRangeChanged);
    connect(m_binsEdit, &QLineEdit::editingFinished, this, &HistogramWindow::onRangeChanged);

    setWindowTitle("Amplitude Spectrum");
    resize(800, 600);
}

HistogramWindow::~HistogramWindow()
{
}

void HistogramWindow::setupUI()
{
    // Create control panel
    QWidget *controlWidget = new QWidget(this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlWidget);

    // Channel selection
    QLabel *channelLabel = new QLabel("Channel:", this);
    m_channelSpinBox->setRange(0, 511);
    m_channelSpinBox->setValue(0);

    // Range controls
    QLabel *minLabel = new QLabel("Min Amp:", this);
    QLabel *maxLabel = new QLabel("Max Amp:", this);
    QLabel *binsLabel = new QLabel("Bins:", this);
    m_binsEdit->setMaximumWidth(60);
    m_minAmpEdit->setMaximumWidth(80);
    m_maxAmpEdit->setMaximumWidth(80);

    controlLayout->addWidget(channelLabel);
    controlLayout->addWidget(m_channelSpinBox);
    controlLayout->addWidget(minLabel);
    controlLayout->addWidget(m_minAmpEdit);
    controlLayout->addWidget(maxLabel);
    controlLayout->addWidget(m_maxAmpEdit);
    controlLayout->addWidget(binsLabel);
    controlLayout->addWidget(m_binsEdit);
    controlLayout->addWidget(m_logScaleCheck);
    controlLayout->addWidget(m_updateButton);
    controlLayout->addStretch();
    controlLayout->addWidget(m_eventTimeLabel);

    // Main layout
    m_mainLayout->addWidget(controlWidget);
    m_mainLayout->addWidget(m_customPlot);

    m_centralWidget->setLayout(m_mainLayout);
    setCentralWidget(m_centralWidget);
}

void HistogramWindow::setupHistogram()
{
    // Setup plot
    m_customPlot->xAxis->setLabel("Amplitude (ADC Channels)");
    m_customPlot->yAxis->setLabel("Counts");
    m_customPlot->xAxis->setRange(0, 60000);
    m_customPlot->yAxis->setRange(0, 100);

    // Setup event line with ORANGE color
    m_eventLinePen.setColor(QColor(255, 165, 0)); // Orange color
    m_eventLinePen.setWidth(2);
    m_eventLinePen.setStyle(Qt::DashLine);
    m_eventLine->setPen(m_eventLinePen);
    m_eventLine->setVisible(false);

    m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_customPlot->replot();
}

void HistogramWindow::setCurrentEventTime(float time)
{
    m_eventTimeLabel->setText(QString("Current Event Time: %1").arg(time, 0, 'f', 2));

    if (m_dataLoaded && !m_amplitudeData.empty())
    {
        // Simple mapping: use normalized time to position the line within the amplitude range
        float minAmp = m_minAmpEdit->text().toFloat();
        float maxAmp = m_maxAmpEdit->text().toFloat();

        // Use modulo to get a value between 0 and 1 for positioning
        float normalizedTime = fmod(time, 1000.0f) / 1000.0f;
        float linePos = minAmp + normalizedTime * (maxAmp - minAmp);

        m_eventLine->point1->setCoords(linePos, 0);
        m_eventLine->point2->setCoords(linePos, m_customPlot->yAxis->range().upper);
        m_eventLine->setVisible(true);
        m_customPlot->replot();
    }
}

void HistogramWindow::loadRootFile(const QString &filePath, int channel)
{
    m_currentRootFile = filePath;
    m_currentChannel = channel;
    m_channelSpinBox->setValue(channel);
    m_dataLoaded = false;

    if (m_threadPool)
    {
        m_threadPool->push([this](int id)
                           { processHistogramData(); });
    }
    else
    {
        // Fallback: process in current thread
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
        m_eventTimes.clear();

        // For now, generate some dummy data since we don't have the exact ROOT structure
        // You'll need to replace this with actual ROOT file reading logic

        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Simulate processing

        // Generate dummy amplitude data (replace with actual ROOT file reading)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> dist(30000, 10000);

        for (int i = 0; i < 10000; ++i)
        {
            float amp = std::max(0.0f, static_cast<float>(dist(gen)));
            if (amp <= 60000)
            {
                m_amplitudeData.push_back(amp);
            }
        }

        m_dataLoaded = true;

        // Update histogram in main thread
        QMetaObject::invokeMethod(this, &HistogramWindow::updateHistogram, Qt::QueuedConnection);
    }
    catch (const std::exception &e)
    {
        QMetaObject::invokeMethod(this, [this, e]()
                                  { QMessageBox::critical(this, "Error", QString("Failed to process ROOT file: %1").arg(e.what())); });
    }
}

void HistogramWindow::updateHistogram()
{
    if (!m_dataLoaded || m_amplitudeData.empty())
    {
        // Show empty plot with message
        m_customPlot->clearPlottables();
        m_customPlot->replot();
        return;
    }

    m_customPlot->clearPlottables();

    float minAmp = m_minAmpEdit->text().toFloat();
    float maxAmp = m_maxAmpEdit->text().toFloat();
    int bins = m_binsEdit->text().toInt();

    if (bins <= 0 || minAmp >= maxAmp)
    {
        QMessageBox::warning(this, "Invalid Range", "Please check min/max amplitude values and number of bins");
        return;
    }

    // Create histogram data
    QVector<double> x(bins), y(bins);
    double binWidth = (maxAmp - minAmp) / bins;

    // Initialize bins
    for (int i = 0; i < bins; ++i)
    {
        x[i] = minAmp + (i + 0.5) * binWidth;
        y[i] = 0;
    }

    // Fill histogram
    for (float amp : m_amplitudeData)
    {
        if (amp >= minAmp && amp <= maxAmp)
        {
            int bin = static_cast<int>((amp - minAmp) / binWidth);
            if (bin >= 0 && bin < bins)
            {
                y[bin]++;
            }
        }
    }

    // Create bar chart
    QCPBars *bars = new QCPBars(m_customPlot->xAxis, m_customPlot->yAxis);
    bars->setData(x, y);
    bars->setPen(QPen(Qt::blue));
    bars->setBrush(QBrush(QColor(0, 0, 255, 100)));
    bars->setWidth(binWidth * 0.8);

    // Set axis ranges
    double maxCount = *std::max_element(y.constBegin(), y.constEnd());
    if (maxCount == 0)
        maxCount = 1; // Avoid division by zero

    m_customPlot->xAxis->setRange(minAmp, maxAmp);
    m_customPlot->yAxis->setRange(0, maxCount * 1.1);

    m_customPlot->replot();
}

void HistogramWindow::onChannelChanged(int channel)
{
    m_currentChannel = channel;
    if (!m_currentRootFile.isEmpty())
    {
        loadRootFile(m_currentRootFile, channel);
    }
}

void HistogramWindow::onRangeChanged()
{
    if (m_dataLoaded)
    {
        updateHistogram();
    }
}

void HistogramWindow::onLogScaleToggled(bool checked)
{
    if (checked)
    {
        m_customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    }
    else
    {
        m_customPlot->yAxis->setScaleType(QCPAxis::stLinear);
    }
    m_customPlot->replot();
}