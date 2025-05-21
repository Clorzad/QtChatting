#include "SqlPool.h"
#include <memory>
#include <mutex>
#include <mysql/mysql.h>
#include <stdexcept>
#include "debug.hpp"

SqlPool& SqlPool::getInstance()
{
    static SqlPool instance;
    return instance;
}

void SqlPool::init(const std::string& host,
    const unsigned int port,
    const std::string& user,
    const std::string& passwd,
    const std::string& db,
    const unsigned int max_conn)
{
    host_ = host;
    port_ = port;
    user_ = user;
    passwd_ = passwd;
    db_ = db;
    max_conn_ = max_conn;
}

SqlPool::~SqlPool()
{
    for (int i = 0; i < pool_.size(); ++i) {
        auto conn = pool_.front();
        pool_.pop();
        mysql_close(conn);
    }
}

std::shared_ptr<MYSQL> SqlPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this]() {
        return !pool_.empty() || current_conn_ < max_conn_;
    });
    if (pool_.empty()) {
        addConnection();
    }
    auto connection = std::shared_ptr<MYSQL>(pool_.front(), [this](MYSQL* p) {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.push(p);
        cond_.notify_one();
    });
    pool_.pop();
    return connection;
}

void SqlPool::addConnection()
{
    MYSQL* conn = mysql_init(nullptr);
    if (!conn)
        throw std::runtime_error("mysql init conn error");
    if (!mysql_real_connect(conn,
            host_.c_str(),
            user_.c_str(),
            passwd_.c_str(),
            db_.c_str(),
            port_,
            nullptr,
            0)) {
        mysql_close(conn);
        debug() << "mysql connect creat error" << conn->db;
        throw std::runtime_error(mysql_error(conn));
    }
    debug() << "创建了一个连接" << host_ << ':' << int(port_) << ":" << db_;
    mysql_set_character_set(conn, "utf8mb4");
    pool_.push(conn);
}
