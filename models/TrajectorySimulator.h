#pragma once

#include "ReconnaissanceDeviceModel.h"
#include <vector>
#include <utility>
#include <string>

// 前向声明
class MapView;

/**
 * @brief 轨迹模拟器类
 */
class TrajectorySimulator {
public:
    /**
     * @brief 获取单例实例
     * @return 返回TrajectorySimulator单例
     */
    static TrajectorySimulator& getInstance();
    
    /**
     * @brief 模拟设备移动
     * @param device 侦察设备
     * @param simulationTime 仿真时间（秒）
     * @return 轨迹点（经纬度）
     */
    std::vector<std::pair<double, double>> simulateDeviceMovement(
        ReconnaissanceDevice& device, 
        int simulationTime
    );

    /**
     * @brief 执行设备移动动画
     * @param mapView 地图视图
     * @param device 侦察设备
     * @param trajectoryPoints 轨迹点
     * @param simulationTime 仿真时间（秒）
     * @param calculatedLongitude 计算得到的经度
     * @param calculatedLatitude 计算得到的纬度
     * @param calculatedAltitude 计算得到的高度
     * @param sourceName 辐射源名称
     * @param radiationSourceLongitude 辐射源经度
     * @param radiationSourceLatitude 辐射源纬度
     * @param radiationSourceAltitude 辐射源高度
     */
    void animateDeviceMovement(
        MapView* mapView,
        const ReconnaissanceDevice& device,
        const std::vector<std::pair<double, double>>& trajectoryPoints,
        int simulationTime,
        double calculatedLongitude,
        double calculatedLatitude,
        double calculatedAltitude,
        const std::string& sourceName,
        double radiationSourceLongitude,
        double radiationSourceLatitude,
        double radiationSourceAltitude
    );

private:
    // 私有构造函数和析构函数
    TrajectorySimulator() = default;
    ~TrajectorySimulator() = default;
    
    // 禁止拷贝
    TrajectorySimulator(const TrajectorySimulator&) = delete;
    TrajectorySimulator& operator=(const TrajectorySimulator&) = delete;
}; 