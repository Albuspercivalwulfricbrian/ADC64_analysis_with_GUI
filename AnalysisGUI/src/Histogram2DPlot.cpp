#include "Histogram2DPlot.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <QPushButton>
#include <QMessageBox>
#include <QFrame>
#include <iostream>

Histogram2DPlot::Histogram2DPlot(const QString &title, const QString &xAxisLabel,
                                 const QString &yAxisLabel, QWidget *parent)
    : QWidget(parent),
      m_customPlot(new QCustomPlot(this)),
      m_colorMap(nullptr),
      m_colorScale(nullptr),
      m_mainLayout(new QVBoxLayout(this)),
      m_controlWidget(new QWidget(this)),
      m_titleLabel(new QLabel(title, this)),
      m_xMinLabel(new QLabel("X Min:", this)),
      m_xMaxLabel(new QLabel("X Max:", this)),
      m_yMinLabel(new QLabel("Y Min:", this)),
      m_yMaxLabel(new QLabel("Y Max:", this)),
      m_xBinsLabel(new QLabel("X Bins:", this)),
      m_yBinsLabel(new QLabel("Y Bins:", this)),
      m_xMinEdit(new QLineEdit("0", this)),
      m_xMaxEdit(new QLineEdit("100", this)),
      m_yMinEdit(new QLineEdit("0", this)),
      m_yMaxEdit(new QLineEdit("100", this)),
      m_xBinsEdit(new QLineEdit("100", this)),
      m_yBinsEdit(new QLineEdit("100", this)),
      m_logXScaleCheck(new QCheckBox("Log X", this)),
      m_logYScaleCheck(new QCheckBox("Log Y", this)),
      m_logZScaleCheck(new QCheckBox("Log Z", this)),
      m_colorGradientCombo(new QComboBox(this)),
      m_dataMinLabel(new QLabel("Z Min:", this)),
      m_dataMaxLabel(new QLabel("Z Max:", this)),
      m_dataMinEdit(new QLineEdit("0", this)),
      m_dataMaxEdit(new QLineEdit("1", this)),
      m_autoScaleButton(new QPushButton("Auto", this)),
      m_xAxisLabel(xAxisLabel),
      m_yAxisLabel(yAxisLabel),
      m_dataMin(0),
      m_dataMax(1),
      m_initialXMin(0),
      m_initialXMax(100),
      m_initialYMin(0),
      m_initialYMax(100),
      m_autoRangeOnDataChange(true),
      m_eventLineX(nullptr),
      m_eventLineY(nullptr),
      m_eventXValue(0),
      m_eventYValue(0),
      m_eventMarkerVisible(false)
{
    setupUI(title, xAxisLabel, yAxisLabel);
    setupPlot(xAxisLabel, yAxisLabel);
    initializeColorGradients();
    setupEventMarkers();

    // Connect signals
    connect(m_xMinEdit, &QLineEdit::editingFinished, this, &Histogram2DPlot::onRangeChanged);
    connect(m_xMaxEdit, &QLineEdit::editingFinished, this, &Histogram2DPlot::onRangeChanged);
    connect(m_yMinEdit, &QLineEdit::editingFinished, this, &Histogram2DPlot::onRangeChanged);
    connect(m_yMaxEdit, &QLineEdit::editingFinished, this, &Histogram2DPlot::onRangeChanged);
    connect(m_xBinsEdit, &QLineEdit::editingFinished, this, &Histogram2DPlot::onRangeChanged);
    connect(m_yBinsEdit, &QLineEdit::editingFinished, this, &Histogram2DPlot::onRangeChanged);
    connect(m_logXScaleCheck, &QCheckBox::toggled, this, &Histogram2DPlot::onLogXScaleToggled);
    connect(m_logYScaleCheck, &QCheckBox::toggled, this, &Histogram2DPlot::onLogYScaleToggled);
    connect(m_logZScaleCheck, &QCheckBox::toggled, this, &Histogram2DPlot::onLogZScaleToggled);
    connect(m_colorGradientCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Histogram2DPlot::onColorGradientChanged);
    connect(m_dataMinEdit, &QLineEdit::editingFinished,
            this, &Histogram2DPlot::onDataRangeChanged);
    connect(m_dataMaxEdit, &QLineEdit::editingFinished,
            this, &Histogram2DPlot::onDataRangeChanged);
    connect(m_autoScaleButton, &QPushButton::clicked,
            this, &Histogram2DPlot::autoRescaleDataRange);
}

