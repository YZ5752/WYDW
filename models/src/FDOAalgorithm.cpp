#include "../FDOAalgorithm.h"
#include "../../constants/PhysicsConstants.h"
#include "../../utils/CoordinateTransform.h"
#include "../../utils/SNRValidator.h"
#include "../../utils/Vector3.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>

// 计算每个观测时刻的频差
std::vector<std::vector<double>> FDOAalgorithm::calculateFrequencyDifferences(
    const std::vector<int>& deviceIds,
    int sourceId,
    double simulationTime) {
    
    std::vector<std::vector<double>> dopplerShifts(3, std::vector<double>(3, 0.0));
    
    // 获取辐射源信息
    RadiationSource source = RadiationSourceDAO::getInstance().getRadiationSourceById(sourceId);
    //辐射源载波频率
    double sourceFrequency = source.getCarrierFrequency() * 1e9;
    
    // 计算观测时刻
    std::vector<double> timePoints = {0.0, simulationTime / 2.0, simulationTime};
    
    // 遍历每个侦察设备
    for (size_t i = 0; i < deviceIds.size(); ++i) {
        // 获取侦察设备信息
        ReconnaissanceDevice device = ReconnaissanceDeviceDAO::getInstance().getReconnaissanceDeviceById(deviceIds[i]);
        
        // 遍历每个观测时刻
        for (size_t j = 0; j < timePoints.size(); ++j) {
            double time = timePoints[j];
            
            // 计算侦察设备在0时刻的位置
            COORD3 devicePos0 = lbh2xyz(device.getLongitude(), device.getLatitude(), device.getAltitude());
            // 计算侦察设备的速度向量
            COORD3 deviceVel0 = velocity_lbh2xyz(device.getLongitude(), device.getLatitude(), 
                device.getMovementSpeed(), device.getMovementAzimuth(), device.getMovementElevation());
            // 计算侦察设备在t时刻的位置
            COORD3 devicePos;
            devicePos.p1 = devicePos0.p1 + deviceVel0.p1 * time;
            devicePos.p2 = devicePos0.p2 + deviceVel0.p2 * time;
            devicePos.p3 = devicePos0.p3 + deviceVel0.p3 * time;

            // 计算辐射源在0时刻的位置
            COORD3 sourcePos0 = lbh2xyz(source.getLongitude(), source.getLatitude(), source.getAltitude());
            // 计算辐射源的速度向量
            COORD3 sourceVel0 = velocity_lbh2xyz(source.getLongitude(), source.getLatitude(),
                source.getMovementSpeed(), source.getMovementAzimuth(), source.getMovementElevation());
            // 计算辐射源在t时刻的位置
            COORD3 sourcePos;
            sourcePos.p1 = sourcePos0.p1 + sourceVel0.p1 * time;
            sourcePos.p2 = sourcePos0.p2 + sourceVel0.p2 * time;
            sourcePos.p3 = sourcePos0.p3 + sourceVel0.p3 * time;

            // 计算相对位置矢量
            Vector3 r(devicePos.p1 - sourcePos.p1, 
                     devicePos.p2 - sourcePos.p2, 
                     devicePos.p3 - sourcePos.p3);
            double distance = r.magnitude();

            // 计算相对速度在视线方向上的分量
            Vector3 relativeVelocity(deviceVel0.p1 - sourceVel0.p1,
                                   deviceVel0.p2 - sourceVel0.p2,
                                   deviceVel0.p3 - sourceVel0.p3);
            double radialVelocity = relativeVelocity.dot(r.normalize());

            // 计算多普勒频移
            double dopplerShift = (radialVelocity / Constants::c) * sourceFrequency;
            dopplerShifts[i][j] = dopplerShift;
        }
    }
    
    return dopplerShifts;
}

// 单例实现
FDOAalgorithm& FDOAalgorithm::getInstance() {
    static FDOAalgorithm instance;
    return instance;
}

FDOAalgorithm::FDOAalgorithm() {
}

