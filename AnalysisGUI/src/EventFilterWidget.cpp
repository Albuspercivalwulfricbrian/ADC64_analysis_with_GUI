#include "EventFilterWidget.h"
#include <QDoubleValidator>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
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
    mainLayout->setSpacing(8);                      // Reduced spacing
    mainLayout->setContentsMargins(10, 10, 10, 10); // Reduced margins

    // Apply filter checkbox at the top
    QHBoxLayout *checkboxLayout = new QHBoxLayout();
    m_applyFilterCheck = new QCheckBox("Apply Event Filtering", this);
    m_applyFilterCheck->setChecked(false);
    m_applyFilterCheck->setFont(QFont("Arial", 11, QFont::Bold));
    checkboxLayout->addWidget(m_applyFilterCheck);
    checkboxLayout->addStretch();

    // Amplitude filter
    QHBoxLayout *ampLayout = new QHBoxLayout();
    ampLayout->setSpacing(5);
    QLabel *ampLabel = new QLabel("Amplitude:", this);
    ampLabel->setFixedWidth(80); // Reduced width

    m_minAmpEdit = new QLineEdit(this);
    m_minAmpEdit->setPlaceholderText("Min");
    m_minAmpEdit->setValidator(new QDoubleValidator(-1e9, 1e9, 2, this));
    m_minAmpEdit->setFixedWidth(70); // Reduced width
    m_minAmpEdit->setMaximumHeight(25);

    QLabel *ampDash = new QLabel("-", this);
    ampDash->setAlignment(Qt::AlignCenter);
    ampDash->setFixedWidth(8);

    m_maxAmpEdit = new QLineEdit(this);
    m_maxAmpEdit->setPlaceholderText("Max");
    m_maxAmpEdit->setValidator(new QDoubleValidator(-1e9, 1e9, 2, this));
    m_maxAmpEdit->setFixedWidth(70); // Reduced width
    m_maxAmpEdit->setMaximumHeight(25);

    ampLayout->addWidget(ampLabel);
    ampLayout->addWidget(m_minAmpEdit);
    ampLayout->addWidget(ampDash);
    ampLayout->addWidget(m_maxAmpEdit);
    ampLayout->addStretch();

    // Charge filter
    QHBoxLayout *chargeLayout = new QHBoxLayout();
    chargeLayout->setSpacing(5);
    QLabel *chargeLabel = new QLabel("Charge:", this);
    chargeLabel->setFixedWidth(80);

    m_minChargeEdit = new QLineEdit(this);
    m_minChargeEdit->setPlaceholderText("Min");
    m_minChargeEdit->setValidator(new QDoubleValidator(-1e9, 1e9, 2, this));
    m_minChargeEdit->setFixedWidth(70);
    m_minChargeEdit->setMaximumHeight(25);

    QLabel *chargeDash = new QLabel("-", this);
    chargeDash->setAlignment(Qt::AlignCenter);
    chargeDash->setFixedWidth(8);

    m_maxChargeEdit = new QLineEdit(this);
    m_maxChargeEdit->setPlaceholderText("Max");
    m_maxChargeEdit->setValidator(new QDoubleValidator(-1e9, 1e9, 2, this));
    m_maxChargeEdit->setFixedWidth(70);
    m_maxChargeEdit->setMaximumHeight(25);

    chargeLayout->addWidget(chargeLabel);
    chargeLayout->addWidget(m_minChargeEdit);
    chargeLayout->addWidget(chargeDash);
    chargeLayout->addWidget(m_maxChargeEdit);
    chargeLayout->addStretch();

    // Time filter
    QHBoxLayout *timeLayout = new QHBoxLayout();
    timeLayout->setSpacing(5);
    QLabel *timeLabel = new QLabel("Time:", this);
    timeLabel->setFixedWidth(80);

    m_minTimeEdit = new QLineEdit(this);
    m_minTimeEdit->setPlaceholderText("Min");
    m_minTimeEdit->setValidator(new QDoubleValidator(-1e9, 1e9, 2, this));
    m_minTimeEdit->setFixedWidth(70);
    m_minTimeEdit->setMaximumHeight(25);

    QLabel *timeDash = new QLabel("-", this);
    timeDash->setAlignment(Qt::AlignCenter);
    timeDash->setFixedWidth(8);

    m_maxTimeEdit = new QLineEdit(this);
    m_maxTimeEdit->setPlaceholderText("Max");
    m_maxTimeEdit->setValidator(new QDoubleValidator(-1e9, 1e9, 2, this));
    m_maxTimeEdit->setFixedWidth(70);
    m_maxTimeEdit->setMaximumHeight(25);

    timeLayout->addWidget(timeLabel);
    timeLayout->addWidget(m_minTimeEdit);
    timeLayout->addWidget(timeDash);
    timeLayout->addWidget(m_maxTimeEdit);
    timeLayout->addStretch();

    // Reset button
    m_resetButton = new QPushButton("Reset All Filters", this);
    m_resetButton->setFixedHeight(30);
    m_resetButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Connect signals
    connect(m_applyFilterCheck, &QCheckBox::toggled, this, &EventFilterWidget::onApplyFilterToggled);
    connect(m_minAmpEdit, &QLineEdit::editingFinished, this, &EventFilterWidget::filtersChanged);
    connect(m_maxAmpEdit, &QLineEdit::editingFinished, this, &EventFilterWidget::filtersChanged);
    connect(m_minChargeEdit, &QLineEdit::editingFinished, this, &EventFilterWidget::filtersChanged);
    connect(m_maxChargeEdit, &QLineEdit::editingFinished, this, &EventFilterWidget::filtersChanged);
    connect(m_minTimeEdit, &QLineEdit::editingFinished, this, &EventFilterWidget::filtersChanged);
    connect(m_maxTimeEdit, &QLineEdit::editingFinished, this, &EventFilterWidget::filtersChanged);
    connect(m_resetButton, &QPushButton::clicked, this, &EventFilterWidget::onResetButtonClicked);

    // Add to main layout
    mainLayout->addLayout(checkboxLayout);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(ampLayout);
    mainLayout->addLayout(chargeLayout);
    mainLayout->addLayout(timeLayout);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_resetButton, 0, Qt::AlignHCenter);

    // Set size policy to shrink to contents
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Calculate and set minimum size
    adjustSize();

    // Set fixed size to current size (no extra space)
    setFixedSize(size());
}

