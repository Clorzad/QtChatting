#pragma once
#include <string>
#include "Singleton.hpp"
#include "SqlPool.h"

class SqlDao : public Singleton<SqlDao> {
    friend class Singleton<SqlDao>;

public:
    bool regUser(const std::string& name, const std::string& email, const std::string& pwd);
    bool emailExists(const std::string& email);
private:
    SqlDao();
    SqlDao(const SqlDao&) = delete;
    SqlDao& operator=(const SqlDao&) = delete;

    SqlPool& pool_;
};