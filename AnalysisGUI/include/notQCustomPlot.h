#ifndef NOTQCUSTOMPLOT_H
#define NOTQCUSTOMPLOT_H

#include "qcustomplot.h"
// #include <QCustomPlot.h>
#include <QKeyEvent>

class notQCustomPlot : public QCustomPlot {
    Q_OBJECT

public:
    explicit notQCustomPlot(QWidget *parent = nullptr) : QCustomPlot(parent) {}

protected:
    void keyPressEvent(QKeyEvent *event) override {
        QCustomPlot::keyPressEvent(event);
        emit keyPressed(event);
    }

signals:
    void keyPressed(QKeyEvent *event);
};
#endif