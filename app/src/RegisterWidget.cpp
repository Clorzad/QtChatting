#include "RegisterWidget.h"
#include <QFile>
#include <qlineedit.h>
#include "global.h"
#include "httpmgr.h"

RegisterWidget::RegisterWidget(QWidget* parent)
    : FramelessWindow(parent)
    , ui(new Ui::RegisterWidget)
{
    mainWindow->setFixedSize(400, 480);
    initialize();
    initHttpHandlers();
    bind();
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
}

void RegisterWidget::bind()
{
    connect(ui->RegSendCodeButton, &QPushButton::clicked, this, &RegisterWidget::slotGetCode);
    connect(HttpMgr::getInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterWidget::slotRegFinish);
}

void RegisterWidget::slotRegFinish(ReqId id, QString res, ErrorCode err)
{
    if (err != ErrorCode::SUCCESS) {
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
    QRegularExpression regx(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regx.match(email).hasMatch();
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
        if (error != ErrorCode::SUCCESS) {
            showTip(tr("服务器发生错误"), false);
            return;
        }
        auto email = json_obj["email"].toString();
        showTip(tr("验证码已发送，注意查收"), true);
        qDebug() << "email is " << email;
    });
}
