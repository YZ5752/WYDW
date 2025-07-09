#include "../DataSelectionController.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include "../../models/DBConnector.h"
#include <mysql/mysql.h>
#include "../../models/RadiationSourceDAO.h"
#include "../../models/RadiationSourceModel.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/ReconnaissanceDeviceModel.h"
#include "../../views/DataSelectionView.h"

// 单例实现
DataSelectionController& DataSelectionController::getInstance() {
    static DataSelectionController instance;
    return instance;
}

// 构造函数
DataSelectionController::DataSelectionController() : m_view(nullptr) {
}

// 析构函数
DataSelectionController::~DataSelectionController() {
}

// 初始化控制器
void DataSelectionController::init(DataSelectionView* view) {
    m_view = view;
    
}
// 获取视图
DataSelectionView* DataSelectionController::getView() const {
    return m_view;
}
//加载数据
std::vector<std::vector<std::string>> DataSelectionController::getRelatedTasks(int radiationId) {
    std::vector<std::vector<std::string>> tasks;
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) {
        std::cerr << "[数据库连接失败] DBConnector::getInstance().getConnection() 返回nullptr" << std::endl;
        return tasks;
    }
    if (mysql_ping(conn) != 0) {
        std::cerr << "[数据库连接断开] mysql_ping(conn) != 0, 错误: " << mysql_error(conn) << std::endl;
        return tasks;
    }
    // 查询单平台任务
    std::string singleSql = "SELECT '单平台', rd.device_name, sp.task_id, sp.target_longitude, sp.target_latitude, sp.target_altitude, sp.azimuth, sp.elevation "
                            "FROM single_platform_task sp "
                            "JOIN reconnaissance_device_models rd ON sp.device_id = rd.device_id "
                            "WHERE sp.radiation_id = " + std::to_string(radiationId);
    if (mysql_query(conn, singleSql.c_str()) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result != nullptr) {
            int rowCount = 0;
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                std::vector<std::string> task;
                for (int i = 0; i < mysql_num_fields(result); ++i) {
                    task.push_back(row[i] ? row[i] : "");
                }
                tasks.push_back(task);
                rowCount++;
            }
            std::cout << "[单平台任务查询] 查到 " << rowCount << " 条记录" << std::endl;
            mysql_free_result(result);
        }
    } else {
        std::cerr << "[单平台任务查询失败] SQL: " << singleSql << std::endl;
        std::cerr << "[单平台任务查询失败] MySQL错误: " << mysql_error(conn) << std::endl;
        db.showError();
    }
    // 查询多平台任务
    std::string multiSql = "SELECT '多平台', GROUP_CONCAT(rd.device_name SEPARATOR ', '), mp.task_id, mp.target_longitude, mp.target_latitude, mp.target_altitude, mp.azimuth, mp.elevation "
                           "FROM multi_platform_task mp "
                           "JOIN platform_task_relation ptr ON mp.task_id = ptr.simulation_id "
                           "JOIN reconnaissance_device_models rd ON ptr.device_id = rd.device_id "
                           "WHERE mp.radiation_id = " + std::to_string(radiationId) +
                           " GROUP BY mp.task_id";
    if (mysql_query(conn, multiSql.c_str()) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result != nullptr) {
            int rowCount = 0;
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                std::vector<std::string> task;
                for (int i = 0; i < mysql_num_fields(result); ++i) {
                    task.push_back(row[i] ? row[i] : "");
                }
                tasks.push_back(task);
                rowCount++;
            }
            std::cout << "[多平台任务查询] 查到 " << rowCount << " 条记录" << std::endl;
            mysql_free_result(result);
        }
    } else {
        std::cerr << "[多平台任务查询失败] SQL: " << multiSql << std::endl;
        std::cerr << "[多平台任务查询失败] MySQL错误: " << mysql_error(conn) << std::endl;
        db.showError();
    }
    return tasks;
}

