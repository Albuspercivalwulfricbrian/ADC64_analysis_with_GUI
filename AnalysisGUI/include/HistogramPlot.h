#ifndef HISTOGRAMPLOT_H
#define HISTOGRAMPLOT_H

#include "qcustomplot.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <vector>

class HistogramPlot : public QWidget
{
    Q_OBJECT

public:
    explicit HistogramPlot(const QString &title, const QString &xAxisLabel, QWidget *parent = nullptr);
    ~HistogramPlot();

    // Data management
    void setData(const std::vector<float> &data);
    void clearData();

    // Range and bin configuration
    void setRange(float min, float max);
    void setBins(int bins);
    void setlogYScale(bool logYScale);
    void setLogXScale(bool logXScale);

    // Getters
    float getMinRange() const { return m_minEdit->text().toFloat(); }
    float getMaxRange() const { return m_maxEdit->text().toFloat(); }
    int getBins() const { return m_binsEdit->text().toInt(); }
    bool getlogYScale() const { return m_logYScaleCheck->isChecked(); }
    bool getLogXScale() const { return m_logXScaleCheck->isChecked(); }

    // Plot access
    QCustomPlot *plot() { return m_customPlot; }

    // Event line management
    void setEventLineVisible(bool visible);
    void setEventLinePosition(float position);

public slots:
    void updatePlot();
    void onRangeChanged();
    void onlogYScaleToggled(bool checked);
    void onLogXScaleToggled(bool checked);

signals:
    void rangeChanged();
    void logYScaleToggled(bool);
    void logXScaleToggled(bool);

private:
    void setupUI(const QString &title, const QString &xAxisLabel);
    void setupPlot(const QString &xAxisLabel);
    void calculateHistogram(QVector<double> &x, QVector<double> &y);

private:
    QCustomPlot *m_customPlot;
    QVBoxLayout *m_mainLayout;
    QWidget *m_controlWidget; // Changed from QGroupBox to QWidget
    QLabel *m_titleLabel;     // Added separate title label

    // Controls
    QLabel *m_minLabel;
    QLabel *m_maxLabel;
    QLabel *m_binsLabel;
    QLineEdit *m_minEdit;
    QLineEdit *m_maxEdit;
    QLineEdit *m_binsEdit;
    QCheckBox *m_logYScaleCheck;
    QCheckBox *m_logXScaleCheck;

    // Event line
    QCPItemStraightLine *m_eventLine;
    QPen m_eventLinePen;

    // Data
    std::vector<float> m_data;
    QString m_xAxisLabel;
};

#endif // HISTOGRAMPLOT_H