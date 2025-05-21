#pragma once
#include "SqlDao.h"
#include "Singleton.hpp"

class SqlMgr : public Singleton<SqlMgr> {
    friend class Singleton<SqlMgr>;

public:
    bool regUser(const std::string& name, const std::string& email, const std::string& pwd);
    int emailExists(const std::string& email);

private:
    SqlMgr();
    SqlDao& DAO_;
};