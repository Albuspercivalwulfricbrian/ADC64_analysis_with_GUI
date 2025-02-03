#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "qcustomplot.h"
#include <qnamespace.h>
#include <QStringList>
#include "DataFileReader.h"
#include "thread"
// #include "FFTFilter.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), p(8)
{
    ui->setupUi(this);
    ui->LeftBoundaryLabel->setStyleSheet("QLabel { color : green; }");
    ui->RightBoundaryLabel->setStyleSheet("QLabel { color : red; }");
    // QToolTip::showText( ui->LeftBoundaryLabel->mapToGlobal( QPoint( 0, 0 ) ), "errorString" );
    customPlot=ui->customPlot;
    // ui->LeftBoundaryLabel->setToolTip("press \"A\" to win");
    coordinateLabel=ui->coordinateLabel;
    LeftBoundaryEdit=ui->LeftBoundaryEdit;
    RightBoundaryEdit=ui->RightBoundaryEdit;
    BranchName=ui->BranchName;
    channelSpinBox=ui->channelSpinBox;
    eventSpinBox=ui->eventSpinBox;
    setupGraph();
    int64_t counter = 0;
    connect(customPlot, &QCustomPlot::mouseMove, this, &MainWindow::onMouseMove);
    connect(customPlot, &QCustomPlot::mousePress, this, &MainWindow::mousePressEvent);
    // connect(customPlot, &QCustomPlot::keyPressEvent, this, &MainWindow::keyPressEvent);
    for (int i = 0; i < 128; i++) channels[i] = new ConfigManager(i, "channel_"+to_string(i+1), 0.0, 2048.,1,1,1);
    LeftBoundaryEdit->setText(QString("%1").arg(xLeftBoundary));
    RightBoundaryEdit->setText(QString("%1").arg(xRightBoundary));
    BranchName->setText(QString::fromStdString(channels[currChannel]->name));
    // graphLayer = new QCPLayer(customPlot, "graphLayer");
    customLayer = new QCPLayer(customPlot, "CustomLayer");
    customPlot->addLayer("CustomLayer", customPlot->layer("main"), QCustomPlot::LayerInsertMode::limAbove);
    // customLayer->setMode(QCPLayer::lmBuffered);

    // line = new QCPItemLine(customPlot);
    // line->setLayer("CustomLayer");
    // line->setPen(QPen(Qt::red,2)); // Задаем цвет линии
    lineLeft = new QCPItemLine(customPlot);
    lineLeft->setLayer("CustomLayer");
    lineLeft->setPen(QPen(Qt::green,4)); // Задаем цвет линии
    lineRight = new QCPItemLine(customPlot);
    lineRight->setLayer("CustomLayer");
    lineRight->setPen(QPen(Qt::red,4)); // Задаем цвет линии
    action_UseSpline=ui->action_Use_Spline;
    action_UseSmartScope=ui->action_Use_Smart_Boarders;
    action_Signal_is_Negative= ui->action_Signal_is_Negative;
    // graphLayer->setMode( QCPLayer::LayerMode::lmLogical);

    connect(LeftBoundaryEdit, &QLineEdit::textChanged, [=](QString obj) { xLeftBoundary = obj.toInt(); ReDrawBoundaries(); });
    connect(RightBoundaryEdit, &QLineEdit::textChanged, [=](QString obj) { xRightBoundary = obj.toInt(); ReDrawBoundaries(); });
}

