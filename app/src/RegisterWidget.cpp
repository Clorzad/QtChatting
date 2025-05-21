#include "RegisterWidget.h"
#include "global.h"
#include "httpmgr.h"
#include <QFile>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <qjsonobject.h>
#include <qline.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <QTimer>

RegisterWidget::RegisterWidget(QWidget* parent)
    : FramelessWindow(parent)
    , ui(new Ui::RegisterWidget)
    , username_reg_exp(R"(^[\p{L}\p{N}_-]{0,10}$)")
    , email_reg_exp(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)")
    , send_code_timer(new QTimer(this))
{
    mainWindow->setFixedSize(400, 480);
    initialize();
    initHttpHandlers();
    bind();
    username_reg_exp.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption); // 关键：启用Unicode支持
    // QRegularExpressionValidator* username_validator = new QRegularExpressionValidator(username_reg_exp, this);
    // ui->RegUserNameEdit->setValidator(username_validator);
}

RegisterWidget::~RegisterWidget()
{
    delete ui;
}

void RegisterWidget::initialize()
{
    ui->setupUi(mainWindow->get_contentWidget());
    QFile file(":/style/assets/RegsiterWidget.qss");
    if (file.open(QFile::OpenModeFlag::ReadOnly))
        mainWindow->get_contentWidget()->setStyleSheet(file.readAll());
    mainWindow->setTitleLabel(tr("注册"));
    ui->RegPasswordEdit->setEchoMode(QLineEdit::Password);
    ui->RegCPasswordEdit->setEchoMode(QLineEdit::Password);
    ui->RegUserNameEdit->setClearButtonEnabled(true);
    ui->RegMailEdit->setClearButtonEnabled(true);
    ui->RegPasswordEdit->setClearButtonEnabled(true);
    ui->RegCPasswordEdit->setClearButtonEnabled(true);
    ui->RegConfirmButton->setEnabled(false);
    QRegularExpression reg_exp(R"(^\d*$)");
    auto validator = new QRegularExpressionValidator(reg_exp, this);
    ui->RegCodeEdit->setValidator(validator);
}

void RegisterWidget::bind()
{
    connect(ui->RegSendCodeButton, &QPushButton::clicked, this, &RegisterWidget::slotGetCode);
    connect(HttpMgr::getInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterWidget::slotRegFinish);
    connect(ui->RegConfirmButton, &QPushButton::clicked, this, &RegisterWidget::slotConfirmReg);

    connect(ui->RegUserNameEdit, &QLineEdit::textChanged, [this]() {
        bool is_valid = username_reg_exp.match(ui->RegUserNameEdit->text()).hasMatch() && ui->RegUserNameEdit->text().length() <= 10 && ui->RegUserNameEdit->text().length() >= 3;
        // qDebug() << username_reg_exp.pattern();
        ui->RegUserNameEdit->setStyleSheet(is_valid ? "border: 2px solid black" : "border: 2px solid red");
    });
    connect(ui->RegMailEdit, &QLineEdit::textChanged, [this]() {
        bool is_valid = email_reg_exp.match(ui->RegMailEdit->text()).hasMatch();
        ui->RegMailEdit->setStyleSheet(is_valid ? "border: 2px solid black" : "border: 2px solid red");
    });

    connect(send_code_timer, &QTimer::timeout, this, &RegisterWidget::slotUpdateButtonText);
}

