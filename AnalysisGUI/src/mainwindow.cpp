#include "mainwindow.h"
#include <QCheckBox>
#include "./ui_mainwindow.h"
#include "qcustomplot.h"
#include <qnamespace.h>
#include <QStringList>
#include "DataFileReader.h"
#include "thread"
#include "FourierFilter.h"
#include <QScrollArea>
#include "HistogramWindow.h"
#include "EventFilterWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), p(8)
{
    ui->setupUi(this);
    ui->LeftBoundaryLabel->setStyleSheet("QLabel { color : green; }");
    ui->RightBoundaryLabel->setStyleSheet("QLabel { color : red; }");

    customPlot = ui->customPlot;
    FileNameLabel = ui->FileNameLabel;
    coordinateLabel = ui->coordinateLabel;
    LeftBoundaryEdit = ui->LeftBoundaryEdit;
    RightBoundaryEdit = ui->RightBoundaryEdit;
    BranchName = ui->BranchName;
    channelSpinBox = ui->channelSpinBox;
    eventSpinBox = ui->eventSpinBox;
    FrequencySpinBox = ui->FrequencySpinBox;
    ThreadsSpinBox = ui->ThreadsSpinBox;
    ThreadsSpinBox->setValue((int32_t)p.size());
    WriteMode = "Single";

    setupGraph();
    connect(ui->MultiplePeaksCheckBox, &QCheckBox::stateChanged, this, &MainWindow::on_MultiplePeaksCheckBox_stateChanged);

    connect(customPlot, &QCustomPlot::mouseMove, this, &MainWindow::onMouseMove);

    for (int i = 0; i < (new DataFormat)->adcmap.size() * 64; i++)
        channels[i] = new ConfigManager(i, "channel_" + to_string(i + 1), 0.0, 2048., 1, 1, 1, 0, 0);

    LeftBoundaryEdit->setText(QString("%1").arg(xLeftBoundary));
    RightBoundaryEdit->setText(QString("%1").arg(xRightBoundary));
    BranchName->setText(QString::fromStdString(channels[currChannel]->name));

    customLayer = new QCPLayer(customPlot, "CustomLayer");
    customPlot->addLayer("CustomLayer", customPlot->layer("main"), QCustomPlot::LayerInsertMode::limAbove);

    lineLeft = new QCPItemLine(customPlot);
    lineLeft->setLayer("CustomLayer");
    lineLeft->setPen(QPen(Qt::green, 4));
    lineRight = new QCPItemLine(customPlot);
    lineRight->setLayer("CustomLayer");
    lineRight->setPen(QPen(Qt::red, 4));

    // Initialize action pointers
    action_UseSpline = ui->action_Use_Spline;
    action_UseSmartScope = ui->action_Use_Smart_Boarders;
    action_Signal_is_Negative = ui->action_Signal_is_Negative;
    action_Show_Fourier_Transform = ui->action_Show_Fourier_Transform;
    action_Show_Filtered = ui->action_Show_Filtered;
    action_Use_Fourier_Filtering = ui->action_Use_Fourier_Filtering;
    action_Event_Filter = ui->action_Event_Filter;
    actionSavePng = ui->actionSavePng;
    actionSaveJpeg = ui->actionSaveJpeg;
    actionSavePdf = ui->actionSavePdf;
    action_Show_Histogram = ui->action_Show_Histogram;

    connect(actionSavePng, &QAction::triggered, this, &MainWindow::savePlotAsPng);
    connect(actionSaveJpeg, &QAction::triggered, this, &MainWindow::savePlotAsJpeg);
    connect(actionSavePdf, &QAction::triggered, this, &MainWindow::savePlotAsPdf);

    // Connect Event Filter action

    connect(LeftBoundaryEdit, &QLineEdit::textChanged, [=](QString obj)
            { xLeftBoundary = obj.toInt(); ReDrawBoundaries(); });
    connect(RightBoundaryEdit, &QLineEdit::textChanged, [=](QString obj)
            { xRightBoundary = obj.toInt(); ReDrawBoundaries(); });

    connect(action_Show_Histogram, &QAction::triggered, this, &MainWindow::on_action_Show_Histogram_triggered);

    if (!action_Event_Filter)
    {
        qDebug() << "ERROR: action_Event_Filter not found in UI!";
        // Create it programmatically if missing
        action_Event_Filter = new QAction("&Event Filter", this);
        ui->menuGraphics->addAction(action_Event_Filter);
    }

    // Connect the action
    connect(action_Event_Filter, &QAction::triggered,
            this, &MainWindow::on_action_Event_Filter_triggered);

    m_histogramWindow = nullptr;
    m_eventFilterWidget = nullptr; // Initialize event filter widget pointer
}

