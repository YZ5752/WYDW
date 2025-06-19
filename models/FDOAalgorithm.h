#pragma once

#include "models/RadiationSourceDAO.h"
#include "models/ReconnaissanceDeviceDAO.h"
#include <vector>
#include <memory>
#include <string>
#include "models/ReconnaissanceDeviceModel.h"
#include "models/RadiationSourceModel.h"
#include "utils/Vector3.h"
#include "utils/SimulationValidator.h"
#include "utils/CoordinateTransform.h"

class FDOAalgorithm {
public:
    // 源位置结果结构体
    struct SourcePositionResult {
        COORD3 position;      // 位置
        Vector3 velocity;     // 速度
        bool converged;       // 是否收敛
        int iterations;       // 迭代次数
        double finalError;    // 最终误差
        double locationTime;  // 定位时间（秒）
    };

    static FDOAalgorithm& getInstance();

    // 初始化算法参数
    void init(const std::vector<std::string>& deviceNames, 
             const std::string& sourceName,
             const std::string& systemType,  // 技术体制："时差体制" 或 "频差体制"
             double simulationTime);         // 仿真时间（秒）

    // 执行定位算法
    bool calculate();

    // 获取定位结果
    SourcePositionResult getResult() const;

    // 计算时间间隔最小值
    double calculateMinimumTimeInterval(int deviceId, int sourceId);

    // 计算时间间隔最大值
    double calculateMaximumTimeInterval(const std::vector<int>& deviceIds, int sourceId);

    // 计算每个观测时刻的频差
    std::vector<std::vector<double>> calculateFrequencyDifferences(
        const std::vector<int>& deviceIds,
        int sourceId,
        double simulationTime,
        double errorStdDev);

    // 非线性优化求解辐射源位置和速度
    SourcePositionResult solveSourcePosition(
        const std::vector<int>& deviceIds,
        const std::vector<std::vector<double>>& observedFDOA,
        double simulationTime,
        COORD3 initialPosition = {0, 0, 0},
        Vector3 initialVelocity = {0, 0, 0},
        int maxIterations = 100,
        double tolerance = 1e-6);

private:
    FDOAalgorithm();
    ~FDOAalgorithm();

    // 禁止拷贝
    FDOAalgorithm(const FDOAalgorithm&) = delete;
    FDOAalgorithm& operator=(const FDOAalgorithm&) = delete;

    // 从数据库获取设备信息
    bool loadDeviceInfo();
    
    // 从数据库获取辐射源信息
    bool loadSourceInfo();

    // 辅助函数：求解二次方程
    std::vector<double> solveQuadratic(double a, double b, double c);

    // 计算每个观测时刻的频差
    std::vector<std::vector<double>> calculateFrequencyDifferences(
        const std::vector<ReconnaissanceDevice>& devices,
        const RadiationSource& source,
        const std::vector<double>& timePoints,
        double signalFrequency);

    // 计算残差向量：观测频差与理论频差之差
    std::vector<double> calculateResiduals(
        const std::vector<double>& params,
        const std::vector<int>& deviceIds,
        const std::vector<std::vector<double>>& observedFDOA,
        const std::vector<double>& timePoints);

    // 计算雅可比矩阵：残差对参数的偏导数
    std::vector<std::vector<double>> calculateJacobian(
        const std::vector<double>& params,
        const std::vector<int>& deviceIds,
        const std::vector<double>& timePoints,
        const std::vector<std::vector<double>>& observedFDOA);

    // 使用高斯消元法求解线性方程组
    std::vector<double> solveLinearEquations(
        const std::vector<std::vector<double>>& A,  // 系数矩阵
        const std::vector<double>& b               // 常数向量
    );

    std::vector<std::string> m_deviceNames;    // 设备名称列表
    std::string m_sourceName;                  // 辐射源名称
    std::string m_systemType;                  // 技术体制
    double m_simulationTime;                   // 仿真时间
    std::vector<ReconnaissanceDevice> m_devices;  // 设备信息
    RadiationSource m_source;                  // 辐射源信息
    SourcePositionResult m_result;                   // 定位结果
}; 