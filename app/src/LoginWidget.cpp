#include "LoginWidget.h"
#include <qapplication.h>
#include <qbrush.h>
#include <qcontainerfwd.h>
#include <qdebug.h>
#include <qevent.h>
#include <qlatin1stringview.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qstyle.h>
#include <qtransform.h>
#include <qwidget.h>
#include <ui_LoginWidget.h>
#include <QFile>
#include <QDebug>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QPainterPath>


LoginWidget::LoginWidget(QWidget* parent)
    : FramelessWindow(parent)
    , ui(new Ui::LoginWidget)
{
    mainWindow->setFixedSize(330,448);
    initialize();
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::keyPressEvent(QKeyEvent* ev)
{
    if (ev->key() == Qt::Key_F6) {
        QFile file("../assets/LoginWidget.qss");
        if (file.open(QFile::OpenModeFlag::ReadOnly))
            mainWindow->get_contentWidget()->setStyleSheet(file.readAll());
    }
}

void LoginWidget::initialize()
{
    ui->setupUi(mainWindow->get_contentWidget());
    QFile file(":/style/assets/LoginWidget.qss");
    if (file.open(QFile::OpenModeFlag::ReadOnly))
        mainWindow->get_contentWidget()->setStyleSheet(file.readAll());
    ui->PhotoLabel->setPixmap(QPixmap(":/image/assets/photo.png").scaled(ui->PhotoLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    mainWindow->set_background(":image/assets/background.jpg");

}

