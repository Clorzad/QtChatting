
#pragma once
#include "ui_LoginWidget.h"
#include <QMainWindow>
#include <qevent.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <QKeyEvent>
#include "FramelessWindow.h"

class LoginWidget : public FramelessWindow{
    Q_OBJECT
    
public:
    LoginWidget(QWidget* parent = nullptr);
    
    ~LoginWidget();

protected:
    void keyPressEvent(QKeyEvent* ev) override;

private:
    Ui::LoginWidget* ui;

    void initialize();
};

