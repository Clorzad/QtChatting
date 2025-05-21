#pragma once
#include "FramelessWindow.h"
#include "global.h"
#include "ui_ResetWidget.h"
#include <qobject.h>
#include <qregularexpression.h>
#include <qwidget.h>

class ResetWidget : public FramelessWindow {
public:
    ResetWidget(QWidget* parent = nullptr);

private:
    Ui::ResetWidget* ui;
    QTimer* timer;
    int remain_time = 0;
    QRegularExpression email_reset_exp;
    QMap<ReqId, std::function<void(const QJsonObject&)>> handlers;

    void widgetInit();
    void bind();
    void httpHandleInit();
    void showTip(QString msg, bool success);

public slots:
    void slotSendCode();
    void slotConfirm();
    void slotResetFinish(ReqId id, QString res, ClientErrorCode err);
    void slotUpdateButton();
};