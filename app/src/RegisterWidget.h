#pragma once
#include "FramelessWindow.h"
#include "ui_RegisterWidget.h"
#include "global.h"
#include <qregularexpression.h>

class RegisterWidget : public FramelessWindow {
    Q_OBJECT
public:
    RegisterWidget(QWidget* parent = nullptr);
    
    ~RegisterWidget();

private:
    Ui::RegisterWidget* ui;
    QMap<ReqId, std::function<void(const QJsonObject&)>> handlers_;
    QRegularExpression username_reg_exp;
    QRegularExpression email_reg_exp;
    QTimer* send_code_timer;
    int remain_time = 0;

    void
    initialize();
    void bind();
    void initHttpHandlers();
    void showTip(QString str, bool is_ok);

signals:

public slots:
    void slotRegFinish(ReqId id, QString res, ClientErrorCode err);
    void slotGetCode();
    void slotConfirmReg();
    void slotUpdateButtonText();
};