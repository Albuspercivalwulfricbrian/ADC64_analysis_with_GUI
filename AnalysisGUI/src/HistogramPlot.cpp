#include "HistogramPlot.h"
#include <cmath>
#include <algorithm>
#include <cmath>

HistogramPlot::HistogramPlot(const QString &title, const QString &xAxisLabel, QWidget *parent)
    : QWidget(parent),
      m_customPlot(new QCustomPlot(this)),
      m_mainLayout(new QVBoxLayout(this)),
      m_controlWidget(new QWidget(this)),    // Changed to QWidget
      m_titleLabel(new QLabel(title, this)), // Added title label
      m_minLabel(new QLabel("Min:", this)),
      m_maxLabel(new QLabel("Max:", this)),
      m_binsLabel(new QLabel("Bins:", this)),
      m_minEdit(new QLineEdit("0", this)),
      m_maxEdit(new QLineEdit("100", this)),
      m_binsEdit(new QLineEdit("300", this)),
      m_logYScaleCheck(new QCheckBox("Log Y Scale", this)),
      m_logXScaleCheck(new QCheckBox("Log X Scale", this)),
      m_eventLine(new QCPItemLine(m_customPlot)),
      m_xAxisLabel(xAxisLabel)
{
    setupUI(title, xAxisLabel);
    setupPlot(xAxisLabel);

    // Connect signals
    connect(m_minEdit, &QLineEdit::editingFinished, this, &HistogramPlot::onRangeChanged);
    connect(m_maxEdit, &QLineEdit::editingFinished, this, &HistogramPlot::onRangeChanged);
    connect(m_binsEdit, &QLineEdit::editingFinished, this, &HistogramPlot::onRangeChanged);
    connect(m_logYScaleCheck, &QCheckBox::toggled, this, &HistogramPlot::onlogYScaleToggled);
    connect(m_logXScaleCheck, &QCheckBox::toggled, this, &HistogramPlot::onLogXScaleToggled);
}

HistogramPlot::~HistogramPlot()
{
}

void HistogramPlot::setupUI(const QString &title, const QString &xAxisLabel)
{
    // Create horizontal layout for controls
    QHBoxLayout *controlLayout = new QHBoxLayout(m_controlWidget);

    // Add controls to horizontal layout - title first, then parameters
    controlLayout->addWidget(m_titleLabel);
    controlLayout->addWidget(m_minLabel);
    controlLayout->addWidget(m_minEdit);
    controlLayout->addWidget(m_maxLabel);
    controlLayout->addWidget(m_maxEdit);
    controlLayout->addWidget(m_binsLabel);
    controlLayout->addWidget(m_binsEdit);
    controlLayout->addWidget(m_logYScaleCheck);
    controlLayout->addWidget(m_logXScaleCheck);
    controlLayout->addStretch(); // Push everything to the left

    // Set fixed sizes for line edits
    m_minEdit->setMaximumWidth(80);
    m_maxEdit->setMaximumWidth(80);
    m_binsEdit->setMaximumWidth(60);

    // Make title label bold
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);

    // Set maximum height for control widget
    m_controlWidget->setMaximumHeight(50);
    m_controlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Main layout
    m_mainLayout->addWidget(m_controlWidget);
    m_mainLayout->addWidget(m_customPlot);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(5); // Reduced spacing between controls and plot
}

// ... rest of the implementation remains the same as previous version
void HistogramPlot::setupPlot(const QString &xAxisLabel)
{
    m_customPlot->xAxis->setLabel(xAxisLabel);
    m_customPlot->yAxis->setLabel("Counts");
    m_customPlot->xAxis->setRange(0, 100);
    m_customPlot->yAxis->setRange(0, 100);
    m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    m_customPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    m_customPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    m_customPlot->axisRect()->setRangeZoomAxes(m_customPlot->xAxis, m_customPlot->yAxis);
    m_customPlot->setSelectionRectMode(QCP::srmZoom);
    m_customPlot->setSelectionRectMode(QCP::srmZoom);
    m_customPlot->addLayer("CustomLayer", m_customPlot->layer("main"), QCustomPlot::LayerInsertMode::limAbove);
    // Setup event line to match MainWindow's style
    m_eventLine->setLayer("CustomLayer");
    QPen eventLinePen(QColor(255, 165, 0), 4); // Orange color, width 4 like MainWindow
    m_eventLine->setPen(eventLinePen);
    m_eventLine->setVisible(false);
    // Initialize line positions
    m_eventLine->start->setCoords(0, m_customPlot->yAxis->range().lower);
    m_eventLine->end->setCoords(0, m_customPlot->yAxis->range().upper);

    m_customPlot->replot();
}

void HistogramPlot::setData(const std::vector<float> &data)
{
    m_data = data;
    updatePlot();
}

void HistogramPlot::clearData()
{
    m_data.clear();
    m_customPlot->clearPlottables();
    m_customPlot->replot();
}

void HistogramPlot::setRange(float min, float max)
{
    m_minEdit->setText(QString::number(min));
    m_maxEdit->setText(QString::number(max));
    updatePlot();
}

