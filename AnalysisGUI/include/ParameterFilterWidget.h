#ifndef PARAMETERFILTERWIDGET_H
#define PARAMETERFILTERWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QDoubleValidator>
#include <QMessageBox>

class ParameterFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ParameterFilterWidget(const QString &parameterName,
                                   float defaultMin = -1e9f,
                                   float defaultMax = 1e9f,
                                   bool allowNegative = true,
                                   QWidget *parent = nullptr);
    ~ParameterFilterWidget();

    float getMinValue() const;
    float getMaxValue() const;

    void setEnabled(bool enabled);
    void clear();

signals:
    void filterChanged();

private slots:
    void onMinEdited();
    void onMaxEdited();

private:
    QString m_parameterName;
    float m_defaultMin;
    float m_defaultMax;

    QLineEdit *m_minEdit;
    QLineEdit *m_maxEdit;

    void validateAndResetValue(QLineEdit *editedEdit, QLineEdit *oppositeEdit, bool editedIsMin);
};

#endif // PARAMETERFILTERWIDGET_H