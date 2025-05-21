#include "ResetWidget.h"
#include "FramelessWindow.h"
#include <QTimer>
#include <qjsonobject.h>
#include <qlineedit.h>
#include <qregularexpression.h>
#include <qvalidator.h>
#include "httpmgr.h"
#include "global.h"
#include <QFile>

ResetWidget::ResetWidget(QWidget* parent)
    : FramelessWindow(parent)
    , ui(new Ui::ResetWidget)
    , timer(new QTimer)
    , email_reset_exp(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)")
{
    ui->setupUi(mainWindow->get_contentWidget());
    ui->pwd_edit->setEchoMode(QLineEdit::Password);
    mainWindow->setTitleLabel("重置密码");
    QFile file(":/style/assets/ResetWidget.qss");
    if (file.open(QFile::OpenModeFlag::ReadOnly))
        mainWindow->get_contentWidget()->setStyleSheet(file.readAll());
    QRegularExpression reg_exp(R"(^\d*$)");
    QRegularExpressionValidator* validator = new QRegularExpressionValidator(reg_exp, this);
    ui->code_edit->setValidator(validator);
    ui->confirm_button->setEnabled(false);
    bind();
    httpHandleInit();
}

void ResetWidget::bind()
{
    connect(ui->send_button, &QPushButton::clicked, this, &ResetWidget::slotSendCode);
    connect(HttpMgr::getInstance().get(), &HttpMgr::sig_reset_mod_finish, this, &ResetWidget::slotResetFinish);
    connect(timer, &QTimer::timeout, this, &ResetWidget::slotUpdateButton);
    connect(ui->confirm_button, &QPushButton::clicked, this ,&ResetWidget::slotConfirm);

}

void ResetWidget::httpHandleInit()
{
    handlers.insert(ReqId::ID_GET_VARIFY_CODE, [this](const QJsonObject json_obj) {
        int error = json_obj["error"].toInt();
        switch (error) {
        case ResErrorCode::ERR_USER_NON:
            showTip("用户不存在", false);
            return;
        case ResErrorCode::ERR_JSON:
            showTip("json解析失败", false);
            return;
        case ResErrorCode::ERR_SERVER:
            showTip("服务器发生错误", false);
            return;
        }
        auto email = json_obj["email"].toString();
        showTip(tr("验证码已发送，注意查收"), true);
        qDebug() << "email is " << email;
        ui->confirm_button->setEnabled(true);
        ui->send_button->setEnabled(false);
        remain_time = 60;
        timer->start(1000);
        slotUpdateButton();
    });

    handlers.insert(ReqId::ID_RESET_PWD, [this](const QJsonObject json_obj) {
        int error = json_obj["error"].toInt();
        switch (error) {
        case ResErrorCode::ERR_USER_NON:
            showTip("用户不存在", false);
            return;
        case ResErrorCode::SUCCESS:
            showTip("重置密码成功", true);
            return;
        case ResErrorCode::ERR_VARIFYCODE:
            showTip("验证码错误", false);
            return;
        case ResErrorCode::ERR_SERVER:
            [[fallthrough]];
        case ResErrorCode::ERR_JSON:
            showTip("服务器错误", false);
            return;
        default:
            showTip("未知错误", false);
            return;
        }
    });
}

void ResetWidget::showTip(QString msg, bool success)
{
    ui->tip_label->setText(msg);
    if (success)
        ui->tip_label->setStyleSheet("QLabel {color: green;}");
    else
        ui->tip_label->setStyleSheet("QLabel {color: red;}");
}

void ResetWidget::slotSendCode()
{
    auto email = ui->email_edit->text();
    bool match = email_reset_exp.match(email).hasMatch();
    if (match) {
        // 发送http请求获取验证码
        QJsonObject json_obj;
        json_obj["email"] = email;
        HttpMgr::getInstance()->postHttpReq(QUrl(gate_url_prefix + "/get_reset_varifycode"),
            json_obj, ReqId::ID_GET_VARIFY_CODE, Modules::RESETMOD);
    } else {
        showTip(tr("邮箱地址不正确"), false);
    }
}

void ResetWidget::slotConfirm()
{
    if (ui->email_edit->text() == "") {
        return;
    }
    if (ui->code_edit->text() == "") {
        showTip("验证码不能为空", false);
        return;
    }
    if (!email_reset_exp.match(ui->email_edit->text()).hasMatch()) {
        showTip("邮箱格式错误", false);
        return;
    }
    if (ui->pwd_edit->text().length() < 8 || ui->pwd_edit->text().length() > 16) {
        showTip("密码长应为8-16", false);
        return;
    }

    QJsonObject json_obj;
    json_obj["email"] = ui->email_edit->text();
    json_obj["reset_code"] = ui->code_edit->text();
    json_obj["passwd"] = ui->pwd_edit->text();
    HttpMgr::getInstance()->postHttpReq(QUrl(gate_url_prefix + "/reset_passwd"), json_obj, ReqId::ID_RESET_PWD, Modules::RESETMOD);
}

void ResetWidget::slotResetFinish(ReqId id, QString res, ClientErrorCode err)
{
    if (err != ClientErrorCode::SUCCESS) {
        showTip("网络请求错误", false);
        return;
    }
    QJsonDocument json_doc = QJsonDocument::fromJson(res.toUtf8());
    if (json_doc.isNull()) {
        showTip(tr("json解析错误"), false);
        return;
    }
    if (!json_doc.isObject()) {
        showTip(tr("json解析错误"), false);
        return;
    }
    QJsonObject json_obj = json_doc.object();
    handlers[id](json_obj);
    return;
}

void ResetWidget::slotUpdateButton()
{
    remain_time--;
    ui->send_button->setText(QString("重新发送(%1s)").arg(remain_time));
    if (remain_time <= 0) {
        timer->stop();
        ui->send_button->setText("重新发送");
        ui->send_button->setEnabled(true);
    }
}
