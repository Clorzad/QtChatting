#include "CServer.h"
#include <boost/asio/io_context.hpp>
#include <boost/beast/core/error.hpp>
#include <memory>
#include "HttpConnection.h"
#include "IOcontextPool.h"
CServer::CServer(asio::io_context& ioc, unsigned short& port)
    : ioc_(ioc)
    , acceptor_(ioc, tcp::endpoint(tcp::v4(), port))
{
}

void CServer::start()
{
    auto self = shared_from_this();
    auto http_con = std::make_shared<HttpConnection>(IOcontextPool::getInstance().getIOcontext());
    acceptor_.async_accept(http_con->getSocket(), [self,http_con](beast::error_code ec) {
        try {
            // 出错放弃连接
            if (ec) {
                self->start();
                return;
            }
            // 创建新连接，并且创建HttpConnection管理这个连接
            http_con->start();
            // 继续监听
            self->start();
        } catch (std::exception& exp) {
            std::cout << "exception: " << exp.what() << std::endl;
            self->start();
        }
    });
}
