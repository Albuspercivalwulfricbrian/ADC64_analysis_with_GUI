#include "HistogramPlot.h"
#include <cmath>
#include <algorithm>

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
      m_binsEdit(new QLineEdit("100", this)),
      m_logScaleCheck(new QCheckBox("Log Scale", this)),
      m_eventLine(new QCPItemStraightLine(m_customPlot)),
      m_xAxisLabel(xAxisLabel)
{
    setupUI(title, xAxisLabel);
    setupPlot(xAxisLabel);

    // Connect signals
    connect(m_minEdit, &QLineEdit::editingFinished, this, &HistogramPlot::onRangeChanged);
    connect(m_maxEdit, &QLineEdit::editingFinished, this, &HistogramPlot::onRangeChanged);
    connect(m_binsEdit, &QLineEdit::editingFinished, this, &HistogramPlot::onRangeChanged);
    connect(m_logScaleCheck, &QCheckBox::toggled, this, &HistogramPlot::onLogScaleToggled);
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
    controlLayout->addWidget(m_logScaleCheck);
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
    m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // Setup event line
    m_eventLinePen.setColor(QColor(255, 165, 0)); // Orange color
    m_eventLinePen.setWidth(2);
    m_eventLinePen.setStyle(Qt::DashLine);
    m_eventLine->setPen(m_eventLinePen);
    m_eventLine->setVisible(false);

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

void HistogramPlot::setLogScale(bool logScale)
{
    m_logScaleCheck->setChecked(logScale);
    onLogScaleToggled(logScale);
}

void HistogramPlot::setEventLineVisible(bool visible)
{
    m_eventLine->setVisible(visible);
    m_customPlot->replot();
}

void HistogramPlot::setEventLinePosition(float position)
{
    if (m_eventLine->visible())
    {
        m_eventLine->point1->setCoords(position, 0);
        m_eventLine->point2->setCoords(position, m_customPlot->yAxis->range().upper);
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
    double binWidth = (maxVal - minVal) / bins;
    QCPBars *bars = new QCPBars(m_customPlot->xAxis, m_customPlot->yAxis);
    bars->setData(x, y);
    bars->setPen(QPen(Qt::blue));
    bars->setBrush(QBrush(QColor(0, 0, 255, 100)));
    bars->setWidth(binWidth * 0.8);

    // Set axis ranges
    double maxCount = *std::max_element(y.constBegin(), y.constEnd());
    if (maxCount == 0)
        maxCount = 1;

    m_customPlot->xAxis->setRange(minVal, maxVal);
    m_customPlot->yAxis->setRange(0, maxCount * 1.1);

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
    double binWidth = (maxVal - minVal) / bins;

    // Initialize bins
    for (int i = 0; i < bins; ++i)
    {
        x[i] = minVal + (i + 0.5) * binWidth;
        y[i] = 0;
    }

    // Fill histogram
    for (float value : m_data)
    {
        if (value >= minVal && value <= maxVal)
        {
            int bin = static_cast<int>((value - minVal) / binWidth);
            if (bin >= 0 && bin < bins)
            {
                y[bin]++;
            }
        }
    }
}

void HistogramPlot::onRangeChanged()
{
    updatePlot();
    emit rangeChanged();
}

void HistogramPlot::onLogScaleToggled(bool checked)
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
    emit logScaleToggled(checked);
}