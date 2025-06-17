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
    
    // 检查数据库连接
    MYSQL* conn = db.getConnection();
    if (!conn) {
        std::cerr << "RadiationSourceDAO: No valid database connection" << std::endl;
        
        std::cout << "RadiationSourceDAO: Returning " << sources.size() << " sample radiation sources for testing" << std::endl;
        return sources;
    }
    
    const char* sql = "SELECT radiation_id, radiation_name, is_stationary, transmit_power, scan_period, carrier_frequency, "
                      "azimuth_start_angle, azimuth_end_angle, elevation_start_angle, elevation_end_angle, movement_speed, movement_azimuth, "
                      "movement_elevation, longitude, latitude, altitude FROM radiation_source_models";
    
    std::cout << "RadiationSourceDAO: Executing SQL: " << sql << std::endl;
    
    if (mysql_query(conn, sql)) {
        std::cerr << "RadiationSourceDAO: Failed to execute query - " << mysql_error(conn) << std::endl;
        return sources;
    }
    
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "RadiationSourceDAO: Failed to store result - " << mysql_error(conn) << std::endl;
        return sources;
    }
    
    std::cout << "RadiationSourceDAO: Found " << mysql_num_rows(res) << " radiation sources" << std::endl;
    
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
        source.setAzimuthStart(row[idx] ? atof(row[idx]) : 0); idx++;  // azimuth_start_angle
        source.setAzimuthEnd(row[idx] ? atof(row[idx]) : 0); idx++;    // azimuth_end_angle
        source.setElevationStart(row[idx] ? atof(row[idx]) : 0); idx++; // elevation_start_angle
        source.setElevationEnd(row[idx] ? atof(row[idx]) : 0); idx++;  // elevation_end_angle
        source.setMovementSpeed(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setMovementAzimuth(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setMovementElevation(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setLongitude(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setLatitude(row[idx] ? atof(row[idx]) : 0); idx++;
        source.setAltitude(row[idx] ? atof(row[idx]) : 0); idx++;
        sources.push_back(source);
    }
    mysql_free_result(res);
    std::cout << "RadiationSourceDAO: Successfully loaded " << sources.size() << " radiation sources" << std::endl;
    return sources;
}

RadiationSource RadiationSourceDAO::getRadiationSourceById(int sourceId) {
    std::cout << "RadiationSourceDAO: Getting radiation source with ID " << sourceId << std::endl;

    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();

    // 打印当前数据库名
    if (mysql_query(conn, "SELECT DATABASE()") == 0) {
        MYSQL_RES* res_dbname = mysql_store_result(conn);
        if (res_dbname) {
            MYSQL_ROW row_dbname = mysql_fetch_row(res_dbname);
            if (row_dbname && row_dbname[0]) {
                std::cout << "当前数据库: " << row_dbname[0] << std::endl;
            }
            mysql_free_result(res_dbname);
        }
    }

    RadiationSource source;

    char sql[512];
    snprintf(sql, sizeof(sql),
        "SELECT radiation_id, radiation_name, is_stationary, transmit_power, scan_period, carrier_frequency, "
        "azimuth_start_angle, azimuth_end_angle, elevation_start_angle, elevation_end_angle, movement_speed, movement_azimuth, "
        "movement_elevation, longitude, latitude, altitude FROM radiation_source_models WHERE radiation_id=%d",
        sourceId);

    if (mysql_query(conn, sql)) {
        std::cerr << "MySQL query failed: " << mysql_error(conn) << std::endl;
        return source;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "Failed to store result for getRadiationSourceById: " << mysql_error(conn) << std::endl;
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
        "azimuth_start_angle, azimuth_end_angle, elevation_start_angle, elevation_end_angle, movement_speed, movement_azimuth, "
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
        "carrier_frequency=%f, azimuth_start_angle=%f, azimuth_end_angle=%f, elevation_start_angle=%f, elevation_end_angle=%f, "
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