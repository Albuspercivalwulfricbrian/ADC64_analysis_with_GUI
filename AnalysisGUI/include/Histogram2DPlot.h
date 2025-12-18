#ifndef HISTOGRAM2DPLOT_H
#define HISTOGRAM2DPLOT_H

#include "qcustomplot.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QList>
#include <QPair>
#include <vector>

class Histogram2DPlot : public QWidget
{
    Q_OBJECT

public:
    explicit Histogram2DPlot(const QString &title, const QString &xAxisLabel,
                             const QString &yAxisLabel, QWidget *parent = nullptr);
    ~Histogram2DPlot();

    void setData(const std::vector<float> &xData, const std::vector<float> &yData);
    void clearData();

    void setXRange(float min, float max);
    void setYRange(float min, float max);
    void setXBins(int bins);
    void setYBins(int bins);
    void setLogXScale(bool logXScale);
    void setLogYScale(bool logYScale);
    void setLogZScale(bool logZScale);
    // Getters
    float getXMinRange() const { return m_xMinEdit->text().toFloat(); }
    float getXMaxRange() const { return m_xMaxEdit->text().toFloat(); }
    float getYMinRange() const { return m_yMinEdit->text().toFloat(); }
    float getYMaxRange() const { return m_yMaxEdit->text().toFloat(); }
    int getXBins() const { return m_xBinsEdit->text().toInt(); }
    int getYBins() const { return m_yBinsEdit->text().toInt(); }
    bool getLogXScale() const { return m_logXScaleCheck->isChecked(); }
    bool getLogYScale() const { return m_logYScaleCheck->isChecked(); }
    bool getLogZScale() const { return m_logZScaleCheck->isChecked(); }

    // Color map configuration
    void setColorMap(const QCPColorGradient &gradient);
    void setColorScaleVisible(bool visible);
    void setDataRange(double lower, double upper);
    void autoRescaleDataRange();

    // Event marker management
    void setEventValue(float xValue, float yValue);
    void setEventMarkerVisible(bool visible);

    // New methods for range control
    void setAutoRangeOnDataChange(bool enabled);
    void resetRangesToInitial();

public slots:
    void updatePlot();
    void onRangeChanged();
    void onLogXScaleToggled(bool checked);
    void onLogYScaleToggled(bool checked);
    void onLogZScaleToggled(bool checked);
    void onColorGradientChanged(int index);
    void onDataRangeChanged();

signals:
    void rangeChanged();
    void logXScaleToggled(bool);
    void logYScaleToggled(bool);
    void logZScaleToggled(bool);

private:
    void setupUI(const QString &title, const QString &xAxisLabel, const QString &yAxisLabel);
    void setupPlot(const QString &xAxisLabel, const QString &yAxisLabel);
    void initializeColorGradients();
    QCPColorGradient getGradientFromIndex(int index);
    void calculate2DHistogram();
    void updateColorScale();
    void setupEventMarkers();
    void updateEventMarker();

private:
    // UI elements
    QCustomPlot *m_customPlot;
    QCPColorMap *m_colorMap;
    QCPColorScale *m_colorScale;
    QVBoxLayout *m_mainLayout;
    QWidget *m_controlWidget;
    QLabel *m_titleLabel;

    QLabel *m_xMinLabel;
    QLabel *m_xMaxLabel;
    QLabel *m_yMinLabel;
    QLabel *m_yMaxLabel;
    QLabel *m_xBinsLabel;
    QLabel *m_yBinsLabel;
    QLineEdit *m_xMinEdit;
    QLineEdit *m_xMaxEdit;
    QLineEdit *m_yMinEdit;
    QLineEdit *m_yMaxEdit;
    QLineEdit *m_xBinsEdit;
    QLineEdit *m_yBinsEdit;
    QCheckBox *m_logXScaleCheck;
    QCheckBox *m_logYScaleCheck;
    QCheckBox *m_logZScaleCheck;
    QComboBox *m_colorGradientCombo;
    QLabel *m_dataMinLabel;
    QLabel *m_dataMaxLabel;
    QLineEdit *m_dataMinEdit;
    QLineEdit *m_dataMaxEdit;
    QPushButton *m_autoScaleButton;

    // Color gradients
    QList<QPair<QString, QCPColorGradient>> m_colorGradients;

    std::vector<float> m_xData;
    std::vector<float> m_yData;
    std::vector<std::vector<double>> m_histogramData;
    QString m_xAxisLabel;
    QString m_yAxisLabel;
    double m_dataMin;
    double m_dataMax;

    // Initial range values
    float m_initialXMin;
    float m_initialXMax;
    float m_initialYMin;
    float m_initialYMax;
    bool m_autoRangeOnDataChange;

    // Event markers
    QCPItemLine *m_eventLineX; // Horizontal line at yValue
    QCPItemLine *m_eventLineY; // Vertical line at xValue
    float m_eventXValue;
    float m_eventYValue;
    bool m_eventMarkerVisible;
};

#endif // HISTOGRAM2DPLOT_H