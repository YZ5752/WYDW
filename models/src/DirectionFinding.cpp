#include "../constants/PhysicsConstants.h"
#include "DirectionFinding.h"
#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>
#include <string>
#include "ReconnaissanceDeviceDAO.h"
#include "RadiationSourceDAO.h"

DirectionFinding& DirectionFinding::getInstance() {
    static DirectionFinding instance;
    return instance;
}

DirectionFinding::DirectionFinding() : m_isInitialized(false), m_simulationTime(0) {}
DirectionFinding::~DirectionFinding() = default;

void DirectionFinding::init(const std::vector<std::string>& deviceNames, const std::string& sourceName, double simulationTime) {
    m_deviceNames = deviceNames;
    m_sourceName = sourceName;
    m_simulationTime = simulationTime;
    m_devices.clear();
    m_isInitialized = false;
    m_result = Result{};
}

bool DirectionFinding::loadDeviceInfo() {
    m_devices.clear();
    ReconnaissanceDeviceDAO& deviceDAO = ReconnaissanceDeviceDAO::getInstance();
    auto allDevices = deviceDAO.getAllReconnaissanceDevices();
    for (const auto& name : m_deviceNames) {
        auto it = std::find_if(allDevices.begin(), allDevices.end(), [&](const ReconnaissanceDevice& d) {
            return d.getDeviceName() == name;
        });
        if (it != allDevices.end()) {
            m_devices.push_back(*it);
        } else {
            std::cerr << "找不到侦察设备: " << name << std::endl;
            return false;
        }
    }
    return m_devices.size() >= 2;
}

bool DirectionFinding::loadSourceInfo() {
    RadiationSourceDAO& sourceDAO = RadiationSourceDAO::getInstance();
    auto allSources = sourceDAO.getAllRadiationSources();
    auto it = std::find_if(allSources.begin(), allSources.end(), [&](const RadiationSource& s) {
        return s.getRadiationName() == m_sourceName;
    });
    if (it != allSources.end()) {
        m_source = *it;
        return true;
    } else {
        std::cerr << "找不到辐射源: " << m_sourceName << std::endl;
        return false;
    }
}

std::vector<int> DirectionFinding::getDeviceIds() const {
    std::vector<int> ids;
    for (const auto& d : m_devices) ids.push_back(d.getDeviceId());
    return ids;
}

int DirectionFinding::getSourceId() const { return m_source.getRadiationId(); }

bool DirectionFinding::calculate() {
    if (!loadDeviceInfo() || !loadSourceInfo()) return false;
    // 只用前两个设备
    const auto& dev1 = m_devices[0];
    const auto& dev2 = m_devices[1];
    double esm1MeanError = 3.0;
    double esm1StdDev = 1.0;
    double esm2MeanError = 3.0;
    double esm2StdDev = 1.0;
    double error = 0.0;
    // COORD3->Vector3
    COORD3 esm1_coord = lbh2xyz(dev1.getLongitude(), dev1.getLatitude(), dev1.getAltitude());
    COORD3 esm2_coord = lbh2xyz(dev2.getLongitude(), dev2.getLatitude(), dev2.getAltitude());
    COORD3 target_coord = lbh2xyz(m_source.getLongitude(), m_source.getLatitude(), m_source.getAltitude());
    Vector3 esm1(esm1_coord.p1, esm1_coord.p2, esm1_coord.p3);
    Vector3 esm2(esm2_coord.p1, esm2_coord.p2, esm2_coord.p3);
    Vector3 target(target_coord.p1, target_coord.p2, target_coord.p3);
    double commonHeight = esm1.z;
    esm2.z = commonHeight;
    target.z = commonHeight;
    Vector3 dir1 = calculateDirectionWithError(esm1, target, esm1MeanError, esm1StdDev);
    Vector3 dir2 = calculateDirectionWithError(esm2, target, esm2MeanError, esm2StdDev);
    Vector3 estimatedPosition = intersectDirections2D(esm1, dir1, esm2, dir2);
    error = std::sqrt((estimatedPosition.x - target.x) * (estimatedPosition.x - target.x) +
                      (estimatedPosition.y - target.y) * (estimatedPosition.y - target.y));
    auto lbh = xyz2lbh(estimatedPosition.x, estimatedPosition.y, estimatedPosition.z);
    m_result.position = {lbh.p1, lbh.p2, lbh.p3};
    m_result.error = error;
    m_isInitialized = true;
    return true;
}

DirectionFinding::Result DirectionFinding::getResult() const { return m_result; }

// 向量相关函数实现
Vector3 calculateDirectionWithError(
    const Vector3& observer, 
    const Vector3& target,
    double meanErrorDeg,  // 均值误差（度）
    double stdDevDeg      // 标准差（度）
) {
    double trueAzimuth = std::atan2(target.y - observer.y, target.x - observer.x);
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<> angleDist(meanErrorDeg, stdDevDeg);
    double angularErrorRad = angleDist(gen) * Constants::DEG2RAD;
    double measuredAzimuth = trueAzimuth + angularErrorRad;
    return Vector3(std::cos(measuredAzimuth), std::sin(measuredAzimuth), 0);
}

Vector3 intersectDirections2D(
    const Vector3& obs1, const Vector3& dir1, 
    const Vector3& obs2, const Vector3& dir2
) {
    Vector3 p1p2 = obs2 - obs1;
    double denominator = dir1.x * dir2.y - dir1.y * dir2.x;
    if (std::abs(denominator) < 1e-10) {
        return Vector3((obs1.x + obs2.x) / 2, (obs1.y + obs2.y) / 2, obs1.z);
    }
    double t = (p1p2.x * dir2.y - p1p2.y * dir2.x) / denominator;
    return Vector3(
        obs1.x + dir1.x * t,
        obs1.y + dir1.y * t,
        obs1.z
    );
} 