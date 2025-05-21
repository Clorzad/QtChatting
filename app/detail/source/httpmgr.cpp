#include "httpmgr.h"
#include "global.h"
#include <qnetworkrequest.h>
#include <qobject.h>


HttpMgr::~HttpMgr()
{
}

HttpMgr::HttpMgr(QObject* parent) : QObject(parent)
{
    connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slotHttpFinish);
}

void HttpMgr::postHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod)
{
    // 创建一个HTTP post请求，并设置请求头和请求体
    QByteArray data = QJsonDocument(json).toJson();
    // 通过url构造请求
    QNetworkRequest request(url);
    request.setRawHeader("Connection", "close");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));
    // 发送请求，并处理响应，获取自己的智能指针，构造伪闭包并增加智能指针引用计数
    QNetworkReply* reply = manager_.post(request, data);
    // 设置信号和槽等待发送完成
    QObject::connect(reply, &QNetworkReply::finished, [reply, this, req_id, mod]() {
        // 处理错误情况
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            // 发送信号通知完成
            emit sig_http_finish(req_id, "", ClientErrorCode::ERR_NETWORK, mod);
            reply->deleteLater();
            return;
        }
        // 无错误则读回请求
        QString res = reply->readAll();
        // 发送信号通知完成
        emit sig_http_finish(req_id, res, ClientErrorCode::SUCCESS, mod);
        reply->deleteLater();
        return;
    });
}

void HttpMgr::slotHttpFinish(ReqId id, QString res, ClientErrorCode err, Modules mod)
{
    if (mod == Modules::REGISTERMOD) {
        // 发送信号通知指定模块http响应结束
        emit sig_reg_mod_finish(id, res, err);
    } else if (mod == Modules::RESETMOD) {
        emit sig_reset_mod_finish(id, res, err);
    }
}
