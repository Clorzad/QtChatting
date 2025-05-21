#pragma once
#include <QByteArray>
#include <QJsonDocument>

// 请求的id
enum ReqId {
    ID_GET_VARIFY_CODE = 1001, // 获取验证码
    ID_REG_USER = 1002, // 注册用户
    ID_RESET_PWD = 1003, // 重置密码

};
// 错误码
enum ResErrorCode {
    SUCCESS = 0,
    ERR_JSON = 1001, // json解析失败
    ERR_RPC_FAILED = 1002,
    ERR_EMAIL_REGISTERED = 1003,
    ERR_VARIFYCODE = 1004,
    ERR_SERVER = 1005,
    ERR_USER_NON = 1006,
};

enum class ClientErrorCode {
    SUCCESS = 0,
    ERR_NETWORK = 501,
};

enum Modules {
    REGISTERMOD = 0, // 注册模块
    RESETMOD = 1,
};

inline QString gate_url_prefix = " ";