void MainWindow::setupGraph()
{
    size = 350;

    QVector<double> x(size);
    QVector<double> y(size);
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(Qt::blue, 2, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    // customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 1.5), QBrush(Qt::white), 9));
    for (int i = 0; i < size; ++i)
    {
        x[i] = i;                    // x от 0 до 10
        y[i] = 20000 * sin(0.2 * i); // x от 0 до 10
    }
    QPen pen(Qt::black, 2);
    customPlot->xAxis->setBasePen(pen);
    customPlot->yAxis->setBasePen(pen);
    QFont axisFont("Arial", 20); // Arial font, size 12
    axisFont.setBold(true);
    customPlot->xAxis->setLabelFont(axisFont);
    customPlot->xAxis->setLabelFont(axisFont);

    customPlot->xAxis->setLabelFont(axisFont);
    customPlot->xAxis->setTickLabelFont(axisFont);
    customPlot->yAxis->setLabelFont(axisFont);
    customPlot->yAxis->setTickLabelFont(axisFont);
    customPlot->graph(0)->setData(x, y);
    customPlot->graph(0)->setPen(QPen(Qt::blue, 3));
    // customPlot->xAxis->setLabel("Time (μs)");
    customPlot->xAxis->setLabel("Time (time steps)");
    customPlot->yAxis->setLabel("ADC Channels");
    customPlot->xAxis->setRange(0, 2048);
    customPlot->yAxis->setRange(-30000, 30000);
    customPlot->xAxis->rescale();

    customPlot->yAxis->rescale();
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    customPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    customPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    customPlot->axisRect()->setRangeZoomAxes(customPlot->xAxis, customPlot->yAxis); // To block y axis zoom NULL axis as horizontal
    customPlot->setSelectionRectMode(QCP::srmZoom);

    customPlot->replot(QCustomPlot::rpQueuedRefresh);
    customPlot->addGraph();
    customPlot->graph(1)->setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
}

void MainWindow::UpdateGraph()
{
    size = DFR.event_waveform.wf.size();
    if (size > 1)
    {
        QVector<double> x(size);
        for (int i = 0; i < size; ++i)
            x[i] = i;

        if (action_Show_Fourier_Transform->isChecked())
        {
            int32_t gate = 0;
            if (xLeftBoundary > 0 && xLeftBoundary < DFR.event_waveform.wf.size())
                gate = xLeftBoundary;
            else
                gate = 20;
            DFR.event_waveform.Set_Zero_Level_Area(gate);
            FourierFilter FF(DFR.event_waveform.wf, DFR.event_waveform.Get_Zero_Level(), gate);
            FF.forwardTransform();
            vector<double> y0 = FF.getFourierTransformedSignal();
            QVector<double> y(y0.begin(), y0.end());
            customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
            customPlot->graph(0)->setData(x, y);
            customPlot->graph(1)->data()->clear();
        }
        else
        {
            QVector<double> y(DFR.event_waveform.wf.begin(), DFR.event_waveform.wf.end());
            customPlot->graph(0)->setData(x, y);
            customPlot->yAxis->setScaleType(QCPAxis::stLinear);

            if (action_Show_Filtered->isChecked())
            {
                int32_t gate = 0;
                if (xLeftBoundary > 0 && xLeftBoundary < DFR.event_waveform.wf.size())
                    gate = xLeftBoundary;
                else
                    gate = 20;
                DFR.event_waveform.Set_Zero_Level_Area(gate);
                FourierFilter FF(DFR.event_waveform.wf, DFR.event_waveform.Get_Zero_Level(), gate);
                FF.forwardTransform();
                if (passfilter > 0 && passfilter < DFR.event_waveform.wf.size())
                {
                    FF.applyLowPassFilter(passfilter);
                    FF.backwardTransform();
                    vector<int32_t> y0 = FF.getFilteredSignal();
                    QVector<double> yfilter(y0.begin(), y0.end());
                    customPlot->graph(1)->setData(x, yfilter);
                }
                else
                    customPlot->graph(1)->data()->clear();
            }
            else
                customPlot->graph(1)->data()->clear();
        }

        customPlot->yAxis->rescale();
        customPlot->xAxis->rescale();
        customPlot->replot();

        ChannelEntry tempWfData = DFR.event_waveform;
        // Calculate waveform parameters if histogram window is visible
        if (m_histogramWindow && m_histogramWindow->isVisible() && tempWfData.wf.size() > 0)
        {
            PeaksInfo *sci = new PeaksInfo();
            if (channels[currChannel])
            {
                tempWfData.Set_Zero_Level_Area(channels[currChannel]->leftBoundary);
            }
            else
            {
                tempWfData.Set_Zero_Level_Area(60);
            }
            sci->zl = tempWfData.CalculateZlwithNoisePeaks(130);
            sci->zl_rms = tempWfData.Get_Zero_Level_RMS();

            if (channels[currChannel])
            {
                tempWfData.SetBoarders(channels[currChannel]->leftBoundary, channels[currChannel]->rightBoundary);
            }
            else
            {
                tempWfData.SetBoarders(50, 100);
            }

            sci->ADC_ID = tempWfData.ADCID;
            int count = 0;

            while (true)
            {
                SinglePeakInfo peakInfo;

                tempWfData.SetBoarders(channels[currChannel]->leftBoundary, channels[currChannel]->rightBoundary);

                int pp = tempWfData.Get_time();
                peakInfo.amp = tempWfData.Get_Amplitude();
                if (channels[currChannel] && channels[currChannel]->UseSmartScope == 1)
                    tempWfData.AssumeSmartScope();
                peakInfo.time = tempWfData.Get_time_gauss();

                peakInfo.charge = tempWfData.Get_Charge();
                peakInfo.II = tempWfData.GetIntegralInfo();
                if (peakInfo.amp < 500 && count > 0)
                    break;
                sci->AddPeak(peakInfo);
                if (WriteMode == "Single")
                    break;
                count++;
                tempWfData.DeleteCurrentPeak();
            }
            cout << sci->amp() << "  " << sci->charge() << " " << sci->time() << endl;
            m_histogramWindow->setEventValues(sci->amp(), sci->charge(), sci->time());
            delete sci;
        }
    }
}

void MainWindow::onMouseMove(QMouseEvent *event)
{
    double x = customPlot->xAxis->pixelToCoord(event->pos().x());
    double y = customPlot->yAxis->pixelToCoord(event->pos().y());
    currentX = x;
    coordinateLabel->setText(QString("X: %1, Y: %2").arg(x).arg(y));
}

void MainWindow::ReDrawBoundaries()
{

    lineLeft->start->setCoords(xLeftBoundary, -32767000);
    lineLeft->end->setCoords(xLeftBoundary, 32767000);
    lineRight->start->setCoords(xRightBoundary, -32767000);
    lineRight->end->setCoords(xRightBoundary, 32767000);
    customPlot->replot(QCustomPlot::rpQueuedRefresh);
}

void MainWindow::on_channelSpinBox_valueChanged(int)
{
    currChannel = (int16_t)channelSpinBox->value();
    if (channels[currChannel])
        BranchName->setText(QString::fromStdString(channels[currChannel]->name));
    if (xLeftBoundary != channels[currChannel]->leftBoundary)
    {
        xLeftBoundary = channels[currChannel]->leftBoundary;
        LeftBoundaryEdit->setText(QString("%1").arg(xLeftBoundary));
    }

    if (xRightBoundary != channels[currChannel]->rightBoundary)
    {
        xRightBoundary = channels[currChannel]->rightBoundary;
        RightBoundaryEdit->setText(QString("%1").arg(xRightBoundary));
    }
    action_UseSmartScope->setChecked(channels[currChannel]->UseSmartScope);
    action_UseSpline->setChecked(channels[currChannel]->UseSpline);
    action_Signal_is_Negative->setChecked(channels[currChannel]->SignalNegative);

    // Update histogram window channel if it exists and is visible
    if (m_histogramWindow && !m_histogramWindow.isNull() && m_histogramWindow->isVisible())
    {
        m_histogramWindow->onChannelChanged(currChannel);
    }

    // ReDrawBoundaries();
}

void MainWindow::on_eventSpinBox_valueChanged(int)
{
    // Skip if we're programmatically changing the event
    if (m_programmaticallyChangingEvent)
        return;

    DFR.event_waveform.Initialize();
    currEvent = (int32_t)eventSpinBox->value();
    if (DFR.FileIsSet == 1 && currEvent < DFR.GetTotalEvents() && currEvent >= 0)
    {
        DFR.ReadEvent(currEvent, currChannel);

        if (!m_eventFilterWidget || !m_eventFilterWidget->isFilteringEnabled() || currentEventPassesFilters())
        {
            UpdateGraph();
        }
    }
}

void MainWindow::on_FrequencySpinBox_valueChanged(int)
{
    passfilter = (int32_t)FrequencySpinBox->value();
    if (DFR.FileIsSet == 1 && currEvent < DFR.GetTotalEvents() && currEvent >= 0 && action_Show_Filtered->isChecked() && !action_Show_Fourier_Transform->isChecked())
    {
        UpdateGraph();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_A)
    {
        xLeftBoundary = (int32_t)currentX;
        ReDrawBoundaries();
        LeftBoundaryEdit->setText(QString("%1").arg(xLeftBoundary));
    }
    if (event->key() == Qt::Key_D)
    {
        xRightBoundary = (int32_t)currentX;
        ReDrawBoundaries();
        RightBoundaryEdit->setText(QString("%1").arg(xRightBoundary));
    }
    if (event->key() == Qt::Key_F)
    {
        FrequencySpinBox->setValue((int32_t)currentX);
    }
    if (event->key() == Qt::Key_F5)
    {
        ReDrawBoundaries();
    }
}

void MainWindow::on_NextEventButton_clicked()
{
    if (m_programmaticallyChangingEvent)
        return;

    m_programmaticallyChangingEvent = true;

    if (currEvent < 0)
        currEvent = 0;

    if (DFR.FileIsSet == 1 && currEvent < DFR.GetTotalEvents() - 1)
    {
        bool NonEmpty = false;
        int originalEvent = currEvent;

        // Start from next event
        currEvent++;

        while (currEvent < DFR.GetTotalEvents())
        {
            DFR.ReadEvent(currEvent, currChannel);

            if (DFR.event_waveform.wf.size() > 0)
            {
                // Check if event passes filters
                if (!m_eventFilterWidget || !m_eventFilterWidget->isFilteringEnabled() ||
                    currentEventPassesFilters())
                {
                    NonEmpty = true;
                    eventSpinBox->setValue(currEvent);
                    UpdateGraph(); // Call UpdateGraph here directly
                    break;
                }
            }

            // Try next event
            currEvent++;
        }

        if (!NonEmpty)
        {
            std::cout << "No more events matching filters" << std::endl;
            currEvent = originalEvent; // Restore original position
            eventSpinBox->setValue(currEvent);
        }
    }

    m_programmaticallyChangingEvent = false;
}

void MainWindow::on_PreviousEventButton_clicked()
{
    if (m_programmaticallyChangingEvent)
        return;

    m_programmaticallyChangingEvent = true;

    if (currEvent > DFR.GetTotalEvents())
        currEvent = DFR.GetTotalEvents();

    if (DFR.FileIsSet == 1 && currEvent >= 1)
    {
        bool NonEmpty = false;
        int originalEvent = currEvent;

        // Start from previous event
        currEvent--;

        while (currEvent >= 0)
        {
            DFR.ReadEvent(currEvent, currChannel);

            if (DFR.event_waveform.wf.size() > 0)
            {
                // Check if event passes filters
                if (!m_eventFilterWidget || !m_eventFilterWidget->isFilteringEnabled() ||
                    currentEventPassesFilters())
                {
                    NonEmpty = true;
                    eventSpinBox->setValue(currEvent);
                    UpdateGraph(); // Call UpdateGraph here directly
                    break;
                }
            }

            // Try previous event
            currEvent--;
        }

        if (!NonEmpty)
        {
            std::cout << "No previous events matching filters" << std::endl;
            currEvent = originalEvent; // Restore original position
            eventSpinBox->setValue(currEvent);
        }
    }

    m_programmaticallyChangingEvent = false;
}

void MainWindow::on_SetChannelBoundaries_clicked()
{
    channels[currChannel]->leftBoundary = xLeftBoundary;
    channels[currChannel]->rightBoundary = xRightBoundary;
    channels[currChannel]->UseSpline = action_UseSpline->isChecked();
    channels[currChannel]->UseSmartScope = action_UseSmartScope->isChecked();
    channels[currChannel]->SignalNegative = action_Signal_is_Negative->isChecked();
    channels[currChannel]->UseFourierFiltering = action_Signal_is_Negative->isChecked();
    channels[currChannel]->FrequencyCutoff = passfilter;
}

void MainWindow::on_SetAllChannelsBoundaries_clicked()
{
    for (int i = 0; i < (new DataFormat)->adcmap.size() * 64; i++)
    {
        channels[i]->leftBoundary = xLeftBoundary;
        channels[i]->rightBoundary = xRightBoundary;
        channels[i]->UseSpline = action_UseSpline->isChecked();
        channels[i]->UseSmartScope = action_UseSmartScope->isChecked();
        channels[i]->SignalNegative = action_Signal_is_Negative->isChecked();
    }
}

void MainWindow::on_setBranchName_clicked()
{
    channels[currChannel]->name = BranchName->displayText().toStdString();
}

void MainWindow::on_SaveConfigButton_clicked()
{
    // cout << fileName.left(fileName.lastIndexOf(".")).toUtf8().toStdString() << endl;
    if (fileName != "")
        ConfigManager::saveToJson(fileName.left(fileName.lastIndexOf(".")).toUtf8().toStdString() + ".json", channels);
    // ConfigManager::loadFromJson();
}

void MainWindow::on_action_Open_triggered()
{
    fileName = QFileDialog::getOpenFileName(
        this, tr("Open File"), "~",
        tr("binary files ( *.data *.bin)"));
    auto a = (fileName.toUtf8().constData());
    QApplication::processEvents();

    if (fileName != "")
    {
        FileNameLabel->setText("Open File: " + fileName);
        std::cout << a << " is set" << std::endl;
        DFR.setName(a);
        ProgressDialog *progressDialog = new ProgressDialog(this);

        // Worker *worker = new Worker();
        QThread *thread = new QThread();

        DFR.moveToThread(thread);
        // QTimer *timer = new QTimer();
        connect(timer, &QTimer::timeout, this, &MainWindow::onTimeout);
        timer->start(100);
        connect(thread, &QThread::started, [=]()
                { DFR.doWork(progressDialog, a); });
        connect(this, &MainWindow::progressUpdated, progressDialog, &ProgressDialog::updateProgress);

        // connect(timer, &QTimer::timeout, progressDialog, &ProgressDialog::updateProgress);
        connect(progressDialog, &QDialog::finished, thread, &QThread::quit);
        // connect(thread, &QThread::finished, worker, &QObject::deleteLater);
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);
        connect(thread, &QThread::finished, timer, &QTimer::stop);

        progressDialog->show();
        thread->start();
    }
    else
    {
        std::cout << "File NOT chosen" << std::endl;
    }
}

