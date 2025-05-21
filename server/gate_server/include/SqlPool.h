#pragma once
#include <cstddef>
#include <string>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <mysql/mysql.h>


class SqlPool {
public:
    static SqlPool& getInstance();
    void init(const std::string& host,
        const unsigned int port,
        const std::string& user,
        const std::string& passwd,
        const std::string& db,
        const unsigned int max_conn = 5);
    ~SqlPool();
    std::shared_ptr<MYSQL> getConnection();
    
private:
    SqlPool() = default;
    void addConnection();

    std::queue<MYSQL*> pool_;
    std::string host_;
    unsigned short port_;
    std::string user_;
    std::string passwd_;
    std::string db_;
    size_t max_conn_;
    size_t current_conn_ = 0;
    std::mutex mutex_;
    std::condition_variable cond_;
};