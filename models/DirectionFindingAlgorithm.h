#pragma once

#include "ReconnaissanceDeviceModel.h"
#include "RadiationSourceModel.h"
#include "../utils/CoordinateTransform.h"
#include "SinglePlatformTaskDAO.h"
#include "MultiPlatformTaskDAO.h"
#include <vector>
#include <string>
#include <Eigen/Dense>

/**
 * @brief 多平台测向定位算法类 - 实现基于多站测向的定位算法
 */
class DirectionFindingAlgorithm {
public:
    // 定位结果结构体
    struct DirectionFindingResult {
        COORD3 position;        // 辐射源位置
        double localizationTime; // 定位时间
        double accuracy;        // 定位精度
        std::vector<double> errorFactors; // 误差因子
    };

    // 获取单例实例
    static DirectionFindingAlgorithm& getInstance();

    // 初始化算法参数
    void init(const std::vector<std::string>& deviceNames, 
              const std::string& sourceName, 
              const std::string& systemType,
              double simulationTime);

    // 执行定位计算
    bool calculate();

    // 获取计算结果
    DirectionFindingResult getResult() const;

    // 计算定位精度 (几何精度因子 GDOP)
    double calculateLocalizationAccuracy(
        const std::vector<int>& deviceIds,
        int sourceId,
        double simulationTime,
        const COORD3& estimatedPosition);

private:
    // 私有构造函数
    DirectionFindingAlgorithm();
    ~DirectionFindingAlgorithm();

    // 禁止拷贝和赋值
    DirectionFindingAlgorithm(const DirectionFindingAlgorithm&) = delete;
    DirectionFindingAlgorithm& operator=(const DirectionFindingAlgorithm&) = delete;

    // 加载设备信息
    bool loadDeviceInfo();

    // 加载辐射源信息
    bool loadSourceInfo();

    // 计算测向误差
    double calculateDirectionError(const ReconnaissanceDevice& device);

    // 泰勒级数法优化定位结果
    COORD3 refinePositionByTaylor(
        const std::vector<COORD3>& stationPositions, 
        const std::vector<double>& azimuths,
        const std::vector<double>& elevations, 
        const COORD3& initialGuess);

    // 计算测向角度对应的函数值
    std::pair<double, double> calculateDirectionFunction(
        const COORD3& stationPosition,
        const COORD3& targetPosition);

    // 计算雅可比矩阵
    Eigen::MatrixXd calculateJacobian(
        const std::vector<COORD3>& stationPositions,
        const COORD3& currentPosition);

    // 计算权重矩阵
    Eigen::MatrixXd calculateWeightMatrix(
        const std::vector<double>& directionErrors);

    // 数据成员
    std::vector<ReconnaissanceDevice> m_devices;
    RadiationSource m_source;
    std::string m_systemType;
    double m_simulationTime;
    DirectionFindingResult m_result;
    std::vector<std::string> m_deviceNames;
    std::string m_sourceName;
    bool m_isInitialized;
}; 