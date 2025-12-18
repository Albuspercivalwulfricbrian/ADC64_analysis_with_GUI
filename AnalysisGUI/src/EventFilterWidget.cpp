#include "EventFilterWidget.h"
#include <QSizePolicy>

EventFilterWidget::EventFilterWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

EventFilterWidget::~EventFilterWidget()
{
}

void EventFilterWidget::setupUI()
{
    setWindowTitle("Event Filter Settings");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(6);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // Apply filter checkbox at the top
    QHBoxLayout *checkboxLayout = new QHBoxLayout();
    m_applyFilterCheck = new QCheckBox("Apply Event Filtering", this);
    m_applyFilterCheck->setChecked(false);
    m_applyFilterCheck->setFont(QFont("Arial", 10, QFont::Bold));
    checkboxLayout->addWidget(m_applyFilterCheck);
    checkboxLayout->addStretch();

    // Create parameter filter widgets
    m_ampFilter = new ParameterFilterWidget("Amplitude", -1e9f, 1e9f, true, this);
    m_chargeFilter = new ParameterFilterWidget("Charge", -1e9f, 1e9f, true, this);
    m_timeFilter = new ParameterFilterWidget("Time", -1e9f, 1e9f, true, this);
    m_baselineFilter = new ParameterFilterWidget("zl", -1e9f, 1e9f, true, this);
    m_baselineRMSFilter = new ParameterFilterWidget("zl_rms", 0.0f, 1e9f, false, this);

    // Reset button
    m_resetButton = new QPushButton("Reset All Filters", this);
    m_resetButton->setFixedHeight(28);
    m_resetButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Connect signals
    connect(m_applyFilterCheck, &QCheckBox::toggled, this, &EventFilterWidget::onApplyFilterToggled);
    connect(m_applyFilterCheck, &QCheckBox::toggled, this, &EventFilterWidget::onApplyFilterToggled);

    // Connect each parameter filter's changed signal
    connect(m_ampFilter, &ParameterFilterWidget::filterChanged, this, &EventFilterWidget::onParameterFilterChanged);
    connect(m_chargeFilter, &ParameterFilterWidget::filterChanged, this, &EventFilterWidget::onParameterFilterChanged);
    connect(m_timeFilter, &ParameterFilterWidget::filterChanged, this, &EventFilterWidget::onParameterFilterChanged);
    connect(m_baselineFilter, &ParameterFilterWidget::filterChanged, this, &EventFilterWidget::onParameterFilterChanged);
    connect(m_baselineRMSFilter, &ParameterFilterWidget::filterChanged, this, &EventFilterWidget::onParameterFilterChanged);

    connect(m_resetButton, &QPushButton::clicked, this, &EventFilterWidget::onResetButtonClicked);

    // Add to main layout
    mainLayout->addLayout(checkboxLayout);
    mainLayout->addSpacing(4);
    mainLayout->addWidget(m_ampFilter);
    mainLayout->addWidget(m_chargeFilter);
    mainLayout->addWidget(m_timeFilter);
    mainLayout->addWidget(m_baselineFilter);
    mainLayout->addWidget(m_baselineRMSFilter);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(m_resetButton, 0, Qt::AlignHCenter);

    // Set size policy to shrink to contents
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Calculate and set minimum size
    adjustSize();

    // Set fixed size to current size (no extra space)
    setFixedSize(size());

    // Initially disable filters
    onApplyFilterToggled(false);
}

void EventFilterWidget::onApplyFilterToggled(bool checked)
{
    m_ampFilter->setEnabled(checked);
    m_chargeFilter->setEnabled(checked);
    m_timeFilter->setEnabled(checked);
    m_baselineFilter->setEnabled(checked);
    m_baselineRMSFilter->setEnabled(checked);
    emit filtersChanged();
}

bool EventFilterWidget::isFilteringEnabled() const
{
    return m_applyFilterCheck->isChecked();
}

float EventFilterWidget::getMinAmplitude() const
{
    return m_ampFilter->getMinValue();
}

float EventFilterWidget::getMaxAmplitude() const
{
    return m_ampFilter->getMaxValue();
}

float EventFilterWidget::getMinCharge() const
{
    return m_chargeFilter->getMinValue();
}

float EventFilterWidget::getMaxCharge() const
{
    return m_chargeFilter->getMaxValue();
}

float EventFilterWidget::getMinTime() const
{
    return m_timeFilter->getMinValue();
}

float EventFilterWidget::getMaxTime() const
{
    return m_timeFilter->getMaxValue();
}

float EventFilterWidget::getMinBaseline() const
{
    return m_baselineFilter->getMinValue();
}

float EventFilterWidget::getMaxBaseline() const
{
    return m_baselineFilter->getMaxValue();
}

float EventFilterWidget::getMinBaselineRMS() const
{
    return m_baselineRMSFilter->getMinValue();
}

float EventFilterWidget::getMaxBaselineRMS() const
{
    return m_baselineRMSFilter->getMaxValue();
}

bool EventFilterWidget::eventPassesFilters(float amplitude, float charge, float time,
                                           float baseline, float baselineRMS) const
{
    if (!isFilteringEnabled())
        return true;

    return (amplitude >= getMinAmplitude() && amplitude <= getMaxAmplitude() &&
            charge >= getMinCharge() && charge <= getMaxCharge() &&
            time >= getMinTime() && time <= getMaxTime() &&
            baseline >= getMinBaseline() && baseline <= getMaxBaseline() &&
            baselineRMS >= getMinBaselineRMS() && baselineRMS <= getMaxBaselineRMS());
}

void EventFilterWidget::onResetButtonClicked()
{
    m_ampFilter->clear();
    m_chargeFilter->clear();
    m_timeFilter->clear();
    m_baselineFilter->clear();
    m_baselineRMSFilter->clear();
    emit filtersChanged();
}

void EventFilterWidget::onParameterFilterChanged()
{
    emit filtersChanged();
}