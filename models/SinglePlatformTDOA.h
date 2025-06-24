#pragma once

#include "ReconnaissanceDeviceModel.h"
#include "RadiationSourceModel.h"
#include "InterferometerPositioning.h" // 复用LocationResult结构体
#include <vector>
#include <utility>

/**
 * @brief 单平台时差体制定位算法类
 */
class SinglePlatformTDOA {
public:
    /**
     * @brief 获取单例实例
     * @return 返回SinglePlatformTDOA单例
     */
    static SinglePlatformTDOA& getInstance();
    
    /**
     * @brief 时差体制定位算法
     * @param device 侦察设备
     * @param source 辐射源
     * @param simulationTime 仿真时间（秒）
     * @return 定位结果
     */
    LocationResult runSimulation(const ReconnaissanceDevice& device, 
                               const RadiationSource& source,
                               int simulationTime);
    
    /**
     * @brief 计算时差体制误差因素
     * @param baselineLength 基线长度
     * @param timeDifference 时间差
     * @param estimatedDistance 估计距离
     * @param incidentAngle 入射角
     * @return 误差因素数组
     */
    std::vector<double> calculateTDOAErrors(double baselineLength, 
                                         double timeDifference, 
                                         double estimatedDistance,
                                         double incidentAngle);

private:
    // 私有构造函数和析构函数
    SinglePlatformTDOA() = default;
    ~SinglePlatformTDOA() = default;
    
    // 禁止拷贝
    SinglePlatformTDOA(const SinglePlatformTDOA&) = delete;
    SinglePlatformTDOA& operator=(const SinglePlatformTDOA&) = delete;
    
    /**
     * @brief 互相关法计算时差
     */
    double calculateTimeDifferenceCorrelation(
        const std::vector<double>& signal1, 
        const std::vector<double>& signal2,
        double samplingRate);
    
    /**
     * @brief 频谱相位法计算时差
     */
    double calculateTimeDifferencePhase(
        const std::vector<double>& signal1, 
        const std::vector<double>& signal2,
        double samplingRate, 
        double frequency);
}; 