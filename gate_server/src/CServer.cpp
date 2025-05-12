#include "CServer.h"
#include <boost/asio/io_context.hpp>
#include <boost/beast/core/error.hpp>
#include "HttpConnection.h"
CServer::CServer(asio::io_context& ioc, unsigned short& port)
    : ioc_(ioc)
    , acceptor_(ioc, tcp::endpoint(tcp::v4(), port))
    , socket_(ioc)
{
}

void CServer::start()
{
    auto self = shared_from_this();
    acceptor_.async_accept(socket_, [self](beast::error_code ec) {
        try {
            // 出错放弃连接
            if (ec) {
                self->start();
                return;
            }
            // 创建新连接，并且创建HttpConnection管理这个连接
            std::make_shared<HttpConnection>(std::move(self->socket_))->start();
            // 继续监听
            self->start();
        } catch (std::exception& exp) {
            std::cout << "exception: " << exp.what() << std::endl;
            self->start();
        }
    });
}
