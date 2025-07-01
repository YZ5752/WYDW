#pragma once

#include "ReconnaissanceDeviceModel.h"
#include "RadiationSourceModel.h"
#include "ReconnaissanceDeviceDAO.h"
#include "RadiationSourceDAO.h"
#include "../utils/CoordinateTransform.h"
#include "../utils/Vector3.h"
#include <vector>
#include <string>
#include <tuple>

class DirectionFinding{
public:
    struct Result {
        COORD3 position; // 定位结果（大地坐标，经度/纬度/高度）
        double error;    // 定位误差
    };
    
    // 测向线结构体，包含设备和目标之间的线信息
    struct DirectionLine {
        int deviceIndex;       // 设备索引
        Vector3 devicePos;     // 设备位置
        Vector3 direction;     // 测向方向
        double meanErrorDeg;   // 均值误差(度)
        double stdDevDeg;      // 标准差(度)
    };

    static DirectionFinding& getInstance();

    DirectionFinding();
    ~DirectionFinding();

    void init(const std::vector<std::string>& deviceNames, const std::string& sourceName, double simulationTime);
    bool loadDeviceInfo();
    bool loadSourceInfo();
    
    // 使用指定的误差参数计算
    bool calculate(double dev1MeanError, double dev1StdDev, 
                 double dev2MeanError, double dev2StdDev);
    
    Result getResult() const;
    std::vector<int> getDeviceIds() const;
    int getSourceId() const;
    
    // 获取测向线信息
    std::vector<DirectionLine> getDirectionLines() const;
    
    // 获取测向误差角度
    std::tuple<double, double> getErrorAngles(int deviceIndex) const;

private:
    DirectionFinding(const DirectionFinding&) = delete;
    DirectionFinding& operator=(const DirectionFinding&) = delete;

    std::vector<std::string> m_deviceNames;
    std::string m_sourceName;
    double m_simulationTime;
    std::vector<ReconnaissanceDevice> m_devices;
    RadiationSource m_source;
    Result m_result;
    bool m_isInitialized;
    
    // 存储测向线信息
    std::vector<DirectionLine> m_directionLines;
    
    // 每个设备的误差设置
    std::vector<std::tuple<double, double>> m_deviceErrors; // 均值误差,标准差
};

// 向量相关函数声明
Vector3 calculateDirectionWithError(
    const Vector3& observer,
    const Vector3& target,
    double meanErrorDeg,
    double stdDevDeg
);

Vector3 intersectDirections2D(
    const Vector3& obs1, const Vector3& dir1,
    const Vector3& obs2, const Vector3& dir2
); 