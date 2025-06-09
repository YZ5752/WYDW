#include "../DBConnector.h"
#include <iostream>
#include <fstream>
#include <mysql/mysql.h>
#include <vector>
#include <string>

// 单例实现
DBConnector& DBConnector::getInstance() {
    static DBConnector instance;
    return instance;
}

DBConnector::DBConnector()
    : m_conn(nullptr), m_connected(false) {
}

DBConnector::~DBConnector() {
    close();
}

bool DBConnector::staticInit(const std::string& host, const std::string& user, const std::string& password, const std::string& db, unsigned int port) {
    std::cout << "Initializing MySQL connection to host: " << host << ", db: " << db << std::endl;
    DBConnector& instance = DBConnector::getInstance();
    instance.m_conn = mysql_init(nullptr);
    if (!instance.m_conn) {
        std::cerr << "mysql_init failed" << std::endl;
        return false;
    }
    mysql_options(instance.m_conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");
    if (!mysql_real_connect(instance.m_conn, host.c_str(), user.c_str(), password.c_str(), db.c_str(), port, nullptr, 0)) {
        std::cerr << "MySQL connection failed: " << mysql_error(instance.m_conn) << std::endl;
        mysql_close(instance.m_conn);
        instance.m_conn = nullptr;
        return false;
    }
    mysql_query(instance.m_conn, "SET NAMES utf8mb4");
    instance.m_connected = true;
    std::cout << "MySQL connection initialized successfully" << std::endl;
    return true;
}

//配置数据库信息
bool DBConnector::initDefaultConnection() {
    return DBConnector::staticInit("localhost", "root", "123456", "passive_location", 3306);
}

void DBConnector::close() {
    if (m_connected && m_conn) {
        std::cout << "Closing MySQL connection" << std::endl;
        mysql_close(m_conn);
        m_conn = nullptr;
        m_connected = false;
        std::cout << "MySQL connection closed" << std::endl;
    }
}

bool DBConnector::executeSQL(const std::string& sql) {
    if (!m_connected || !m_conn) {
        std::cerr << "MySQL not connected" << std::endl;
        return false;
    }
    if (mysql_query(m_conn, sql.c_str())) {
        std::cerr << "MySQL query failed: " << mysql_error(m_conn) << std::endl;
        return false;
    }
    return true;
}

bool DBConnector::beginTransaction() {
    if (!m_connected || !m_conn) return false;
    return executeSQL("START TRANSACTION");
}

bool DBConnector::commitTransaction() {
    if (!m_connected || !m_conn) return false;
    return executeSQL("COMMIT");
}

bool DBConnector::rollbackTransaction() {
    if (!m_connected || !m_conn) return false;
    return executeSQL("ROLLBACK");
}

void DBConnector::handleError() {
    if (m_conn) std::cerr << "MySQL Error: " << mysql_error(m_conn) << std::endl;
}


