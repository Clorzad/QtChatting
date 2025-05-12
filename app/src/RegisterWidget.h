#pragma once
#include "FramelessWindow.h"
#include "ui_RegisterWidget.h"
#include "global.h"

class RegisterWidget : public FramelessWindow {
    Q_OBJECT
public:
    RegisterWidget(QWidget* parent = nullptr);
    
    ~RegisterWidget();

private:
    Ui::RegisterWidget* ui;
    QMap<ReqId, std::function<void(const QJsonObject&)>> handlers_;

    void initialize();
    void bind();
    void initHttpHandlers();
    void showTip(QString str, bool is_ok);


signals:

public slots:
    void slotRegFinish(ReqId id, QString res, ErrorCode err);
    void slotGetCode();
};