#include "ParameterFilterWidget.h"

ParameterFilterWidget::ParameterFilterWidget(const QString &parameterName,
                                             float defaultMin,
                                             float defaultMax,
                                             bool allowNegative,
                                             QWidget *parent)
    : QWidget(parent), m_parameterName(parameterName), m_defaultMin(defaultMin), m_defaultMax(defaultMax)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(4);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *label = new QLabel(parameterName + ":", this);
    label->setFixedWidth(70);

    m_minEdit = new QLineEdit(this);
    m_minEdit->setPlaceholderText("Min");
    m_minEdit->setValidator(new QDoubleValidator(allowNegative ? -1e9 : 0, 1e9, 2, this));
    m_minEdit->setFixedWidth(60);
    m_minEdit->setMaximumHeight(22);

    QLabel *dash = new QLabel("-", this);
    dash->setAlignment(Qt::AlignCenter);
    dash->setFixedWidth(6);

    m_maxEdit = new QLineEdit(this);
    m_maxEdit->setPlaceholderText("Max");
    m_maxEdit->setValidator(new QDoubleValidator(allowNegative ? -1e9 : 0, 1e9, 2, this));
    m_maxEdit->setFixedWidth(60);
    m_maxEdit->setMaximumHeight(22);

    layout->addWidget(label);
    layout->addWidget(m_minEdit);
    layout->addWidget(dash);
    layout->addWidget(m_maxEdit);
    layout->addStretch();

    // Connect signals
    connect(m_minEdit, &QLineEdit::editingFinished, this, &ParameterFilterWidget::onMinEdited);
    connect(m_maxEdit, &QLineEdit::editingFinished, this, &ParameterFilterWidget::onMaxEdited);
}

ParameterFilterWidget::~ParameterFilterWidget()
{
}

float ParameterFilterWidget::getMinValue() const
{
    QString text = m_minEdit->text();
    return text.isEmpty() ? m_defaultMin : text.toFloat();
}

float ParameterFilterWidget::getMaxValue() const
{
    QString text = m_maxEdit->text();
    return text.isEmpty() ? m_defaultMax : text.toFloat();
}

void ParameterFilterWidget::setEnabled(bool enabled)
{
    m_minEdit->setEnabled(enabled);
    m_maxEdit->setEnabled(enabled);
}

void ParameterFilterWidget::clear()
{
    m_minEdit->clear();
    m_maxEdit->clear();
}

void ParameterFilterWidget::validateAndResetValue(QLineEdit *editedEdit, QLineEdit *oppositeEdit, bool editedIsMin)
{
    QString editedText = editedEdit->text();
    QString oppositeText = oppositeEdit->text();

    if (editedText.isEmpty() || oppositeText.isEmpty())
        return;

    bool editedOk, oppositeOk;
    float editedValue = editedText.toFloat(&editedOk);
    float oppositeValue = oppositeText.toFloat(&oppositeOk);

    if (!editedOk || !oppositeOk)
        return;

    bool isInvalid = false;
    if (editedIsMin && editedValue > oppositeValue)
    {
        isInvalid = true;
        editedEdit->clear();
    }
    else if (!editedIsMin && editedValue < oppositeValue)
    {
        isInvalid = true;
        editedEdit->clear();
    }

    if (isInvalid)
    {
        QString message;
        if (editedIsMin)
        {
            message = QString("%1: Minimum value cannot be greater than maximum value.\n"
                              "Minimum has been reset to default (%2).")
                          .arg(m_parameterName)
                          .arg(m_defaultMin);
        }
        else
        {
            message = QString("%1: Maximum value cannot be less than minimum value.\n"
                              "Maximum has been reset to default (%2).")
                          .arg(m_parameterName)
                          .arg(m_defaultMax);
        }

        QMessageBox::warning(this, "Invalid Range", message);
    }
}

void ParameterFilterWidget::onMinEdited()
{
    validateAndResetValue(m_minEdit, m_maxEdit, true);
    emit filterChanged();
}

void ParameterFilterWidget::onMaxEdited()
{
    validateAndResetValue(m_maxEdit, m_minEdit, false);
    emit filterChanged();
}