Histogram2DPlot::~Histogram2DPlot()
{
}

void Histogram2DPlot::setupUI(const QString &title, const QString &xAxisLabel, const QString &yAxisLabel)
{
    Q_UNUSED(xAxisLabel);
    Q_UNUSED(yAxisLabel);

    // Create main vertical layout for controls
    QVBoxLayout *controlMainLayout = new QVBoxLayout(m_controlWidget);
    controlMainLayout->setSpacing(5); // Reduced spacing between rows
    controlMainLayout->setContentsMargins(5, 5, 5, 5);

    // First row: Title and X/Y parameters
    QHBoxLayout *firstRow = new QHBoxLayout();

    // Title (bold and larger)
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    m_titleLabel->setFont(titleFont);
    firstRow->addWidget(m_titleLabel);
    firstRow->addStretch();

    // X Controls
    firstRow->addWidget(new QLabel("X:", this));
    firstRow->addWidget(m_xMinLabel);
    firstRow->addWidget(m_xMinEdit);
    firstRow->addWidget(m_xMaxLabel);
    firstRow->addWidget(m_xMaxEdit);
    firstRow->addWidget(m_xBinsLabel);
    firstRow->addWidget(m_xBinsEdit);
    firstRow->addWidget(m_logXScaleCheck);

    // Separator between X and Y
    firstRow->addSpacing(15);

    // Y Controls
    firstRow->addWidget(new QLabel("Y:", this));
    firstRow->addWidget(m_yMinLabel);
    firstRow->addWidget(m_yMinEdit);
    firstRow->addWidget(m_yMaxLabel);
    firstRow->addWidget(m_yMaxEdit);
    firstRow->addWidget(m_yBinsLabel);
    firstRow->addWidget(m_yBinsEdit);
    firstRow->addWidget(m_logYScaleCheck);

    // Second row: Z and Color parameters
    QHBoxLayout *secondRow = new QHBoxLayout();
    secondRow->addStretch();

    // Z Controls
    secondRow->addWidget(new QLabel("Z:", this));
    secondRow->addWidget(m_logZScaleCheck);
    secondRow->addWidget(m_dataMinLabel);
    secondRow->addWidget(m_dataMinEdit);
    secondRow->addWidget(m_dataMaxLabel);
    secondRow->addWidget(m_dataMaxEdit);
    secondRow->addWidget(m_autoScaleButton);

    // Separator between Z and Color
    secondRow->addSpacing(15);

    // Color Controls
    secondRow->addWidget(new QLabel("Color:", this));
    secondRow->addWidget(m_colorGradientCombo);

    // Add rows to main control layout
    controlMainLayout->addLayout(firstRow);
    controlMainLayout->addLayout(secondRow);

    // Set fixed sizes for line edits and buttons
    m_xMinEdit->setMaximumWidth(80);
    m_xMaxEdit->setMaximumWidth(80);
    m_yMinEdit->setMaximumWidth(80);
    m_yMaxEdit->setMaximumWidth(80);
    m_xBinsEdit->setMaximumWidth(60);
    m_yBinsEdit->setMaximumWidth(60);
    m_dataMinEdit->setMaximumWidth(80);
    m_dataMaxEdit->setMaximumWidth(80);
    m_colorGradientCombo->setMaximumWidth(120);
    m_autoScaleButton->setMaximumWidth(60);

    // Set maximum height for control widget (2 rows)
    m_controlWidget->setMaximumHeight(70);
    m_controlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Main layout - add control widget directly (no frame)
    m_mainLayout->addWidget(m_controlWidget);
    m_mainLayout->addWidget(m_customPlot);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(5);
}