void MainWindow::on_action_Open_Config_triggered()
{
    QString configName = QFileDialog::getOpenFileName(
        this, tr("Open File"), "~",
        tr("config files ( *.json)"));
    auto a = (fileName.toUtf8().constData());
    if (configName != "")
    {
        std::cout << configName.toStdString() << endl;
        // channels.clear();
        channels = ConfigManager::loadFromJson(configName.toStdString());
    }
    // }
    else
    {
        std::cout << "File NOT chosen" << std::endl;
    }
}

void MainWindow::on_action_Show_Fourier_Transform_changed()
{
    if (DFR.FileIsSet == 1 && currEvent < DFR.GetTotalEvents() && currEvent >= 0)
        UpdateGraph();
}

void MainWindow::on_action_Show_Filtered_changed()
{
    if (DFR.FileIsSet == 1 && currEvent < DFR.GetTotalEvents() && currEvent >= 0)
        UpdateGraph();
}

// void MainWindow::mousePressEvent(QMouseEvent *event){
//     if (event->button() == Qt::RightButton) {
//             showContextMenu(event->globalPos());
//     }
//     // Вызываем базовый класс для обработки других событий
//     QMainWindow::mousePressEvent(event);
// }

void MainWindow::windowEnable()
{
    setEnabled(true);
}