void EventFilterWidget::onApplyFilterToggled(bool checked)
{
    // Enable/disable filter inputs based on checkbox state
    m_minAmpEdit->setEnabled(checked);
    m_maxAmpEdit->setEnabled(checked);
    m_minChargeEdit->setEnabled(checked);
    m_maxChargeEdit->setEnabled(checked);
    m_minTimeEdit->setEnabled(checked);
    m_maxTimeEdit->setEnabled(checked);

    emit filtersChanged();
}

bool EventFilterWidget::isFilteringEnabled() const
{
    return m_applyFilterCheck->isChecked();
}

float EventFilterWidget::getMinAmplitude() const
{
    if (!isFilteringEnabled())
        return -1e9;
    QString text = m_minAmpEdit->text();
    return text.isEmpty() ? -1e9 : text.toFloat();
}

float EventFilterWidget::getMaxAmplitude() const
{
    if (!isFilteringEnabled())
        return 1e9;
    QString text = m_maxAmpEdit->text();
    return text.isEmpty() ? 1e9 : text.toFloat();
}

float EventFilterWidget::getMinCharge() const
{
    if (!isFilteringEnabled())
        return -1e9;
    QString text = m_minChargeEdit->text();
    return text.isEmpty() ? -1e9 : text.toFloat();
}

float EventFilterWidget::getMaxCharge() const
{
    if (!isFilteringEnabled())
        return 1e9;
    QString text = m_maxChargeEdit->text();
    return text.isEmpty() ? 1e9 : text.toFloat();
}

float EventFilterWidget::getMinTime() const
{
    if (!isFilteringEnabled())
        return -1e9;
    QString text = m_minTimeEdit->text();
    return text.isEmpty() ? -1e9 : text.toFloat();
}

float EventFilterWidget::getMaxTime() const
{
    if (!isFilteringEnabled())
        return 1e9;
    QString text = m_maxTimeEdit->text();
    return text.isEmpty() ? 1e9 : text.toFloat();
}

bool EventFilterWidget::eventPassesFilters(float amplitude, float charge, float time) const
{
    // If filtering is disabled, all events pass
    if (!isFilteringEnabled())
        return true;

    return (amplitude >= getMinAmplitude() && amplitude <= getMaxAmplitude() &&
            charge >= getMinCharge() && charge <= getMaxCharge() &&
            time >= getMinTime() && time <= getMaxTime());
}

void EventFilterWidget::onResetButtonClicked()
{
    m_minAmpEdit->clear();
    m_maxAmpEdit->clear();
    m_minChargeEdit->clear();
    m_maxChargeEdit->clear();
    m_minTimeEdit->clear();
    m_maxTimeEdit->clear();
    emit filtersChanged();
}