void Histogram2DPlot::setupPlot(const QString &xAxisLabel, const QString &yAxisLabel)
{
    // Set up axes
    m_customPlot->xAxis->setLabel(xAxisLabel);
    m_customPlot->yAxis->setLabel(yAxisLabel);
    m_customPlot->xAxis->setRange(0, 100);
    m_customPlot->yAxis->setRange(0, 100);

    // Add color scale
    m_colorScale = new QCPColorScale(m_customPlot);
    m_customPlot->plotLayout()->addElement(0, 1, m_colorScale);
    m_colorScale->setType(QCPAxis::atRight);
    m_colorScale->axis()->setLabel("Counts");

    // Create color map
    m_colorMap = new QCPColorMap(m_customPlot->xAxis, m_customPlot->yAxis);
    m_colorMap->setColorScale(m_colorScale);

    // Gradient now set in initializeColorGradients()

    // DISABLE INTERPOLATION - show distinct bins
    m_colorMap->setInterpolate(false);

    // Set up interactions
    m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    m_customPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    m_customPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    m_customPlot->setSelectionRectMode(QCP::srmZoom);

    m_customPlot->replot();
}

void Histogram2DPlot::setupEventMarkers()
{
    // Create horizontal line for y value
    m_eventLineX = new QCPItemLine(m_customPlot);
    m_eventLineX->setLayer("overlay");

    // Use SOLID LINE for both lines
    QPen crossPen(Qt::red, 2, Qt::SolidLine); // Changed from Qt::DashLine to Qt::SolidLine
    crossPen.setCosmetic(true);

    m_eventLineX->setPen(crossPen);
    m_eventLineX->start->setType(QCPItemPosition::ptPlotCoords);
    m_eventLineX->end->setType(QCPItemPosition::ptPlotCoords);
    m_eventLineX->setClipToAxisRect(true);
    m_eventLineX->setVisible(false);

    // Create vertical line for x value - USE SAME PEN
    m_eventLineY = new QCPItemLine(m_customPlot);
    m_eventLineY->setLayer("overlay");
    m_eventLineY->setPen(crossPen); // Same pen for consistency
    m_eventLineY->start->setType(QCPItemPosition::ptPlotCoords);
    m_eventLineY->end->setType(QCPItemPosition::ptPlotCoords);
    m_eventLineY->setClipToAxisRect(true);
    m_eventLineY->setVisible(false);
}

void Histogram2DPlot::initializeColorGradients()
{

    QCPColorGradient birdGradient;

    // ROOT kBird exact RGB values (9 anchors)
    birdGradient.setColorStopAt(0.0, QColor("transparent"));

    birdGradient.setColorStopAt(0.1e-9, QColor(53, 42, 135));  // RGB: 0.2082,0.1664,0.5293
    birdGradient.setColorStopAt(0.125, QColor(15, 92, 221));   // RGB: 0.0592,0.3599,0.8684
    birdGradient.setColorStopAt(0.250, QColor(20, 129, 214));  // RGB: 0.0780,0.5041,0.8385
    birdGradient.setColorStopAt(0.375, QColor(6, 164, 202));   // RGB: 0.0232,0.6419,0.7914
    birdGradient.setColorStopAt(0.500, QColor(46, 183, 164));  // RGB: 0.1802,0.7178,0.6425
    birdGradient.setColorStopAt(0.625, QColor(135, 191, 119)); // RGB: 0.5301,0.7492,0.4662
    birdGradient.setColorStopAt(0.750, QColor(209, 187, 89));  // RGB: 0.8186,0.7328,0.3499
    birdGradient.setColorStopAt(0.875, QColor(254, 200, 50));  // RGB: 0.9956,0.7862,0.1968
    birdGradient.setColorStopAt(1.000, QColor(249, 251, 14));  // RGB: 0.9764,0.9832,0.0539

    birdGradient.setLevelCount(256);

    m_colorGradients.append(qMakePair(QString("ROOT Bird"), birdGradient));
    m_colorGradients.append(qMakePair(QString("Hot"), QCPColorGradient::gpHot));
    m_colorGradients.append(qMakePair(QString("Cold"), QCPColorGradient::gpCold));
    m_colorGradients.append(qMakePair(QString("Night"), QCPColorGradient::gpNight));
    m_colorGradients.append(qMakePair(QString("Candy"), QCPColorGradient::gpCandy));
    m_colorGradients.append(qMakePair(QString("Geography"), QCPColorGradient::gpGeography));
    m_colorGradients.append(qMakePair(QString("Ion"), QCPColorGradient::gpIon));
    m_colorGradients.append(qMakePair(QString("Thermal"), QCPColorGradient::gpThermal));
    m_colorGradients.append(qMakePair(QString("Polar"), QCPColorGradient::gpPolar));
    m_colorGradients.append(qMakePair(QString("Spectrum"), QCPColorGradient::gpSpectrum));
    m_colorGradients.append(qMakePair(QString("Jet"), QCPColorGradient::gpJet));
    m_colorGradients.append(qMakePair(QString("Hues"), QCPColorGradient::gpHues));

    for (const auto &gradient : m_colorGradients)
    {
        m_colorGradientCombo->addItem(gradient.first);
    }
    // Set default selection to ROOT Bird (first in list)
    m_colorGradientCombo->setCurrentIndex(0);
    m_colorMap->setGradient(birdGradient);
}