void MainWindow::windowDisable()
{
    setEnabled(false);
}

void MainWindow::showContextMenu(const QPoint &pos)
{

    windowDisable();
    QMenu contextMenu(tr("Save Menu"), this);
    // contextMenu.setModal(true);
    // connect(&contextMenu, &QMenu::aboutToShow, this, &MainWindow::windowDisable);
    connect(&contextMenu, &QMenu::aboutToHide, this, &MainWindow::windowEnable);

    QAction actionSavePng("Save Plot as PNG", this);
    connect(&actionSavePng, &QAction::triggered, this, &MainWindow::savePlotAsPng);

    QAction actionSaveJpeg("Save Plot as JPEG", this);
    connect(&actionSaveJpeg, &QAction::triggered, this, &MainWindow::savePlotAsJpeg);

    QAction actionSavePdf("Save Plot as PDF", this);
    connect(&actionSavePdf, &QAction::triggered, this, &MainWindow::savePlotAsPdf);

    customPlot->clearItems();

    contextMenu.addAction(&actionSavePng);
    contextMenu.addAction(&actionSaveJpeg);
    contextMenu.addAction(&actionSavePdf);

    contextMenu.exec(pos);
}

void MainWindow::savePlotAsPng()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Plot as PNG"), "", tr("PNG Files (*.png)"));
    if (!fileName.isEmpty())
    {
        customPlot->savePng(fileName);
    }
}

