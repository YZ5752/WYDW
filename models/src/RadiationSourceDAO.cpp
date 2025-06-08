#include "../RadiationSourceDAO.h"
#include "../DBConnector.h"
#include <iostream>
#include <mysql/mysql.h>

// 单例实现
RadiationSourceDAO& RadiationSourceDAO::getInstance() {
    static RadiationSourceDAO instance;
    return instance;
}

RadiationSourceDAO::RadiationSourceDAO() {
}

RadiationSourceDAO::~RadiationSourceDAO() {
}

std::vector<RadiationSource> RadiationSourceDAO::getAllRadiationSources() {
    std::cout << "RadiationSourceDAO: Getting all radiation sources..." << std::endl;
    
    std::vector<RadiationSource> sources;
    DBConnector& db = DBConnector::getInstance();
    
    const char* sql = "SELECT radiation_id, radiation_name, is_stationary, transmit_power, scan_period, carrier_frequency, "
                      "azimuth_start, azimuth_end, elevation_start, elevation_end, movement_speed, movement_azimuth, "
                      "movement_elevation, longitude, latitude, altitude FROM radiation_source_models";
    
    if (!db.executeSQL(sql)) {
        std::cerr << "Failed to execute query for getAllRadiationSources" << std::endl;
        return sources;
    }
    
    MYSQL* conn = db.getConnection();
    if (!conn) {
        std::cerr << "No valid connection for getAllRadiationSources" << std::endl;
        return sources;
    }
    
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "Failed to store result for getAllRadiationSources" << std::endl;
        return sources;
    }
    
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        RadiationSource source;
        int idx = 0;
        source.setRadiationId(row[idx] ? atoi(row[idx]) : 0); idx++;
        source.setRadiationName(row[idx] ? row[idx] : ""); idx++;
        source.setIsStationary(row[idx] ? atoi(row[idx]) != 0 : true); idx++;
        source.setTransmitPower(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setScanPeriod(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setCarrierFrequency(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setAzimuthStart(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setAzimuthEnd(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setElevationStart(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setElevationEnd(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setMovementSpeed(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setMovementAzimuth(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setMovementElevation(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setLongitude(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setLatitude(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setAltitude(row[idx] ? atof(row[idx]) : 0); idx++;
        sources.push_back(source);
    }
    mysql_free_result(res);
    return sources;
}

RadiationSource RadiationSourceDAO::getRadiationSourceById(int sourceId) {
    std::cout << "RadiationSourceDAO: Getting radiation source with ID " << sourceId << std::endl;
    
    RadiationSource source;
    DBConnector& db = DBConnector::getInstance();
    
    char sql[256];
    snprintf(sql, sizeof(sql), 
        "SELECT radiation_id, radiation_name, is_stationary, transmit_power, scan_period, carrier_frequency, "
        "azimuth_start, azimuth_end, elevation_start, elevation_end, movement_speed, movement_azimuth, "
        "movement_elevation, longitude, latitude, altitude FROM radiation_source_models WHERE radiation_id=%d", 
        sourceId);
    
    if (!db.executeSQL(sql)) {
        std::cerr << "Failed to execute query for getRadiationSourceById" << std::endl;
        return source;
    }
    
    MYSQL* conn = db.getConnection();
    if (!conn) {
        std::cerr << "No valid connection for getRadiationSourceById" << std::endl;
        return source;
    }
    
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "Failed to store result for getRadiationSourceById" << std::endl;
        return source;
    }
    
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        int idx = 0;
        source.setRadiationId(row[idx] ? atoi(row[idx]) : 0); idx++;
        source.setRadiationName(row[idx] ? row[idx] : ""); idx++;
        source.setIsStationary(row[idx] ? atoi(row[idx]) != 0 : true); idx++;
        source.setTransmitPower(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setScanPeriod(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setCarrierFrequency(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setAzimuthStart(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setAzimuthEnd(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setElevationStart(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setElevationEnd(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setMovementSpeed(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setMovementAzimuth(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setMovementElevation(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setLongitude(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setLatitude(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setAltitude(row[idx] ? atof(row[idx]) : 0); idx++;
    }
    mysql_free_result(res);
    return source;
}

bool RadiationSourceDAO::addRadiationSource(const RadiationSource& source, int& sourceId) {
    std::cout << "RadiationSourceDAO: Adding radiation source..." << std::endl;
    
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return false;
    
    // 转义辐射源名称，避免SQL注入并处理特殊字符
    char escaped_name[source.getRadiationName().length()*2+1];
    mysql_real_escape_string(conn, escaped_name, source.getRadiationName().c_str(), source.getRadiationName().length());
    
    char sql[512];
    snprintf(sql, sizeof(sql),
        "INSERT INTO radiation_source_models (radiation_name, is_stationary, transmit_power, scan_period, carrier_frequency, "
        "azimuth_start, azimuth_end, elevation_start, elevation_end, movement_speed, movement_azimuth, "
        "movement_elevation, longitude, latitude, altitude) "
        "VALUES ('%s', %d, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f)",
        escaped_name,
        source.getIsStationary() ? 1 : 0,
        source.getTransmitPower(),
        source.getScanPeriod(),
        source.getCarrierFrequency(),
        source.getAzimuthStart(),
        source.getAzimuthEnd(),
        source.getElevationStart(),
        source.getElevationEnd(),
        source.getMovementSpeed(),
        source.getMovementAzimuth(),
        source.getMovementElevation(),
        source.getLongitude(),
        source.getLatitude(),
        source.getAltitude()
    );
    
    if (!db.executeSQL(sql)) {
        return false;
    }
    
    // 获取自增ID
    sourceId = (int)mysql_insert_id(conn);
    return true;
}

bool RadiationSourceDAO::updateRadiationSource(const RadiationSource& source) {
    std::cout << "RadiationSourceDAO: Updating radiation source with ID " 
              << source.getRadiationId() << std::endl;
    
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return false;
    
    // 转义辐射源名称，避免SQL注入并处理特殊字符
    char escaped_name[source.getRadiationName().length()*2+1];
    mysql_real_escape_string(conn, escaped_name, source.getRadiationName().c_str(), source.getRadiationName().length());
    
    char sql[512];
    snprintf(sql, sizeof(sql),
        "UPDATE radiation_source_models SET radiation_name='%s', is_stationary=%d, transmit_power=%f, scan_period=%f, "
        "carrier_frequency=%f, azimuth_start=%f, azimuth_end=%f, elevation_start=%f, elevation_end=%f, "
        "movement_speed=%f, movement_azimuth=%f, movement_elevation=%f, longitude=%f, latitude=%f, altitude=%f "
        "WHERE radiation_id=%d",
        escaped_name,
        source.getIsStationary() ? 1 : 0,
        source.getTransmitPower(),
        source.getScanPeriod(),
        source.getCarrierFrequency(),
        source.getAzimuthStart(),
        source.getAzimuthEnd(),
        source.getElevationStart(),
        source.getElevationEnd(),
        source.getMovementSpeed(),
        source.getMovementAzimuth(),
        source.getMovementElevation(),
        source.getLongitude(),
        source.getLatitude(),
        source.getAltitude(),
        source.getRadiationId()
    );
    
    return db.executeSQL(sql);
}

bool RadiationSourceDAO::deleteRadiationSource(int sourceId) {
    std::cout << "RadiationSourceDAO: Deleting radiation source with ID " << sourceId << std::endl;
    
    DBConnector& db = DBConnector::getInstance();
    
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM radiation_source_models WHERE radiation_id=%d", sourceId);
    
    return db.executeSQL(sql);
} 