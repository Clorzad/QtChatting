#include "common.h"
#include <cstdlib>
#include <exception>
#include <CServer.h>
int main(int argc, char* argv[])
{
    unsigned short port = 6679;
    if (argc >= 2) {
        try {
            auto port = static_cast<unsigned short>(std::stoi(argv[1]));
        } catch (std::exception const& e) {
            std::cout << "Usage: " << argv[0] << " <port>" << " \tServer listening port" << std::endl;
            return 0;
        }
    }
    try {
        beast::net::io_context ioc{ 1 };
        asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& ec, int signal_number) {
            if (ec) {
                std::cerr << "Error: " << ec.message() << std::endl;
            }
            ioc.stop();
        });
        std::make_shared<CServer>(ioc, port)->start();
        std::cout << "Gate Server is listening on port: " << port << std::endl;
        ioc.run();
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}