#include "SqlMgr.h"
#include <bcrypt/BCrypt.hpp>
#include <exception>
#include <iostream>

bool SqlMgr::regUser(const std::string& name, const std::string& email, const std::string& pwd)
{
    try {
        auto hash = BCrypt::generateHash(pwd);
        auto success = DAO_.regUser(name, email, hash);
        return success;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

int SqlMgr::emailExists(const std::string& email)
{
    try {
        if (DAO_.emailExists(email)) {
            return 1;
        } else {
            return 0;
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}

SqlMgr::SqlMgr() : DAO_(SqlDao::getInstance())
{
}
