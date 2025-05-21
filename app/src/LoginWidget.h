
#pragma once
#include "ui_LoginWidget.h"
#include <QMainWindow>
#include <qcontainerfwd.h>
#include <qevent.h>
#include <qmap.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <QKeyEvent>
#include "FramelessWindow.h"
#include <QDialog>

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
    void bind();

public slots:
    void registerWidgetShow();
    void resetWidgetShow();
};



