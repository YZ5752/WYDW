#include "../SinglePlatformTaskDAO.h"
#include <iostream>
#include <cstring>
#include <gtk/gtk.h>

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
        "target_longitude, target_latitude, target_altitude, azimuth, elevation, angle_error, "
        "positioning_distance, positioning_time, positioning_accuracy, direction_finding_accuracy) "
        "VALUES ('%s', %d, %d, %f, %.6f, %.6f, %.2f, %.2f, %.2f, %.6f, %f, %f, %.6f, %.6f)",
        task.techSystem.c_str(),
        task.deviceId,
        task.radiationId,
        task.executionTime,
        task.targetLongitude,
        task.targetLatitude,
        task.targetAltitude,
        task.azimuth,
        task.elevation,
        task.angleError,
        task.positioningDistance,
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
        "target_longitude, target_latitude, target_altitude, azimuth, elevation, angle_error, "
        "positioning_distance, positioning_time, positioning_accuracy, direction_finding_accuracy, "
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
                     "target_longitude, target_latitude, target_altitude, azimuth, elevation, angle_error, "
                     "positioning_distance, positioning_time, positioning_accuracy, direction_finding_accuracy, "
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
        "target_longitude, target_latitude, target_altitude, azimuth, elevation, angle_error, "
        "positioning_distance, positioning_time, positioning_accuracy, direction_finding_accuracy, "
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
        "target_longitude, target_latitude, target_altitude, azimuth, elevation, angle_error, "
        "positioning_distance, positioning_time, positioning_accuracy, direction_finding_accuracy, "
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
        "azimuth = %.2f, elevation = %.2f, angle_error = %.6f, positioning_distance = %f, positioning_time = %f, "
        "positioning_accuracy = %.6f, direction_finding_accuracy = %.6f WHERE task_id = %d",
        task.techSystem.c_str(),
        task.deviceId,
        task.radiationId,
        task.executionTime,
        task.targetLongitude,
        task.targetLatitude,
        task.targetAltitude,
        task.azimuth,
        task.elevation,
        task.angleError,
        task.positioningDistance,
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
    task.azimuth = row[8] ? atof(row[8]) : 0.0;
    task.elevation = row[9] ? atof(row[9]) : 0.0;
    task.angleError = row[10] ? atof(row[10]) : 0.0;
    task.positioningDistance = row[11] ? atof(row[11]) : 0.0f;
    task.positioningTime = row[12] ? atof(row[12]) : 0.0f;
    task.positioningAccuracy = row[13] ? atof(row[13]) : 0.0;
    task.directionFindingAccuracy = row[14] ? atof(row[14]) : 0.0;
    task.createdAt = row[15] ? row[15] : "";
    
    return task;
}

// 根据辐射源ID获取任务
std::vector<SinglePlatformTask> SinglePlatformTaskDAO::getTasksBySourceId(int sourceId) {
    std::vector<SinglePlatformTask> tasks;
    
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) {
        g_print("SinglePlatformTaskDAO: No valid database connection\n");
        return tasks;
    }
    
    // 打印当前数据库名称
    if (mysql_query(conn, "SELECT DATABASE()") == 0) {
        MYSQL_RES* res_dbname = mysql_store_result(conn);
        if (res_dbname) {
            MYSQL_ROW row_dbname = mysql_fetch_row(res_dbname);
            if (row_dbname && row_dbname[0]) {
                g_print("SinglePlatformTaskDAO: 当前数据库: %s\n", row_dbname[0]);
            }
            mysql_free_result(res_dbname);
        }
    }
    
    // 增加缓冲区大小，从256增加到1024，避免SQL查询被截断
    char sql[1024];
    snprintf(sql, sizeof(sql), 
        "SELECT task_id, tech_system, device_id, radiation_id, execution_time, "
        "target_longitude, target_latitude, target_altitude, azimuth, elevation, angle_error, "
        "positioning_distance, positioning_time, positioning_accuracy, direction_finding_accuracy "
        "FROM single_platform_task WHERE radiation_id=%d", 
        sourceId);
    
    g_print("SinglePlatformTaskDAO: 执行SQL: %s\n", sql);
    
    if (mysql_query(conn, sql)) {
        g_print("SinglePlatformTaskDAO: Failed to execute query - %s\n", mysql_error(conn));
        return tasks;
    }
    
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        g_print("SinglePlatformTaskDAO: Failed to store result - %s\n", mysql_error(conn));
        return tasks;
    }
    
    g_print("SinglePlatformTaskDAO: 查询到 %lu 条记录\n", mysql_num_rows(res));
    
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        SinglePlatformTask task;
        
        int idx = 0;
        task.taskId = row[idx] ? atoi(row[idx]) : 0; idx++;
        task.techSystem = row[idx] ? row[idx] : ""; idx++;
        task.deviceId = row[idx] ? atoi(row[idx]) : 0; idx++;
        task.radiationId = row[idx] ? atoi(row[idx]) : 0; idx++;
        task.executionTime = row[idx] ? atof(row[idx]) : 0.0f; idx++;
        task.targetLongitude = row[idx] ? atof(row[idx]) : 0.0; idx++;
        task.targetLatitude = row[idx] ? atof(row[idx]) : 0.0; idx++;
        task.targetAltitude = row[idx] ? atof(row[idx]) : 0.0; idx++;
        task.azimuth = row[idx] ? atof(row[idx]) : 0.0; idx++;
        task.elevation = row[idx] ? atof(row[idx]) : 0.0; idx++;
        task.angleError = row[idx] ? atof(row[idx]) : 0.0; idx++;
        task.positioningDistance = row[idx] ? atof(row[idx]) : 0.0f; idx++;
        task.positioningTime = row[idx] ? atof(row[idx]) : 0.0f; idx++;
        task.positioningAccuracy = row[idx] ? atof(row[idx]) : 0.0; idx++;
        task.directionFindingAccuracy = row[idx] ? atof(row[idx]) : 0.0; idx++;
        
        g_print("SinglePlatformTaskDAO: 加载任务 ID=%d, radiation_id=%d\n", task.taskId, task.radiationId);
        tasks.push_back(task);
    }
    
    mysql_free_result(res);
    g_print("SinglePlatformTaskDAO: 总共加载了 %zu 个任务\n", tasks.size());
    return tasks;
} 