void MainWindow::setupGraph() {
    size = 350;   

    QVector<double> x(size);
    QVector<double> y(size);
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(Qt::blue,2, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    // customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 1.5), QBrush(Qt::white), 9));
    for (int i = 0; i < size; ++i) {
        x[i] = i; // x от 0 до 10
        y[i] = 20000*sin(0.2*i); // x от 0 до 10

    }    
    QPen pen(Qt::black, 2);
    customPlot->xAxis->setBasePen(pen);
    customPlot->yAxis->setBasePen(pen);    
    QFont axisFont("Arial", 15); // Arial font, size 12
    customPlot->xAxis->setLabelFont(axisFont);
    customPlot->xAxis->setLabelFont(axisFont);

    customPlot->xAxis->setLabelFont(axisFont);
    customPlot->xAxis->setTickLabelFont(axisFont);
    customPlot->yAxis->setLabelFont(axisFont);
    customPlot->yAxis->setTickLabelFont(axisFont);
    customPlot->graph(0)->setData(x, y);
    customPlot->xAxis->setLabel("Time (time steps)");
    customPlot->yAxis->setLabel("ADC Channels");
    customPlot->xAxis->setRange(0, 2048);
    customPlot->yAxis->setRange(-30000, 30000);
    customPlot->xAxis->rescale();

    customPlot->yAxis->rescale();
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    customPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    customPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    customPlot->axisRect()->setRangeZoomAxes(customPlot->xAxis, customPlot->yAxis); //To block y axis zoom NULL axis as horizontal
    customPlot->setSelectionRectMode(QCP::srmZoom);

    customPlot->replot(QCustomPlot::rpQueuedRefresh);
  
}

void MainWindow::UpdateGraph() {
    size = DFR.event_waveform.wf.size();   
    if (size > 1)
    {
        QVector<double> x(size);
        for (int i = 0; i < size; ++i) {
            x[i] = i; // x от 0 до 10
        }    
        QVector<double> y(DFR.event_waveform.wf.begin(), DFR.event_waveform.wf.end()); 

        customPlot->graph(0)->setData(x, y);
        customPlot->yAxis->rescale();
        customPlot->xAxis->rescale();
        customPlot->replot();        
    }
}
void MainWindow::onMouseMove(QMouseEvent *event) {
    // Получаем координаты курсора на графикеc
    double x = customPlot->xAxis->pixelToCoord(event->pos().x());
    double y = customPlot->yAxis->pixelToCoord(event->pos().y());
    // Обновляем текст метки с координатами
    currentX = x;
    coordinateLabel->setText(QString("X: %1, Y: %2").arg(x).arg(y));
        // ReDrawBoundaries();
}

void MainWindow::ReDrawBoundaries() {

    // customPlot->layer("main")->setMode(QCPLayer::lmLogical);
    // customLayer->setMode(QCPLayer::lmBuffered);
    //Отрисовка вертикального курсора
    // line->start->setCoords(currentX, -32767);
    // line->end->setCoords(currentX, 32767);
    lineLeft->start->setCoords(xLeftBoundary, -32767);
    lineLeft->end->setCoords(xLeftBoundary, 32767);
    lineRight->start->setCoords(xRightBoundary, -32767);
    lineRight->end->setCoords(xRightBoundary, 32767);
    customPlot->replot(QCustomPlot::rpQueuedRefresh);
}


void MainWindow::on_channelSpinBox_valueChanged()
{
    currChannel = (int16_t)channelSpinBox->value();
    if (channels[currChannel]) BranchName->setText(QString::fromStdString(channels[currChannel]->name));
    if (xLeftBoundary!=channels[currChannel]->leftBoundary) 
    {
        xLeftBoundary=channels[currChannel]->leftBoundary;
        LeftBoundaryEdit->setText(QString("%1").arg(xLeftBoundary));
    }

    if (xRightBoundary!=channels[currChannel]->rightBoundary)
    {
        xRightBoundary=channels[currChannel]->rightBoundary;
        RightBoundaryEdit->setText(QString("%1").arg(xRightBoundary));
    }
    // ReDrawBoundaries();
}

void MainWindow::on_eventSpinBox_valueChanged()
{
    DFR.event_waveform.Initialize();
    currEvent = (int32_t)eventSpinBox->value();
    if (DFR.FileIsSet == 1 && currEvent < DFR.GetTotalEvents())
    {
        DFR.ReadEvent(currEvent,currChannel);
        UpdateGraph();        
    }
}


void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_A) {xLeftBoundary = (int32_t)currentX;    ReDrawBoundaries(); LeftBoundaryEdit->setText(QString("%1").arg(xLeftBoundary));}
    if (event->key() == Qt::Key_D) {xRightBoundary = (int32_t)currentX;    ReDrawBoundaries(); RightBoundaryEdit->setText(QString("%1").arg(xRightBoundary));}
    if (event->key() == Qt::Key_F5) {ReDrawBoundaries();}
}

