#include "mainwindow.h"
#include "ChannelMap.h"
#include "fstream"
#include "iomanip"
#include "iostream"
#include "json.hpp"
#include "sstream"
#include "ui_mainwindow.h"
#include "vector"
#include "QFileDialog"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), chMap(new ChannelMap) {
  ui->setupUi(this);
  QPalette palette = QPalette();
  palette.setColor(QPalette::Window, QColor(53, 53, 53));
  palette.setColor(QPalette::WindowText, Qt::white);
  palette.setColor(QPalette::Base, QColor(25, 25, 25));
  palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
  palette.setColor(QPalette::ToolTipBase, Qt::black);
  palette.setColor(QPalette::ToolTipText, Qt::white);
  palette.setColor(QPalette::Text, Qt::white);
  palette.setColor(QPalette::Button, QColor(53, 53, 53));
  palette.setColor(QPalette::ButtonText, Qt::white);
  palette.setColor(QPalette::BrightText, Qt::red);
  palette.setColor(QPalette::Link, QColor(42, 130, 218));
  palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  palette.setColor(QPalette::HighlightedText, Qt::black);
  this->setPalette(palette);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_RowSpinBox_valueChanged(int arg1) {
  ui->connectorTable->setRowCount(arg1);
}

void MainWindow::on_ColumnSpinBox_valueChanged(int arg1) {
  ui->connectorTable->setColumnCount(arg1);
}

void MainWindow::on_connectorTable_cellClicked(int row, int column) {
  current_row = row;

  current_column = column;
  for (int i = 0; i < chMap->GetX3Size(); i++) {
    QTableWidgetItem *widItem = new QTableWidgetItem;
    widItem->setData(Qt::EditRole, chMap->chconfig->v[column][row][i + 1]);
    ui->channelTable->setItem(0, i, widItem);
    //        cout << chMap->chconfig->v[column][row][i] << endl;
  }
}

void MainWindow::on_SetConfigButton_clicked() {
  std::ifstream iFile;
  if (fileName != "")
    iFile.open(fileName.toStdString());
  else {
    cout << "File NOT found" << endl;
    return;
  }
  chMap->ReadConfig(json::parse(iFile));
  cout << " shit" << endl;

  // cout << saveMap->fileName.toStdString();
  ui->connectorTable->setColumnCount(chMap->GetX1Size());
  ui->connectorTable->setRowCount(chMap->GetX2Size());
  ui->channelTable->setColumnCount(chMap->GetX3Size());

  for (int i = 0; i < chMap->GetX1Size(); i++) {

    for (int j = 0; j < chMap->GetX2Size(); j++) {
      QTableWidgetItem *widItem = new QTableWidgetItem;
      widItem->setData(Qt::EditRole, chMap->chconfig->v[j][i][0]);
      ui->connectorTable->setItem(i, j, widItem);
    }
  }
  iFile.close();
}

void MainWindow::on_connectorTable_cellChanged(int row, int column) {
  chMap->chconfig->v[column][row][0] =
      stoi(ui->connectorTable->item(row, column)->text().toStdString());
  //    cout << chMap->chconfig->v[column][row][0] << endl;
}

void MainWindow::on_channelTable_cellChanged(int row, int column) {
  // cout << current_column << " click " << current_row << " clack "<<column <<
  // endl;
  chMap->chconfig->v[current_column][current_row][column + 1] =
      stoi(ui->channelTable->item(row, column)->text().toStdString());
}

void MainWindow::on_LengthSpinBox_valueChanged(int arg1) {
  ui->channelTable->setColumnCount(arg1);
}

void MainWindow::on_SetSaveButton_clicked() {

  //    saveMap = new SaveMap(nullptr);
  //    saveMap->show();
  std::ofstream o(fileName.toStdString());

  o << std::setw(4) << chMap->SaveConfig() << std::endl;
}

void MainWindow::on_BrowseButton_clicked() {
  fileName = QFileDialog::getExistingDirectory(
      this, tr("Open Directory"), "/home/doc/afiadc/ChannelMap",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  fileName += "/ChannelMap.json";
  ui->fileNameEdit->setText(fileName);
}

void MainWindow::on_fileNameEdit_textChanged(const QString &arg1) {
  fileName = arg1;
}
