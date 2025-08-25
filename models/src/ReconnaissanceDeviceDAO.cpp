#include "../ReconnaissanceDeviceDAO.h"
#include "../DBConnector.h"
#include <stdexcept>
#include <iostream>
#include <mysql/mysql.h>
#include <sstream>
#include <iomanip>

// 单例实现
ReconnaissanceDeviceDAO& ReconnaissanceDeviceDAO::getInstance() {
    static ReconnaissanceDeviceDAO instance;
    return instance;
}

// 构造函数
ReconnaissanceDeviceDAO::ReconnaissanceDeviceDAO() {
    // 初始化代码
}

// 析构函数
ReconnaissanceDeviceDAO::~ReconnaissanceDeviceDAO() {
    // 清理代码
}

// 获取所有侦察设备
std::vector<ReconnaissanceDevice> ReconnaissanceDeviceDAO::getAllReconnaissanceDevices() {
    std::vector<ReconnaissanceDevice> devices;
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    const char* sql = "SELECT device_id, device_name, is_stationary, tech_system, baseline_length, noise_psd, sample_rate, freq_range_min, freq_range_max, angle_azimuth_min, angle_azimuth_max, angle_elevation_min, angle_elevation_max, movement_speed, movement_azimuth, movement_elevation, longitude, latitude, altitude FROM reconnaissance_device_models";
    if (mysql_query(conn, sql)) {
        std::cerr << "Failed to execute query for getAllReconnaissanceDevices: " << mysql_error(conn) << std::endl;
        return devices;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "Failed to store result for getAllReconnaissanceDevices: " << mysql_error(conn) << std::endl;
        return devices;
    }
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        ReconnaissanceDevice device;
        int idx = 0;
        device.setDeviceId(row[idx] ? atoi(row[idx]) : 0); idx++;
        device.setDeviceName(row[idx] ? row[idx] : ""); idx++;
        device.setIsStationary(row[idx] ? atoi(row[idx]) != 0 : true); idx++;
        device.setTechSystem(row[idx] ? row[idx] : "INTERFEROMETER"); idx++;
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

// 通过ID获取侦察设备
ReconnaissanceDevice ReconnaissanceDeviceDAO::getReconnaissanceDeviceById(int deviceId) {
    ReconnaissanceDevice device;
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    char sql[512];
    snprintf(sql, sizeof(sql),
        "SELECT device_id, device_name, is_stationary, tech_system, baseline_length, noise_psd, sample_rate, freq_range_min, freq_range_max, angle_azimuth_min, angle_azimuth_max, angle_elevation_min, angle_elevation_max, movement_speed, movement_azimuth, movement_elevation, longitude, latitude, altitude FROM reconnaissance_device_models WHERE device_id=%d",
        deviceId);
    if (mysql_query(conn, sql)) {
        std::cerr << "Failed to execute query for getReconnaissanceDeviceById: " << mysql_error(conn) << std::endl;
        return device;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "Failed to store result for getReconnaissanceDeviceById: " << mysql_error(conn) << std::endl;
        return device;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        int idx = 0;
        device.setDeviceId(row[idx] ? atoi(row[idx]) : 0); idx++;
        device.setDeviceName(row[idx] ? row[idx] : ""); idx++;
        device.setIsStationary(row[idx] ? atoi(row[idx]) != 0 : true); idx++;
        device.setTechSystem(row[idx] ? row[idx] : "INTERFEROMETER"); idx++;
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
    }
    mysql_free_result(res);
    return device;
}

// 添加侦察设备
bool ReconnaissanceDeviceDAO::addReconnaissanceDevice(const ReconnaissanceDevice& device, int& deviceId) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return false;
    
    // 转义设备名称，避免SQL注入并处理特殊字符
    std::string escaped;
    escaped.resize(device.getDeviceName().length() * 2 + 1);
    unsigned long outLen = mysql_real_escape_string(conn, &escaped[0], device.getDeviceName().c_str(), device.getDeviceName().length());
    escaped.resize(outLen);
    
    std::ostringstream oss;
    oss.setf(std::ios::fixed);
    oss << std::setprecision(6);
    oss << "INSERT INTO reconnaissance_device_models (device_name, is_stationary, tech_system, baseline_length, noise_psd, sample_rate, freq_range_min, freq_range_max, angle_azimuth_min, angle_azimuth_max, angle_elevation_min, angle_elevation_max, movement_speed, movement_azimuth, movement_elevation, longitude, latitude, altitude) ";
    oss << "VALUES ('" << escaped << "', "
        << (device.getIsStationary() ? 1 : 0) << ", '" << device.getTechSystem() << "', "
        << device.getBaselineLength() << ", "
        << device.getNoisePsd() << ", "
        << device.getSampleRate() << ", "
        << device.getFreqRangeMin() << ", "
        << device.getFreqRangeMax() << ", "
        << device.getAngleAzimuthMin() << ", "
        << device.getAngleAzimuthMax() << ", "
        << device.getAngleElevationMin() << ", "
        << device.getAngleElevationMax() << ", "
        << device.getMovementSpeed() << ", "
        << device.getMovementAzimuth() << ", "
        << device.getMovementElevation() << ", "
        << device.getLongitude() << ", "
        << device.getLatitude() << ", "
        << device.getAltitude() << ")";
    
    std::string sql = oss.str();
    if (!db.executeSQL(sql)) {
        std::cerr << "Failed to add reconnaissance device: " << mysql_error(conn) << std::endl;
        std::cerr << "SQL: " << sql << std::endl;
        return false;
    }
    
    // 获取自增ID
    deviceId = (int)mysql_insert_id(conn);
    return true;
}

