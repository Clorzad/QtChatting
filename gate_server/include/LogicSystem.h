#pragma once

#include "Singleton.hpp"
#include "common.h"


class HttpConnection;
class LogicSystem : public Singleton<LogicSystem> {
    friend class Singleton<LogicSystem>;
    using HttpHandle = std::function<void(std::shared_ptr<HttpConnection>)>;
public:
    bool handleGet(std::string path, std::shared_ptr<HttpConnection> http_connection);
    bool handlePost(std::string path, std::shared_ptr<HttpConnection>
                                          http_connection);
    void regGet(std::string url, HttpHandle http_handle);
    void regPost(std::string url, HttpHandle http_handle);

private:
    LogicSystem();
    // 不同的post请求和get请求调用到回调函数
    std::map<std::string, HttpHandle> post_handles_;
    std::map<std::string, HttpHandle> get_handles;
};