QCPColorGradient Histogram2DPlot::getGradientFromIndex(int index)
{
    if (index >= 0 && index < m_colorGradients.size())
    {
        return m_colorGradients[index].second;
    }
    return QCPColorGradient::gpJet;
}

void Histogram2DPlot::setAutoRangeOnDataChange(bool enabled)
{
    m_autoRangeOnDataChange = enabled;
}

void Histogram2DPlot::resetRangesToInitial()
{
    // Reset to hardcoded default ranges (not using stored m_initialXMin, etc.)
    m_xMinEdit->setText("0");
    m_xMaxEdit->setText("60000");
    m_yMinEdit->setText("0");
    m_yMaxEdit->setText("10000000");

    // Reset bins to default values
    m_xBinsEdit->setText("300");
    m_yBinsEdit->setText("300");

    // Also reset Z scale
    m_logZScaleCheck->setChecked(false);
    m_dataMinEdit->setText("0");
    m_dataMaxEdit->setText("1");

    std::cout << "Resetting to initial ranges:" << std::endl;
    std::cout << "X: 0 to 60000" << std::endl;
    std::cout << "Y: 0 to 10000000" << std::endl;
    std::cout << "Bins: 300x300" << std::endl;

    updatePlot();
}

void Histogram2DPlot::setEventValue(float xValue, float yValue)
{
    m_eventXValue = xValue;
    m_eventYValue = yValue;

    if (m_eventMarkerVisible)
    {
        updateEventMarker();
        m_customPlot->replot();
    }
}

void Histogram2DPlot::setEventMarkerVisible(bool visible)
{
    m_eventMarkerVisible = visible;

    if (m_eventLineX)
        m_eventLineX->setVisible(visible);
    if (m_eventLineY)
        m_eventLineY->setVisible(visible);

    if (visible)
    {
        updateEventMarker();
    }

    m_customPlot->replot();
}

void Histogram2DPlot::updateEventMarker()
{
    if (!m_eventLineX || !m_eventLineY)
        return;

    // Get current axis ranges
    double xMin = m_customPlot->xAxis->range().lower;
    double xMax = m_customPlot->xAxis->range().upper;
    double yMin = m_customPlot->yAxis->range().lower;
    double yMax = m_customPlot->yAxis->range().upper;

    // Set horizontal line (across entire x range at yValue)
    m_eventLineX->start->setCoords(xMin, m_eventYValue);
    m_eventLineX->end->setCoords(xMax, m_eventYValue);

    // Set vertical line (across entire y range at xValue)
    m_eventLineY->start->setCoords(m_eventXValue, yMin);
    m_eventLineY->end->setCoords(m_eventXValue, yMax);
}

