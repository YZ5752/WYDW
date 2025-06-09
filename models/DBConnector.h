#ifndef DB_CONNECTOR_H
#define DB_CONNECTOR_H

#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <map>

// 数据库连接器
class DBConnector {
public:
    static DBConnector& getInstance();
    
    // 初始化方法
    static bool staticInit(const std::string& host, const std::string& user, const std::string& password, const std::string& db, unsigned int port = 3306);
    
    // 关闭连接
    void close();
    
    // 执行SQL语句
    bool executeSQL(const std::string& sql);
    
    // 获取MySQL连接句柄
    MYSQL* getConnection() { return m_conn; }
    
    // 事务控制
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

     static bool initDefaultConnection();

private:
    DBConnector();
    ~DBConnector();
    
    MYSQL* m_conn;      // MySQL连接句柄
    bool m_connected;   // 连接状态
    
    // 处理MySQL错误
    void handleError();
};

#endif // DB_CONNECTOR_H 