void DataSelectionController::deleteSelectedItems(DataSelectionView* view) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0) {
        std::cerr << "数据库连接异常" << std::endl;
        std::cerr.flush();
        return;
    }

    GtkTreeView* m_dataList = GTK_TREE_VIEW(view->getDataList());
    GtkComboBox* m_targetCombo = GTK_COMBO_BOX(view->getTargetCombo());
    GtkTreeSelection *selection = gtk_tree_view_get_selection(m_dataList);
    GtkTreeModel *model;
    GList *selectedRows = gtk_tree_selection_get_selected_rows(selection, &model);
    GList *iter = selectedRows;
    while (iter != nullptr) {
        GtkTreePath *path = static_cast<GtkTreePath*>(iter->data);
        GtkTreeIter treeIter;
        if (gtk_tree_model_get_iter(model, &treeIter, path)) {
            // 获取任务类型
            gchar *taskTypeValue;
            gtk_tree_model_get(model, &treeIter, 1, &taskTypeValue, -1);
            std::string taskType = taskTypeValue ? taskTypeValue : "";
            g_free(taskTypeValue);
            // 获取任务ID
            int taskId = 0;
            gtk_tree_model_get(model, &treeIter, 5, &taskId, -1);
            // 根据任务类型和任务ID删除
            std::string tableName;
            if (taskType == "单平台") {
                tableName = "single_platform_task";
            } else if (taskType == "多平台") {
                tableName = "multi_platform_task";
            } else {
                std::cerr << "未知任务类型: " << taskType << std::endl;
                continue;
            }
            std::string sql = "DELETE FROM " + tableName + " WHERE task_id = " + std::to_string(taskId);
            if (mysql_query(conn, sql.c_str()) != 0) {
                std::cerr << "删除失败: " << mysql_error(conn) << std::endl;
            } else {
                std::cout << "成功删除任务ID: " << taskId << std::endl;
            }
        }
        iter = g_list_next(iter);
    }
    g_list_free_full(selectedRows, reinterpret_cast<GDestroyNotify>(gtk_tree_path_free));
    // 刷新列表
    if (GTK_IS_COMBO_BOX(m_targetCombo)) {
        int active = gtk_combo_box_get_active(m_targetCombo);
        if (active >= 0) {
            auto sources = RadiationSourceDAO::getInstance().getAllRadiationSources();
            if (active < static_cast<int>(sources.size())) {
                view->updateTaskList(sources[active].getRadiationId());
            }
        }
    }
}

// 向数据库录入数据
bool DataSelectionController::importData(DataSelectionView* view, bool isSingle, const std::vector<std::string>& values,
                                       int deviceId, int radiationId, const std::string& techSystem) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    
    if (!conn || mysql_ping(conn) != 0) {
        std::cerr << "数据库连接异常" << std::endl;
        std::cerr.flush();
        return false;
    }
    
    // SQL语句准备
    char sql[1024];
    if (isSingle) {
        // 单平台任务插入
        snprintf(sql, sizeof(sql),
            "INSERT INTO single_platform_task (device_id, radiation_id, tech_system, execution_time, "
            "target_longitude, target_latitude, target_altitude, azimuth, elevation, angle_error, "
            "positioning_distance, positioning_time, positioning_accuracy, direction_finding_accuracy) "
            "VALUES (%d, %d, '%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
            deviceId, radiationId, techSystem.c_str(),
            values[0].c_str(), values[1].c_str(), values[2].c_str(), values[3].c_str(), 
            values[4].c_str(), values[5].c_str(), values[6].c_str(), values[7].c_str(), 
            values[8].c_str(), values[9].c_str(), values[10].c_str()
        );
    } else {
        // 多平台任务插入
        snprintf(sql, sizeof(sql),
            "INSERT INTO multi_platform_task (radiation_id, tech_system, execution_time, "
            "target_longitude, target_latitude, target_altitude, azimuth, elevation, "
            "movement_speed, movement_azimuth, movement_elevation, "
            "positioning_distance, positioning_time, positioning_accuracy) "
            "VALUES (%d, '%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
            radiationId, techSystem.c_str(),
            values[0].c_str(), values[1].c_str(), values[2].c_str(), values[3].c_str(),
            values[4].c_str(), values[5].c_str(), values[6].c_str(), values[7].c_str(),
            values[8].c_str(), values[9].c_str(), values[10].c_str(), values[11].c_str()
        );
    }
    
    // 执行SQL语句
    if (mysql_query(conn, sql) != 0) {
        std::cerr << "数据库插入失败: " << mysql_error(conn) << std::endl;
        return false;
    }
    
    // 如果是多平台任务，还需要添加设备关联关系
    if (!isSingle) {
        int taskId = mysql_insert_id(conn);
        // 这里应该添加设备关联代码，但因为没有完整的多平台录入代码，先留空
    }
    
    // 刷新列表显示
    if (view && GTK_IS_COMBO_BOX(view->getTargetCombo())) {
        int active = gtk_combo_box_get_active(GTK_COMBO_BOX(view->getTargetCombo()));
        auto sources = RadiationSourceDAO::getInstance().getAllRadiationSources();
        if (active >= 0 && active < static_cast<int>(sources.size())) {
            view->updateTaskList(sources[active].getRadiationId());
        }
    }
    
    return true;
} 

