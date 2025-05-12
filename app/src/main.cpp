#include "LoginWidget.h"
#include "global.h"
#include <QApplication>
#include <qcoreapplication.h>
#include <qlogging.h>
#include <QDir>
#include <QSettings>
#include <qsettings.h>
#pragma comment(lib, "user32.lib")

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString app_path = QCoreApplication::applicationDirPath();
    QString config_filename = "config.ini";
    QString config_path = app_path + QDir::separator() + config_filename;
    QSettings settings(config_path, QSettings::IniFormat);
    QString gate_server_host = settings.value("GateServer/host").toString();
    QString gate_server_port = settings.value("GateServer/port").toString();
    gate_url_prefix = "http://" + gate_server_host + ':' + gate_server_port;
    qDebug() << "gate_url: " << gate_url_prefix;

    LoginWidget w;
    w.show();
    return a.exec();
}