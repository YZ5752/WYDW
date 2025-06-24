#include "../MultiPlatformTaskDAO.h"
#include <iostream>
#include <cstring>
#include <gtk/gtk.h>

// 单例实现
MultiPlatformTaskDAO& MultiPlatformTaskDAO::getInstance() {
    static MultiPlatformTaskDAO instance;
    return instance;
}

// 添加多平台任务
bool MultiPlatformTaskDAO::addMultiPlatformTask(const MultiPlatformTask& task, int& taskId) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return false;
    
    char sql[1024];
    snprintf(sql, sizeof(sql),
        "INSERT INTO multi_platform_task (tech_system, radiation_id, execution_time, "
        "target_longitude, target_latitude, target_altitude, movement_speed, "
        "movement_azimuth, movement_elevation, azimuth, elevation, positioning_distance, "
        "positioning_time, positioning_accuracy) VALUES ('%s', %d, %f, %.6f, %.6f, %.2f, %f, "
        "%.2f, %.2f, %.2f, %.2f, %f, %f, %.6f)",
        task.techSystem.c_str(),
        task.radiationId,
        task.executionTime,
        task.targetLongitude,
        task.targetLatitude,
        task.targetAltitude,
        task.movementSpeed,
        task.movementAzimuth,
        task.movementElevation,
        task.azimuth,
        task.elevation,
        task.positioningDistance,
        task.positioningTime,
        task.positioningAccuracy
    );
    
    if (!db.executeSQL(sql)) {
        std::cerr << "Failed to add multi platform task: " << mysql_error(conn) << std::endl;
        std::cerr << "SQL: " << sql << std::endl;
        return false;
    }
    
    // 获取自增ID
    taskId = (int)mysql_insert_id(conn);
    
    // 保存任务与设备的关联关系
    if (!saveTaskDeviceRelations(taskId, task.deviceIds)) {
        // 如果保存关联关系失败，删除刚插入的任务
        deleteMultiPlatformTask(taskId);
        return false;
    }
    
    return true;
}

// 根据任务ID获取多平台任务
MultiPlatformTask MultiPlatformTaskDAO::getMultiPlatformTaskById(int taskId) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return MultiPlatformTask{};
    
    char sql[512];
    snprintf(sql, sizeof(sql),
        "SELECT task_id, tech_system, radiation_id, execution_time, "
        "target_longitude, target_latitude, target_altitude, movement_speed, "
        "movement_azimuth, movement_elevation, positioning_distance, positioning_time, "
        "positioning_accuracy, created_at FROM multi_platform_task WHERE task_id = %d",
        taskId
    );
    
    if (mysql_query(conn, sql)) {
        std::cerr << "Failed to execute query: " << mysql_error(conn) << std::endl;
        return MultiPlatformTask{};
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        std::cerr << "Failed to store result: " << mysql_error(conn) << std::endl;
        return MultiPlatformTask{};
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return MultiPlatformTask{};
    }
    
    MultiPlatformTask task = createTaskFromRow(row);
    mysql_free_result(result);
    
    // 获取关联的设备ID列表
    task.deviceIds = getDeviceIdsByTaskId(taskId);
    
    return task;
}

// 获取所有多平台任务
std::vector<MultiPlatformTask> MultiPlatformTaskDAO::getAllMultiPlatformTasks() {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return std::vector<MultiPlatformTask>{};
    
    const char* sql = "SELECT task_id, tech_system, radiation_id, execution_time, "
                     "target_longitude, target_latitude, target_altitude, movement_speed, "
                     "movement_azimuth, movement_elevation, positioning_distance, positioning_time, "
                     "positioning_accuracy, created_at FROM multi_platform_task ORDER BY task_id DESC";
    
    if (mysql_query(conn, sql)) {
        std::cerr << "Failed to execute query: " << mysql_error(conn) << std::endl;
        return std::vector<MultiPlatformTask>{};
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        std::cerr << "Failed to store result: " << mysql_error(conn) << std::endl;
        return std::vector<MultiPlatformTask>{};
    }
    
    std::vector<MultiPlatformTask> tasks;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        MultiPlatformTask task = createTaskFromRow(row);
        task.deviceIds = getDeviceIdsByTaskId(task.taskId);
        tasks.push_back(task);
    }
    
    mysql_free_result(result);
    return tasks;
}

// 根据辐射源ID获取多平台任务
std::vector<MultiPlatformTask> MultiPlatformTaskDAO::getMultiPlatformTasksByRadiationId(int radiationId) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return std::vector<MultiPlatformTask>{};
    
    char sql[512];
    snprintf(sql, sizeof(sql),
        "SELECT task_id, tech_system, radiation_id, execution_time, "
        "target_longitude, target_latitude, target_altitude, movement_speed, "
        "movement_azimuth, movement_elevation, positioning_distance, positioning_time, "
        "positioning_accuracy, created_at FROM multi_platform_task WHERE radiation_id = %d "
        "ORDER BY task_id DESC",
        radiationId
    );
    
    if (mysql_query(conn, sql)) {
        std::cerr << "Failed to execute query: " << mysql_error(conn) << std::endl;
        return std::vector<MultiPlatformTask>{};
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        std::cerr << "Failed to store result: " << mysql_error(conn) << std::endl;
        return std::vector<MultiPlatformTask>{};
    }
    
    std::vector<MultiPlatformTask> tasks;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        MultiPlatformTask task = createTaskFromRow(row);
        task.deviceIds = getDeviceIdsByTaskId(task.taskId);
        tasks.push_back(task);
    }
    
    mysql_free_result(result);
    return tasks;
}

