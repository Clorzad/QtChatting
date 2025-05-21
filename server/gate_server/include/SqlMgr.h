#pragma once
#include "SqlDao.h"
#include "Singleton.hpp"

class SqlMgr : public Singleton<SqlMgr> {
    friend class Singleton<SqlMgr>;

public:
    bool regUser(const std::string& name, const std::string& email, const std::string& pwd);
    int emailExists(const std::string& email);
    bool resetPwd(const std::string& email, const std::string& pwd);
    bool varifyPwd(const std::string& email, const std::string& pwd);
        
private:
    SqlMgr();
    SqlDao& DAO_;
};