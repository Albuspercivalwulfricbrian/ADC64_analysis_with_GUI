#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ChannelMap;

class SaveMap;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:

  void on_RowSpinBox_valueChanged(int arg1);

  void on_ColumnSpinBox_valueChanged(int arg1);

  void on_connectorTable_cellClicked(int row, int column);

  void on_SetConfigButton_clicked();

  void on_connectorTable_cellChanged(int row, int column);

  void on_channelTable_cellChanged(int row, int column);

  void on_LengthSpinBox_valueChanged(int arg1);

  void on_SetSaveButton_clicked();

  void on_BrowseButton_clicked();

  void on_fileNameEdit_textChanged(const QString &arg1);

private:
  ChannelMap *chMap;
  Ui::MainWindow *ui;
  SaveMap *saveMap;
  int current_row = -100;
  int current_column = -100;
  QString fileName = "";
};

#endif // MAINWINDOW_H