void MainWindow::on_NextEventButton_clicked() {
    if (DFR.FileIsSet == 1 && currEvent < DFR.GetTotalEvents()-1)
    {
        bool NonEmpty = 0;
        while (NonEmpty == 0)
        {
            currEvent++;
            if (currEvent < DFR.GetTotalEvents())
            {
                DFR.ReadEvent(currEvent,currChannel);

                if (DFR.event_waveform.wf.size() > 0) 
                {
                    NonEmpty = 1;   
                    eventSpinBox->setValue(currEvent);
                    // UpdateGraph();    
                }                
            }
            else {eventSpinBox->setValue(currEvent); std::cout << "Oops. Out of file Bounds" << std::endl; break;}

        }
     }
}
void MainWindow::on_PreviousEventButton_clicked() {
    DFR.event_waveform.Initialize();

    if (DFR.FileIsSet == 1 && currEvent >= 1)
    {
        bool NonEmpty = 0;
        while (NonEmpty == 0)
        {
            currEvent--;
            if (currEvent >= 0)
            {
                DFR.ReadEvent(currEvent,currChannel);
                if (DFR.event_waveform.wf.size() > 0) 
                {
                    NonEmpty = 1;   
                    eventSpinBox->setValue(currEvent);
                    // UpdateGraph();    
                }
            }
            else {eventSpinBox->setValue(currEvent); cout << "Oops. Out of file Bounds" << endl; break;}

        }        
    }
}

void MainWindow::on_SetChannelBoundaries_clicked() 
{
        channels[currChannel]->leftBoundary = xLeftBoundary;
        channels[currChannel]->rightBoundary = xRightBoundary;
        channels[currChannel]->UseSpline = action_UseSpline->isChecked();
        channels[currChannel]->UseSmartScope = action_UseSmartScope->isChecked();
        channels[currChannel]->SignalNegative = action_Signal_is_Negative->isChecked();
}

void MainWindow::on_SetAllChannelsBoundaries_clicked() 
{
    for (int i = 0; i < 128; i++) 
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


void MainWindow::on_SaveConfigButton_clicked() {
        // cout << fileName.left(fileName.lastIndexOf(".")).toUtf8().toStdString() << endl;
    if (fileName!="")
        ConfigManager::saveToJson(fileName.left(fileName.lastIndexOf(".")).toUtf8().toStdString()+".json", channels);
        // ConfigManager::loadFromJson();
}

void MainWindow::on_action_Open_triggered() {
  fileName = QFileDialog::getOpenFileName(
      this, tr("Open File"), "~",
      tr( "binary files ( *.data *.bin)"));
    auto a = (fileName.toUtf8().constData());
    QApplication::processEvents();

    if (fileName!="") {
        std::cout << a << " is set" << std::endl;
        DFR.setName(a);
        ProgressDialog *progressDialog = new ProgressDialog(this);
        
        // Worker *worker = new Worker();
        QThread *thread = new QThread();

        DFR.moveToThread(thread);
        // QTimer *timer = new QTimer();
        connect(timer, &QTimer::timeout, this, &MainWindow::onTimeout);
        timer->start(100);
        connect(thread, &QThread::started, [=]() { DFR.doWork(progressDialog, a); });
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

void MainWindow::on_action_Open_Config_triggered() {
    QString configName = QFileDialog::getOpenFileName(
      this, tr("Open File"), "~",
      tr( "config files ( *.json)"));
    auto a = (fileName.toUtf8().constData());
    if (configName!="")
    {
        std::cout << configName.toStdString() << endl;
        // channels.clear();
        channels=ConfigManager::loadFromJson(configName.toStdString());
    }
    // }
    else
    {
        std::cout << "File NOT chosen" << std::endl;
    }
    
}

void MainWindow::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::RightButton) {
            showContextMenu(event->globalPos());
    }
    // Вызываем базовый класс для обработки других событий
    QMainWindow::mousePressEvent(event);
}

void MainWindow::showContextMenu(const QPoint &pos) 
{


    QMenu contextMenu(tr("Save Menu"), this);

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

void MainWindow::savePlotAsPng() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Plot as PNG"), "", tr("PNG Files (*.png)"));
    if (!fileName.isEmpty()) {
        customPlot->savePng(fileName);
    }
}

void MainWindow::savePlotAsJpeg() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Plot as JPEG"), "", tr("JPEG Files (*.jpeg *.jpg)"));
    if (!fileName.isEmpty()) {
        customPlot->saveJpg(fileName);
    }
}

