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

bool DBConnector::init(const std::string& host, const std::string& user, const std::string& password, const std::string& db, unsigned int port) {
    std::cout << "Initializing MySQL connection to host: " << host << ", db: " << db << std::endl;
    m_conn = mysql_init(nullptr);
    if (!m_conn) {
        std::cerr << "mysql_init failed" << std::endl;
        return false;
    }
    
    // 设置字符集为utf8mb4
    mysql_options(m_conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");
    
    if (!mysql_real_connect(m_conn, host.c_str(), user.c_str(), password.c_str(), db.c_str(), port, nullptr, 0)) {
        std::cerr << "MySQL connection failed: " << mysql_error(m_conn) << std::endl;
        mysql_close(m_conn);
        m_conn = nullptr;
        return false;
    }
    
    // 连接后执行SET NAMES命令确保字符集一致
    mysql_query(m_conn, "SET NAMES utf8mb4");
    
    m_connected = true;
    std::cout << "MySQL connection initialized successfully" << std::endl;
    return true;
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

bool DBConnector::createTables() {
    if (!m_connected || !m_conn) {
        std::cerr << "MySQL not connected" << std::endl;
        return false;
    }
    // 跳过表创建过程，因为数据库已经在外部创建
    std::cout << "Skipping table creation as database is already set up" << std::endl;
    return true;
    /*
    // 从SQL文件读取创建表的SQL语句
    std::ifstream sqlFile("database/schema.sql");
    if (!sqlFile.is_open()) {
        std::cerr << "Failed to open schema.sql file" << std::endl;
        return false;
    }
    std::string line;
    std::string sql;
    while (std::getline(sqlFile, line)) {
        sql += line + "\n";
        if (line.find(';') != std::string::npos) {
            if (!executeSQL(sql)) {
                return false;
            }
            sql.clear();
        }
    }
    sqlFile.close();
    std::cout << "Database tables created successfully" << std::endl;
    */
}

bool DBConnector::saveLocationResult(const LocationResult& result, int deviceId, int sourceId) {
    // 简化实现
    return true;
}

std::vector<LocationResult> DBConnector::getAllLocationResults() {
    // 简化实现
    return std::vector<LocationResult>();
}

bool DBConnector::saveTargetIntelligence(const TargetIntelligence& target) {
    // 简化实现
    return true;
}

std::vector<TargetIntelligence> DBConnector::getAllTargetIntelligence() {
    // 简化实现
    return std::vector<TargetIntelligence>();
}

bool DBConnector::saveReferencePoint(const std::string& name, const Coordinate& position) {
    // 简化实现
    return true;
}

std::map<std::string, Coordinate> DBConnector::getAllReferencePoints() {
    // 简化实现
    return std::map<std::string, Coordinate>();
}

bool DBConnector::saveEvaluationResult(const std::string& name, double value) {
    // 简化实现
    return true;
}

std::vector<std::pair<std::string, double>> DBConnector::getAllEvaluationResults() {
    // 简化实现
    return std::vector<std::pair<std::string, double>>();
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