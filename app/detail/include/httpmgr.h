#pragma once
#include "global.h"
#include "singleton.hpp"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QUrl>



class HttpMgr : public QObject, public Singleton<HttpMgr> {

    Q_OBJECT
public:
    ~HttpMgr();
    void postHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod);

private:
    // CRTP奇异递归模板
    friend class Singleton<HttpMgr>;
    // 单例私有构造
    HttpMgr(QObject* parent = nullptr);
    QNetworkAccessManager manager_;
    // 发送http的post请求，url请求的网址，json请求的数据，req_id请求的id，mod哪个模块发出的请求
signals:
    void sig_http_finish(ReqId id, QString res, ErrorCode err, Modules mod);
    void sig_reg_mod_finish(ReqId id, QString res, ErrorCode err);

protected slots:
    void slotHttpFinish(ReqId id, QString res, ErrorCode err, Modules mod);
};
