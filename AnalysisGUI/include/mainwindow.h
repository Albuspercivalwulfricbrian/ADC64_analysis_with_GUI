#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <string>

#include <QMainWindow>
#include "qcustomplot.h"
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>
// #include "DataFileReader.h"
#include "configmanager.h"
#include <QTimer>
#include "Worker.h"
#include <QAction>
#include <ctpl_stl.h>
#include <ProgressWidget.h>
#include "HistogramWindow.h"
#include <QPointer>
#include "EventFilterWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QCustomPlot *customPlot;

public:
    MainWindow(QWidget *parent = nullptr);
    int64_t getMaxReadoutEvents() const { return maxReadoutEvents; }
    ~MainWindow();
    std::string WriteMode;

private:
    Ui::MainWindow *ui;
    QPointer<HistogramWindow> m_histogramWindow;
    QPointer<EventFilterWidget> m_eventFilterWidget;

    int64_t maxReadoutEvents;
    bool m_programmaticallyChangingEvent = false;
    int32_t passfilter = 0;
    int64_t currEvent = 1;
    int16_t currChannel = 0;
    double currentX = 0;
    int32_t xLeftBoundary = -1000;
    int32_t xRightBoundary = 3000;
    int32_t smartScopeLeft = 0;  // Smart scope left boundary
    int32_t smartScopeRight = 0; // Smart scope right boundary

    QLabel *coordinateLabel = new QLabel();
    QLabel *FileNameLabel;
    Worker DFR;
    QLineEdit *LeftBoundaryEdit;
    QLineEdit *RightBoundaryEdit;
    QLineEdit *BranchName;
    QSpinBox *channelSpinBox;
    QSpinBox *eventSpinBox;
    QSpinBox *FrequencySpinBox;
    QSpinBox *ThreadsSpinBox;
    std::map<int, ConfigManager *> channels;
    QCPItemLine *lineLeft;
    QCPItemLine *lineRight;
    QCPItemLine *lineSmartScopeLeft;  // Smart scope left line
    QCPItemLine *lineSmartScopeRight; // Smart scope right line
    QCPLayer *customLayer;
    int32_t size;
    QTimer *timer = new QTimer();
    QString fileName = "";

    QAction *action_UseSpline;
    QAction *action_UseSmartScope;
    QAction *action_Use_Fourier_Filtering;
    QAction *action_Signal_is_Negative;
    QAction *action_Show_Fourier_Transform;
    QAction *action_Show_Filtered;
    QAction *actionSavePng;
    QAction *actionSavePdf;
    QAction *actionSaveJpeg;
    QAction *action_Show_Histogram;
    QAction *action_Event_Filter;
    QAction *action_Show_Prony_Fit;

    void setupGraph();
    void UpdateGraph();
    void ReDrawBoundaries();
    void showContextMenu(const QPoint &pos);
    void processFiles(const QStringList &files);
    void updateSmartScopeLines(int left, int right); // Update smart scope lines
    void clearSmartScopeLines();                     // Clear smart scope lines
    bool currentEventPassesFilters();
    bool showEventLimitDialog();

    ctpl::thread_pool p;

private slots:

    void onMouseMove(QMouseEvent *event);
    void onPlotDoubleClick(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event) override;
    void on_NextEventButton_clicked();
    void on_PreviousEventButton_clicked();
    void on_SetFileAnalysisButton_clicked();
    void on_action_Open_triggered();
    void on_action_Open_Config_triggered();
    void on_channelSpinBox_valueChanged(int);
    void on_eventSpinBox_valueChanged(int);
    void on_FrequencySpinBox_valueChanged(int);
    void on_SaveConfigButton_clicked();
    void on_SetChannelBoundaries_clicked();
    void on_SetAllChannelsBoundaries_clicked();
    void on_setBranchName_clicked();
    void on_action_Show_Fourier_Transform_changed();
    void on_action_Show_Filtered_changed();
    void on_MultiplePeaksCheckBox_stateChanged(int state);
    void savePlotAsPng();
    void savePlotAsJpeg();
    void savePlotAsPdf();
    void onTimeout();
    void windowEnable();
    void windowDisable();
    void on_action_Show_Histogram_triggered();
    void on_action_Event_Filter_triggered();
    void onEventFilterChanged();
    void updateEventCountDisplay();

signals:
    void progressUpdated(int value);
};
#endif // MAINWINDOW_H