void Histogram2DPlot::setData(const std::vector<float> &xData, const std::vector<float> &yData)
{
    if (xData.size() != yData.size())
    {
        QMessageBox::warning(this, "Data Error",
                             "X and Y data must have the same number of elements");
        return;
    }

    // DEBUG: Print first few values
    std::cout << "2D Plot - Setting data with " << xData.size() << " points" << std::endl;
    if (!xData.empty() && !yData.empty())
    {
        std::cout << "First 5 points:" << std::endl;
        for (int i = 0; i < std::min(5, (int)xData.size()); i++)
        {
            std::cout << "  Point " << i << ": X=" << xData[i] << ", Y=" << yData[i] << std::endl;
        }
    }

    m_xData = xData;
    m_yData = yData;

    // Clear histogram data before recalculating
    m_histogramData.clear();

    // Auto-determine ranges if enabled
    if (m_autoRangeOnDataChange && !xData.empty() && !yData.empty())
    {
        auto xMinMax = std::minmax_element(xData.begin(), xData.end());
        auto yMinMax = std::minmax_element(yData.begin(), yData.end());

        // DEBUG: Print min/max values
        std::cout << "Auto-ranging: X range: " << *xMinMax.first << " to " << *xMinMax.second << std::endl;
        std::cout << "Auto-ranging: Y range: " << *yMinMax.first << " to " << *yMinMax.second << std::endl;

        // Calculate ranges with small padding
        float xRange = *xMinMax.second - *xMinMax.first;
        float yRange = *yMinMax.second - *yMinMax.first;

        // Avoid zero range
        if (xRange == 0)
            xRange = 1.0f;
        if (yRange == 0)
            yRange = 1.0f;

        float xMin = *xMinMax.first - 0.05f * xRange;
        float xMax = *xMinMax.second + 0.05f * xRange;
        float yMin = *yMinMax.first - 0.05f * yRange;
        float yMax = *yMinMax.second + 0.05f * yRange;

        // Apply amplitude limits for X axis
        xMin = std::max(0.0f, xMin);
        xMax = std::min(70000.0f, xMax);

        // Ensure positive values for log scales
        if (m_logXScaleCheck->isChecked() && xMin <= 0)
            xMin = 0.1f;
        if (m_logYScaleCheck->isChecked() && yMin <= 0)
            yMin = 0.1f;

        m_xMinEdit->setText(QString::number(xMin));
        m_xMaxEdit->setText(QString::number(xMax));
        m_yMinEdit->setText(QString::number(yMin));
        m_yMaxEdit->setText(QString::number(yMax));

        std::cout << "Setting X range: " << xMin << " to " << xMax << std::endl;
        std::cout << "Setting Y range: " << yMin << " to " << yMax << std::endl;
    }
    else if (!m_autoRangeOnDataChange)
    {
        std::cout << "Using user-defined ranges:" << std::endl;
        std::cout << "X: " << m_xMinEdit->text().toFloat() << " to " << m_xMaxEdit->text().toFloat() << std::endl;
        std::cout << "Y: " << m_yMinEdit->text().toFloat() << " to " << m_yMaxEdit->text().toFloat() << std::endl;
    }

    updatePlot();
}

void Histogram2DPlot::clearData()
{
    m_xData.clear();
    m_yData.clear();
    m_histogramData.clear();

    if (m_colorMap)
    {
        m_colorMap->data()->clear();
    }
    m_customPlot->replot();
}

void Histogram2DPlot::setXRange(float min, float max)
{
    m_xMinEdit->setText(QString::number(min));
    m_xMaxEdit->setText(QString::number(max));
    updatePlot();
}

void Histogram2DPlot::setYRange(float min, float max)
{
    m_yMinEdit->setText(QString::number(min));
    m_yMaxEdit->setText(QString::number(max));
    updatePlot();
}

void Histogram2DPlot::setXBins(int bins)
{
    m_xBinsEdit->setText(QString::number(bins));
    updatePlot();
}

void Histogram2DPlot::setYBins(int bins)
{
    m_yBinsEdit->setText(QString::number(bins));
    updatePlot();
}

void Histogram2DPlot::setLogXScale(bool logXScale)
{
    m_logXScaleCheck->setChecked(logXScale);
    onLogXScaleToggled(logXScale);
}

void Histogram2DPlot::setLogYScale(bool logYScale)
{
    m_logYScaleCheck->setChecked(logYScale);
    onLogYScaleToggled(logYScale);
}

void Histogram2DPlot::setLogZScale(bool logZScale)
{
    m_logZScaleCheck->setChecked(logZScale);
    onLogZScaleToggled(logZScale);
}

void Histogram2DPlot::setColorMap(const QCPColorGradient &gradient)
{
    if (m_colorMap)
    {
        m_colorMap->setGradient(gradient);
        updateColorScale();
    }
}

void Histogram2DPlot::setColorScaleVisible(bool visible)
{
    if (m_colorScale)
    {
        m_colorScale->setVisible(visible);
        m_customPlot->replot();
    }
}

void Histogram2DPlot::setDataRange(double lower, double upper)
{
    m_dataMin = lower;
    m_dataMax = upper;
    m_dataMinEdit->setText(QString::number(lower));
    m_dataMaxEdit->setText(QString::number(upper));

    if (m_colorMap)
    {
        m_colorMap->setDataRange(QCPRange(lower, upper));
        updateColorScale();
        m_customPlot->replot(); // FIXED: Add replot when data range changes
    }
}

