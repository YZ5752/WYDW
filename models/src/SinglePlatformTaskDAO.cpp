#include "../SinglePlatformTaskDAO.h"
#include <iostream>
#include <cstring>

// 单例实现
SinglePlatformTaskDAO& SinglePlatformTaskDAO::getInstance() {
    static SinglePlatformTaskDAO instance;
    return instance;
}

// 添加单平台任务
bool SinglePlatformTaskDAO::addSinglePlatformTask(const SinglePlatformTask& task, int& taskId) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return false;
    
    char sql[1024];
    snprintf(sql, sizeof(sql),
        "INSERT INTO single_platform_task (tech_system, device_id, radiation_id, execution_time, "
        "target_longitude, target_latitude, target_altitude, target_angle, angle_error, "
        "max_positioning_distance, positioning_time, positioning_accuracy, direction_finding_accuracy) "
        "VALUES ('%s', %d, %d, %f, %.6f, %.6f, %.2f, %.5f, %.6f, %f, %f, %.6f, %.6f)",
        task.techSystem.c_str(),
        task.deviceId,
        task.radiationId,
        task.executionTime,
        task.targetLongitude,
        task.targetLatitude,
        task.targetAltitude,
        task.targetAngle,
        task.angleError,
        task.maxPositioningDistance,
        task.positioningTime,
        task.positioningAccuracy,
        task.directionFindingAccuracy
    );
    
    if (!db.executeSQL(sql)) {
        std::cerr << "Failed to add single platform task: " << mysql_error(conn) << std::endl;
        std::cerr << "SQL: " << sql << std::endl;
        return false;
    }
    
    // 获取自增ID
    taskId = (int)mysql_insert_id(conn);
    return true;
}

// 根据任务ID获取单平台任务
SinglePlatformTask SinglePlatformTaskDAO::getSinglePlatformTaskById(int taskId) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return SinglePlatformTask{};
    
    char sql[256];
    snprintf(sql, sizeof(sql),
        "SELECT task_id, tech_system, device_id, radiation_id, execution_time, "
        "target_longitude, target_latitude, target_altitude, target_angle, angle_error, "
        "max_positioning_distance, positioning_time, positioning_accuracy, direction_finding_accuracy, "
        "created_at FROM single_platform_task WHERE task_id = %d",
        taskId
    );
    
    if (mysql_query(conn, sql)) {
        std::cerr << "Failed to execute query: " << mysql_error(conn) << std::endl;
        return SinglePlatformTask{};
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        std::cerr << "Failed to store result: " << mysql_error(conn) << std::endl;
        return SinglePlatformTask{};
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return SinglePlatformTask{};
    }
    
    SinglePlatformTask task = createTaskFromRow(row);
    mysql_free_result(result);
    return task;
}

// 获取所有单平台任务
std::vector<SinglePlatformTask> SinglePlatformTaskDAO::getAllSinglePlatformTasks() {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return std::vector<SinglePlatformTask>{};
    
    const char* sql = "SELECT task_id, tech_system, device_id, radiation_id, execution_time, "
                     "target_longitude, target_latitude, target_altitude, target_angle, angle_error, "
                     "max_positioning_distance, positioning_time, positioning_accuracy, direction_finding_accuracy, "
                     "created_at FROM single_platform_task ORDER BY task_id DESC";
    
    if (mysql_query(conn, sql)) {
        std::cerr << "Failed to execute query: " << mysql_error(conn) << std::endl;
        return std::vector<SinglePlatformTask>{};
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        std::cerr << "Failed to store result: " << mysql_error(conn) << std::endl;
        return std::vector<SinglePlatformTask>{};
    }
    
    std::vector<SinglePlatformTask> tasks;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        tasks.push_back(createTaskFromRow(row));
    }
    
    mysql_free_result(result);
    return tasks;
}

// 根据设备ID获取单平台任务
std::vector<SinglePlatformTask> SinglePlatformTaskDAO::getSinglePlatformTasksByDeviceId(int deviceId) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return std::vector<SinglePlatformTask>{};
    
    char sql[256];
    snprintf(sql, sizeof(sql),
        "SELECT task_id, tech_system, device_id, radiation_id, execution_time, "
        "target_longitude, target_latitude, target_altitude, target_angle, angle_error, "
        "max_positioning_distance, positioning_time, positioning_accuracy, direction_finding_accuracy, "
        "created_at FROM single_platform_task WHERE device_id = %d ORDER BY task_id DESC",
        deviceId
    );
    
    if (mysql_query(conn, sql)) {
        std::cerr << "Failed to execute query: " << mysql_error(conn) << std::endl;
        return std::vector<SinglePlatformTask>{};
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        std::cerr << "Failed to store result: " << mysql_error(conn) << std::endl;
        return std::vector<SinglePlatformTask>{};
    }
    
    std::vector<SinglePlatformTask> tasks;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        tasks.push_back(createTaskFromRow(row));
    }
    
    mysql_free_result(result);
    return tasks;
}

