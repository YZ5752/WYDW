#include "../include/db_connector.h"
#include "../include/reconnaissance_device_model.h"
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

bool DBConnector::saveReconnaissanceDevice(const ReconnaissanceDevice& device, int& deviceId) {
    // 简化实现
    deviceId = 1;
    return true;
}

std::vector<ReconnaissanceDevice> DBConnector::getAllReconnaissanceDevices() {
    std::vector<ReconnaissanceDevice> devices;
    if (!m_connected || !m_conn) return devices;
    const char* sql = "SELECT device_id, device_name, is_stationary, baseline_length, noise_psd, sample_rate, freq_range_min, freq_range_max, angle_azimuth_min, angle_azimuth_max, angle_elevation_min, angle_elevation_max, movement_speed, movement_azimuth, movement_elevation, longitude, latitude, altitude FROM reconnaissance_device_models";
    if (mysql_query(m_conn, sql) != 0) {
        std::cerr << "MySQL query failed: " << mysql_error(m_conn) << std::endl;
        return devices;
    }
    MYSQL_RES* res = mysql_store_result(m_conn);
    if (!res) return devices;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        ReconnaissanceDevice device;
        int idx = 0;
        device.setDeviceId(row[idx] ? atoi(row[idx]) : 0); idx++;
        device.setDeviceName(row[idx] ? row[idx] : ""); idx++;
        device.setIsStationary(row[idx] ? atoi(row[idx]) != 0 : true); idx++;
        device.setBaselineLength(row[idx] ? static_cast<float>(atof(row[idx])) : 0); idx++;
        device.setNoisePsd(row[idx] ? static_cast<float>(atof(row[idx])) : 0); idx++;
        device.setSampleRate(row[idx] ? static_cast<float>(atof(row[idx])) : 0); idx++;
        device.setFreqRangeMin(row[idx] ? static_cast<float>(atof(row[idx])) : 0); idx++;
        device.setFreqRangeMax(row[idx] ? static_cast<float>(atof(row[idx])) : 0); idx++;
        device.setAngleAzimuthMin(row[idx] ? static_cast<float>(atof(row[idx])) : 0); idx++;
        device.setAngleAzimuthMax(row[idx] ? static_cast<float>(atof(row[idx])) : 0); idx++;
        device.setAngleElevationMin(row[idx] ? static_cast<float>(atof(row[idx])) : 0); idx++;
        device.setAngleElevationMax(row[idx] ? static_cast<float>(atof(row[idx])) : 0); idx++;
        device.setMovementSpeed(row[idx] ? static_cast<float>(atof(row[idx])) : 0); idx++;
        device.setMovementAzimuth(row[idx] ? static_cast<float>(atof(row[idx])) : 0); idx++;
        device.setMovementElevation(row[idx] ? static_cast<float>(atof(row[idx])) : 0); idx++;
        device.setLongitude(row[idx] ? atof(row[idx]) : 0); idx++;
        device.setLatitude(row[idx] ? atof(row[idx]) : 0); idx++;
        device.setAltitude(row[idx] ? atof(row[idx]) : 0); idx++;
        devices.push_back(device);
    }
    mysql_free_result(res);
    return devices;
}

bool DBConnector::saveRadiationSource(const RadiationSource& source, int& sourceId) {
    // 简化实现
    sourceId = 1;
    return true;
}

std::vector<RadiationSource> DBConnector::getAllRadiationSources() {
    // 简化实现
    return std::vector<RadiationSource>();
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

// 新增
bool DBConnector::addReconnaissanceDevice(const ReconnaissanceDevice& device) {
    if (!m_connected || !m_conn) return false;
    
    // 转义设备名称，避免SQL注入并处理特殊字符
    char escaped_name[device.getDeviceName().length()*2+1];
    mysql_real_escape_string(m_conn, escaped_name, device.getDeviceName().c_str(), device.getDeviceName().length());
    
    char sql[512];
    snprintf(sql, sizeof(sql),
        "INSERT INTO reconnaissance_device_models (device_name, is_stationary, baseline_length, freq_range_min, freq_range_max, angle_azimuth_min, angle_azimuth_max, angle_elevation_min, angle_elevation_max) "
        "VALUES ('%s', %d, %f, %f, %f, %f, %f, %f, %f)",
        escaped_name,
        device.getIsStationary() ? 1 : 0,
        device.getBaselineLength(),
        device.getFreqRangeMin(),
        device.getFreqRangeMax(),
        device.getAngleAzimuthMin(),
        device.getAngleAzimuthMax(),
        device.getAngleElevationMin(),
        device.getAngleElevationMax()
    );
    return mysql_query(m_conn, sql) == 0;
}

// 编辑
bool DBConnector::updateReconnaissanceDevice(const ReconnaissanceDevice& device) {
    if (!m_connected || !m_conn) return false;
    
    // 转义设备名称，避免SQL注入并处理特殊字符
    char escaped_name[device.getDeviceName().length()*2+1];
    mysql_real_escape_string(m_conn, escaped_name, device.getDeviceName().c_str(), device.getDeviceName().length());
    
    char sql[512];
    snprintf(sql, sizeof(sql),
        "UPDATE reconnaissance_device_models SET device_name='%s', is_stationary=%d, baseline_length=%f, freq_range_min=%f, freq_range_max=%f, angle_azimuth_min=%f, angle_azimuth_max=%f, angle_elevation_min=%f, angle_elevation_max=%f WHERE device_id=%d",
        escaped_name,
        device.getIsStationary() ? 1 : 0,
        device.getBaselineLength(),
        device.getFreqRangeMin(),
        device.getFreqRangeMax(),
        device.getAngleAzimuthMin(),
        device.getAngleAzimuthMax(),
        device.getAngleElevationMin(),
        device.getAngleElevationMax(),
        device.getDeviceId()
    );
    return mysql_query(m_conn, sql) == 0;
}

// 删除
bool DBConnector::deleteReconnaissanceDevice(int deviceId) {
    if (!m_connected || !m_conn) return false;
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM reconnaissance_device_models WHERE device_id=%d", deviceId);
    return mysql_query(m_conn, sql) == 0;
} 