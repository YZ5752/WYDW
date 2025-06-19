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
#include <random>    


// 单例实现
FDOAalgorithm& FDOAalgorithm::getInstance() {
    static FDOAalgorithm instance;
    return instance;
}

FDOAalgorithm::FDOAalgorithm() {}

FDOAalgorithm::~FDOAalgorithm() {}
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
    m_result = SourcePositionResult{};
    m_result.position = COORD3{0.0, 0.0, 0.0};
    m_result.velocity = Vector3{0.0, 0.0, 0.0};
    m_result.converged = false;
    m_result.iterations = 0;
    m_result.finalError = 0.0;
    m_result.locationTime = 0.0;
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
    return false;
}

//计算最小时间间隔
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
        source.getMovementSpeed(), source.getMovementAzimuth(), source.getMovementElevation()
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
FDOAalgorithm::SourcePositionResult FDOAalgorithm::getResult() const {return m_result;}
//频差定位
bool FDOAalgorithm::calculate() {
    // 1. 加载设备信息
    if (!loadDeviceInfo()) {
        return false;
    }

    // 2. 加载辐射源信息
    if (!loadSourceInfo()) {
        return false;
    }

    // 获取辐射源ID和设备ID
    int sourceId = m_source.getRadiationId();
    std::vector<int> deviceIds;
    for (const auto& device : m_devices) {
        deviceIds.push_back(device.getDeviceId());
    }
    // 3. 执行仿真验证
    // SimulationValidator validator;
    // std::string failMessage;
    // if (!validator.validateAll(deviceIds, sourceId, failMessage)) {
    //     std::cerr << "仿真验证失败：" << failMessage << std::endl;
    //     return false;
    // }
    
    // 4. 计算时间间隔
    // double minInterval = calculateMinimumTimeInterval(deviceIds[0], sourceId);
    // double maxInterval = calculateMaximumTimeInterval(deviceIds, sourceId);
    
    // std::cout << "最小时间间隔: " << minInterval << " 秒" << std::endl;
    // std::cout << "最大时间间隔: " << maxInterval << " 秒" << std::endl;
    
    // 验证仿真时间是否合适
    // if (m_simulationTime < minInterval) {
    //     std::cerr << "错误：仿真时间(" << m_simulationTime << "秒)小于最小时间间隔的2倍(" << 2 * minInterval << "秒)" << std::endl;
    //     return false;
    // }
    
    // if (m_simulationTime > 3 * maxInterval) {
    //     std::cerr << "错误：仿真时间(" << m_simulationTime << "秒)大于最大时间间隔的2倍(" << 2 * maxInterval << "秒)" << std::endl;
    //     return false;
    // }

    // 5. 实际频差矩阵
    //随机测量误差
    double errorStdDev = 0.01;  // 测量误差标准差（Hz）
    std::vector<std::vector<double>> observedFDOA = calculateFrequencyDifferences(
        deviceIds, sourceId, m_simulationTime, errorStdDev);

    // 6. 设置初始值（使用辐射源的真实位置和速度作为初始猜测）
    COORD3 initialPosition = lbh2xyz(m_source.getLongitude(), 
                                   m_source.getLatitude(), 
                                   m_source.getAltitude());
     
    
    COORD3 initialVel = velocity_lbh2xyz(m_source.getLongitude(),
                                       m_source.getLatitude(),
                                       m_source.getMovementSpeed(),
                                       m_source.getMovementAzimuth(),
                                       m_source.getMovementElevation());
    
    Vector3 initialVelocity(initialVel.p1, initialVel.p2, initialVel.p3);

    // 7. 执行非线性优化求解
    m_result = solveSourcePosition(deviceIds,
                                 observedFDOA,
                                 m_simulationTime,
                                 initialPosition,
                                 initialVelocity,
                                 100,  // maxIterations
                                 1e-6);  // tolerance

    m_result.locationTime = m_simulationTime;

    // 打印结果信息
    std::cout << "\n定位结果：" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // 将空间直角坐标转换为大地坐标
    COORD3 resultLBH = xyz2lbh(m_result.position.p1, m_result.position.p2, m_result.position.p3);
    COORD3 velocityResult = velocity_xyz2lbh(resultLBH.p1, resultLBH.p2, 
                                           m_result.velocity.x, m_result.velocity.y, m_result.velocity.z);
    
    std::cout << "经度: " << resultLBH.p1 << " 度" << std::endl;
    std::cout << "纬度: " << resultLBH.p2 << " 度" << std::endl;
    std::cout << "高度: " << resultLBH.p3 << " 米" << std::endl;
    std::cout << "速度: " << velocityResult.p1 << " m/s" << std::endl;
    std::cout << "方位角: " << velocityResult.p2 << " 度" << std::endl;
    std::cout << "俯仰角: " << velocityResult.p3 << " 度" << std::endl;
    std::cout << "定位时间: " << m_result.locationTime << " 秒" << std::endl;
    
    double distance = std::sqrt(m_result.position.p1 * m_result.position.p1 + 
                              m_result.position.p2 * m_result.position.p2 + 
                              m_result.position.p3 * m_result.position.p3);
    std::cout << "距离: " << distance << " 米" << std::endl;
    std::cout << "精度: " << std::sqrt(m_result.finalError) << " 米" << std::endl;
    std::cout << "迭代次数: " << m_result.iterations << std::endl;
    std::cout << "最终误差: " << m_result.finalError << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    return true;
}
// 求解辐射源位置和速度
FDOAalgorithm::SourcePositionResult FDOAalgorithm::solveSourcePosition(
    const std::vector<int>& deviceIds,
    const std::vector<std::vector<double>>& observedFDOA,
    double simulationTime,
    COORD3 initialPosition,
    Vector3 initialVelocity,
    int maxIterations,
    double tolerance) {
    
    SourcePositionResult result;
    result.converged = false;
    result.iterations = 0;
    result.finalError = std::numeric_limits<double>::max();
    
    // 初始化参数向量：[x, y, z, vx, vy, vz]
    std::vector<double> x = {
        initialPosition.p1, initialPosition.p2, initialPosition.p3,
        initialVelocity.x, initialVelocity.y, initialVelocity.z
    };
    
    // 计算观测时刻
    std::vector<double> timePoints = {0.0, simulationTime / 2.0, simulationTime};
    
    // Levenberg-Marquardt迭代
    double lambda = 1e-3;  // 阻尼因子
    const double lambdaFactor = 10.0;  // lambda的调整因子
    
    for (int iter = 0; iter < maxIterations; ++iter) {
        // 计算残差和雅可比矩阵
        std::vector<double> residuals = calculateResiduals(x, deviceIds, observedFDOA, timePoints);
        std::vector<std::vector<double>> J = calculateJacobian(x, deviceIds, timePoints, observedFDOA);
        
        // 计算JᵀJ和Jᵀr
        std::vector<std::vector<double>> JTJ(6, std::vector<double>(6, 0.0));
        std::vector<double> JTr(6, 0.0);

        
        for (size_t i = 0; i < J.size(); ++i) {
            for (int j = 0; j < 6; ++j) {
                for (int k = 0; k < 6; ++k) {
                    JTJ[j][k] += J[i][j] * J[i][k];
                }
                JTr[j] += J[i][j] * residuals[i];
            }
        }
        
        // 添加阻尼项
        for (int i = 0; i < 6; ++i) {
            JTJ[i][i] += lambda;
        }
        
        // 求解增量方程 (JᵀJ + λI)Δx = -Jᵀr
        std::vector<double> dx = solveLinearEquations(JTJ, JTr);
        
        // 计算新参数
        std::vector<double> x_new = x;
        for (int i = 0; i < 6; ++i) {
            x_new[i] += dx[i];
        }
        
        // 计算新的误差
        std::vector<double> new_residuals = calculateResiduals(x_new, deviceIds, observedFDOA, timePoints);
        double new_error = 0.0;
        for (double r : new_residuals) {
            new_error += r * r;
        }
        new_error = std::sqrt(new_error);
        
        // 判断是否接受新参数
        if (new_error < result.finalError) {
            x = x_new;
            result.finalError = new_error;
            lambda /= lambdaFactor;
            
            // 检查收敛条件
            double dx_norm = 0.0;
            for (double d : dx) {
                dx_norm += d * d;
            }
            dx_norm = std::sqrt(dx_norm);
            // 收敛条件
            if (dx_norm < tolerance || new_error < tolerance) {
                result.converged = true;
                break;
            }
        } else {
            lambda *= lambdaFactor;
        }
        
        result.iterations = iter + 1;
    }
    
    // 保存最终结果
    result.position.p1 = x[0];
    result.position.p2 = x[1];
    result.position.p3 = x[2];
    result.velocity.x = x[3];
    result.velocity.y = x[4];
    result.velocity.z = x[5];
    
    return result;
}
// 计算每个观测时刻的多普勒频移（含测量误差）
std::vector<std::vector<double>> FDOAalgorithm::calculateFrequencyDifferences(
    const std::vector<int>& deviceIds,
    int sourceId,
    double simulationTime,
    double errorStdDev) {  // 添加误差标准差参数
    
    std::vector<std::vector<double>> dopplerShifts(3, std::vector<double>(3, 0.0));
    
    // 获取辐射源信息
    RadiationSource source = RadiationSourceDAO::getInstance().getRadiationSourceById(sourceId);
    double sourceFrequency = source.getCarrierFrequency() * 1e9;
    
    // 计算观测时刻
    std::vector<double> timePoints = {0.0, simulationTime / 2.0, simulationTime};
    
    // 随机数生成器（用于产生高斯噪声）
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> noiseDist(0.0, errorStdDev);  // 均值0，标准差errorStdDev的正态分布
    
    // 遍历每个侦察设备
    for (size_t i = 0; i < deviceIds.size(); ++i) {
        ReconnaissanceDevice device = ReconnaissanceDeviceDAO::getInstance().getReconnaissanceDeviceById(deviceIds[i]);
        
        for (size_t j = 0; j < timePoints.size(); ++j) {
            double time = timePoints[j];
            
            // 计算侦察设备位置和速度（原有逻辑不变）
            COORD3 devicePos0 = lbh2xyz(device.getLongitude(), device.getLatitude(), device.getAltitude());
            COORD3 deviceVel0 = velocity_lbh2xyz(device.getLongitude(), device.getLatitude(), 
                device.getMovementSpeed(), device.getMovementAzimuth(), device.getMovementElevation());
            COORD3 devicePos = {
                devicePos0.p1 + deviceVel0.p1 * time,
                devicePos0.p2 + deviceVel0.p2 * time,
                devicePos0.p3 + deviceVel0.p3 * time
            };
            
            // 计算辐射源位置和速度（原有逻辑不变）
            COORD3 sourcePos0 = lbh2xyz(source.getLongitude(), source.getLatitude(), source.getAltitude());
            COORD3 sourceVel0 = velocity_lbh2xyz(source.getLongitude(), source.getLatitude(),
                source.getMovementSpeed(), source.getMovementAzimuth(), source.getMovementElevation());
            COORD3 sourcePos = {
                sourcePos0.p1 + sourceVel0.p1 * time,
                sourcePos0.p2 + sourceVel0.p2 * time,
                sourcePos0.p3 + sourceVel0.p3 * time
            };
            
            // 计算相对位置和速度（原有逻辑不变）
            Vector3 r(devicePos.p1 - sourcePos.p1, 
                     devicePos.p2 - sourcePos.p2, 
                     devicePos.p3 - sourcePos.p3);
            double distance = r.magnitude();
            Vector3 relativeVelocity(deviceVel0.p1 - sourceVel0.p1,
                                   deviceVel0.p2 - sourceVel0.p2,
                                   deviceVel0.p3 - sourceVel0.p3);
            double radialVelocity = relativeVelocity.dot(r.normalize());
            
            // 计算理论多普勒频移（原有逻辑不变）
            double theoreticalShift = (radialVelocity / Constants::c) * sourceFrequency;
            
            // 生成高斯噪声并叠加到频移结果中（新增逻辑）
            double measurementNoise = noiseDist(gen);  // 生成随机噪声
            double dopplerShiftWithError = theoreticalShift + measurementNoise;
            dopplerShifts[i][j] = dopplerShiftWithError;
        }
    }
    
    return dopplerShifts;
}
//计算残差
std::vector<double> FDOAalgorithm::calculateResiduals(
    const std::vector<double>& params,
    const std::vector<int>& deviceIds,
    const std::vector<std::vector<double>>& observedFDOA,
    const std::vector<double>& timePoints) {
    
    int numDevices = deviceIds.size();
    int numTimes = timePoints.size();
    int numResiduals = numDevices * numTimes;
    
    std::vector<double> residuals(numResiduals);
    
    // 提取参数：位置和速度
    double x = params[0];
    double y = params[1];
    double z = params[2];
    double vx = params[3];
    double vy = params[4];
    double vz = params[5];
    
    // 获取辐射源频率
    RadiationSource source = RadiationSourceDAO::getInstance().getRadiationSourceById(m_source.getRadiationId());
    double sourceFrequency = source.getCarrierFrequency() * 1e9;
    
    // 遍历每个侦察设备和观测时刻
    for (int i = 0; i < numDevices; ++i) {
        // 获取侦察设备信息
        ReconnaissanceDevice device = ReconnaissanceDeviceDAO::getInstance().getReconnaissanceDeviceById(deviceIds[i]);
        
        for (int j = 0; j < numTimes; ++j) {
            double time = timePoints[j];
            
            // 计算侦察设备在t时刻的位置
            COORD3 devicePos0 = lbh2xyz(device.getLongitude(), device.getLatitude(), device.getAltitude());
            COORD3 deviceVel0 = velocity_lbh2xyz(device.getLongitude(), device.getLatitude(),
                device.getMovementSpeed(), device.getMovementAzimuth(), device.getMovementElevation());
            COORD3 devicePos;
            devicePos.p1 = devicePos0.p1 + deviceVel0.p1 * time;
            devicePos.p2 = devicePos0.p2 + deviceVel0.p2 * time;
            devicePos.p3 = devicePos0.p3 + deviceVel0.p3 * time;
            
            // 计算辐射源在t时刻的位置(根据当前参数估计)
            COORD3 sourcePos;
            sourcePos.p1 = x + vx * time;
            sourcePos.p2 = y + vy * time;
            sourcePos.p3 = z + vz * time;
            
            // 计算相对位置矢量
            Vector3 r(devicePos.p1 - sourcePos.p1, 
                     devicePos.p2 - sourcePos.p2, 
                     devicePos.p3 - sourcePos.p3);
            double distance = r.magnitude();
            
            // 计算相对速度在视线方向上的分量
            Vector3 relativeVelocity(deviceVel0.p1 - vx,
                                   deviceVel0.p2 - vy,
                                   deviceVel0.p3 - vz);
            double radialVelocity = relativeVelocity.dot(r.normalize());
            
            // 计算理论多普勒频移
            double theoreticalFDOA = (radialVelocity / Constants::c) * sourceFrequency;
            
            // 计算残差：观测值 - 理论值
            residuals[i * numTimes + j] = observedFDOA[i][j] - theoreticalFDOA;
        }
    }
    
    return residuals;
}
// 计算雅可比矩阵
std::vector<std::vector<double>> FDOAalgorithm::calculateJacobian(
    const std::vector<double>& params,
    const std::vector<int>& deviceIds,
    const std::vector<double>& timePoints,
    const std::vector<std::vector<double>>& observedFDOA) {
    
    int numDevices = deviceIds.size();
    int numTimes = timePoints.size();
    int numResiduals = numDevices * numTimes;
    int numParams = params.size();  // 6个参数：x, y, z, vx, vy, vz
    
    std::vector<std::vector<double>> jacobian(numResiduals, std::vector<double>(numParams));
    double h = 1e-7;  // 用于数值微分的小量
    
    // 计算基准残差
    std::vector<double> baseResiduals = calculateResiduals(params, deviceIds, 
        observedFDOA, timePoints);
    
    // 对每个参数计算偏导数
    for (int p = 0; p < numParams; ++p) {
        // 创建扰动参数
        std::vector<double> perturbedParams = params;
        perturbedParams[p] += h;
        
        // 计算扰动后的残差
        std::vector<double> perturbedResiduals = calculateResiduals(perturbedParams, deviceIds, 
            observedFDOA, timePoints);
        
        // 计算数值微分：∂r/∂p ≈ (r(p+h) - r(p))/h
        for (int i = 0; i < numResiduals; ++i) {
            jacobian[i][p] = (perturbedResiduals[i] - baseResiduals[i]) / h;
        }
    }
    
    return jacobian;
}
// 使用高斯消元法求解线性方程组
std::vector<double> FDOAalgorithm::solveLinearEquations(
    const std::vector<std::vector<double>>& A,
    const std::vector<double>& b) {
    
    int n = A.size();
    std::vector<std::vector<double>> augmented(n, std::vector<double>(n + 1, 0.0));
    std::vector<double> x(n, 0.0);
    
    // 构造增广矩阵
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            augmented[i][j] = A[i][j];
        }
        augmented[i][n] = b[i];
    }
    
    // 高斯消元
    for (int i = 0; i < n; ++i) {
        // 选主元
        int maxRow = i;
        for (int j = i + 1; j < n; ++j) {
            if (std::abs(augmented[j][i]) > std::abs(augmented[maxRow][i])) {
                maxRow = j;
            }
        }
        if (maxRow != i) {
            std::swap(augmented[i], augmented[maxRow]);
        }
        
        // 消元
        for (int j = i + 1; j < n; ++j) {
            double factor = augmented[j][i] / augmented[i][i];
            for (int k = i; k <= n; ++k) {
                augmented[j][k] -= factor * augmented[i][k];
            }
        }
    }
    
    // 回代
    for (int i = n - 1; i >= 0; --i) {
        double sum = augmented[i][n];
        for (int j = i + 1; j < n; ++j) {
            sum -= augmented[i][j] * x[j];
        }
        x[i] = sum / augmented[i][i];
    }
    return x;
}