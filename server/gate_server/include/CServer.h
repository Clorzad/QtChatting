#pragma once
#include "common.h"

class CServer : public std::enable_shared_from_this<CServer> {
public:
    CServer(boost::asio::io_context& ioc, unsigned short& port);
    void start();

private:
    tcp::acceptor acceptor_;
    asio::io_context& ioc_;
};