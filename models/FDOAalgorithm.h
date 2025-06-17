#pragma once

#include "../models/RadiationSourceDAO.h"
#include "../models/ReconnaissanceDeviceDAO.h"
#include <vector>
#include <memory>
#include <string>
#include "../models/ReconnaissanceDeviceModel.h"
#include "../models/RadiationSourceModel.h"
#include "../utils/Vector3.h"

class FDOAalgorithm {
public:
    // 定位结果结构体
    struct LocationResult {
        double longitude;        // 经度
        double latitude;         // 纬度
        double altitude;         // 高度（米）
        double azimuth;          // 运动方位角（度）
        double elevation;        // 运动俯仰角（度）
        double velocity;         // 运动速度（米/秒）
        double locationTime;     // 定位时间（秒）
        double distance;         // 定位距离（米）
        double accuracy;         // 定位精度（米）
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
    LocationResult getResult() const;

    // 计算时间间隔最小值
    double calculateMinimumTimeInterval(int deviceId, int sourceId);

    // 计算时间间隔最大值
    double calculateMaximumTimeInterval(const std::vector<int>& deviceIds, int sourceId);

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

    std::vector<std::string> m_deviceNames;    // 设备名称列表
    std::string m_sourceName;                  // 辐射源名称
    std::string m_systemType;                  // 技术体制
    double m_simulationTime;                   // 仿真时间
    std::vector<ReconnaissanceDevice> m_devices;  // 设备信息
    RadiationSource m_source;                  // 辐射源信息
    LocationResult m_result;                   // 定位结果
}; 