void Histogram2DPlot::autoRescaleDataRange()
{
    if (!m_histogramData.empty())
    {
        // Collect all non-zero, non-NaN values
        std::vector<double> allValues;
        for (const auto &row : m_histogramData)
        {
            for (double val : row)
            {
                if (val > 0 && !std::isnan(val))
                {
                    allValues.push_back(val);
                }
            }
        }

        if (allValues.empty())
        {
            std::cout << "No valid data points for auto-rescaling" << std::endl;
            return;
        }

        // Sort values to find percentiles
        std::sort(allValues.begin(), allValues.end());

        size_t percentileIndex = static_cast<size_t>(allValues.size() * 0.96);
        percentileIndex = std::min(percentileIndex, allValues.size() - 1);

        double minVal = allValues.front();
        double percentileVal = allValues[percentileIndex];

        std::cout << "Auto-rescaling data range:" << std::endl;
        std::cout << "  Min value: " << minVal << std::endl;
        std::cout << "  82nd percentile: " << percentileVal << std::endl;
        std::cout << "  Max value: " << allValues.back() << std::endl;
        std::cout << "  Total data points: " << allValues.size() << std::endl;

        // If using log scale for Z, we need to handle it differently
        if (getLogZScale())
        {
            // For log scale, take log of the values
            minVal = log10(minVal + 1);
            percentileVal = log10(percentileVal + 1);

            // Scale so percentile maps to ~0.8 on color scale
            double rangeScale = 1.25; // 1/0.8 = 1.25
            double targetMax = percentileVal * rangeScale;

            // Ensure we have a reasonable range
            if (targetMax > percentileVal * 1.1) // At least 10% above percentile
            {
                setDataRange(minVal, targetMax);
            }
            else
            {
                setDataRange(minVal, percentileVal * 1.2);
            }
        }
        else
        {
            // For linear scale, scale so percentile maps to ~0.8 on color scale
            double rangeScale = 1.25; // 1/0.8 = 1.25
            double targetMax = percentileVal * rangeScale;

            // Ensure we have a reasonable range
            // Don't let max be too close to percentile
            if (targetMax > percentileVal * 1.1) // At least 10% above percentile
            {
                setDataRange(minVal, targetMax);
            }
            else
            {
                setDataRange(minVal, percentileVal * 1.2);
            }
        }
    }
    else
    {
        std::cout << "No histogram data for auto-rescaling" << std::endl;
    }
}

