#include "../FDOAalgorithm.h"
#include "../../constants/PhysicsConstants.h"
#include "../../utils/CoordinateTransform.h"
#include "../../utils/SNRValidator.h"
#include "../../utils/SNRValidator.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

// 单例实现
FDOAalgorithm& FDOAalgorithm::getInstance() {
    static FDOAalgorithm instance;
    return instance;
}

FDOAalgorithm::FDOAalgorithm() {
}

FDOAalgorithm::~FDOAalgorithm() {
}

void FDOAalgorithm::init(const std::vector<std::string>& deviceNames, 
                        const std::string& sourceName,
                        const std::string& systemType,
                        double simulationTime) {
    m_deviceNames = deviceNames;
    m_sourceName = sourceName;
    m_systemType = systemType;
    m_simulationTime = simulationTime;
    m_devices.clear();
    // 初始化定位结果
    m_result = LocationResult{
        0.0,    // longitude
        0.0,    // latitude
        0.0,    // altitude
        0.0,    // azimuth
        0.0,    // elevation
        0.0,    // velocity
        0.0,    // locationTime
        0.0,    // distance
        0.0     // accuracy
    };
}

// 加载设备信息
bool FDOAalgorithm::loadDeviceInfo() {
    ReconnaissanceDeviceDAO& deviceDAO = ReconnaissanceDeviceDAO::getInstance();
    
    // 获取所有设备
    std::vector<ReconnaissanceDevice> allDevices = deviceDAO.getAllReconnaissanceDevices();
    
    // 根据名称查找设备
    for (const std::string& deviceName : m_deviceNames) {
        bool found = false;
        for (const ReconnaissanceDevice& device : allDevices) {
            if (device.getDeviceName() == deviceName) {
                m_devices.push_back(device);
                found = true;
                break;
            }
        }
        if (!found) {
            std::cerr << "Failed to find device with name: " << deviceName << std::endl;
            return false;
        }
    }
    return true;
}

// 加载辐射源信息
bool FDOAalgorithm::loadSourceInfo() {
    RadiationSourceDAO& sourceDAO = RadiationSourceDAO::getInstance();
    
    // 获取所有辐射源
    std::vector<RadiationSource> allSources = sourceDAO.getAllRadiationSources();
    
    // 根据名称查找辐射源
    for (const RadiationSource& source : allSources) {
        if (source.getRadiationName() == m_sourceName) {
            m_source = source;
            return true;
        }
    }
    
    std::cerr << "Failed to find source with name: " << m_sourceName << std::endl;
    return false;
}

// 计算时间间隔最小值
double FDOAalgorithm::calculateMinimumTimeInterval(int deviceId, int sourceId) {
    // 获取侦察设备和辐射源信息
    ReconnaissanceDevice device = ReconnaissanceDeviceDAO::getInstance().getReconnaissanceDeviceById(deviceId);
    RadiationSource source = RadiationSourceDAO::getInstance().getRadiationSourceById(sourceId);
    
    // 计算侦察设备速度向量
    COORD3 deviceVelocity = velocity_lbh2xyz(device.getLongitude(),device.getLatitude(),
        device.getMovementSpeed(),device.getMovementAzimuth(),device.getMovementElevation()
    );
    //计算辐射源速度向量
    COORD3 sourceVelocity = velocity_lbh2xyz(source.getLongitude(),source.getLatitude(),
        source.getMovementSpeed(),source.getMovementAzimuth(),source.getMovementElevation()
    );
    
    // 计算相对速度
    Vector3 relativeVelocity(
        sourceVelocity.p1 - deviceVelocity.p1,
        sourceVelocity.p2 - deviceVelocity.p2,
        sourceVelocity.p3 - deviceVelocity.p3
    );
    
    // 将大地坐标转换为空间直角坐标
    COORD3 deviceXYZ = lbh2xyz(device.getLongitude(),device.getLatitude(),device.getAltitude());
    
    COORD3 sourceXYZ = lbh2xyz(source.getLongitude(),source.getLatitude(),source.getAltitude());
    
    // 计算初始相对位置（在空间直角坐标系中）
    Vector3 initialRelativePosition(
        sourceXYZ.p1 - deviceXYZ.p1,
        sourceXYZ.p2 - deviceXYZ.p2,
        sourceXYZ.p3 - deviceXYZ.p3
    );
    //计算模长
    double initialDistance = initialRelativePosition.magnitude();
    
    // 计算径向速度变化率的最大值
    double relativeSpeedSquared = relativeVelocity.magnitudeSquared();
    double dotProduct = initialRelativePosition.dot(relativeVelocity);
    double radialVelocityRate = (relativeSpeedSquared * initialDistance * initialDistance - dotProduct * dotProduct) 
                               / (initialDistance * initialDistance * initialDistance);
    
    // 取绝对值
    radialVelocityRate = std::abs(radialVelocityRate);
    
    // 计算频率分辨率（Hz）
    double samplingFrequency = device.getSampleRate() * 1e9;  // 将GHz转换为Hz
    double frequencyResolution = samplingFrequency / 4096;  // 使用固定采样点数
    
    // 计算时间间隔最小值
    return (frequencyResolution * Constants::c) / (radialVelocityRate * source.getCarrierFrequency());
}

