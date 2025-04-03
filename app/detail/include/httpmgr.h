#pragma once
#include "singleton.hpp"
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>
#include <QJsonObject>
#include "global.h"
#include <memory>
#include <QString>
#include <QNetworkReply>
#include <QDebug>
#include <qcontainerfwd.h>

class HttpMgr : public QObject, public Singleton<HttpMgr>, public std::enable_shared_from_this<HttpMgr> {
    Q_OBJECT
public:
    ~HttpMgr();

private:
    // CRTP奇异递归模板
    friend class Singleton<HttpMgr>;
    // 单例私有构造
    HttpMgr();
    QNetworkAccessManager manager_;
    // 发送http的post请求，url请求的网址，json请求的数据，req_id请求的id，mod哪个模块发出的请求
    void post_http_req(QUrl url, QJsonObject json, ReqId req_id, Modules mod);
signals:
    void sig_http_finish(ReqId id, QString res, ErrorCode err, Modules mod);
    void sig_reg_mod_finish(ReqId id, QString res, ErrorCode err);
};


