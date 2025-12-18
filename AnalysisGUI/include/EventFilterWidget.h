#ifndef EVENTFILTERWIDGET_H
#define EVENTFILTERWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include "ParameterFilterWidget.h"

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
    float getMinBaseline() const;
    float getMaxBaseline() const;
    float getMinBaselineRMS() const;
    float getMaxBaselineRMS() const;

    // Check if filtering is enabled
    bool isFilteringEnabled() const;

    // Check if event passes filters (only if filtering is enabled)
    bool eventPassesFilters(float amplitude, float charge, float time,
                            float baseline, float baselineRMS) const;

signals:
    void filtersChanged();

private slots:
    void onResetButtonClicked();
    void onApplyFilterToggled(bool checked);
    void onParameterFilterChanged();

private:
    QCheckBox *m_applyFilterCheck;
    ParameterFilterWidget *m_ampFilter;
    ParameterFilterWidget *m_chargeFilter;
    ParameterFilterWidget *m_timeFilter;
    ParameterFilterWidget *m_baselineFilter;
    ParameterFilterWidget *m_baselineRMSFilter;
    QPushButton *m_resetButton;

    void setupUI();
};

#endif // EVENTFILTERWIDGET_H