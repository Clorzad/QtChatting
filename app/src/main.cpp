#include "LoginWidget.h"

#include <QApplication>
#include <qlogging.h>
#pragma comment(lib, "user32.lib")

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginWidget w;
    w.show();
    return a.exec();
}