void Histogram2DPlot::calculate2DHistogram()
{
    float xMin = getXMinRange();
    float xMax = getXMaxRange();
    float yMin = getYMinRange();
    float yMax = getYMaxRange();
    int xBins = getXBins();
    int yBins = getYBins();

    if (xBins <= 0 || yBins <= 0 || xMin >= xMax || yMin >= yMax)
    {
        std::cout << "Invalid parameters for 2D histogram calculation" << std::endl;
        std::cout << "X: " << xMin << " to " << xMax << ", bins: " << xBins << std::endl;
        std::cout << "Y: " << yMin << " to " << yMax << ", bins: " << yBins << std::endl;
        return;
    }

    std::cout << "Calculating 2D histogram with " << m_xData.size() << " points" << std::endl;
    std::cout << "X range: " << xMin << " to " << xMax << " (" << xBins << " bins)" << std::endl;
    std::cout << "Y range: " << yMin << " to " << yMax << " (" << yBins << " bins)" << std::endl;

    // Clear and resize histogram data - IMPORTANT: Clear old data first!
    m_histogramData.clear(); // Clear existing data
    m_histogramData.resize(yBins);
    for (auto &row : m_histogramData)
    {
        row.resize(xBins, 0.0); // Initialize all bins to 0.0
    }

    // Count points in range for debugging
    int pointsInRange = 0;
    int pointsOutOfRange = 0;

    // Calculate histogram
    for (size_t i = 0; i < m_xData.size(); ++i)
    {
        float xVal = m_xData[i];
        float yVal = m_yData[i];

        // Check if point is within range
        if (xVal < xMin || xVal > xMax || yVal < yMin || yVal > yMax)
        {
            pointsOutOfRange++;
            continue;
        }
        pointsInRange++;

        // Calculate bin indices - FIXED: Proper bin calculation
        int xBin = 0, yBin = 0;

        if (getLogXScale())
        {
            if (xVal <= 0)
                continue; // Skip non-positive values for log scale
            double logXMin = log10(xMin);
            double logXMax = log10(xMax);
            double logXVal = log10(xVal);
            // Calculate normalized position in log scale (0 to 1)
            double normalized = (logXVal - logXMin) / (logXMax - logXMin);
            xBin = static_cast<int>(normalized * xBins);
        }
        else
        {
            // Calculate normalized position in linear scale (0 to 1)
            double normalized = (xVal - xMin) / (xMax - xMin);
            xBin = static_cast<int>(normalized * xBins);
        }

        if (getLogYScale())
        {
            if (yVal <= 0)
                continue; // Skip non-positive values for log scale
            double logYMin = log10(yMin);
            double logYMax = log10(yMax);
            double logYVal = log10(yVal);
            // Calculate normalized position in log scale (0 to 1)
            double normalized = (logYVal - logYMin) / (logYMax - logYMin);
            yBin = static_cast<int>(normalized * yBins);
        }
        else
        {
            // Calculate normalized position in linear scale (0 to 1)
            double normalized = (yVal - yMin) / (yMax - yMin);
            yBin = static_cast<int>(normalized * yBins);
        }

        // Clamp bin indices - handle edge case where value equals max
        if (xBin == xBins)
            xBin = xBins - 1;
        if (yBin == yBins)
            yBin = yBins - 1;
        if (xBin < 0 || xBin >= xBins || yBin < 0 || yBin >= yBins)
        {
            continue;
        }

        // Increment count
        m_histogramData[yBin][xBin] += 1.0;
    }

    std::cout << "Points in range: " << pointsInRange << ", out of range: " << pointsOutOfRange << std::endl;

    // Debug: Print histogram stats
    if (!m_histogramData.empty() && !m_histogramData[0].empty())
    {
        double totalCount = 0;
        double maxCount = 0;
        int nonZeroBins = 0;

        for (const auto &row : m_histogramData)
        {
            for (double val : row)
            {
                totalCount += val;
                if (val > maxCount)
                    maxCount = val;
                if (val > 0)
                    nonZeroBins++;
            }
        }

        std::cout << "Histogram stats: total=" << totalCount
                  << ", max=" << maxCount
                  << ", non-zero bins=" << nonZeroBins
                  << "/" << (xBins * yBins) << std::endl;
    }
}

