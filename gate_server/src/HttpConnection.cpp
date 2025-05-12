#include "HttpConnection.h"
#include "LogicSystem.h"
#include <boost/asio/error.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/core/ignore_unused.hpp>


HttpConnection::HttpConnection(tcp::socket socket)
    : socket_(std::move(socket))
{
}

void HttpConnection::start()
{
    auto self = shared_from_this();
    http::async_read(socket_, buffer_, request_, [self](beast::error_code ec, size_t bytes_transferred) {
        try {
            if (ec) {
                std::cout << "Error: http async_read:" << ec.what() << std::endl;
                self->socket_.close();
                return;
            }
            boost::ignore_unused(bytes_transferred);// 忽略不需要的参数用ignore_unused
            self->handleReq();// 调用回调
            self->checkDeadline();// 设置定时器
        } catch (std::exception& exp) {
            std::cout << "exception: " << exp.what() << std::endl;
        }
    });
}

void HttpConnection::handleReq()
{
    // 设置版本
    response_.version(request_.version());
    // 设置为短链接
    response_.keep_alive(false);

    if (request_.method() == http::verb::get) {
        bool success = LogicSystem::getInstance().handleGet(request_.target(), shared_from_this());
        if (!success) {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "url not found\r\n";
            writeResponse();
            return;
        }

        response_.result(http::status::ok);
        response_.set(http::field::server, "Gate server");
        writeResponse();
        return;
    } else if (request_.method() == http::verb::post) {
        bool success = LogicSystem::getInstance().handlePost(request_.target(), shared_from_this());
        if (!success) {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "url not found\r\n";
            writeResponse();
            return;
        }
        response_.result(http::status::ok);
        response_.set(http::field::server, "GateServer");
        writeResponse();
        return;
    }
    
}

void HttpConnection::writeResponse()
{
    auto self = shared_from_this();
    response_.content_length(response_.body().size());
    http::async_write(socket_, response_, [self](beast::error_code ec, std::size_t) {
        if (ec) {
            std::cerr << "Error: during async_write: " << ec.message() << std::endl;
            return;
        }
        self->socket_.shutdown(tcp::socket::shutdown_send, ec);
        if (ec) {
            std::cerr << "Socket shutdown error: " << ec.message() << std::endl;
            return;
        }
        self->deadline_.cancel();
    });
}

void HttpConnection::checkDeadline()
{
    auto self = shared_from_this();

    deadline_.async_wait([self](beast::error_code ec) {

        if (ec && ec != asio::error::operation_aborted) {
            std::cerr << "CheckDeadline error: " << ec.message() << std::endl;
        }
        self->socket_.close();

    });
}