void HistogramPlot::setBins(int bins)
{
    m_binsEdit->setText(QString::number(bins));
    updatePlot();
}

void HistogramPlot::setlogYScale(bool logYScale)
{
    m_logYScaleCheck->setChecked(logYScale);
    onlogYScaleToggled(logYScale);
}

void HistogramPlot::setLogXScale(bool logXScale)
{
    m_logXScaleCheck->setChecked(logXScale);
    onLogXScaleToggled(logXScale);
}

void HistogramPlot::setEventLineVisible(bool visible)
{
    m_eventLine->setVisible(visible);
    m_customPlot->replot();
}

void HistogramPlot::setEventValue(float value)
{
    m_eventValue = value;
    if (m_eventLine->visible())
    {
        m_eventLine->start->setCoords(m_eventValue, m_customPlot->yAxis->range().lower);
        m_eventLine->end->setCoords(m_eventValue, m_customPlot->yAxis->range().upper);
        m_customPlot->replot();
    }
}

void HistogramPlot::updatePlot()
{
    if (m_data.empty())
    {
        m_customPlot->clearPlottables();
        m_customPlot->replot();
        return;
    }

    float minVal = getMinRange();
    float maxVal = getMaxRange();
    int bins = getBins();

    if (bins <= 0 || minVal >= maxVal)
    {
        return; // Invalid range
    }

    m_customPlot->clearPlottables();

    QVector<double> x, y;
    calculateHistogram(x, y);

    // Create bar chart
    QCPBars *bars = new QCPBars(m_customPlot->xAxis, m_customPlot->yAxis);
    bars->setData(x, y);
    bars->setPen(QPen(Qt::blue));
    bars->setBrush(QBrush(QColor(0, 0, 255, 100)));

    // Set axis ranges
    double maxCount = *std::max_element(y.constBegin(), y.constEnd());
    if (maxCount == 0)
        maxCount = 1;

    m_customPlot->yAxis->setRange(0, maxCount * 1.1);

    // Handle x-axis scaling
    if (m_logXScaleCheck->isChecked())
    {
        // For log scale - width will automatically scale with plot
        bars->setWidthType(QCPBars::wtPlotCoords);
        double logMin = log10(minVal);
        double logMax = log10(maxVal);
        double logRange = logMax - logMin;
        bars->setWidth(logRange / bins); // Bin width in plot coordinates
        m_customPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
        m_customPlot->xAxis->setRange(minVal, maxVal);
    }
    else
    {
        // For linear scale - width will automatically scale with plot
        double binWidth = (maxVal - minVal) / bins;
        bars->setWidthType(QCPBars::wtPlotCoords);
        bars->setWidth(binWidth); // Bin width in plot coordinates
        m_customPlot->xAxis->setScaleType(QCPAxis::stLinear);
        m_customPlot->xAxis->setRange(minVal, maxVal);
    }

    m_customPlot->replot();
}

void HistogramPlot::calculateHistogram(QVector<double> &x, QVector<double> &y)
{
    float minVal = getMinRange();
    float maxVal = getMaxRange();
    int bins = getBins();

    if (bins <= 0)
        return;

    x.resize(bins);
    y.resize(bins);

    // Fill histogram
    for (float value : m_data)
    {
        if (value >= minVal && value <= maxVal)
        {
            if (m_logXScaleCheck->isChecked())
            {
                // For log scale, use logarithmic binning
                double logMin = log10(minVal);
                double logMax = log10(maxVal);
                double logRange = logMax - logMin;
                double logValue = log10(value);
                int bin = static_cast<int>((logValue - logMin) / logRange * bins);
                if (bin >= 0 && bin < bins)
                {
                    y[bin]++;
                }
            }
            else
            {
                // For linear scale, use linear binning
                double binWidth = (maxVal - minVal) / bins;
                int bin = static_cast<int>((value - minVal) / binWidth);
                if (bin >= 0 && bin < bins)
                {
                    y[bin]++;
                }
            }
        }
    }

    // Calculate bin centers for display
    for (int i = 0; i < bins; ++i)
    {
        if (m_logXScaleCheck->isChecked())
        {
            // For log scale, use geometric centers
            double logMin = log10(minVal);
            double logMax = log10(maxVal);
            double logRange = logMax - logMin;
            double logBinCenter = logMin + (i + 0.5) * logRange / bins;
            x[i] = pow(10.0, logBinCenter);
        }
        else
        {
            // For linear scale, use arithmetic centers
            double binWidth = (maxVal - minVal) / bins;
            x[i] = minVal + (i + 0.5) * binWidth;
        }
    }
}

void HistogramPlot::onRangeChanged()
{
    updatePlot();
    emit rangeChanged();
}

void HistogramPlot::onlogYScaleToggled(bool checked)
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
    emit logYScaleToggled(checked);
}

void HistogramPlot::onLogXScaleToggled(bool checked)
{
    if (checked)
    {
        m_customPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
    }
    else
    {
        m_customPlot->xAxis->setScaleType(QCPAxis::stLinear);
    }
    m_customPlot->replot();
    emit logXScaleToggled(checked);
}