void RegisterWidget::slotRegFinish(ReqId id, QString res, ClientErrorCode err)
{
    if (err != ClientErrorCode::SUCCESS) {
        showTip(tr("网络请求错误"), false);
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
    handlers_[id](json_obj);
    return;
}

void RegisterWidget::showTip(QString str, bool is_ok)
{
    if (is_ok == false) {
        ui->TipLabel->setText(str);
        ui->TipLabel->setStyleSheet("QLabel {color: red;}");
    } else {
        ui->TipLabel->setText(str);
        ui->TipLabel->setStyleSheet("QLabel {color: green;}");
    }
}

void RegisterWidget::slotGetCode()
{
    auto email = ui->RegMailEdit->text();
    bool match = email_reg_exp.match(email).hasMatch();
    if (match) {
        // 发送http请求获取验证码
        QJsonObject json_obj;
        json_obj["email"] = email;
        HttpMgr::getInstance()->postHttpReq(QUrl(gate_url_prefix + "/get_varifycode"),
            json_obj, ReqId::ID_GET_VARIFY_CODE, Modules::REGISTERMOD);
    } else {
        showTip(tr("邮箱地址不正确"), false);
    }
}

void RegisterWidget::initHttpHandlers()
{
    handlers_.insert(ReqId::ID_GET_VARIFY_CODE, [this](QJsonObject json_obj) {
        int error = json_obj["error"].toInt();
        if (error != ResErrorCode::SUCCESS) {
            showTip(tr("服务器发生错误"), false);
            return;
        }
        auto email = json_obj["email"].toString();
        showTip(tr("验证码已发送，注意查收"), true);
        qDebug() << "email is " << email;
        ui->RegConfirmButton->setEnabled(true);
        ui->RegSendCodeButton->setEnabled(false);
        remain_time = 60;
        send_code_timer->start(1000);
    });

    handlers_.insert(ReqId::ID_REG_USER, [this](QJsonObject json_obj) {
        int error = json_obj["error"].toInt();
        switch (error) {
        case ResErrorCode::SUCCESS:
            showTip("注册成功", true);
            return;
        case ResErrorCode::ERR_VARIFYCODE:
            showTip("验证码错误", false);
            return;
        case ResErrorCode::ERR_EMAIL_REGISTERED:
            showTip("邮箱已注册", false);
            return;
        case ResErrorCode::ERR_JSON:
        case ResErrorCode::ERR_RPC_FAILED:
        case ResErrorCode::ERR_SERVER:
            showTip("服务器错误", false);
            return;
        }
    });
}

void RegisterWidget::slotConfirmReg()
{
    if (ui->RegUserNameEdit->text() == "") {
        showTip("用户名不能为空", false);
        return;
    }
    if (!username_reg_exp.match(ui->RegUserNameEdit->text()).hasMatch()) {
        showTip("用户名不符合要求", false);
        return;
    }
    if (ui->RegMailEdit->text() == "") {
        showTip("邮箱不能为空", false);
        return;
    }
    if (ui->RegPasswordEdit->text() == "") {
        showTip("密码不能为空", false);
        return;
    }
    if (ui->RegPasswordEdit->text().length() < 8 || ui->RegPasswordEdit->text().length() > 16) {
        showTip("密码长度应该为8-16", false);
        return;
    }
    if (ui->RegCPasswordEdit->text() == "") {
        showTip("确认密码不能为空", false);
        return;
    }
    if (ui->RegPasswordEdit->text() != ui->RegCPasswordEdit->text()) {
        showTip("确认密码不匹配", false);
        return;
    }
    if (ui->RegCodeEdit->text() == "") {
        showTip("验证码不能为空", false);
        return;
    }

    QJsonObject json_obj;
    json_obj["username"] = ui->RegUserNameEdit->text();
    json_obj["email"] = ui->RegMailEdit->text();
    json_obj["passwd"] = ui->RegPasswordEdit->text();
    json_obj["reg_varifycode"] = ui->RegCodeEdit->text();
    HttpMgr::getInstance()->postHttpReq(QUrl(gate_url_prefix + "/user_register"), json_obj, ReqId::ID_REG_USER, Modules::REGISTERMOD);
}

void RegisterWidget::slotUpdateButtonText()
{
    remain_time--;
    ui->RegSendCodeButton->setText(QString("重新发送(%1秒)").arg(remain_time));
    if (remain_time <= 0) {
        send_code_timer->stop();
        ui->RegSendCodeButton->setEnabled(true);
        ui->RegSendCodeButton->setText("重新发送");
    }
}
