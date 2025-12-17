#ifndef EVENTFILTERWIDGET_H
#define EVENTFILTERWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

class EventFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EventFilterWidget(QWidget *parent = nullptr);
    ~EventFilterWidget();

    // Getters for filter values
    float getMinAmplitude() const;
    float getMaxAmplitude() const;
    float getMinCharge() const;
    float getMaxCharge() const;
    float getMinTime() const;
    float getMaxTime() const;

    // Check if filtering is enabled
    bool isFilteringEnabled() const;

    // Check if event passes filters (only if filtering is enabled)
    bool eventPassesFilters(float amplitude, float charge, float time) const;

signals:
    void filtersChanged();

private slots:
    void onResetButtonClicked();
    void onApplyFilterToggled(bool checked);

private:
    QCheckBox *m_applyFilterCheck;
    QLineEdit *m_minAmpEdit;
    QLineEdit *m_maxAmpEdit;
    QLineEdit *m_minChargeEdit;
    QLineEdit *m_maxChargeEdit;
    QLineEdit *m_minTimeEdit;
    QLineEdit *m_maxTimeEdit;
    QPushButton *m_resetButton;

    void setupUI();
};

#endif // EVENTFILTERWIDGET_H