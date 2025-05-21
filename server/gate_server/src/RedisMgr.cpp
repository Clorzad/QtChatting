#include "RedisMgr.h"
#include <chrono>
#include <cstdlib>
#include <exception>
#include <memory>
#include <sw/redis++/connection.h>
#include <sw/redis++/redis.h>

void RedisMgr::init(const std::string& host,const std::string& port, const std::string& passwd, size_t pool_size)
{
    try {
        sw::redis::ConnectionOptions co;
        co.host = host;
        co.port = atoi(port.c_str());
        co.password = passwd;
        co.db = 0;
        co.socket_timeout = std::chrono::milliseconds(300);
        sw::redis::ConnectionPoolOptions cpo;
        cpo.size = pool_size;
        cpo.connection_idle_time = std::chrono::minutes(1);
        redis_ = std::make_shared<sw::redis::Redis>(co, cpo);
    } catch (const std::exception& e) {
        std::cerr << "Redis初始化失败： " << e.what() << std::endl;
        throw;
    }
}

std::shared_ptr<sw::redis::Redis> RedisMgr::getRedis()
{
    return redis_;
}

