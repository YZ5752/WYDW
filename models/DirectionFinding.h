#pragma once

#include "ReconnaissanceDeviceModel.h"
#include "RadiationSourceModel.h"
#include "ReconnaissanceDeviceDAO.h"
#include "RadiationSourceDAO.h"
#include "../utils/CoordinateTransform.h"
#include "../utils/Vector3.h"
#include <vector>
#include <string>

class DirectionFinding{
public:
    struct Result {
        COORD3 position; // 定位结果（大地坐标，经度/纬度/高度）
        double error;    // 定位误差
    };

    static DirectionFinding& getInstance();

    DirectionFinding();
    ~DirectionFinding();

    void init(const std::vector<std::string>& deviceNames, const std::string& sourceName, double simulationTime);
    bool loadDeviceInfo();
    bool loadSourceInfo();
    bool calculate();
    Result getResult() const;
    std::vector<int> getDeviceIds() const;
    int getSourceId() const;

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