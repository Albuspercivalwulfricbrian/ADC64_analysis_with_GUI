# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'mainwindow.ui'
##
## Created by: Qt User Interface Compiler version 6.6.0
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QCursor,
    QFont, QFontDatabase, QGradient, QIcon,
    QImage, QKeySequence, QLinearGradient, QPainter,
    QPalette, QPixmap, QRadialGradient, QTransform)
from PySide6.QtWidgets import (QApplication, QHeaderView, QLabel, QLineEdit,
    QMainWindow, QMenuBar, QPushButton, QSizePolicy,
    QSpinBox, QStatusBar, QTableWidget, QTableWidgetItem,
    QWidget)

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        if not MainWindow.objectName():
            MainWindow.setObjectName(u"MainWindow")
        MainWindow.resize(1800, 1225)
        self.centralwidget = QWidget(MainWindow)
        self.centralwidget.setObjectName(u"centralwidget")
        self.connectorTable = QTableWidget(self.centralwidget)
        if (self.connectorTable.columnCount() < 3):
            self.connectorTable.setColumnCount(3)
        __qtablewidgetitem = QTableWidgetItem()
        self.connectorTable.setHorizontalHeaderItem(0, __qtablewidgetitem)
        __qtablewidgetitem1 = QTableWidgetItem()
        self.connectorTable.setHorizontalHeaderItem(1, __qtablewidgetitem1)
        __qtablewidgetitem2 = QTableWidgetItem()
        self.connectorTable.setHorizontalHeaderItem(2, __qtablewidgetitem2)
        if (self.connectorTable.rowCount() < 5):
            self.connectorTable.setRowCount(5)
        __qtablewidgetitem3 = QTableWidgetItem()
        self.connectorTable.setVerticalHeaderItem(0, __qtablewidgetitem3)
        __qtablewidgetitem4 = QTableWidgetItem()
        self.connectorTable.setVerticalHeaderItem(1, __qtablewidgetitem4)
        __qtablewidgetitem5 = QTableWidgetItem()
        self.connectorTable.setVerticalHeaderItem(2, __qtablewidgetitem5)
        __qtablewidgetitem6 = QTableWidgetItem()
        self.connectorTable.setVerticalHeaderItem(3, __qtablewidgetitem6)
        __qtablewidgetitem7 = QTableWidgetItem()
        self.connectorTable.setVerticalHeaderItem(4, __qtablewidgetitem7)
        self.connectorTable.setObjectName(u"connectorTable")
        self.connectorTable.setGeometry(QRect(80, 20, 1351, 801))
        self.RowSpinBox = QSpinBox(self.centralwidget)
        self.RowSpinBox.setObjectName(u"RowSpinBox")
        self.RowSpinBox.setGeometry(QRect(1690, 100, 73, 49))
        self.ColumnSpinBox = QSpinBox(self.centralwidget)
        self.ColumnSpinBox.setObjectName(u"ColumnSpinBox")
        self.ColumnSpinBox.setGeometry(QRect(1690, 20, 73, 49))
        self.SetConfigButton = QPushButton(self.centralwidget)
        self.SetConfigButton.setObjectName(u"SetConfigButton")
        self.SetConfigButton.setGeometry(QRect(1600, 1010, 170, 48))
        self.channelTable = QTableWidget(self.centralwidget)
        if (self.channelTable.columnCount() < 7):
            self.channelTable.setColumnCount(7)
        __qtablewidgetitem8 = QTableWidgetItem()
        self.channelTable.setHorizontalHeaderItem(0, __qtablewidgetitem8)
        __qtablewidgetitem9 = QTableWidgetItem()
        self.channelTable.setHorizontalHeaderItem(1, __qtablewidgetitem9)
        __qtablewidgetitem10 = QTableWidgetItem()
        self.channelTable.setHorizontalHeaderItem(2, __qtablewidgetitem10)
        __qtablewidgetitem11 = QTableWidgetItem()
        self.channelTable.setHorizontalHeaderItem(3, __qtablewidgetitem11)
        __qtablewidgetitem12 = QTableWidgetItem()
        self.channelTable.setHorizontalHeaderItem(4, __qtablewidgetitem12)
        __qtablewidgetitem13 = QTableWidgetItem()
        self.channelTable.setHorizontalHeaderItem(5, __qtablewidgetitem13)
        __qtablewidgetitem14 = QTableWidgetItem()
        self.channelTable.setHorizontalHeaderItem(6, __qtablewidgetitem14)
        if (self.channelTable.rowCount() < 1):
            self.channelTable.setRowCount(1)
        __qtablewidgetitem15 = QTableWidgetItem()
        self.channelTable.setVerticalHeaderItem(0, __qtablewidgetitem15)
        self.channelTable.setObjectName(u"channelTable")
        self.channelTable.setGeometry(QRect(20, 840, 1761, 131))
        self.SetSaveButton = QPushButton(self.centralwidget)
        self.SetSaveButton.setObjectName(u"SetSaveButton")
        self.SetSaveButton.setGeometry(QRect(1600, 1080, 170, 48))
        self.label = QLabel(self.centralwidget)
        self.label.setObjectName(u"label")
        self.label.setGeometry(QRect(1440, 20, 251, 51))
        self.label_2 = QLabel(self.centralwidget)
        self.label_2.setObjectName(u"label_2")
        self.label_2.setGeometry(QRect(1440, 100, 251, 51))
        self.LengthSpinBox = QSpinBox(self.centralwidget)
        self.LengthSpinBox.setObjectName(u"LengthSpinBox")
        self.LengthSpinBox.setGeometry(QRect(270, 990, 73, 49))
        self.label_3 = QLabel(self.centralwidget)
        self.label_3.setObjectName(u"label_3")
        self.label_3.setGeometry(QRect(20, 990, 251, 51))
        self.fileNameEdit = QLineEdit(self.centralwidget)
        self.fileNameEdit.setObjectName(u"fileNameEdit")
        self.fileNameEdit.setGeometry(QRect(730, 1010, 701, 51))
        self.BrowseButton = QPushButton(self.centralwidget)
        self.BrowseButton.setObjectName(u"BrowseButton")
        self.BrowseButton.setGeometry(QRect(970, 1080, 241, 48))
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QMenuBar(MainWindow)
        self.menubar.setObjectName(u"menubar")
        self.menubar.setGeometry(QRect(0, 0, 1800, 39))
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QStatusBar(MainWindow)
        self.statusbar.setObjectName(u"statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)

        QMetaObject.connectSlotsByName(MainWindow)
    # setupUi

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(QCoreApplication.translate("MainWindow", u"MainWindow", None))
        ___qtablewidgetitem = self.connectorTable.horizontalHeaderItem(0)
        ___qtablewidgetitem.setText(QCoreApplication.translate("MainWindow", u"1", None));
        ___qtablewidgetitem1 = self.connectorTable.horizontalHeaderItem(1)
        ___qtablewidgetitem1.setText(QCoreApplication.translate("MainWindow", u"2", None));
        ___qtablewidgetitem2 = self.connectorTable.horizontalHeaderItem(2)
        ___qtablewidgetitem2.setText(QCoreApplication.translate("MainWindow", u"3", None));
        ___qtablewidgetitem3 = self.connectorTable.verticalHeaderItem(0)
        ___qtablewidgetitem3.setText(QCoreApplication.translate("MainWindow", u"1", None));
        ___qtablewidgetitem4 = self.connectorTable.verticalHeaderItem(1)
        ___qtablewidgetitem4.setText(QCoreApplication.translate("MainWindow", u"2", None));
        ___qtablewidgetitem5 = self.connectorTable.verticalHeaderItem(2)
        ___qtablewidgetitem5.setText(QCoreApplication.translate("MainWindow", u"3", None));
        ___qtablewidgetitem6 = self.connectorTable.verticalHeaderItem(3)
        ___qtablewidgetitem6.setText(QCoreApplication.translate("MainWindow", u"4", None));
        ___qtablewidgetitem7 = self.connectorTable.verticalHeaderItem(4)
        ___qtablewidgetitem7.setText(QCoreApplication.translate("MainWindow", u"5", None));
        self.SetConfigButton.setText(QCoreApplication.translate("MainWindow", u"Load", None))
        ___qtablewidgetitem8 = self.channelTable.horizontalHeaderItem(0)
        ___qtablewidgetitem8.setText(QCoreApplication.translate("MainWindow", u"1", None));
        ___qtablewidgetitem9 = self.channelTable.horizontalHeaderItem(1)
        ___qtablewidgetitem9.setText(QCoreApplication.translate("MainWindow", u"2", None));
        ___qtablewidgetitem10 = self.channelTable.horizontalHeaderItem(2)
        ___qtablewidgetitem10.setText(QCoreApplication.translate("MainWindow", u"3", None));
        ___qtablewidgetitem11 = self.channelTable.horizontalHeaderItem(3)
        ___qtablewidgetitem11.setText(QCoreApplication.translate("MainWindow", u"4", None));
        ___qtablewidgetitem12 = self.channelTable.horizontalHeaderItem(4)
        ___qtablewidgetitem12.setText(QCoreApplication.translate("MainWindow", u"5", None));
        ___qtablewidgetitem13 = self.channelTable.horizontalHeaderItem(5)
        ___qtablewidgetitem13.setText(QCoreApplication.translate("MainWindow", u"6", None));
        ___qtablewidgetitem14 = self.channelTable.horizontalHeaderItem(6)
        ___qtablewidgetitem14.setText(QCoreApplication.translate("MainWindow", u"7", None));
        ___qtablewidgetitem15 = self.channelTable.verticalHeaderItem(0)
        ___qtablewidgetitem15.setText(QCoreApplication.translate("MainWindow", u"channels", None));
        self.SetSaveButton.setText(QCoreApplication.translate("MainWindow", u"Save", None))
        self.label.setText(QCoreApplication.translate("MainWindow", u"Change Columns:", None))
        self.label_2.setText(QCoreApplication.translate("MainWindow", u"Change Rows:", None))
        self.label_3.setText(QCoreApplication.translate("MainWindow", u"Change Length:", None))
        self.BrowseButton.setText(QCoreApplication.translate("MainWindow", u"set Map File", None))
    # retranslateUi

