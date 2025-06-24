#pragma once

#include "DBConnector.h"
#include <string>
#include <vector>

/**
 * @brief 多平台任务数据结构体
 */
struct MultiPlatformTask {
    int taskId;                 // 任务ID
    std::string techSystem;     // 技术体制：FDOA或TDOA
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
public:
    /**
     * @brief 获取单例实例
     * @return 返回MultiPlatformTaskDAO单例
     */
    static MultiPlatformTaskDAO& getInstance();
    
    /**
     * @brief 添加多平台任务
     * @param task 任务数据
     * @param taskId 返回的任务ID
     * @return 是否成功
     */
    bool addMultiPlatformTask(const MultiPlatformTask& task, int& taskId);
    
    /**
     * @brief 根据任务ID获取多平台任务
     * @param taskId 任务ID
     * @return 任务数据，如果不存在则返回空结构体
     */
    MultiPlatformTask getMultiPlatformTaskById(int taskId);
    
    /**
     * @brief 获取所有多平台任务
     * @return 任务列表
     */
    std::vector<MultiPlatformTask> getAllMultiPlatformTasks();
    
    /**
     * @brief 根据辐射源ID获取多平台任务
     * @param radiationId 辐射源ID
     * @return 任务列表
     */
    std::vector<MultiPlatformTask> getMultiPlatformTasksByRadiationId(int radiationId);
    
    /**
     * @brief 更新多平台任务
     * @param task 任务数据
     * @return 是否成功
     */
    bool updateMultiPlatformTask(const MultiPlatformTask& task);
    
    /**
     * @brief 删除多平台任务
     * @param taskId 任务ID
     * @return 是否成功
     */
    bool deleteMultiPlatformTask(int taskId);

private:
    // 私有构造函数和析构函数
    MultiPlatformTaskDAO() = default;
    ~MultiPlatformTaskDAO() = default;
    
    // 禁止拷贝
    MultiPlatformTaskDAO(const MultiPlatformTaskDAO&) = delete;
    MultiPlatformTaskDAO& operator=(const MultiPlatformTaskDAO&) = delete;
    
    /**
     * @brief 从数据库结果集创建任务对象
     * @param row 数据库行数据
     * @return 任务对象
     */
    MultiPlatformTask createTaskFromRow(MYSQL_ROW row);
    
    /**
     * @brief 获取任务关联的设备ID列表
     * @param taskId 任务ID
     * @return 设备ID列表
     */
    std::vector<int> getDeviceIdsByTaskId(int taskId);
    
    /**
     * @brief 保存任务与设备的关联关系
     * @param taskId 任务ID
     * @param deviceIds 设备ID列表
     * @return 是否成功
     */
    bool saveTaskDeviceRelations(int taskId, const std::vector<int>& deviceIds);
}; 