#pragma once
#include "common.h"
#include <sw/redis++/connection_pool.h>
#include <sw/redis++/redis++.h>
#include "Singleton.hpp"

class RedisMgr : public Singleton<RedisMgr>{
    friend class Singleton<RedisMgr>;
public:
    void init(const std::string& host, const std::string& port, const std::string& passwd, const size_t pool_size);
    std::shared_ptr<sw::redis::Redis> getRedis();
        
private:
    RedisMgr(const RedisMgr&) = delete;
    RedisMgr& operator=(const RedisMgr&) = delete;
    RedisMgr() = default;
    std::shared_ptr<sw::redis::Redis> redis_;    
};