void MainWindow::savePlotAsJpeg()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Plot as JPEG"), "", tr("JPEG Files (*.jpeg *.jpg)"));
    if (!fileName.isEmpty())
    {
        customPlot->saveJpg(fileName);
    }
}

void MainWindow::savePlotAsPdf()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Plot as PDF"), "", tr("PDF Files (*.pdf)"));
    if (!fileName.isEmpty())
    {
        customPlot->savePdf(fileName);
    }
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTimeout()
{
    // Update the value in the custom class
    emit progressUpdated(1000 * DFR.getIndexationProgress());
    timer->start(100);
}

void MainWindow::on_SetFileAnalysisButton_clicked()
{
    // Create a QFileDialog instance
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("Select one or more files to open");
    fileDialog->setDirectory(QFileInfo(fileName).absolutePath());
    fileDialog->setNameFilter("Binary files (*.data)");
    fileDialog->setFileMode(QFileDialog::ExistingFiles);

    // Connect the accepted signal to a custom slot
    connect(fileDialog, &QFileDialog::accepted, this, [this, fileDialog]()
            {
                QStringList files = fileDialog->selectedFiles();
                processFiles(files);       // Call the method to process selected files
                fileDialog->deleteLater(); // Clean up the dialog
            });

    // Show the dialog
    fileDialog->show();
    return;
}