void MainWindow::savePlotAsPdf() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Plot as PDF"), "", tr("PDF Files (*.pdf)"));
    if (!fileName.isEmpty()) {
        customPlot->savePdf(fileName);
    }
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTimeout() {
    // Update the value in the custom class
    emit progressUpdated(1000*DFR.getIndexationProgress());
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
    connect(fileDialog, &QFileDialog::accepted, this, [this, fileDialog]() {
        QStringList files = fileDialog->selectedFiles();
        processFiles(files); // Call the method to process selected files
        fileDialog->deleteLater(); // Clean up the dialog
    });

    // Show the dialog
    fileDialog->show();
}

void MainWindow::processFiles(const QStringList &files)
{
    vector<Progress*> progress_vector;
    vector<std::string> *processed_files = nullptr;
    for (const auto &file : files) {
        progress_vector.push_back(new Progress(file.toStdString()));  
    }
    bool *changelayout = new bool(false);
    p.push(
        [&, progress_vector, processed_files, changelayout](int id)
        {
            std::unique_ptr<ProgressWidget> progressWidget = std::make_unique<ProgressWidget>(progress_vector, this);
            // ProgressWidget* progressWidget = new ProgressWidget(progress_vector, changelayout, this);
            progressWidget->show();
            while (true) 
            {

                // std::cout<< u8"\033[2J\033[1;1H"; 
                // std::cout << *changelayout << std::endl;
                // std::cout << "\rProgress:\n";
                // for (auto progress : progress_vector) {
                //     std::cout << "File: " << progress->fileName << " - " 
                //             << progress->percentage * 100  << "%; is active: " <<progress->active << 
                //             "is processed: " <<progress->processed <<  "\n";
                // }
                if (*changelayout==true)
                {
                    *changelayout=false;
                    progressWidget->close(); // Close the existing widget
                    progressWidget.reset();   // Resetting the unique pointer will delete the old instance
                    progressWidget = std::make_unique<ProgressWidget>(progress_vector, this);
                    progressWidget->show();
                }
                progressWidget->updateProgress();
                bool all_done = true;
                for (auto progress : progress_vector) if (!progress->processed) {all_done = false;break;}

                if (all_done) break; // Exit if all files are done
                std::this_thread::sleep_for(std::chrono::seconds(1)); // Update every second
            }
            progressWidget->close();
        }
    );


    for (auto &analysis_process : progress_vector)
    {
        p.push([&, analysis_process, changelayout](int id)
        {
            analysis_process->active = true;            
            DataFileReader DFR1;
            DFR1.setName(analysis_process->fileName.c_str(), channels); 
            DFR1.CreateRootFile();
            DFR1.ConsequentialEventsReading(analysis_process);
            analysis_process->active = false;    
            analysis_process->processed = true;
            *changelayout = true;

        });
    }
    
}
