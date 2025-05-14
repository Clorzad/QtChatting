#pragma once
#include "common.h"
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/flat_buffer.hpp>

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
    friend class LogicSystem;
public:
    HttpConnection(tcp::socket socket);
    void start();

private:
    void checkDeadline();
    void writeResponse();
    void handleReq();

    tcp::socket socket_;
    // 接收数据
    beast::flat_buffer buffer_ { 8192 };
    // 解析请求
    http::request<http::dynamic_body> request_;
    // 回应请求
    http::response<http::dynamic_body> response_;
    // 判断请求是否超时
    asio::steady_timer deadline_{socket_.get_executor(), std::chrono::seconds(30)};
};