// 获取任务详情数据
std::map<std::string, std::string> DataSelectionController::showTaskDetails(int taskId, const std::string& taskType) {
    std::map<std::string, std::string> taskDetails;
    
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0) {
        std::cerr << "数据库连接异常" << std::endl;
        std::cerr.flush();
        taskDetails["error"] = "数据库连接异常";
        return taskDetails;
    }
    
    // 根据任务类型和ID查询详细信息
    std::string sql;
    if (taskType == "单平台") {
        sql = "SELECT spt.*, rdm.device_name, rsm.radiation_name "
              "FROM single_platform_task spt "
              "JOIN reconnaissance_device_models rdm ON spt.device_id = rdm.device_id "
              "JOIN radiation_source_models rsm ON spt.radiation_id = rsm.radiation_id "
              "WHERE spt.task_id = " + std::to_string(taskId);
    } else if (taskType == "多平台") {
        sql = "SELECT mpt.*, rsm.radiation_name "
              "FROM multi_platform_task mpt "
              "JOIN radiation_source_models rsm ON mpt.radiation_id = rsm.radiation_id "
              "WHERE mpt.task_id = " + std::to_string(taskId);
    } else {
        taskDetails["error"] = "未知任务类型: " + taskType;
        return taskDetails;
    }

    if (mysql_query(conn, sql.c_str()) != 0) {
        taskDetails["error"] = "查询失败: " + std::string(mysql_error(conn));
        return taskDetails;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        taskDetails["error"] = "获取结果集失败: " + std::string(mysql_error(conn));
        return taskDetails;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        taskDetails["error"] = "未找到任务ID: " + std::to_string(taskId);
        mysql_free_result(result);
        return taskDetails;
    }
    
    // 基础信息
    taskDetails["taskType"] = taskType;
    taskDetails["taskId"] = std::to_string(taskId);
    
    // 获取字段名和值
    unsigned int num_fields = mysql_num_fields(result);
    MYSQL_FIELD* fields = mysql_fetch_fields(result);
    
    // 处理字段值
    if (taskType == "单平台") {
        int deviceNameIdx = num_fields - 2;
        int radiationNameIdx = num_fields - 1;
        
        taskDetails["deviceName"] = row[deviceNameIdx] ? row[deviceNameIdx] : "";
        taskDetails["radiationName"] = row[radiationNameIdx] ? row[radiationNameIdx] : "";
    } else if (taskType == "多平台") {
        int mpRadiationNameIdx = mysql_num_fields(result) - 1;
        taskDetails["radiationName"] = row[mpRadiationNameIdx] ? row[mpRadiationNameIdx] : "";
        
        // 获取关联设备
        std::string deviceSql = "SELECT rdm.device_name "
                             "FROM platform_task_relation ptr "
                             "JOIN reconnaissance_device_models rdm ON ptr.device_id = rdm.device_id "
                             "WHERE ptr.simulation_id = " + std::to_string(taskId);
        if (mysql_query(conn, deviceSql.c_str()) == 0) {
            MYSQL_RES* deviceResult = mysql_store_result(conn);
            if (deviceResult) {
                std::vector<std::string> deviceNames;
                MYSQL_ROW deviceRow;
                while ((deviceRow = mysql_fetch_row(deviceResult))) {
                    if (deviceRow[0]) deviceNames.push_back(deviceRow[0]);
                }
                
                std::string allDevices;
                for (size_t i = 0; i < deviceNames.size(); ++i) {
                    if (i > 0) allDevices += "，";
                    allDevices += deviceNames[i];
                }
                taskDetails["deviceNames"] = allDevices;
                mysql_free_result(deviceResult);
            }
        }
    }
    
    // 处理其他字段
    for (unsigned int i = 0; i < num_fields; i++) {
        const char* fieldName = fields[i].name;
        const char* value = row[i] ? row[i] : "NULL";
        
        // 跳过一些字段
        if (taskType == "单平台") {
            if (strcmp(fieldName, "device_id") == 0 || strcmp(fieldName, "radiation_id") == 0 || 
                strcmp(fieldName, "task_id") == 0) {
                continue;
            }
        } else if (taskType == "多平台") {
            if (strcmp(fieldName, "radiation_id") == 0 || strcmp(fieldName, "task_id") == 0) {
                continue;
            }
        }
        
        // 添加字段和值到结果
        taskDetails[fieldName] = value;
    }
    
    mysql_free_result(result);
    return taskDetails;
} 