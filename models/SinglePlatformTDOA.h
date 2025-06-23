#pragma once

#include "ReconnaissanceDeviceModel.h"
#include "RadiationSourceModel.h"
#include "InterferometerPositioning.h" // 包含定义了LocationResult的头文件

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
     * @brief 互相关法计算时差
     * @param signal1 第一个信号采样序列
     * @param signal2 第二个信号采样序列
     * @param samplingRate 采样率（Hz）
     * @return 估计的时间差（秒）
     */
    double calculateTimeDifferenceCorrelation(
        const std::vector<double>& signal1, 
        const std::vector<double>& signal2,
        double samplingRate);
    
    /**
     * @brief 频谱相位法计算时差
     * @param signal1 第一个信号采样序列
     * @param signal2 第二个信号采样序列
     * @param samplingRate 采样率（Hz）
     * @param frequency 待分析的信号频率（Hz）
     * @return 估计的时间差（秒）
     */
    double calculateTimeDifferencePhase(
        const std::vector<double>& signal1, 
        const std::vector<double>& signal2,
        double samplingRate,
        double frequency);
    
    /**
     * @brief 计算时差体制误差因素
     * @param baselineLength 基线长度（米）
     * @param timeDifference 时间差（秒）
     * @param estimatedDistance 估计距离（米）
     * @param incidentAngle 入射角（弧度）
     * @return 误差因素数组
     */
    std::vector<double> calculateTDOAErrors(double baselineLength = 100.0, 
                                          double timeDifference = 1e-9, 
                                          double estimatedDistance = 1000.0,
                                          double incidentAngle = 0.0);
                                          
    /**
     * @brief 验证SNR是否满足要求
     * @param device 侦察设备
     * @param source 辐射源
     * @param distance 距离（米）
     * @return SNR验证结果
     */
    bool validateSNR(const ReconnaissanceDevice& device,
                    const RadiationSource& source,
                    double distance);
                    
    /**
     * @brief 验证角度是否在辐射源工作扇区范围内
     * @param source 辐射源
     * @param azimuth 方位角（度）
     * @param elevation 俯仰角（度）
     * @return 角度验证结果
     */
    bool validateAngle(const RadiationSource& source,
                      double azimuth,
                      double elevation);

private:
    // 私有构造函数和析构函数
    SinglePlatformTDOA() = default;
    ~SinglePlatformTDOA() = default;
    
    // 禁止拷贝
    SinglePlatformTDOA(const SinglePlatformTDOA&) = delete;
    SinglePlatformTDOA& operator=(const SinglePlatformTDOA&) = delete;
}; 