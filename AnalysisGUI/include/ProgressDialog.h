#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

// #include <QApplication>
#include <QDialog>
#include <QProgressBar>
#include <QVBoxLayout>

class ProgressDialog : public QDialog {
    Q_OBJECT

public:
    ProgressDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("Чтение файла");
        setModal(true); // Блокирует основное окно

        QVBoxLayout *layout = new QVBoxLayout(this);
        progressBar = new QProgressBar(this);
        layout->addWidget(progressBar);
        setLayout(layout);

        progressBar->setRange(0, 1000);
    }

    void updateProgress(int value) {
        // cout << value << endl;
        progressBar->setValue(value);
    }

private:
    QProgressBar *progressBar;
};

#endif