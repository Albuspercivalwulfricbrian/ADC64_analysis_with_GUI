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
from PySide6.QtGui import (QAction, QBrush, QColor, QConicalGradient,
    QCursor, QFont, QFontDatabase, QGradient,
    QIcon, QImage, QKeySequence, QLinearGradient,
    QPainter, QPalette, QPixmap, QRadialGradient,
    QTransform)
from PySide6.QtWidgets import (QApplication, QHBoxLayout, QLabel, QLayout,
    QLineEdit, QMainWindow, QMenu, QMenuBar,
    QPushButton, QSizePolicy, QSpinBox, QStatusBar,
    QVBoxLayout, QWidget)

from qcustomplot import QCustomPlot

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        if not MainWindow.objectName():
            MainWindow.setObjectName(u"MainWindow")
        MainWindow.resize(1116, 929)
        self.action_Open = QAction(MainWindow)
        self.action_Open.setObjectName(u"action_Open")
        font = QFont()
        font.setPointSize(14)
        self.action_Open.setFont(font)
        self.centralwidget = QWidget(MainWindow)
        self.centralwidget.setObjectName(u"centralwidget")
        self.horizontalLayout_2 = QHBoxLayout(self.centralwidget)
        self.horizontalLayout_2.setObjectName(u"horizontalLayout_2")
        self.verticalLayout_3 = QVBoxLayout()
        self.verticalLayout_3.setSpacing(15)
        self.verticalLayout_3.setObjectName(u"verticalLayout_3")
        self.verticalLayout_3.setContentsMargins(50, 20, 50, 20)
        self.horizontalLayout_4 = QHBoxLayout()
        self.horizontalLayout_4.setSpacing(10)
        self.horizontalLayout_4.setObjectName(u"horizontalLayout_4")
        self.horizontalLayout_4.setSizeConstraint(QLayout.SetDefaultConstraint)
        self.ChannelLabel = QLabel(self.centralwidget)
        self.ChannelLabel.setObjectName(u"ChannelLabel")
        self.ChannelLabel.setMaximumSize(QSize(140, 16777215))
        self.ChannelLabel.setFont(font)

        self.horizontalLayout_4.addWidget(self.ChannelLabel)

        self.channelSpinBox = QSpinBox(self.centralwidget)
        self.channelSpinBox.setObjectName(u"channelSpinBox")
        self.channelSpinBox.setFont(font)
        self.channelSpinBox.setMaximum(127)

        self.horizontalLayout_4.addWidget(self.channelSpinBox)


        self.verticalLayout_3.addLayout(self.horizontalLayout_4)

        self.horizontalLayout_11 = QHBoxLayout()
        self.horizontalLayout_11.setObjectName(u"horizontalLayout_11")
        self.horizontalLayout_11.setSizeConstraint(QLayout.SetDefaultConstraint)
        self.LeftBoundaryLabel = QLabel(self.centralwidget)
        self.LeftBoundaryLabel.setObjectName(u"LeftBoundaryLabel")
        self.LeftBoundaryLabel.setMinimumSize(QSize(150, 0))
        self.LeftBoundaryLabel.setFont(font)
        self.LeftBoundaryLabel.setScaledContents(False)

        self.horizontalLayout_11.addWidget(self.LeftBoundaryLabel)

        self.LeftBoundaryEdit = QLineEdit(self.centralwidget)
        self.LeftBoundaryEdit.setObjectName(u"LeftBoundaryEdit")
        self.LeftBoundaryEdit.setFont(font)

        self.horizontalLayout_11.addWidget(self.LeftBoundaryEdit)

        self.RightBoundaryLabel = QLabel(self.centralwidget)
        self.RightBoundaryLabel.setObjectName(u"RightBoundaryLabel")
        self.RightBoundaryLabel.setMinimumSize(QSize(150, 0))
        self.RightBoundaryLabel.setFont(font)

        self.horizontalLayout_11.addWidget(self.RightBoundaryLabel)

        self.RightBoundaryEdit = QLineEdit(self.centralwidget)
        self.RightBoundaryEdit.setObjectName(u"RightBoundaryEdit")
        self.RightBoundaryEdit.setFont(font)

        self.horizontalLayout_11.addWidget(self.RightBoundaryEdit)

        self.SetChannelBoundaries = QPushButton(self.centralwidget)
        self.SetChannelBoundaries.setObjectName(u"SetChannelBoundaries")
        self.SetChannelBoundaries.setFont(font)

        self.horizontalLayout_11.addWidget(self.SetChannelBoundaries)

        self.SetAllChannelsBoundaries = QPushButton(self.centralwidget)
        self.SetAllChannelsBoundaries.setObjectName(u"SetAllChannelsBoundaries")
        self.SetAllChannelsBoundaries.setFont(font)

        self.horizontalLayout_11.addWidget(self.SetAllChannelsBoundaries)


        self.verticalLayout_3.addLayout(self.horizontalLayout_11)

        self.horizontalLayout_12 = QHBoxLayout()
        self.horizontalLayout_12.setObjectName(u"horizontalLayout_12")
        self.horizontalLayout_12.setSizeConstraint(QLayout.SetDefaultConstraint)
        self.BranchName = QLineEdit(self.centralwidget)
        self.BranchName.setObjectName(u"BranchName")
        self.BranchName.setFont(font)

        self.horizontalLayout_12.addWidget(self.BranchName)

        self.setBranchName = QPushButton(self.centralwidget)
        self.setBranchName.setObjectName(u"setBranchName")
        self.setBranchName.setFont(font)

        self.horizontalLayout_12.addWidget(self.setBranchName)


        self.verticalLayout_3.addLayout(self.horizontalLayout_12)

        self.coordinateLabel = QLabel(self.centralwidget)
        self.coordinateLabel.setObjectName(u"coordinateLabel")
        sizePolicy = QSizePolicy(QSizePolicy.Minimum, QSizePolicy.Minimum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.coordinateLabel.sizePolicy().hasHeightForWidth())
        self.coordinateLabel.setSizePolicy(sizePolicy)
        self.coordinateLabel.setMaximumSize(QSize(60000, 50))
        font1 = QFont()
        font1.setPointSize(15)
        self.coordinateLabel.setFont(font1)
        self.coordinateLabel.setAlignment(Qt.AlignCenter)

        self.verticalLayout_3.addWidget(self.coordinateLabel)

        self.customPlot = QCustomPlot(self.centralwidget)
        self.customPlot.setObjectName(u"customPlot")
        self.horizontalLayout = QHBoxLayout(self.customPlot)
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.horizontalLayout.setContentsMargins(10, -1, -1, -1)

        self.verticalLayout_3.addWidget(self.customPlot)

        self.horizontalLayout_1 = QHBoxLayout()
        self.horizontalLayout_1.setObjectName(u"horizontalLayout_1")
        self.horizontalLayout_1.setSizeConstraint(QLayout.SetDefaultConstraint)
        self.EventLabel = QLabel(self.centralwidget)
        self.EventLabel.setObjectName(u"EventLabel")
        self.EventLabel.setMaximumSize(QSize(100, 16777215))
        self.EventLabel.setFont(font)

        self.horizontalLayout_1.addWidget(self.EventLabel)

        self.PreviousEventButton = QPushButton(self.centralwidget)
        self.PreviousEventButton.setObjectName(u"PreviousEventButton")
        self.PreviousEventButton.setFont(font)

        self.horizontalLayout_1.addWidget(self.PreviousEventButton)

        self.eventSpinBox = QSpinBox(self.centralwidget)
        self.eventSpinBox.setObjectName(u"eventSpinBox")
        self.eventSpinBox.setFont(font)
        self.eventSpinBox.setMinimum(1)
        self.eventSpinBox.setMaximum(999999999)

        self.horizontalLayout_1.addWidget(self.eventSpinBox)

        self.NextEventButton = QPushButton(self.centralwidget)
        self.NextEventButton.setObjectName(u"NextEventButton")
        self.NextEventButton.setFont(font)

        self.horizontalLayout_1.addWidget(self.NextEventButton)


        self.verticalLayout_3.addLayout(self.horizontalLayout_1)

        self.SaveConfigButton = QPushButton(self.centralwidget)
        self.SaveConfigButton.setObjectName(u"SaveConfigButton")
        font2 = QFont()
        font2.setPointSize(16)
        self.SaveConfigButton.setFont(font2)

        self.verticalLayout_3.addWidget(self.SaveConfigButton)


        self.horizontalLayout_2.addLayout(self.verticalLayout_3)

        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QMenuBar(MainWindow)
        self.menubar.setObjectName(u"menubar")
        self.menubar.setGeometry(QRect(0, 0, 1116, 31))
        self.menubar.setFont(font)
        self.menu_File = QMenu(self.menubar)
        self.menu_File.setObjectName(u"menu_File")
        self.menu_File.setFont(font)
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QStatusBar(MainWindow)
        self.statusbar.setObjectName(u"statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.menubar.addAction(self.menu_File.menuAction())
        self.menu_File.addAction(self.action_Open)

        self.retranslateUi(MainWindow)

        QMetaObject.connectSlotsByName(MainWindow)
    # setupUi

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(QCoreApplication.translate("MainWindow", u"MainWindow", None))
        self.action_Open.setText(QCoreApplication.translate("MainWindow", u"&Open", None))
        self.ChannelLabel.setText(QCoreApplication.translate("MainWindow", u"Channel: ", None))
        self.LeftBoundaryLabel.setText(QCoreApplication.translate("MainWindow", u"Left Boundary:", None))
        self.RightBoundaryLabel.setText(QCoreApplication.translate("MainWindow", u"Right Boundary:", None))
        self.SetChannelBoundaries.setText(QCoreApplication.translate("MainWindow", u"Set for current channel", None))
        self.SetAllChannelsBoundaries.setText(QCoreApplication.translate("MainWindow", u"Set for all channels", None))
        self.setBranchName.setText(QCoreApplication.translate("MainWindow", u"Set Branch Name for channel", None))
        self.coordinateLabel.setText("")
        self.EventLabel.setText(QCoreApplication.translate("MainWindow", u"Event:", None))
        self.PreviousEventButton.setText(QCoreApplication.translate("MainWindow", u"Previous non-empty", None))
        self.NextEventButton.setText(QCoreApplication.translate("MainWindow", u"Next non-empty", None))
        self.SaveConfigButton.setText(QCoreApplication.translate("MainWindow", u"Save Analysis Configuration", None))
        self.menu_File.setTitle(QCoreApplication.translate("MainWindow", u"&File", None))
    # retranslateUi