void MainWindow::processFiles(const QStringList &files)
{
    // Initialize progress data in main thread
    std::vector<Progress *> progress_vector;
    for (const auto &file : files)
    {
        progress_vector.push_back(new Progress(file.toStdString()));
    }

    // Create and show progress widget in main thread
    // ProgressWidget* progressWidget = new ProgressWidget(progress_vector, this);
    // progressWidget->setAttribute(Qt::WA_DeleteOnClose);
    QPointer<ProgressWidget> progressWidget = new ProgressWidget(progress_vector, this);
    progressWidget->setAttribute(Qt::WA_DeleteOnClose);
    progressWidget->setWindowModality(Qt::ApplicationModal);

    // Set proper window flags for resizing
    progressWidget->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint |
                                   Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    progressWidget->show();

    // Create thread pool
    p.~thread_pool();
    new (&p) ctpl::thread_pool((int32_t)ThreadsSpinBox->value() + 1);

    // Create update timer in main thread
    QTimer *updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, progressWidget, &ProgressWidget::updateProgress);
    updateTimer->start(200); // Update every 200ms for smooth progress

    // Connect progress updates
    connect(progressWidget, &ProgressWidget::requestUpdate, this, [=]()
            { progressWidget->setProgressList(progress_vector); });
    std::atomic<bool> *StopAnalysis = new std::atomic<bool>(false);

    p.push([=](int id)
           {

        while (StopAnalysis->load()==false) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Request UI update in main thread
            if (progressWidget) QMetaObject::invokeMethod(progressWidget, [progressWidget]() {
                progressWidget->updateProgress();
            }, Qt::QueuedConnection);

        } });
    for (auto &analysis_process : progress_vector)
    {
        p.push([=](int id)
               {
                    if (StopAnalysis->load() || !progressWidget)
                        return;

                    QMetaObject::invokeMethod(progressWidget, &ProgressWidget::requestUpdate, Qt::QueuedConnection);

                    connect(progressWidget, &ProgressWidget::requestUpdate, this, [=]()
                            { progressWidget->setProgressList(progress_vector); });

                    // Process file
                    analysis_process->active = true;
                    DataFileReader DFR1;
                    DFR1.setName(analysis_process->fileName.c_str(), channels);
                    DFR1.SetWriteMode(WriteMode);
                    DFR1.CreateRootFile();
                    auto connection = connect(progressWidget, &ProgressWidget::aboutToClose, [&DFR1, &StopAnalysis]()
                            {DFR1.SetStopAnalysis(true);StopAnalysis->store(true); });
                    DFR1.ConsequentialEventsReading(analysis_process);
                    DFR1.SaveRootFile();
                    analysis_process->active = false;
                    analysis_process->processed = true;
                    disconnect(connection);
                    // Final update
                    QMetaObject::invokeMethod(progressWidget, &ProgressWidget::requestUpdate, Qt::QueuedConnection); });
    }

    // Handle completion
    QTimer *completionTimer = new QTimer(this);
    connect(completionTimer, &QTimer::timeout, this, [=]()
            {
        bool all_done = true;
        for (auto progress : progress_vector) {
            if (!progress->processed) {
                all_done = false;
                break;
            }
        }
        
        if (all_done || StopAnalysis->load()==true) {
            updateTimer->stop();
            completionTimer->stop();
            // progressWidget->close();
            updateTimer->deleteLater();
            completionTimer->deleteLater();
            // progressWidget->~ProgressWidget();
            delete StopAnalysis; 
            return;
        } });
    completionTimer->start(500);
}

