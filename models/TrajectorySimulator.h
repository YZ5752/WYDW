#ifndef TRAJECTORY_SIMULATOR_H
#define TRAJECTORY_SIMULATOR_H

#include <vector>
#include <utility>
#include <string>
#include "../models/ReconnaissanceDeviceModel.h"
#include "../models/RadiationSourceModel.h"

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
     * @brief 模拟设备移动轨迹（兼容旧方法）
     * @param device 侦察设备
     * @param simulationTime 仿真时间（秒）
     * @return 轨迹点（经纬度）
     */
    std::vector<std::pair<double, double>> simulateDeviceMovement(
        ReconnaissanceDevice& device, 
        int simulationTime
    );
    
    /**
     * @brief 模拟辐射源移动轨迹
     * @param source 辐射源
     * @param simulationTime 仿真时间（秒）
     * @return 轨迹点（经纬度）
     */
    std::vector<std::pair<double, double>> simulateSourceMovement(
        RadiationSource& source, 
        int simulationTime
    );
    
    /**
     * @brief 通用移动轨迹模拟方法
     * @param initialLongitude 初始经度
     * @param initialLatitude 初始纬度
     * @param initialAltitude 初始高度
     * @param speed 速度
     * @param azimuth 方位角
     * @param elevation 仰角
     * @param simulationTime 仿真时间（秒）
     * @param updatePosition 是否更新位置
     * @param objectPtr 对象指针
     * @param isDevice 是否为设备
     * @return 轨迹点（经纬度）
     */
    std::vector<std::pair<double, double>> simulateMovement(
        double initialLongitude, 
        double initialLatitude, 
        double initialAltitude,
        double speed,
        double azimuth,
        double elevation,
        int simulationTime,
        bool updatePosition,
        void* objectPtr,
        bool isDevice
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

    /**
     * @brief 执行多设备移动动画
     * @param mapView 地图视图
     * @param devices 侦察设备列表
     * @param source 辐射源
     * @param simulationTime 仿真时间（秒）
     * @param calculatedLongitude 计算得到的经度
     * @param calculatedLatitude 计算得到的纬度
     * @param calculatedAltitude 计算得到的高度
     */
    void animateMultipleDevicesMovement(
        MapView* mapView,
        const std::vector<ReconnaissanceDevice>& devices,
        const RadiationSource& source,
        int simulationTime,
        double calculatedLongitude,
        double calculatedLatitude,
        double calculatedAltitude
    );

private:
    // 私有构造函数和析构函数
    TrajectorySimulator() = default;
    ~TrajectorySimulator() = default;
    
    // 禁止拷贝
    TrajectorySimulator(const TrajectorySimulator&) = delete;
    TrajectorySimulator& operator=(const TrajectorySimulator&) = delete;
};

#endif // TRAJECTORY_SIMULATOR_H 