#include "mainwindow.h"
#include "DataFormat.h"
#include "AdcMapConfig.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Load adcmap override from configs/adcmap.json if present
    {
        std::string exeDir = QCoreApplication::applicationDirPath().toStdString();
        std::string cfgDir = AdcMapConfig::findConfigsDir(exeDir);
        if (!cfgDir.empty())
        {
            auto loaded = AdcMapConfig::load(cfgDir + "/adcmap.json");
            if (!loaded.empty())
                DataFormat::adcmap = loaded;
        }
    }

    MainWindow w;
    w.show();
    return a.exec();
}