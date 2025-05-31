#pragma once

#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include "simulation.h"
#include "data_processor.h"

// 数据库连接器
class DBConnector {
public:
    static DBConnector& getInstance();
    
    // 初始化连接
    bool init(const std::string& dsn, const std::string& user, const std::string& password);
    
    // 关闭连接
    void close();
    
    // 执行SQL语句
    bool executeSQL(const std::string& sql);
    
    // 创建数据库表
    bool createTables();
    
    // 保存雷达设备模型
    bool saveRadarDevice(const RadarDevice& device, int& deviceId);
    
    // 获取所有雷达设备模型
    std::vector<RadarDevice> getAllRadarDevices();
    
    // 保存辐射源模型
    bool saveRadiationSource(const RadiationSource& source, int& sourceId);
    
    // 获取所有辐射源模型
    std::vector<RadiationSource> getAllRadiationSources();
    
    // 保存定位结果
    bool saveLocationResult(const LocationResult& result, int deviceId, int sourceId);
    
    // 获取所有定位结果
    std::vector<LocationResult> getAllLocationResults();
    
    // 保存目标情报数据
    bool saveTargetIntelligence(const TargetIntelligence& target);
    
    // 获取所有目标情报数据
    std::vector<TargetIntelligence> getAllTargetIntelligence();
    
    // 保存基准点
    bool saveReferencePoint(const std::string& name, const Coordinate& position);
    
    // 获取所有基准点
    std::map<std::string, Coordinate> getAllReferencePoints();
    
    // 保存评估结果
    bool saveEvaluationResult(const std::string& name, double value);
    
    // 获取所有评估结果
    std::vector<std::pair<std::string, double>> getAllEvaluationResults();
    
    // 事务控制
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

private:
    DBConnector();
    ~DBConnector();
    
    SQLHENV m_env;        // 环境句柄
    SQLHDBC m_dbc;        // 连接句柄
    SQLHSTMT m_stmt;      // 语句句柄
    bool m_connected;     // 连接状态
    
    // 处理ODBC错误
    void handleError(SQLHANDLE handle, SQLSMALLINT type);
}; 