// 更新侦察设备
bool ReconnaissanceDeviceDAO::updateReconnaissanceDevice(const ReconnaissanceDevice& device) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return false;
    
    // 转义设备名称，避免SQL注入并处理特殊字符
    std::string escaped;
    escaped.resize(device.getDeviceName().length() * 2 + 1);
    unsigned long outLen = mysql_real_escape_string(conn, &escaped[0], device.getDeviceName().c_str(), device.getDeviceName().length());
    escaped.resize(outLen);
    
    std::ostringstream oss;
    oss.setf(std::ios::fixed);
    oss << std::setprecision(6);
    oss << "UPDATE reconnaissance_device_models SET ";
    oss << "device_name='" << escaped << "', ";
    oss << "is_stationary=" << (device.getIsStationary() ? 1 : 0) << ", ";
    oss << "tech_system='" << device.getTechSystem() << "', ";
    oss << "baseline_length=" << device.getBaselineLength() << ", ";
    oss << "noise_psd=" << device.getNoisePsd() << ", ";
    oss << "sample_rate=" << device.getSampleRate() << ", ";
    oss << "freq_range_min=" << device.getFreqRangeMin() << ", ";
    oss << "freq_range_max=" << device.getFreqRangeMax() << ", ";
    oss << "angle_azimuth_min=" << device.getAngleAzimuthMin() << ", ";
    oss << "angle_azimuth_max=" << device.getAngleAzimuthMax() << ", ";
    oss << "angle_elevation_min=" << device.getAngleElevationMin() << ", ";
    oss << "angle_elevation_max=" << device.getAngleElevationMax() << ", ";
    oss << "movement_speed=" << device.getMovementSpeed() << ", ";
    oss << "movement_azimuth=" << device.getMovementAzimuth() << ", ";
    oss << "movement_elevation=" << device.getMovementElevation() << ", ";
    oss << "longitude=" << device.getLongitude() << ", ";
    oss << "latitude=" << device.getLatitude() << ", ";
    oss << "altitude=" << device.getAltitude() << " ";
    oss << "WHERE device_id=" << device.getDeviceId();
    
    std::string sql = oss.str();
    if (!db.executeSQL(sql)) {
        std::cerr << "Failed to update reconnaissance device: " << mysql_error(conn) << std::endl;
        std::cerr << "SQL: " << sql << std::endl;
        return false;
    }
    return true;
}

// 删除侦察设备
bool ReconnaissanceDeviceDAO::deleteReconnaissanceDevice(int deviceId) {
    DBConnector& db = DBConnector::getInstance();
    
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM reconnaissance_device_models WHERE device_id=%d", deviceId);
    
    return db.executeSQL(sql);
}