void MainWindow::on_MultiplePeaksCheckBox_stateChanged(int state)
{
    WriteMode = (state == Qt::Checked) ? "Multiple" : "Single";
}

void MainWindow::on_action_Show_Histogram_triggered()
{
    if (m_histogramWindow.isNull())
    {
        m_histogramWindow = new HistogramWindow(this);
        m_histogramWindow->setAnalysisThreadPool(&p);
    }

    // Set the current channel from main window to histogram window
    m_histogramWindow->onChannelChanged(currChannel);

    // if (DFR.FileIsSet)
    // {
    //     // You'll need to implement getRootFilePath() in your DataFileReader class
    //     // For now, using a placeholder - adjust according to your actual file naming
    //     QString rootFilePath = QString::fromStdString(DFR.fileName) + ".root";
    //     m_histogramWindow->loadRootFile(rootFilePath, currChannel);
    // }

    m_histogramWindow->show();
    m_histogramWindow->raise();
    m_histogramWindow->activateWindow();
}

void MainWindow::on_action_Event_Filter_triggered()
{
    qDebug() << "Event Filter action triggered!";

    if (m_eventFilterWidget.isNull())
    {
        qDebug() << "Creating new EventFilterWidget";
        m_eventFilterWidget = new EventFilterWidget(this); // 'this' as parent

        // Set window flags to ensure it's a proper top-level window
        m_eventFilterWidget->setWindowFlags(Qt::Window | Qt::WindowTitleHint |
                                            Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

        // Make it modal to the main window
        m_eventFilterWidget->setWindowModality(Qt::ApplicationModal);

        connect(m_eventFilterWidget, &EventFilterWidget::filtersChanged,
                this, &MainWindow::onEventFilterChanged);
    }

    qDebug() << "Showing EventFilterWidget at position:" << pos();

    // Show and activate
    m_eventFilterWidget->show();
    m_eventFilterWidget->raise();
    m_eventFilterWidget->activateWindow();

    // Force it to be on top
    m_eventFilterWidget->setWindowState((m_eventFilterWidget->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    m_eventFilterWidget->setFocus();
}

void MainWindow::onEventFilterChanged()
{
    // Optional: Update UI or perform any action when filters change
}
bool MainWindow::currentEventPassesFilters()
{
    if (!m_eventFilterWidget || m_eventFilterWidget.isNull() ||
        !m_eventFilterWidget->isFilteringEnabled() ||
        !DFR.FileIsSet || DFR.event_waveform.wf.size() == 0)
        return true;

    // Create a DEEP COPY of the waveform for filtering analysis
    ChannelEntry filterWaveform = DFR.event_waveform;
    filterWaveform.wf = DFR.event_waveform.wf; // This creates a copy of the vector

    // Set up the waveform EXACTLY as in UpdateGraph()
    if (channels[currChannel])
    {
        filterWaveform.Set_Zero_Level_Area(channels[currChannel]->leftBoundary);
    }
    else
    {
        filterWaveform.Set_Zero_Level_Area(60);
    }

    // Calculate ZL and ZL_RMS like in UpdateGraph()
    filterWaveform.CalculateZlwithNoisePeaks(130);
    // float zl_rms = filterWaveform.Get_Zero_Level_RMS(); // Uncomment if needed

    if (channels[currChannel])
    {
        filterWaveform.SetBoarders(channels[currChannel]->leftBoundary,
                                   channels[currChannel]->rightBoundary);
    }
    else
    {
        filterWaveform.SetBoarders(50, 100);
    }

    // Create a PeaksInfo object to get aggregate values
    PeaksInfo peaksInfo;

    int count = 0;

    // Use the EXACT SAME loop structure as in UpdateGraph()
    while (true)
    {
        SinglePeakInfo peakInfo;

        filterWaveform.SetBoarders(channels[currChannel]->leftBoundary,
                                   channels[currChannel]->rightBoundary);

        int pp = filterWaveform.Get_time(); // Added like in UpdateGraph()
        peakInfo.amp = filterWaveform.Get_Amplitude();

        if (channels[currChannel] && channels[currChannel]->UseSmartScope == 1)
            filterWaveform.AssumeSmartScope();

        peakInfo.time = filterWaveform.Get_time_gauss();
        peakInfo.charge = filterWaveform.Get_Charge();
        peakInfo.II = filterWaveform.GetIntegralInfo();

        // Use the SAME break condition as UpdateGraph()
        if (peakInfo.amp < 500 && count > 0)
            break;

        for (auto el : filterWaveform.wf)
            cout << el << " ";
        cout << endl;
        cout << WriteMode << "; Peak no =  " << count + 1 << "; Amp = " << peakInfo.amp << "; Charge = " << peakInfo.charge << "; Time = " << peakInfo.time << endl;
        peaksInfo.AddPeak(peakInfo);

        if (WriteMode == "Single")
            break;

        count++;
        filterWaveform.DeleteCurrentPeak();

        // Safety check
        if (filterWaveform.wf.size() == 0)
        {
            break;
        }
    }

    // Now use the aggregate functions
    float totalAmp = peaksInfo.amp();
    float totalCharge = peaksInfo.charge();
    float totalTime = peaksInfo.time();

    qDebug() << "Multiple peaks aggregate - amp:" << totalAmp
             << "charge:" << totalCharge << "time:" << totalTime;
    return m_eventFilterWidget->eventPassesFilters(totalAmp, totalCharge, totalTime);
}
