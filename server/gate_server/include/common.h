#pragma once
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <json/json.h>
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include "debug.hpp"


namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

enum ErrorCodes {
    SUCCESS = 0,
    ERROR_JSON = 1001,
    PRC_FAILED = 1002,
};

class ConfigMgr;
extern ConfigMgr config_mgr;
