#pragma once

#include "DBConnector.h"
#include <string>
#include <vector>

/**
 * @brief 多平台任务数据结构体
 */
struct MultiPlatformTask {
    int taskId;                 // 任务ID
    std::string positioningAlgorithm; // 定位算法：'时差定位'、'频差定位'和'测向定位'
    int radiationId;            // 关联辐射源模型ID
    float executionTime;        // 仿真执行时长（秒）
    double targetLongitude;     // 目标经度（度）
    double targetLatitude;      // 目标纬度（度）
    double targetAltitude;      // 目标高度（米）
    float movementSpeed;        // 运动速度（米/秒）
    double movementAzimuth;     // 运动方位角（度）
    double movementElevation;   // 运动俯仰角（度）
    double azimuth;             // 方位角（度）
    double elevation;           // 俯仰角（度）
    float positioningDistance;  // 定位距离（米）
    float positioningTime;      // 定位时间（秒）
    double positioningAccuracy; // 定位精度（米）
    std::string createdAt;      // 任务创建时间
    std::vector<int> deviceIds; // 关联的侦察设备ID列表
};

/**
 * @brief 多平台任务数据访问对象类
 */
class MultiPlatformTaskDAO {
private:
    MultiPlatformTaskDAO() {};  // 私有构造函数
    ~MultiPlatformTaskDAO() {}; // 私有析构函数
    
    // 禁止复制和赋值
    MultiPlatformTaskDAO(const MultiPlatformTaskDAO&) = delete;
    MultiPlatformTaskDAO& operator=(const MultiPlatformTaskDAO&) = delete;
    
    // 辅助方法：从数据库结果行创建任务对象
    MultiPlatformTask createTaskFromRow(MYSQL_ROW row);
    
    // 辅助方法：获取任务关联的设备ID列表
    std::vector<int> getTaskDeviceIds(int taskId);
    
    // 辅助方法：删除任务的设备关联
    bool deleteTaskDeviceRelations(int taskId);
    
public:
    // 获取单例实例
    static MultiPlatformTaskDAO& getInstance();
    
    // 添加多平台任务
    bool addMultiPlatformTask(const MultiPlatformTask& task, int& taskId);
    
    // 根据任务ID获取多平台任务
    MultiPlatformTask getMultiPlatformTaskById(int taskId);
    
    // 获取所有多平台任务
    std::vector<MultiPlatformTask> getAllMultiPlatformTasks();
    
    // 根据辐射源ID获取多平台任务
    std::vector<MultiPlatformTask> getMultiPlatformTasksByRadiationId(int radiationId);
    
    // 更新多平台任务
    bool updateMultiPlatformTask(const MultiPlatformTask& task);
    
    // 删除多平台任务
    bool deleteMultiPlatformTask(int taskId);
    
    // 保存任务与设备的关联关系
    bool saveTaskDeviceRelations(int taskId, const std::vector<int>& deviceIds);
}; 