// 计算时间间隔最大值
double FDOAalgorithm::calculateMaximumTimeInterval(const std::vector<int>& deviceIds, int sourceId) {
    // 获取辐射源信息
    RadiationSource source = RadiationSourceDAO::getInstance().getRadiationSourceById(sourceId);
    
    double minMaxTime = std::numeric_limits<double>::max();
    
    // 将辐射源的大地坐标转换为空间直角坐标
    COORD3 sourceXYZ = lbh2xyz(source.getLongitude(),source.getLatitude(),source.getAltitude());
    
    // 计算所有侦察设备的频率范围交集
    double intersectFreqMin = -std::numeric_limits<double>::infinity();
    double intersectFreqMax = std::numeric_limits<double>::infinity();
    
    // 遍历所有侦察设备，计算侦收频率范围交集
    for (int deviceId : deviceIds) {
        ReconnaissanceDevice device = ReconnaissanceDeviceDAO::getInstance().getReconnaissanceDeviceById(deviceId);
        intersectFreqMin = std::max(intersectFreqMin, static_cast<double>(device.getFreqRangeMin()));
        intersectFreqMax = std::min(intersectFreqMax, static_cast<double>(device.getFreqRangeMax()));
    }
    
    // 计算实际带宽 (GHz)
    double commonBandwidth = intersectFreqMax - intersectFreqMin;
    
    // 对每个设备计算最大时间间隔
    for (int deviceId : deviceIds) {
        // 获取侦察设备信息
        ReconnaissanceDevice device = ReconnaissanceDeviceDAO::getInstance().getReconnaissanceDeviceById(deviceId);
        
        // 使用CoordinateTransform计算速度向量
        COORD3 deviceVelocity = velocity_lbh2xyz(device.getLongitude(),device.getLatitude(),
            device.getMovementSpeed(),device.getMovementAzimuth(),device.getMovementElevation());
        
        COORD3 sourceVelocity = velocity_lbh2xyz(source.getLongitude(),source.getLatitude(),
            source.getMovementSpeed(), source.getMovementAzimuth(), source.getMovementElevation());
        
        // 计算相对速度
        Vector3 relativeVelocity(
            sourceVelocity.p1 - deviceVelocity.p1,
            sourceVelocity.p2 - deviceVelocity.p2,
            sourceVelocity.p3 - deviceVelocity.p3
        );
        
        // 将设备的大地坐标转换为空间直角坐标
        COORD3 deviceXYZ = lbh2xyz(device.getLongitude(),device.getLatitude(),device.getAltitude());
        
        // 计算初始相对位置（在空间直角坐标系中）
        Vector3 initialRelativePosition(
            sourceXYZ.p1 - deviceXYZ.p1,
            sourceXYZ.p2 - deviceXYZ.p2,
            sourceXYZ.p3 - deviceXYZ.p3
        );
        
        // 计算观测阈值（最大可探测距离）
        double observationThreshold = calculateMaxDetectionRange(
            source.getTransmitPower(),    // 发射功率（kW）
            source.getCarrierFrequency(), // 载波频率（GHz）
            device.getNoisePsd(),         // 噪声功率谱密度（dBm/Hz）
            commonBandwidth               // 带宽（GHz）
        );
        
        // 计算距离方程的系数
        double A = relativeVelocity.magnitudeSquared();
        double B = 2.0 * initialRelativePosition.dot(relativeVelocity);
        double C = initialRelativePosition.magnitudeSquared() - observationThreshold * observationThreshold;
        
        // 求解二次方程 At² + Bt + C = 0
        std::vector<double> roots = solveQuadratic(A, B, C);
        
        // 找出有效正根
        double maxTime = 0.0;
        for (double root : roots) {
            if (root > 0) {
                if (maxTime == 0.0 || root < maxTime) {
                    maxTime = root;
                }
            }
        }
        
        // 更新最小最大时间
        if (maxTime > 0 && maxTime < minMaxTime) {
            minMaxTime = maxTime;
        }
    }
    
    // 如果没有有效正根，返回0
    if (minMaxTime == std::numeric_limits<double>::max()) {
        return 0.0;
    }
    
    // 计算时间间隔最大值
    return minMaxTime;
}

