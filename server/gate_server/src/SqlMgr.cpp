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
        std::cerr << "SQLMgr-regUser error: " << e.what() << std::endl;
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
        std::cerr << "SQLmgr-emailExists error: " << e.what() << std::endl;
        return -1;
    }
}

SqlMgr::SqlMgr() : DAO_(SqlDao::getInstance())
{
}

bool SqlMgr::resetPwd(const std::string& email, const std::string& pwd)
{
    try {
        auto hash = BCrypt::generateHash(pwd);
        return DAO_.updatePwd(email, hash);
    } catch (const std::exception& e) {
        std::cerr << "SQLMgr-resetPwd error: " << e.what() << std::endl;
        return false;
    }
}

bool SqlMgr::varifyPwd(const std::string& email, const std::string& pwd)
{
    try {
        auto hash = BCrypt::generateHash(pwd);
        return DAO_.verifyPwd(email, hash);
    } catch (const std::exception& e) {
        std::cerr << "SQLMgr-varifyPwd error: " << e.what() << std::endl;
        return false;
    }
}