// 更新多平台任务
bool MultiPlatformTaskDAO::updateMultiPlatformTask(const MultiPlatformTask& task) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return false;
    
    char sql[1024];
    snprintf(sql, sizeof(sql),
        "UPDATE multi_platform_task SET tech_system = '%s', radiation_id = %d, "
        "execution_time = %f, target_longitude = %.6f, target_latitude = %.6f, "
        "target_altitude = %.2f, movement_speed = %f, movement_azimuth = %.2f, "
        "movement_elevation = %.2f, positioning_distance = %f, positioning_time = %f, "
        "positioning_accuracy = %.6f WHERE task_id = %d",
        task.techSystem.c_str(),
        task.radiationId,
        task.executionTime,
        task.targetLongitude,
        task.targetLatitude,
        task.targetAltitude,
        task.movementSpeed,
        task.movementAzimuth,
        task.movementElevation,
        task.positioningDistance,
        task.positioningTime,
        task.positioningAccuracy,
        task.taskId
    );
    
    if (!db.executeSQL(sql)) {
        std::cerr << "Failed to update multi platform task: " << mysql_error(conn) << std::endl;
        std::cerr << "SQL: " << sql << std::endl;
        return false;
    }
    
    // 更新任务与设备的关联关系
    if (!saveTaskDeviceRelations(task.taskId, task.deviceIds)) {
        return false;
    }
    
    return true;
}

// 删除多平台任务
bool MultiPlatformTaskDAO::deleteMultiPlatformTask(int taskId) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return false;
    
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM multi_platform_task WHERE task_id = %d", taskId);
    
    if (!db.executeSQL(sql)) {
        std::cerr << "Failed to delete multi platform task: " << mysql_error(conn) << std::endl;
        std::cerr << "SQL: " << sql << std::endl;
        return false;
    }
    
    return true;
}

// 从数据库结果集创建任务对象
MultiPlatformTask MultiPlatformTaskDAO::createTaskFromRow(MYSQL_ROW row) {
    MultiPlatformTask task;
    
    int idx = 0;
    task.taskId = row[idx] ? atoi(row[idx]) : 0; idx++;
    task.techSystem = row[idx] ? row[idx] : ""; idx++;
    task.radiationId = row[idx] ? atoi(row[idx]) : 0; idx++;
    task.executionTime = row[idx] ? atof(row[idx]) : 0.0f; idx++;
    task.targetLongitude = row[idx] ? atof(row[idx]) : 0.0; idx++;
    task.targetLatitude = row[idx] ? atof(row[idx]) : 0.0; idx++;
    task.targetAltitude = row[idx] ? atof(row[idx]) : 0.0; idx++;
    task.movementSpeed = row[idx] ? atof(row[idx]) : 0.0f; idx++;
    task.movementAzimuth = row[idx] ? atof(row[idx]) : 0.0; idx++;
    task.movementElevation = row[idx] ? atof(row[idx]) : 0.0; idx++;
    task.azimuth = row[idx] ? atof(row[idx]) : 0.0; idx++;
    task.elevation = row[idx] ? atof(row[idx]) : 0.0; idx++;
    task.positioningDistance = row[idx] ? atof(row[idx]) : 0.0f; idx++;
    task.positioningTime = row[idx] ? atof(row[idx]) : 0.0f; idx++;
    task.positioningAccuracy = row[idx] ? atof(row[idx]) : 0.0; idx++;
    task.createdAt = row[idx] ? row[idx] : ""; idx++;
    
    return task;
}

// 获取任务关联的设备ID列表
std::vector<int> MultiPlatformTaskDAO::getDeviceIdsByTaskId(int taskId) {
    std::vector<int> deviceIds;
    
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return deviceIds;
    
    char sql[256];
    snprintf(sql, sizeof(sql),
        "SELECT device_id FROM platform_task_relation WHERE simulation_id = %d ORDER BY device_id",
        taskId
    );
    
    if (mysql_query(conn, sql)) {
        std::cerr << "Failed to get device IDs: " << mysql_error(conn) << std::endl;
        return deviceIds;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        std::cerr << "Failed to store result: " << mysql_error(conn) << std::endl;
        return deviceIds;
    }
    
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        deviceIds.push_back(atoi(row[0]));
    }
    
    mysql_free_result(result);
    return deviceIds;
}

// 保存任务与设备的关联关系
bool MultiPlatformTaskDAO::saveTaskDeviceRelations(int taskId, const std::vector<int>& deviceIds) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return false;
    
    // 先删除原有关联关系
    char deleteSql[128];
    snprintf(deleteSql, sizeof(deleteSql),
        "DELETE FROM platform_task_relation WHERE simulation_id = %d",
        taskId
    );
    
    if (!db.executeSQL(deleteSql)) {
        std::cerr << "Failed to delete old relations: " << mysql_error(conn) << std::endl;
        return false;
    }
    
    // 插入新的关联关系
    for (int deviceId : deviceIds) {
        char insertSql[256];
        snprintf(insertSql, sizeof(insertSql),
            "INSERT INTO platform_task_relation (simulation_id, device_id) VALUES (%d, %d)",
            taskId, deviceId
        );
        
        if (!db.executeSQL(insertSql)) {
            std::cerr << "Failed to insert relation: " << mysql_error(conn) << std::endl;
            return false;
        }
    }
    
    return true;
} 