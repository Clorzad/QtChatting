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
#include <vector>
#include <thread>
#include "debug.hpp"
#include "ConfigMgr.h"
#include "RedisMgr.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;

enum ErrorCodes {
    SUCCESS = 0,
    ERR_JSON = 1001,
    ERR_RPC_FAILED = 1002,
    ERR_EMAIL_REGISTERED = 1003,
    ERR_VARIFYCODE = 1004,
    ERR_SERVER = 1005,
};
