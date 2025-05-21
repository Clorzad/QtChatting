#include "SqlDao.h"
#include "ConfigMgr.h"
#include "SqlPool.h"
#include <cstddef>
#include <cstring>
#include <exception>
#include <iostream>
#include <mysql/mysql.h>
#include <stdexcept>


SqlDao::SqlDao() : pool_(SqlPool::getInstance())
{
    auto& cnf_mgr = ConfigMgr::getInstance();
    auto host = cnf_mgr["MariaDB"]["host"];
    auto port = std::stoi(cnf_mgr["MariaDB"]["port"]);
    auto user = cnf_mgr["MariaDB"]["user"];
    auto passwd = cnf_mgr["MariaDB"]["passwd"];
    auto db = cnf_mgr["MariaDB"]["db"];
    pool_.init(host, port, user, passwd, db);
}

bool SqlDao::regUser(const std::string& name, const std::string& email, const std::string& pwd_hash)
{
    MYSQL_STMT* stmt = nullptr;
    bool success = false;
    try {
        auto conn = pool_.getConnection();
        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        std::string query = "INSERT INTO user_info (name, email, pwd_hash) VALUES (?, ?, ?)";
        if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));      

        unsigned long lengths[3] = {
            name.length(),
            email.length(),
            pwd_hash.length(),
        };

        // name
        bind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
        bind[0].buffer = const_cast<char*>(name.data());
        bind[0].buffer_length = name.size();
        bind[0].length = &lengths[0];

        // email
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = const_cast<char*>(email.data());
        bind[1].buffer_length = email.size();
        bind[1].length = &lengths[1];

        // password
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = const_cast<char*>(pwd_hash.data());
        bind[2].buffer_length = pwd_hash.size();
        bind[2].length = &lengths[2];


        if (mysql_stmt_bind_param(stmt, bind)) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }

        // 执行插入
        if (mysql_stmt_execute(stmt)) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }

        success = (mysql_stmt_affected_rows(stmt) == 1);
    } catch (std::exception& ep) {
        if (stmt)
            mysql_stmt_close(stmt);
        throw;
    }
    if (stmt)
        mysql_stmt_close(stmt);
    return success;
}

bool SqlDao::emailExists(const std::string& email)
{
    MYSQL_STMT* stmt = nullptr;
    bool exists = false;
    try {
        auto conn = pool_.getConnection();
        stmt = mysql_stmt_init(conn.get());
        std::string query = "SELECT 1 FROM user_info WHERE email = ? LIMIT 1";
        if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }
        auto email_length = email.length();
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = const_cast<char*>(email.data());
        bind[0].buffer_length = email.size();
        bind[0].length = &email_length;

        if (mysql_stmt_bind_param(stmt, bind)) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }

        // 执行查询
        if (mysql_stmt_execute(stmt)) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }

        // 处理结果
        int result;
        MYSQL_BIND result_bind;
        memset(&result_bind, 0, sizeof(result_bind));
        result_bind.buffer_type = MYSQL_TYPE_LONG;
        result_bind.buffer = &result;

        mysql_stmt_store_result(stmt);
        if (mysql_stmt_bind_result(stmt, &result_bind)) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }

        exists = (mysql_stmt_fetch(stmt) == 0);
        if (mysql_stmt_error(stmt)[0] != '\0') {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }
    } catch (const std::exception& ep) {
        if (stmt)
            mysql_stmt_close(stmt);
        throw;
    }
    if (stmt)
        mysql_stmt_close(stmt);
    return exists;
}

bool SqlDao::updatePwd(const std::string& email, const std::string& pwd_hash)
{
    MYSQL_STMT* stmt = nullptr;
    bool success = false;
    try {
        auto conn = pool_.getConnection();
        stmt = mysql_stmt_init(conn.get());
        std::string query = "UPDATE user_info SET pwd_hash = ? WHERE email = ?";
        if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));
        size_t length[2] = { email.length(), pwd_hash.length()};
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = const_cast<char*>(email.data());
        bind[0].buffer_length = email.size();
        bind[0].length = &length[0];

        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = const_cast<char*>(pwd_hash.data());
        bind[1].buffer_length = pwd_hash.size();
        bind[1].length = &length[1];
        if (mysql_stmt_bind_param(stmt, bind))
            throw std::runtime_error(mysql_stmt_error(stmt));
        if (mysql_stmt_execute(stmt))
            throw std::runtime_error(mysql_stmt_error(stmt));
        auto result = mysql_stmt_affected_rows(stmt);
        if (result >= 0) {
            std::cout << "修改了" << result << "条数据" << std::endl;
            success = true;
        }
        else if (result == -1)
            throw std::runtime_error(mysql_stmt_error(stmt));
    } catch (const std::exception& e) {
        if (stmt)
            mysql_stmt_close(stmt);
        throw;
    }
    if (stmt) {
        mysql_stmt_close(stmt);
    }
    return success;
}

bool SqlDao::verifyPwd(const std::string& email, const std::string& pwd_hash)
{

    MYSQL_STMT* stmt = nullptr;
    bool is_valid = false;
    try {
        auto conn = pool_.getConnection();
        stmt = mysql_stmt_init(conn.get());
        std::string query = "SELECT 1 FROM user_info WHERE email = ? AND pwd_hash = ? LIMIT 1";

        if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }

        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        unsigned long lengths[2] = {
            email.length(),
            pwd_hash.length()
        };

        // email
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = const_cast<char*>(email.data());
        bind[0].buffer_length = email.size();
        bind[0].length = &lengths[0];

        // password hash
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = const_cast<char*>(pwd_hash.data());
        bind[1].buffer_length = pwd_hash.size();
        bind[1].length = &lengths[1];

        if (mysql_stmt_bind_param(stmt, bind)) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }

        // 执行查询
        if (mysql_stmt_execute(stmt)) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }

        // 处理结果
        int result;
        MYSQL_BIND result_bind;
        memset(&result_bind, 0, sizeof(result_bind));
        result_bind.buffer_type = MYSQL_TYPE_LONG;
        result_bind.buffer = &result;

        mysql_stmt_store_result(stmt);
        if (mysql_stmt_bind_result(stmt, &result_bind)) {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }

        is_valid = (mysql_stmt_fetch(stmt) == 0);
        if (mysql_stmt_error(stmt)[0] != '\0') {
            throw std::runtime_error(mysql_stmt_error(stmt));
        }
    } catch (const std::exception& e) {
        if (stmt)
            mysql_stmt_close(stmt);
        throw;
    }

    if (stmt)
        mysql_stmt_close(stmt);
    return is_valid;
}
