#include "LogicSystem.h"
#include "ConfigMgr.h"
#include "HttpConnection.h"
#include "RedisMgr.h"
#include "SqlMgr.h"
#include "VarifyGrpcClient.h"
#include "common.h"
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/ostream.hpp>
#include <exception>
#include <json/reader.h>
#include <json/value.h>
#include <memory>
#include <thread>

LogicSystem::LogicSystem()
    : sql_(SqlMgr::getInstance())
{
    if (!tryConnectRedis()) {
        std::terminate();
    }

    regGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        debug() << "get_test";
        beast::ostream(connection->response_.body()) << "receive get_test req";
    });

    regPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
        debug() << "get_varifycodeg";
        auto body_str = boost::beast::buffers_to_string(connection->request_.body().data());
        connection->response_.set(http::field::content_type, "text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
            debug() << "Error: Failed to parse Json data";
            root["error"] = ErrorCodes::ERR_JSON;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }
        auto email = src_root["email"].asString();
        message::GetVarifyRsp rsp = VarifyGrpcClient::getInstance().getVarifyCode(email, Modules::REGISTER_MOD);
        debug() << "email is " << email;
        root["error"] = rsp.error();
        root["email"] = src_root["email"];
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    });

    regPost("/get_reset_varifycode", [this](std::shared_ptr<HttpConnection> connection) {
        debug() << "get_reset_varifycode";
        auto body_str = boost::beast::buffers_to_string(connection->request_.body().data());
        connection->response_.set(http::field::content_type, "text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
            debug() << "Error: Failed to parse Json data";
            root["error"] = ErrorCodes::ERR_JSON;
        } else {
            auto email = src_root["email"].asString();
            auto exists = sql_.emailExists(email);
            if (exists == 1) {
                debug() << "存在" << email;
                message::GetVarifyRsp rsp = VarifyGrpcClient::getInstance().getVarifyCode(email, Modules::RESETPWD_MOD);
                debug() << "email is " << email;
                root["error"] = rsp.error();
                root["email"] = src_root["email"];
            } else if (exists == 0) {
                debug() << "不存在" << email;
                root["error"] = ErrorCodes::ERR_USER_NON;
            } else if (exists == -1) {
                debug() << "sql检测失败";
                root["error"] = ErrorCodes::ERR_SERVER;
            }
        }
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    });

    regPost("/user_register", [this](std::shared_ptr<HttpConnection> connection) {
        debug() << "user_register";
        auto body_str = beast::buffers_to_string(connection->request_.body().data());
        connection->response_.set(http::field::content_type, "text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
            std::cerr << "Error: Failed to parse Json data" << std::endl;
            root["error"] = ErrorCodes::ERR_JSON;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }
        auto email = src_root["email"].asString();
        auto code = src_root["reg_varifycode"].asString();
        try {
            auto value = this->redis_->get("RegisterCode:" + email);
            debug() << value;
            if (code != value) {
                root["error"] = ErrorCodes::ERR_VARIFYCODE;
            } else {
                debug() << "redis验证完成";
                auto exist = sql_.emailExists(email);
                debug() << "mysql验证完成";
                if (exist == 1) {
                    debug() << "邮箱已注册";
                    root["error"] = ErrorCodes::ERR_EMAIL_REGISTERED;
                } else if (exist == -1) {
                    debug() << "mysql服务器错误";
                    root["error"] = ErrorCodes::ERR_SERVER;
                } else if (exist == 0) {
                    auto name = src_root["username"].asString();
                    auto pwd = src_root["passwd"].asString();
                    debug() << "写入数据库前";
                    if (sql_.regUser(name, email, pwd)) {
                        root["error"] = ErrorCodes::SUCCESS;
                        debug() << "成功写入";
                    } else {
                        debug() << "写入失败";
                        root["error"] = ErrorCodes::ERR_SERVER;
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            root["error"] = ErrorCodes::ERR_SERVER;
        }
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    });

    regPost("/reset_passwd", [this](std::shared_ptr<HttpConnection> connection) {
        debug() << "reset_passwd";
        auto body_str = beast::buffers_to_string(connection->request_.body().data());
        connection->response_.set(http::field::content_type, "text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
            std::cerr << "Error: Failed to parse Json data" << std::endl;
            root["error"] = ErrorCodes::ERR_JSON;
        } else {
            auto email = src_root["email"].asString();
            auto exists = sql_.emailExists(email);
            if (exists == 1) {
                debug() << "存在用户: " << email; 
                try {
                    auto code = src_root["reset_code"].asString();
                    auto value = redis_->get("ResetPwdCode:" + email);
                    if (code == value) {
                        debug() << "验证码正确";
                        auto passwd = src_root["passwd"].asString();
                        auto success = sql_.resetPwd(email, passwd);
                        if (success) {
                            debug() << "重置密码成功";
                            root["error"] = ErrorCodes::SUCCESS;
                        } else {
                            debug() << "重置密码失败";
                            root["error"] = ErrorCodes::ERR_SERVER;
                        }
                    } else {
                        debug() << "验证码错误";
                        root["error"] = ErrorCodes::ERR_VARIFYCODE;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Redis get error: " << e.what() << std::endl;
                    root["error"] = ErrorCodes::ERR_SERVER;
                }
            } else if (exists == 0) {
                debug() << "用户不存在";
                root["error"] = ErrorCodes::ERR_USER_NON;
            } else {
                debug() << "sql发生错误";
                root["error"] = ErrorCodes::ERR_SERVER;
            }
        }
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    });
}

bool LogicSystem::handleGet(std::string path, std::shared_ptr<HttpConnection> http_connection)
{
    if (get_handles.find(path) == get_handles.end()) {
        return false;
    }
    get_handles[path](http_connection);
    return true;
}

bool LogicSystem::handlePost(std::string path, std::shared_ptr<HttpConnection> http_connection)
{
    if (post_handles_.find(path) == post_handles_.end()) {
        return false;
    }
    post_handles_[path](http_connection);
    return true;
}

void LogicSystem::regGet(std::string url, HttpHandle http_handle)
{
    get_handles.insert(make_pair(url, http_handle));
}

void LogicSystem::regPost(std::string url, HttpHandle http_handle)
{
    post_handles_.insert(make_pair(url, http_handle));
}

bool LogicSystem::tryConnectRedis()
{

    auto& config_mgr = ConfigMgr::getInstance();
    auto redis_host = config_mgr["RedisServer"]["host"];
    auto redis_port = config_mgr["RedisServer"]["port"];
    auto redis_passwd = config_mgr["RedisServer"]["passwd"];
    auto redis_pool_size = atoi(config_mgr["RedisServer"]["pool_size"].c_str());
    RedisMgr::getInstance().init(redis_host, redis_port, redis_passwd, redis_pool_size);
    debug() << redis_host << ':' << redis_port;
    redis_ = RedisMgr::getInstance().getRedis();
    for (int i = 0; i < 5; ++i) {
        try {
            redis_->ping();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Redis连接失败（尝试 " << i + 1 << "5): " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    return false;
}
