#pragma once

#include "ReconnaissanceDeviceModel.h"
#include "RadiationSourceModel.h"
#include <vector>
#include <utility>

/**
 * @brief 定位结果结构体
 */
struct LocationResult {
    double azimuth;        // 方位角（度）
    double elevation;      // 俯仰角（度）
    double longitude;      // 经度（度）
    double latitude;       // 纬度（度）
    double altitude;       // 高度（米）
    double accuracy;       // 精度（米）
    std::vector<double> errorFactors; // 各种误差因素
};

/**
 * @brief 干涉仪体制定位算法类
 */
class InterferometerPositioning {
public:
    /**
     * @brief 获取单例实例
     * @return 返回InterferometerPositioning单例
     */
    static InterferometerPositioning& getInstance();
    
    /**
     * @brief 干涉仪体制定位算法
     * @param device 侦察设备
     * @param source 辐射源
     * @param simulationTime 仿真时间（秒）
     * @return 定位结果
     */
    LocationResult runSimulation(const ReconnaissanceDevice& device, 
                               const RadiationSource& source,
                               int simulationTime);
    
    /**
     * @brief 计算测向数据 - 干涉仪体制
     * @param device 侦察设备
     * @param source 辐射源
     * @return 方位角和俯仰角
     */
    std::pair<double, double> calculateDirectionData(const ReconnaissanceDevice& device, 
                                                   const RadiationSource& source);
    
    /**
     * @brief 计算定位数据 - 干涉仪体制
     * @param device 侦察设备
     * @param azimuth 方位角（度）
     * @param elevation 俯仰角（度）
     * @return 经度、纬度和高度
     */
    std::pair<std::pair<double, double>, double> calculateLocationData(const ReconnaissanceDevice& device,
                                                                     double azimuth,
                                                                     double elevation);
    
    /**
     * @brief 计算误差因素 - 干涉仪体制
     * @param device 侦察设备
     * @param source 辐射源
     * @param distance 距离（米）
     * @return 误差因素数组
     */
    std::vector<double> calculateErrors(const ReconnaissanceDevice& device,
                                      const RadiationSource& source,
                                      double distance);

private:
    // 私有构造函数和析构函数
    InterferometerPositioning() = default;
    ~InterferometerPositioning() = default;
    
    // 禁止拷贝
    InterferometerPositioning(const InterferometerPositioning&) = delete;
    InterferometerPositioning& operator=(const InterferometerPositioning&) = delete;
}; 