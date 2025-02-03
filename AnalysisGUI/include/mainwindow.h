#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QCustomPlot *customPlot;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    int64_t currEvent = 1;
    int16_t currChannel = 0;
    double currentX = 0;
    int32_t xLeftBoundary = -1000;
    int32_t xRightBoundary = 3000;
    QLabel* coordinateLabel = new QLabel();
    Worker DFR;
    QLineEdit *LeftBoundaryEdit;
    QLineEdit *RightBoundaryEdit;
    QLineEdit *BranchName;
    QSpinBox *channelSpinBox;
    QSpinBox *eventSpinBox;
    std::map<int, ConfigManager*> channels;
    QCPItemLine *lineLeft;
    QCPItemLine *lineRight;
    QCPLayer *customLayer;
    int32_t size;
    QTimer *timer = new QTimer();
    QString fileName = "";

    QAction *action_UseSpline;
    QAction *action_UseSmartScope;
    QAction *action_Signal_is_Negative;

    void setupGraph();
    void UpdateGraph();
    void ReDrawBoundaries();
    void showContextMenu(const QPoint &pos);
    void processFiles(const QStringList &files);

    ctpl::thread_pool p;


private slots:

    void onMouseMove(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event) override;
    void on_NextEventButton_clicked();
    void on_PreviousEventButton_clicked();
    void on_SetFileAnalysisButton_clicked();
    void on_action_Open_triggered();
    void on_action_Open_Config_triggered();
    // void on_action_Use_Smart_Boarders();
    // void on_action_Use_Spline();
    void on_channelSpinBox_valueChanged();
    void on_eventSpinBox_valueChanged();
    void on_SaveConfigButton_clicked();
    void on_SetChannelBoundaries_clicked();
    void on_SetAllChannelsBoundaries_clicked();
    void on_setBranchName_clicked(); 
    // void savePlot();
    void savePlotAsPng();
    void savePlotAsJpeg();
    void savePlotAsPdf();
    void onTimeout();
protected:
    void mousePressEvent(QMouseEvent *event) override;
signals:
    void progressUpdated(int value);


};
#endif // MAINWINDOW_H