// 根据辐射源ID获取单平台任务
std::vector<SinglePlatformTask> SinglePlatformTaskDAO::getSinglePlatformTasksByRadiationId(int radiationId) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return std::vector<SinglePlatformTask>{};
    
    char sql[256];
    snprintf(sql, sizeof(sql),
        "SELECT task_id, tech_system, device_id, radiation_id, execution_time, "
        "target_longitude, target_latitude, target_altitude, target_angle, angle_error, "
        "max_positioning_distance, positioning_time, positioning_accuracy, direction_finding_accuracy, "
        "created_at FROM single_platform_task WHERE radiation_id = %d ORDER BY task_id DESC",
        radiationId
    );
    
    if (mysql_query(conn, sql)) {
        std::cerr << "Failed to execute query: " << mysql_error(conn) << std::endl;
        return std::vector<SinglePlatformTask>{};
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        std::cerr << "Failed to store result: " << mysql_error(conn) << std::endl;
        return std::vector<SinglePlatformTask>{};
    }
    
    std::vector<SinglePlatformTask> tasks;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        tasks.push_back(createTaskFromRow(row));
    }
    
    mysql_free_result(result);
    return tasks;
}

// 更新单平台任务
bool SinglePlatformTaskDAO::updateSinglePlatformTask(const SinglePlatformTask& task) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return false;
    
    char sql[1024];
    snprintf(sql, sizeof(sql),
        "UPDATE single_platform_task SET tech_system = '%s', device_id = %d, radiation_id = %d, "
        "execution_time = %f, target_longitude = %.6f, target_latitude = %.6f, target_altitude = %.2f, "
        "target_angle = %.5f, angle_error = %.6f, max_positioning_distance = %f, positioning_time = %f, "
        "positioning_accuracy = %.6f, direction_finding_accuracy = %.6f WHERE task_id = %d",
        task.techSystem.c_str(),
        task.deviceId,
        task.radiationId,
        task.executionTime,
        task.targetLongitude,
        task.targetLatitude,
        task.targetAltitude,
        task.targetAngle,
        task.angleError,
        task.maxPositioningDistance,
        task.positioningTime,
        task.positioningAccuracy,
        task.directionFindingAccuracy,
        task.taskId
    );
    
    if (!db.executeSQL(sql)) {
        std::cerr << "Failed to update single platform task: " << mysql_error(conn) << std::endl;
        std::cerr << "SQL: " << sql << std::endl;
        return false;
    }
    
    return true;
}

// 删除单平台任务
bool SinglePlatformTaskDAO::deleteSinglePlatformTask(int taskId) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) return false;
    
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM single_platform_task WHERE task_id = %d", taskId);
    
    if (!db.executeSQL(sql)) {
        std::cerr << "Failed to delete single platform task: " << mysql_error(conn) << std::endl;
        std::cerr << "SQL: " << sql << std::endl;
        return false;
    }
    
    return true;
}

// 从数据库结果集创建任务对象
SinglePlatformTask SinglePlatformTaskDAO::createTaskFromRow(MYSQL_ROW row) {
    SinglePlatformTask task;
    
    task.taskId = row[0] ? atoi(row[0]) : 0;
    task.techSystem = row[1] ? row[1] : "";
    task.deviceId = row[2] ? atoi(row[2]) : 0;
    task.radiationId = row[3] ? atoi(row[3]) : 0;
    task.executionTime = row[4] ? atof(row[4]) : 0.0f;
    task.targetLongitude = row[5] ? atof(row[5]) : 0.0;
    task.targetLatitude = row[6] ? atof(row[6]) : 0.0;
    task.targetAltitude = row[7] ? atof(row[7]) : 0.0;
    task.targetAngle = row[8] ? atof(row[8]) : 0.0;
    task.angleError = row[9] ? atof(row[9]) : 0.0;
    task.maxPositioningDistance = row[10] ? atof(row[10]) : 0.0f;
    task.positioningTime = row[11] ? atof(row[11]) : 0.0f;
    task.positioningAccuracy = row[12] ? atof(row[12]) : 0.0;
    task.directionFindingAccuracy = row[13] ? atof(row[13]) : 0.0;
    task.createdAt = row[14] ? row[14] : "";
    
    return task;
} 