// 辅助函数：求解二次方程
std::vector<double> FDOAalgorithm::solveQuadratic(double a, double b, double c) {
    std::vector<double> roots;
    
    // 检查是否为线性方程
    if (a == 0) {
        if (b != 0) {
            roots.push_back(-c / b);
        }
        return roots;
    }
    
    // 计算判别式
    double discriminant = b * b - 4 * a * c;
    
    // 判别式小于0，无实根
    if (discriminant < 0) {
        return roots;
    }
    
    // 判别式等于0，有一个实根
    if (discriminant == 0) {
        roots.push_back(-b / (2 * a));
        return roots;
    }
    
    // 判别式大于0，有两个实根
    double sqrtDiscriminant = std::sqrt(discriminant);
    roots.push_back((-b + sqrtDiscriminant) / (2 * a));
    roots.push_back((-b - sqrtDiscriminant) / (2 * a));
    
    return roots;
}

// 获取定位结果
FDOAalgorithm::LocationResult FDOAalgorithm::getResult() const {
    return m_result;
}

// 执行定位算法
bool FDOAalgorithm::calculate() {
    // 1. 加载设备信息
    if (!loadDeviceInfo()) {
        std::cerr << "Failed to load device information" << std::endl;
        return false;
    }

    // 2. 加载辐射源信息
    if (!loadSourceInfo()) {
        std::cerr << "Failed to load source information" << std::endl;
        return false;
    }

    // 获取辐射源ID
    int sourceId = m_source.getRadiationId();
    
    // 获取设备ID
    int device1Id = m_devices[0].getDeviceId();
    std::vector<int> deviceIds = {m_devices[0].getDeviceId(), m_devices[1].getDeviceId(), m_devices[2].getDeviceId()};
    
    // 计算时间间隔
    double minInterval = calculateMinimumTimeInterval(device1Id, sourceId);
    double maxInterval = calculateMaximumTimeInterval(deviceIds, sourceId);
    
    std::cout << "最小时间间隔: " << minInterval << " 秒" << std::endl;
    std::cout << "最大时间间隔: " << maxInterval << " 秒" << std::endl;
    
    // TODO: 实现频差定位算法
    std::cout << "执行频差定位算法..." << std::endl;

    // 临时返回一个示例结果
    m_result.longitude = 116.3;    // 经度
    m_result.latitude = 39.9;      // 纬度
    m_result.altitude = 100.0;     // 高度（米）
    m_result.azimuth = 45.0;       // 运动方位角（度）
    m_result.elevation = 30.0;     // 运动俯仰角（度）
    m_result.velocity = 100.0;     // 运动速度（米/秒）
    m_result.locationTime = 10.0;  // 定位时间（秒）
    m_result.distance = 5000.0;    // 定位距离（米）
    m_result.accuracy = 50.0;      // 定位精度（米）

    return true;
}