void Histogram2DPlot::updatePlot()
{
    if (m_xData.empty() || m_yData.empty())
    {
        std::cout << "No data to plot" << std::endl;
        return;
    }

    calculate2DHistogram();

    if (m_histogramData.empty() || m_histogramData[0].empty())
    {
        std::cout << "Histogram data is empty" << std::endl;
        return;
    }

    int xBins = getXBins();
    int yBins = getYBins();
    float xMin = getXMinRange();
    float xMax = getXMaxRange();
    float yMin = getYMinRange();
    float yMax = getYMaxRange();

    std::cout << "Updating plot with " << xBins << "x" << yBins << " bins" << std::endl;
    std::cout << "X: " << xMin << " to " << xMax << std::endl;
    std::cout << "Y: " << yMin << " to " << yMax << std::endl;

    // Clear the color map data before setting new data
    if (m_colorMap)
    {
        m_colorMap->data()->clear();
    }

    // Set up color map data
    m_colorMap->data()->setSize(xBins, yBins);
    m_colorMap->data()->setRange(QCPRange(xMin, xMax), QCPRange(yMin, yMax));

    // Get current gradient and modify it
    QCPColorGradient gradient = m_colorMap->gradient();
    gradient.setNanColor(QColor(0, 0, 0, 0)); // Transparent for zero bins
    m_colorMap->setGradient(gradient);        // Apply modified gradient

    // Determine data range for color scaling
    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();

    // Fill color map data
    for (int y = 0; y < yBins; ++y)
    {
        for (int x = 0; x < xBins; ++x)
        {
            double value = m_histogramData[y][x];

            // Apply log Z scale if requested
            if (getLogZScale() && value > 0)
            {
                value = log10(value + 1);
            }

            // For bins with less than 1 count (empty bins), set to NaN
            // This will make them transparent due to setNanColor(QColor(0, 0, 0, 0))
            if (value < 1.0)
            {
                m_colorMap->data()->setCell(x, y, std::numeric_limits<double>::quiet_NaN());
            }
            else
            {
                m_colorMap->data()->setCell(x, y, value);

                if (value < minVal)
                    minVal = value;
                if (value > maxVal)
                    maxVal = value;
            }
        }
    }

    // Enable grid lines so they show through transparent bins
    m_customPlot->xAxis->grid()->setVisible(true);
    m_customPlot->yAxis->grid()->setVisible(true);
    m_customPlot->xAxis->grid()->setPen(QPen(QColor(150, 150, 150), 1, Qt::DashLine));
    m_customPlot->yAxis->grid()->setPen(QPen(QColor(150, 150, 150), 1, Qt::DashLine));

    // Also enable sub-grid for better visibility
    m_customPlot->xAxis->grid()->setSubGridVisible(true);
    m_customPlot->yAxis->grid()->setSubGridVisible(true);
    m_customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(200, 200, 200), 0.5, Qt::DotLine));
    m_customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(200, 200, 200), 0.5, Qt::DotLine));

    std::cout << "Data range: " << minVal << " to " << maxVal << std::endl;

    // Update data range
    if (minVal < maxVal)
    {
        m_dataMin = minVal;
        m_dataMax = maxVal;

        if (m_dataMinEdit->text().isEmpty() || m_dataMaxEdit->text().isEmpty())
        {
            m_dataMinEdit->setText(QString::number(minVal));
            m_dataMaxEdit->setText(QString::number(maxVal));
            m_colorMap->setDataRange(QCPRange(minVal, maxVal));
        }
        else
        {
            // Fixed: Use the actual data range, not X range
            m_colorMap->setDataRange(QCPRange(m_dataMin, m_dataMax));
        }
    }

    // Set axis ranges
    m_customPlot->xAxis->setRange(xMin, xMax);
    m_customPlot->yAxis->setRange(yMin, yMax);

    // Apply log scales to axes
    if (getLogXScale())
    {
        m_customPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
    }
    else
    {
        m_customPlot->xAxis->setScaleType(QCPAxis::stLinear);
    }

    if (getLogYScale())
    {
        m_customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    }
    else
    {
        m_customPlot->yAxis->setScaleType(QCPAxis::stLinear);
    }

    // Make sure event markers are on top layer
    if (m_eventLineX)
        m_eventLineX->setLayer("overlay");
    if (m_eventLineY)
        m_eventLineY->setLayer("overlay");

    // Update event marker if visible
    if (m_eventMarkerVisible)
    {
        updateEventMarker();
    }

    updateColorScale();
    m_customPlot->replot();

    std::cout << "Plot updated" << std::endl;
}

void Histogram2DPlot::updateColorScale()
{
    if (m_colorScale && m_colorMap)
    {
        if (getLogZScale())
        {
            m_colorScale->axis()->setLabel("log(Counts + 1)");
        }
        else
        {
            m_colorScale->axis()->setLabel("Counts");
        }
        m_colorScale->setDataRange(m_colorMap->dataRange());
    }
}

void Histogram2DPlot::onRangeChanged()
{
    updatePlot();
    emit rangeChanged();
}

void Histogram2DPlot::onLogXScaleToggled(bool checked)
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

void Histogram2DPlot::onLogYScaleToggled(bool checked)
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

void Histogram2DPlot::onLogZScaleToggled(bool checked)
{
    updatePlot(); // Recalculate with log scaling
    emit logZScaleToggled(checked);
}

void Histogram2DPlot::onColorGradientChanged(int index)
{
    if (m_colorMap)
    {
        m_colorMap->setGradient(getGradientFromIndex(index));
        m_customPlot->replot();
    }
}

void Histogram2DPlot::onDataRangeChanged()
{
    double minVal = m_dataMinEdit->text().toDouble();
    double maxVal = m_dataMaxEdit->text().toDouble();

    if (minVal < maxVal)
    {
        setDataRange(minVal, maxVal);
    }
    else
    {
        // If invalid range, still replot with current settings
        m_customPlot->replot();
    }
}