#pragma once

#include "DBConnector.h"
#include <string>
#include <vector>
#include <mysql/mysql.h> // Add this line to define MYSQL_ROW

/**
 * @brief 单平台任务数据结构体
 */
struct SinglePlatformTask {
    int taskId;                 // 任务ID
    std::string techSystem;     // 技术体制：INTERFEROMETER或TDOA
    int deviceId;               // 关联侦察设备模型ID
    int radiationId;            // 关联辐射源模型ID
    float executionTime;        // 仿真执行时长（秒）
    double targetLongitude;     // 目标经度（度）
    double targetLatitude;      // 目标纬度（度）
    double targetAltitude;      // 目标高度（米）
    double targetAngle;         // 测向数据（度）
    double angleError;          // 测向误差（度）
    float maxPositioningDistance; // 最远定位距离（米）
    float positioningTime;      // 定位时间（秒）
    double positioningAccuracy; // 定位精度（米）
    double directionFindingAccuracy; // 测向精度（度）
    std::string createdAt;      // 任务创建时间
};

/**
 * @brief 单平台任务数据访问对象类
 */
class SinglePlatformTaskDAO {
public:
    /**
     * @brief 获取单例实例
     * @return 返回SinglePlatformTaskDAO单例
     */
    static SinglePlatformTaskDAO& getInstance();
    
    /**
     * @brief 添加单平台任务
     * @param task 任务数据
     * @param taskId 返回的任务ID
     * @return 是否成功
     */
    bool addSinglePlatformTask(const SinglePlatformTask& task, int& taskId);
    
    /**
     * @brief 根据任务ID获取单平台任务
     * @param taskId 任务ID
     * @return 任务数据，如果不存在则返回空结构体
     */
    SinglePlatformTask getSinglePlatformTaskById(int taskId);
    
    /**
     * @brief 获取所有单平台任务
     * @return 任务列表
     */
    std::vector<SinglePlatformTask> getAllSinglePlatformTasks();
    
    /**
     * @brief 根据设备ID获取单平台任务
     * @param deviceId 设备ID
     * @return 任务列表
     */
    std::vector<SinglePlatformTask> getSinglePlatformTasksByDeviceId(int deviceId);
    
    /**
     * @brief 根据辐射源ID获取单平台任务
     * @param radiationId 辐射源ID
     * @return 任务列表
     */
    std::vector<SinglePlatformTask> getSinglePlatformTasksByRadiationId(int radiationId);
    
    /**
     * @brief 根据辐射源ID获取单平台任务（别名方法）
     * @param sourceId 辐射源ID
     * @return 任务列表
     */
    std::vector<SinglePlatformTask> getTasksBySourceId(int sourceId);
    
    /**
     * @brief 更新单平台任务
     * @param task 任务数据
     * @return 是否成功
     */
    bool updateSinglePlatformTask(const SinglePlatformTask& task);
    
    /**
     * @brief 删除单平台任务
     * @param taskId 任务ID
     * @return 是否成功
     */
    bool deleteSinglePlatformTask(int taskId);

private:
    // 私有构造函数和析构函数
    SinglePlatformTaskDAO() = default;
    ~SinglePlatformTaskDAO() = default;
    
    // 禁止拷贝
    SinglePlatformTaskDAO(const SinglePlatformTaskDAO&) = delete;
    SinglePlatformTaskDAO& operator=(const SinglePlatformTaskDAO&) = delete;
    
    /**
     * @brief 从数据库结果集创建任务对象
     * @param row 数据库行数据
     * @return 任务对象
     */
    SinglePlatformTask createTaskFromRow(MYSQL_ROW row);
}; 