FDOAalgorithm::~FDOAalgorithm() {
}
//初始化
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
            
            // 打印辐射源详细信息
            std::cout << "\n辐射源详细信息：" << std::endl;
            std::cout << "----------------------------------------" << std::endl;
            std::cout << "辐射源ID: " << source.getRadiationId() << std::endl;
            std::cout << "辐射源名称: " << source.getRadiationName() << std::endl;
            std::cout << "经度: " << source.getLongitude() << " 度" << std::endl;
            std::cout << "纬度: " << source.getLatitude() << " 度" << std::endl;
            std::cout << "高度: " << source.getAltitude() << " 米" << std::endl;
            std::cout << "运动速度: " << source.getMovementSpeed() << " m/s" << std::endl;
            std::cout << "运动方位角: " << source.getMovementAzimuth() << " 度" << std::endl;
            std::cout << "运动仰角: " << source.getMovementElevation() << " 度" << std::endl;
            std::cout << "载波频率: " << source.getCarrierFrequency() << " GHz" << std::endl;
            std::cout << "----------------------------------------\n" << std::endl;
            
            return true;
        }
    }
    
    std::cerr << "Failed to find source with name: " << m_sourceName << std::endl;
    return false;
}

//计算最小时间间隔
double FDOAalgorithm::calculateMinimumTimeInterval(int deviceId, int sourceId) {
    // 获取侦察设备和辐射源信息
    ReconnaissanceDevice device = ReconnaissanceDeviceDAO::getInstance().getReconnaissanceDeviceById(deviceId);
    RadiationSource source = RadiationSourceDAO::getInstance().getRadiationSourceById(sourceId);
    
    // 计算侦察设备速度向量
    std::cout << "\n计算侦察设备速度向量：" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "设备参数：" << std::endl;
    std::cout << "经度: " << device.getLongitude() << " 度" << std::endl;
    std::cout << "纬度: " << device.getLatitude() << " 度" << std::endl;
    std::cout << "速度: " << device.getMovementSpeed() << " m/s" << std::endl;
    std::cout << "方位角: " << device.getMovementAzimuth() << " 度" << std::endl;
    std::cout << "俯仰角: " << device.getMovementElevation() << " 度" << std::endl;
    
    COORD3 deviceVelocity = velocity_lbh2xyz(device.getLongitude(),device.getLatitude(),
        device.getMovementSpeed(),device.getMovementAzimuth(),device.getMovementElevation()
    );
    
    //计算辐射源速度向量
    std::cout << "\n计算辐射源速度向量：" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "辐射源参数：" << std::endl;
    std::cout << "经度: " << source.getLongitude() << " 度" << std::endl;
    std::cout << "纬度: " << source.getLatitude() << " 度" << std::endl;
    std::cout << "速度: " << source.getMovementSpeed() << " m/s" << std::endl;
    std::cout << "方位角: " << source.getMovementAzimuth() << " 度" << std::endl;
    std::cout << "俯仰角: " << source.getMovementElevation() << " 度" << std::endl;
    
    COORD3 sourceVelocity = velocity_lbh2xyz(source.getLongitude(),source.getLatitude(),
        source.getMovementSpeed(),source.getMovementAzimuth(),source.getMovementElevation()
    );
    
    // 计算相对速度
    Vector3 relativeVelocity(
        sourceVelocity.p1 - deviceVelocity.p1,
        sourceVelocity.p2 - deviceVelocity.p2,
        sourceVelocity.p3 - deviceVelocity.p3
    );
    
    std::cout << "\n相对速度计算结果：" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "设备速度 (vx, vy, vz): " << deviceVelocity.p1 << ", " << deviceVelocity.p2 << ", " << deviceVelocity.p3 << " m/s" << std::endl;
    std::cout << "辐射源速度 (vx, vy, vz): " << sourceVelocity.p1 << ", " << sourceVelocity.p2 << ", " << sourceVelocity.p3 << " m/s" << std::endl;
    std::cout << "相对速度 (vx, vy, vz): " << relativeVelocity.x << ", " << relativeVelocity.y << ", " << relativeVelocity.z << " m/s" << std::endl;
    std::cout << "相对速度大小: " << relativeVelocity.magnitude() << " m/s" << std::endl;
    std::cout << "----------------------------------------\n" << std::endl;
    
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
    // 计算时间间隔最小值
    double carrierFrequencyHz = source.getCarrierFrequency() * 1e9;// 将GHz转换为Hz
    double minInterval = (Constants::FREQUENCY_RESOLUTION * Constants::c) / (radialVelocityRate * carrierFrequencyHz);
   
    
    return minInterval;
}
//计算最大时间间隔
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
    std::vector<int> deviceIds = {m_devices[0].getDeviceId(), m_devices[1].getDeviceId(), m_devices[2].getDeviceId()};
    
    // 3. 执行仿真验证
    // SimulationValidator validator;
    // std::string failMessage;
    // if (!validator.validateAll(deviceIds, sourceId, failMessage)) {
    //     std::cerr << "仿真验证失败：" << failMessage << std::endl;
    //     return false;
    // }
    
    // 4. 计算时间间隔
    double minInterval = calculateMinimumTimeInterval(deviceIds[0], sourceId);
    double maxInterval = calculateMaximumTimeInterval(deviceIds, sourceId);
    
    std::cout << "最小时间间隔: " << minInterval << " 秒" << std::endl;
    std::cout << "最大时间间隔: " << maxInterval << " 秒" << std::endl;
    
    // 验证仿真时间是否合适
    // if (m_simulationTime < minInterval) {
    //     std::cerr << "错误：仿真时间(" << m_simulationTime << "秒)小于最小时间间隔的2倍(" << 2 * minInterval << "秒)" << std::endl;
    //     return false;
    // }
    
    // if (m_simulationTime > 3 * maxInterval) {
    //     std::cerr << "错误：仿真时间(" << m_simulationTime << "秒)大于最大时间间隔的2倍(" << 2 * maxInterval << "秒)" << std::endl;
    //     return false;
    // }
    
    // 5. 执行频差定位算法
    std::cout << "执行频差定位算法..." << std::endl;

    // 计算每个观测时刻的频差
    std::vector<std::vector<double>> measuredFreqDiffs = this->calculateFrequencyDifferences(
        deviceIds, sourceId, m_simulationTime);

    // 数据归一化
    double maxFreqDiff = 0;
    for (const auto& deviceFreqDiffs : measuredFreqDiffs) {
        for (double diff : deviceFreqDiffs) {
            maxFreqDiff = std::max(maxFreqDiff, std::abs(diff));
        }
    }
    for (auto& deviceFreqDiffs : measuredFreqDiffs) {
        for (double& diff : deviceFreqDiffs) {
            diff /= maxFreqDiff;
        }
    }

    // 初始化参数向量 [x0, y0, z0, vx, vy, vz]
    std::vector<double> x(6, 0.0);  // 初始化为全零

    // 使用辐射源的位置和速度作为初始值
    std::cout << "\n辐射源原始参数：" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "经度: " << m_source.getLongitude() << " 度" << std::endl;
    std::cout << "纬度: " << m_source.getLatitude() << " 度" << std::endl;
    std::cout << "高度: " << m_source.getAltitude() << " 米" << std::endl;
    std::cout << "运动速度: " << m_source.getMovementSpeed() << " m/s" << std::endl;
    std::cout << "运动方位角: " << m_source.getMovementAzimuth() << " 度" << std::endl;
    std::cout << "运动俯仰角: " << m_source.getMovementElevation() << " 度" << std::endl;

    // 将大地坐标转换为空间直角坐标
    COORD3 sourcePos = lbh2xyz(m_source.getLongitude(), m_source.getLatitude(), m_source.getAltitude());
    // 设置位置初始值（空间直角坐标）
    x[0] = sourcePos.p1;
    x[1] = sourcePos.p2;
    x[2] = sourcePos.p3;
    
    // // 计算速度分量
    // double speed = m_source.getMovementSpeed();
    // double azimuth = m_source.getMovementAzimuth();  // 已经是角度制
    // double elevation = m_source.getMovementElevation();  // 已经是角度制
    
    // 使用velocity_lbh2xyz函数计算空间直角坐标系中的速度分量
    COORD3 velocity = velocity_lbh2xyz(
        m_source.getLongitude(),
        m_source.getLatitude(),
        m_source.getMovementSpeed(),
        m_source.getMovementAzimuth(),
        m_source.getMovementElevation()
    );
    
    // 设置速度初始值（空间直角坐标）
    x[3] = velocity.p1;  // Vx
    x[4] = velocity.p2;  // Vy
    x[5] = velocity.p3;  // Vz

    // 打印转换后的空间直角坐标
    std::cout << "\n转换后的空间直角坐标：" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "位置 (x, y, z): " << x[0] << ", " << x[1] << ", " << x[2] << " 米" << std::endl;
    std::cout << "速度 (vx, vy, vz): " <<velocity.p1 << ", " << velocity.p2 << ", " << velocity.p3 << " 米/秒" << std::endl;

    // 验证转换的正确性
    COORD3 verifyLBH = xyz2lbh(x[0], x[1], x[2]);
    COORD3 verifyVelocity = velocity_xyz2lbh(verifyLBH.p1, verifyLBH.p2, x[3], x[4], x[5]);
    std::cout << "\n验证转换结果：" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "转换回大地坐标：" << std::endl;
    std::cout<<"位置："<<verifyLBH.p1<<","<<verifyLBH.p2<<","<<verifyLBH.p3<<std::endl;
    std::cout<<"方位角："<<verifyVelocity.p2<<std::endl;
    std::cout<<"俯仰角："<<verifyVelocity.p3<<std::endl;
    std::cout<<"速度："<<verifyVelocity.p1<<std::endl;

    // 设置初始正则化参数
    double lambda = 0.0001;  // 减小初始正则化参数
    const double lambdaFactor = 1.5;  // 调整正则化参数调整因子
    const double minLambda = 1e-10;    // 减小最小正则化参数
    const double maxLambda = 1e4;     // 最大正则化参数

    // 添加调试信息
    std::cout << "Initial parameters: ";
    for (int i = 0; i < 6; ++i) {
        std::cout << x[i] << " ";
    }
    std::cout << std::endl;

    const int maxIterations = 100;
    const double tolerance = 1e-6;
    const double h = 1e-6;  // 数值差分步长
    double prevError = std::numeric_limits<double>::max();
    bool hasConverged = false;
    //辐射源载波频率
    double signalFrequency = m_source.getCarrierFrequency() * 1e9;
    // 计算观测时刻
    std::vector<double> timePoints = {0.0, m_simulationTime / 2.0, m_simulationTime};
    
    // 声明方位角和俯仰角变量
    double azimuth = 0.0;
    double elevation = 0.0;

    for (int iter = 0; iter < maxIterations; ++iter) {
        // 构建雅可比矩阵和残差向量
        int numEquations = m_devices.size() * timePoints.size();
        std::vector<std::vector<double>> J(numEquations, std::vector<double>(6));
        std::vector<double> residuals(numEquations);
        
        int row = 0;
        for (size_t i = 0; i < m_devices.size(); ++i) {
            for (size_t j = 0; j < timePoints.size(); ++j) {
                double time = timePoints[j];
                
                Vector3 targetPos(x[0] + x[3] * time,
                                x[1] + x[4] * time,
                                x[2] + x[5] * time);
                Vector3 targetVel(x[3], x[4], x[5]);
               

                // 计算侦察设备在t时刻的位置和速度
                COORD3 devicePos0 = lbh2xyz(m_devices[i].getLongitude(), m_devices[i].getLatitude(), m_devices[i].getAltitude());
                COORD3 deviceVel0 = velocity_lbh2xyz(m_devices[i].getLongitude(), m_devices[i].getLatitude(),
                    m_devices[i].getMovementSpeed(), m_devices[i].getMovementAzimuth(), m_devices[i].getMovementElevation());
                
                COORD3 devicePos;
                devicePos.p1 = devicePos0.p1 + deviceVel0.p1 * time;
                devicePos.p2 = devicePos0.p2 + deviceVel0.p2 * time;
                devicePos.p3 = devicePos0.p3 + deviceVel0.p3 * time;

                Vector3 observerPos(devicePos.p1, devicePos.p2, devicePos.p3);
                Vector3 observerVel(deviceVel0.p1, deviceVel0.p2, deviceVel0.p3);

                // 计算相对位置矢量
                Vector3 r = observerPos - targetPos;
                double distance = r.magnitude();

                // 计算相对速度在视线方向上的分量
                Vector3 relativeVelocity = observerVel - targetVel;
                double radialVelocity = relativeVelocity.dot(r.normalize());

                // 计算预测的频差
                double predictedFreqDiff = (radialVelocity / Constants::c) * signalFrequency / maxFreqDiff;

                // 计算残差
                residuals[row] = measuredFreqDiffs[i][j] - predictedFreqDiff;

                // 使用数值差分法计算雅可比矩阵
                for (int k = 0; k < 6; ++k) {
                    std::vector<double> x_perturbed = x;
                    x_perturbed[k] += h;

                    Vector3 targetPos_perturbed(x_perturbed[0] + x_perturbed[3] * time,
                                              x_perturbed[1] + x_perturbed[4] * time,
                                              x_perturbed[2] + x_perturbed[5] * time);
                    Vector3 targetVel_perturbed(x_perturbed[3], x_perturbed[4], x_perturbed[5]);

                    Vector3 r_perturbed = observerPos - targetPos_perturbed;
                    double distance_perturbed = r_perturbed.magnitude();
                    if (distance_perturbed < 1e-10) {
                        distance_perturbed = 1e-10;
                    }
                    Vector3 relativeVelocity_perturbed = observerVel - targetVel_perturbed;
                    double radialVelocity_perturbed = relativeVelocity_perturbed.dot(r_perturbed.normalize());
                    double predictedFreqDiff_perturbed = (radialVelocity_perturbed / Constants::c) * signalFrequency / maxFreqDiff;

                    // 修改雅可比矩阵计算
                    double weight = 1.0;
                    if (k >= 3) {  // 速度参数
                        // 根据参数类型设置不同的权重
                        if (k == 3) weight = 10.0;  // vx
                        else if (k == 4) weight = 10.0;  // vy
                        else if (k == 5) weight = 10.0;  // vz
                    }
                    J[row][k] = weight * (predictedFreqDiff_perturbed - predictedFreqDiff) / h;
                }
                row++;
            }
        }

        // 计算误差范数
        double errorNorm = 0;
        for (double r : residuals) {
            errorNorm += r * r;
        }
        errorNorm = std::sqrt(errorNorm);

        // 计算速度参数的误差
        double speedError = std::abs(std::sqrt(x[3] * x[3] + x[4] * x[4] + x[5] * x[5]) - m_source.getMovementSpeed());
        double azimuthError = std::abs(std::atan2(x[4], x[3]) * Constants::RAD2DEG - m_source.getMovementAzimuth());
        double elevationError = std::abs(std::atan2(x[5], std::sqrt(x[3] * x[3] + x[4] * x[4])) * Constants::RAD2DEG - m_source.getMovementElevation());

        // 计算方位角（正北为0度，顺时针为正）
        azimuth = atan2(x[4], x[3]) * Constants::RAD2DEG;
        if (azimuth < 0) {
            azimuth += 360;
        }
        azimuth = 360 - azimuth;  // 转换为顺时针方向

        // 计算俯仰角（水平为0度，向上为正）
        elevation = atan2(x[5], std::sqrt(x[3] * x[3] + x[4] * x[4])) * Constants::RAD2DEG;

        // 打印转换后的空间直角坐标
        std::cout << "\n当前速度参数：" << std::endl;
        std::cout << "计算得到的速度: " << std::sqrt(x[3] * x[3] + x[4] * x[4] + x[5] * x[5]) << " m/s" << std::endl;
        std::cout << "计算得到的方位角: " << azimuth << " 度" << std::endl;
        std::cout << "计算得到的俯仰角: " << elevation << " 度" << std::endl;
        std::cout << "速度分量 (vx, vy, vz): " << x[3] << ", " << x[4] << ", " << x[5] << " m/s" << std::endl;

        // 修改总误差计算
        double totalError = errorNorm + 0.1 * speedError + 0.1 * std::abs(azimuth - m_source.getMovementAzimuth()) + 0.1 * std::abs(elevation - m_source.getMovementElevation());

        // 修改收敛条件
        if (iter > 0) {
            double errorChange = std::abs(totalError - prevError);
            double relativeErrorChange = errorChange / (prevError + 1e-10);
            if (relativeErrorChange < tolerance || totalError < 1e-3) {  // 放宽收敛条件
                hasConverged = true;
                std::cout << "Converged after " << iter << " iterations." << std::endl;
                break;
            }
        }
        prevError = totalError;

        // 计算JTJ矩阵
        std::vector<std::vector<double>> JTJ(6, std::vector<double>(6, 0));
        for (int i = 0; i < numEquations; ++i) {
            for (int j = 0; j < 6; ++j) {
                for (int k = 0; k < 6; ++k) {
                    JTJ[j][k] += J[i][j] * J[i][k];
                }
            }
        }

        // 添加正则化项
        for (int i = 0; i < 6; ++i) {
            JTJ[i][i] += lambda;
        }

        // 计算梯度
        std::vector<double> gradient(6, 0);
        for (int i = 0; i < numEquations; ++i) {
            for (int j = 0; j < 6; ++j) {
                gradient[j] += J[i][j] * residuals[i];
            }
        }

        // 求解线性方程组
        std::vector<double> dx(6, 0);
        for (int i = 0; i < 6; ++i) {
            int maxRow = i;
            for (int j = i + 1; j < 6; ++j) {
                if (std::abs(JTJ[j][i]) > std::abs(JTJ[maxRow][i])) {
                    maxRow = j;
                }
            }
            if (maxRow != i) {
                std::swap(JTJ[i], JTJ[maxRow]);
                std::swap(gradient[i], gradient[maxRow]);
            }

            if (std::abs(JTJ[i][i]) < 1e-10) {
                JTJ[i][i] = 1e-10;
            }

            for (int j = i + 1; j < 6; ++j) {
                double factor = JTJ[j][i] / JTJ[i][i];
                gradient[j] -= factor * gradient[i];
                for (int k = i; k < 6; ++k) {
                    JTJ[j][k] -= factor * JTJ[i][k];
                }
            }
        }

        for (int i = 5; i >= 0; --i) {
            double sum = 0;
            for (int j = i + 1; j < 6; ++j) {
                sum += JTJ[i][j] * dx[j];
            }
            dx[i] = (-gradient[i] - sum) / JTJ[i][i];
        }

        // 修改步长限制
        double maxStep = 1000.0;  // 增加最大步长
        double dxNorm = 0;
        for (int j = 0; j < 6; ++j) {
            dxNorm += dx[j] * dx[j];
        }
        dxNorm = std::sqrt(dxNorm);
        if (dxNorm > maxStep) {
            double scale = maxStep / dxNorm;
            for (int j = 0; j < 6; ++j) {
                dx[j] *= scale;
            }
        }

        // 打印参数更新信息
        std::cout << "Parameter updates: ";
        for (int j = 0; j < 6; ++j) {
            std::cout << dx[j] << " ";
        }
        std::cout << std::endl;

        // 线搜索
        double alpha = 1.0;  // 初始步长
        const double alphaFactor = 0.5;  // 步长衰减因子
        const int maxLineSearch = 10;    // 最大线搜索次数
        bool foundBetter = false;

        for (int ls = 0; ls < maxLineSearch; ++ls) {
            // 计算新参数
            std::vector<double> x_new = x;
            for (int j = 0; j < 6; ++j) {
                x_new[j] += alpha * dx[j];
            }

            // 计算新参数下的误差
            double newError = 0;
            row = 0;
            for (size_t i = 0; i < m_devices.size(); ++i) {
                for (size_t j = 0; j < timePoints.size(); ++j) {
                    double time = timePoints[j];
                    Vector3 targetPos(x_new[0] + x_new[3] * time,
                                    x_new[1] + x_new[4] * time,
                                    x_new[2] + x_new[5] * time);
                    Vector3 targetVel(x_new[3], x_new[4], x_new[5]);

                    COORD3 devicePos0 = lbh2xyz(m_devices[i].getLongitude(), m_devices[i].getLatitude(), m_devices[i].getAltitude());
                    COORD3 deviceVel0 = velocity_lbh2xyz(m_devices[i].getLongitude(), m_devices[i].getLatitude(),
                        m_devices[i].getMovementSpeed(), m_devices[i].getMovementAzimuth(), m_devices[i].getMovementElevation());
                    
                    COORD3 devicePos;
                    devicePos.p1 = devicePos0.p1 + deviceVel0.p1 * time;
                    devicePos.p2 = devicePos0.p2 + deviceVel0.p2 * time;
                    devicePos.p3 = devicePos0.p3 + deviceVel0.p3 * time;

                    Vector3 observerPos(devicePos.p1, devicePos.p2, devicePos.p3);
                    Vector3 observerVel(deviceVel0.p1, deviceVel0.p2, deviceVel0.p3);

                    Vector3 r = observerPos - targetPos;
                    double distance = r.magnitude();
                    if (distance < 1e-10) {
                        distance = 1e-10;
                    }

                    Vector3 relativeVelocity = observerVel - targetVel;
                    double radialVelocity = relativeVelocity.dot(r.normalize());
                    double predictedFreqDiff = (radialVelocity / Constants::c) * signalFrequency / maxFreqDiff;
                    double residual = measuredFreqDiffs[i][j] - predictedFreqDiff;
                    newError += residual * residual;
                }
            }
            newError = std::sqrt(newError);

            if (newError < totalError) {
                x = x_new;
                lambda = std::max(lambda / lambdaFactor, minLambda);
                std::cout << "Accepted update with alpha = " << alpha << ", new error: " << newError << std::endl;
                foundBetter = true;
                break;
            }

            alpha *= alphaFactor;
        }

        if (!foundBetter) {
            lambda = std::min(lambda * lambdaFactor, maxLambda);
            std::cout << "No better solution found in line search" << std::endl;
        }

        // 检查参数更新是否过小
        double xNorm = 0;
        for (int j = 0; j < 6; ++j) {
            xNorm += x[j] * x[j];
        }
        xNorm = std::sqrt(xNorm);
        if (dxNorm < 1e-8 * xNorm && iter > 0) {  // 放宽收敛条件
            std::cout << "No significant parameter update. Stopping." << std::endl;
            break;
        }
    }

    if (!hasConverged) {
        std::cerr << "Warning: Algorithm did not converge within maximum iterations." << std::endl;
    }

    // 将求解结果转换为大地坐标
    COORD3 resultLBH = xyz2lbh(x[0], x[1], x[2]);
    // 将全局坐标系速度转换到东北天局部坐标系
    Vector3 targetVel(x[3], x[4], x[5]);
    COORD3 localVel = velocity_xyz2lbh(
        resultLBH.p1, resultLBH.p2,  // 当前经纬度
        targetVel.x, targetVel.y, targetVel.z
    );

    // 更新定位结果
    m_result.longitude = resultLBH.p1;
    m_result.latitude = resultLBH.p2;
    m_result.altitude = resultLBH.p3;
    m_result.velocity = std::sqrt(x[3] * x[3] + x[4] * x[4] + x[5] * x[5]);
    m_result.azimuth = azimuth;
    m_result.elevation = elevation;
    m_result.locationTime = m_simulationTime;
    m_result.distance = std::sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    m_result.accuracy = 50.0;  // 定位精度（米）

    return true;
}