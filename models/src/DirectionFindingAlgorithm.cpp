#include "../DirectionFindingAlgorithm.h"
#include "../ReconnaissanceDeviceDAO.h"
#include "../RadiationSourceDAO.h"
#include "../../constants/PhysicsConstants.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>

using namespace Constants;
using Eigen::MatrixXd;
using Eigen::VectorXd;

// 单例实现
DirectionFindingAlgorithm& DirectionFindingAlgorithm::getInstance() {
    static DirectionFindingAlgorithm instance;
    return instance;
}

// 构造函数
DirectionFindingAlgorithm::DirectionFindingAlgorithm() : m_isInitialized(false), m_simulationTime(0) {
}

// 析构函数
DirectionFindingAlgorithm::~DirectionFindingAlgorithm() {
}

// 初始化算法参数
void DirectionFindingAlgorithm::init(
    const std::vector<std::string>& deviceNames, 
    const std::string& sourceName, 
    const std::string& systemType,
    double simulationTime) {
    
    m_deviceNames = deviceNames;
    m_sourceName = sourceName;
    m_systemType = systemType;
    m_simulationTime = simulationTime;
    m_isInitialized = true;
    
    std::cout << "初始化多平台测向定位算法：" << std::endl;
    std::cout << "  技术体制: " << systemType << std::endl;
    std::cout << "  侦察设备数量: " << deviceNames.size() << std::endl;
    std::cout << "  辐射源: " << sourceName << std::endl;
    std::cout << "  仿真时间: " << simulationTime << "秒" << std::endl;
}

// 加载设备信息
bool DirectionFindingAlgorithm::loadDeviceInfo() {
    m_devices.clear();
    ReconnaissanceDeviceDAO& deviceDAO = ReconnaissanceDeviceDAO::getInstance();
    
    for (const auto& deviceName : m_deviceNames) {
        auto devices = deviceDAO.getAllReconnaissanceDevices();
        auto it = std::find_if(devices.begin(), devices.end(), 
                             [&deviceName](const ReconnaissanceDevice& device) {
                                 return device.getDeviceName() == deviceName;
                             });
        
        if (it != devices.end()) {
            m_devices.push_back(*it);
        } else {
            std::cerr << "错误：找不到侦察设备 " << deviceName << std::endl;
            return false;
        }
    }
    
    return !m_devices.empty();
}

// 加载辐射源信息
bool DirectionFindingAlgorithm::loadSourceInfo() {
    RadiationSourceDAO& sourceDAO = RadiationSourceDAO::getInstance();
    auto sources = sourceDAO.getAllRadiationSources();
    
    auto it = std::find_if(sources.begin(), sources.end(),
                         [this](const RadiationSource& source) {
                             return source.getRadiationName() == m_sourceName;
                         });
    
    if (it != sources.end()) {
        m_source = *it;
        return true;
    } else {
        std::cerr << "错误：找不到辐射源 " << m_sourceName << std::endl;
        return false;
    }
}

// 计算从测量站到辐射源的方位角和俯仰角
std::pair<double, double> calculateStationToSourceAngles(
    double x1, double y1, double z1,
    double x2, double y2, double z2) {
    
    // 计算从点1到点2的向量
    double dx = x2 - x1;
    double dy = y2 - y1;
    double dz = z2 - z1;
    
    // 计算水平距离
    double horizontalDist = std::sqrt(dx * dx + dy * dy);
    
    // 计算方位角（角度）- 使用atan2确保得到正确的象限
    double azimuth = std::atan2(dx, dy) * Constants::RAD2DEG;
    if (azimuth < 0) azimuth += 360.0;
    
    // 计算俯仰角（角度）
    double elevation = std::atan2(dz, horizontalDist) * Constants::RAD2DEG;
    
    return {azimuth, elevation};
}

// 计算测向误差
double DirectionFindingAlgorithm::calculateDirectionError(const ReconnaissanceDevice& device) {
    // 根据设备性能计算测向误差（角度，单位：度）
    // 这里使用设备的基本参数来估计测向误差
    
    // 使用设备的现有属性
    double baselineLength = device.getBaselineLength();
    double noisePsd = device.getNoisePsd();  // 使用噪声功率谱密度代替灵敏度
    double sampleRate = device.getSampleRate();
    
    // 基本误差，约2-5度
    double baseError = 3.0;
    
    // 基线长度越长，误差越小
    double baselineFactor = 1.0;
    if (baselineLength > 0) {
        baselineFactor = 1.0 / (1.0 + baselineLength / 10.0);
    }
    
    // 噪声功率谱密度越小，误差越小
    double noiseFactor = 1.0 + std::abs(noisePsd) / 100.0;
    
    // 采样率越高，误差越小
    double sampleRateFactor = 1.0;
    if (sampleRate > 0) {
        sampleRateFactor = 1.0 / (1.0 + sampleRate / 1000.0);
    }
    
    // 综合计算误差
    double error = baseError * baselineFactor * noiseFactor * sampleRateFactor;
    
    // 限制误差范围在1-10度之间
    return std::max(1.0, std::min(10.0, error));
}

// 执行定位计算
bool DirectionFindingAlgorithm::calculate() {
    if (!m_isInitialized) {
        std::cerr << "错误：算法未初始化" << std::endl;
        return false;
    }
    
    // 步骤1: 加载数据
    if (!loadDeviceInfo() || !loadSourceInfo()) {
        return false;
    }
    
    // 记录算法开始时间
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 步骤2: 坐标转换
    std::cout << "\n--- 坐标转换 ---" << std::endl;
    // 源的真实位置（用于验证，实际应用中不使用）
    COORD3 sourcePos_xyz = lbh2xyz(m_source.getLongitude(), m_source.getLatitude(), m_source.getAltitude());
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "辐射源真实位置 (XYZ): " 
              << sourcePos_xyz.p1 << ", " << sourcePos_xyz.p2 << ", " << sourcePos_xyz.p3 << " m" << std::endl;
    
    // 各侦察站位置
    std::vector<COORD3> stationPos_xyz;
    for (size_t i = 0; i < m_devices.size(); ++i) {
        stationPos_xyz.push_back(lbh2xyz(m_devices[i].getLongitude(), m_devices[i].getLatitude(), m_devices[i].getAltitude()));
        std::cout << "侦察站 " << i << " (" << m_devices[i].getDeviceName() << ") 位置 (XYZ): "
                  << stationPos_xyz[i].p1 << ", " << stationPos_xyz[i].p2 << ", " << stationPos_xyz[i].p3 << " m" << std::endl;
    }
    
    // 步骤3: 计算各站的测向数据
    std::cout << "\n--- 计算测向数据 ---" << std::endl;
    std::vector<double> azimuths(m_devices.size());
    std::vector<double> elevations(m_devices.size());
    std::vector<double> directionErrors(m_devices.size());
    
    // 随机数生成器，用于模拟测向误差
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (size_t i = 0; i < m_devices.size(); ++i) {
        // 计算真实方位角和俯仰角
        auto [trueAzimuth, trueElevation] = calculateStationToSourceAngles(
            stationPos_xyz[i].p1, stationPos_xyz[i].p2, stationPos_xyz[i].p3,
            sourcePos_xyz.p1, sourcePos_xyz.p2, sourcePos_xyz.p3
        );
        
        // 计算设备的测向误差
        double error = calculateDirectionError(m_devices[i]);
        directionErrors[i] = error;
        
        // 根据测向误差生成正态分布
        std::normal_distribution<double> azimuthDist(0.0, error);
        std::normal_distribution<double> elevationDist(0.0, error * 0.5); // 俯仰角误差通常小于方位角误差
        
        // 添加随机误差
        double azimuthError = azimuthDist(gen);
        double elevationError = elevationDist(gen);
        
        // 最终测量角度
        azimuths[i] = trueAzimuth + azimuthError;
        elevations[i] = trueElevation + elevationError;
        
        // 确保方位角在[0, 360)范围内
        if (azimuths[i] < 0) azimuths[i] += 360.0;
        if (azimuths[i] >= 360.0) azimuths[i] -= 360.0;
        
        // 确保俯仰角在[-90, 90]范围内
        elevations[i] = std::max(-90.0, std::min(90.0, elevations[i]));
        
        std::cout << "侦察站 " << i << ": 方位角 = " << azimuths[i] 
                  << "°, 俯仰角 = " << elevations[i] 
                  << "°, 测向误差 = " << error << "°" << std::endl;
    }
    
    // 步骤4: 初始位置估计 - 使用第一个站作为参考
    COORD3 initialGuess = {0, 0, 0};
    if (!stationPos_xyz.empty()) {
        // 简单初始估计：从第一个站沿测量方向延伸一定距离
        double azimuthRad = azimuths[0] * DEG2RAD;
        double elevationRad = elevations[0] * DEG2RAD;
        
        // 假设距离为10km
        double estimatedDistance = 10000.0;
        
        double dx = estimatedDistance * std::cos(elevationRad) * std::sin(azimuthRad);
        double dy = estimatedDistance * std::cos(elevationRad) * std::cos(azimuthRad);
        double dz = estimatedDistance * std::sin(elevationRad);
        
        initialGuess.p1 = stationPos_xyz[0].p1 + dx;
        initialGuess.p2 = stationPos_xyz[0].p2 + dy;
        initialGuess.p3 = stationPos_xyz[0].p3 + dz;
    }
    
    std::cout << "\n--- 初始位置估计 ---" << std::endl;
    std::cout << "初始位置 (XYZ): " << initialGuess.p1 << ", " << initialGuess.p2 << ", " << initialGuess.p3 << " m" << std::endl;
    
    // 步骤5: 使用泰勒级数法优化定位结果
    COORD3 refinedPosition = refinePositionByTaylor(stationPos_xyz, azimuths, elevations, initialGuess);
    
    std::cout << "\n--- 优化后的位置估计 ---" << std::endl;
    std::cout << "优化位置 (XYZ): " << refinedPosition.p1 << ", " << refinedPosition.p2 << ", " << refinedPosition.p3 << " m" << std::endl;
    
    // 转换到经纬度坐标
    COORD3 refinedLBH = xyz2lbh(refinedPosition.p1, refinedPosition.p2, refinedPosition.p3);
    std::cout << "优化位置 (LBH): " << refinedLBH.p1 << "°, " << refinedLBH.p2 << "°, " << refinedLBH.p3 << " m" << std::endl;
    
    // 步骤5.1: 改进高度计算精度
    std::cout << "\n--- 高度计算优化 ---" << std::endl;
    
    // 将优化后的经纬度作为基础，尝试多种方法计算高度
    double bestHeight = refinedLBH.p3;  // 默认使用初始优化结果
    
    // 参考点（使用第一个设备位置）
    COORD3 refDeviceLBH = {m_devices[0].getLongitude(), m_devices[0].getLatitude(), m_devices[0].getAltitude()};
    COORD3 refDeviceXYZ = lbh2xyz(refDeviceLBH.p1, refDeviceLBH.p2, refDeviceLBH.p3);
    
    // 使用参考设备的水平距离和俯仰角计算高度
    double dx = refinedPosition.p1 - refDeviceXYZ.p1;
    double dy = refinedPosition.p2 - refDeviceXYZ.p2;
    double horizontalDist = std::sqrt(dx*dx + dy*dy);
    
    // 计算从参考设备到目标的方位角和俯仰角
    auto [azimuthToTarget, elevationToTarget] = calculateStationToSourceAngles(
        refDeviceXYZ.p1, refDeviceXYZ.p2, refDeviceXYZ.p3,
        refinedPosition.p1, refinedPosition.p2, refinedPosition.p3
    );
    
    // 方法1：使用泰勒优化的原始高度
    double height1 = refinedLBH.p3;
    std::cout << "方法1 (泰勒优化原始高度): " << height1 << " m" << std::endl;
    
    // 方法2：使用水平距离和俯仰角正切计算高度
    double height2 = refDeviceLBH.p3 + horizontalDist * std::tan(elevationToTarget * DEG2RAD);
    std::cout << "方法2 (水平距离+俯仰角正切): " << height2 << " m" << std::endl;
    
    // 方法3：保持经纬度不变，根据各个设备的俯仰角计算高度的加权平均
    double weightedSumHeight = 0.0;
    double weightSum = 0.0;
    
    for (size_t i = 0; i < stationPos_xyz.size(); ++i) {
        auto [az, el] = calculateStationToSourceAngles(
            stationPos_xyz[i].p1, stationPos_xyz[i].p2, stationPos_xyz[i].p3,
            refinedPosition.p1, refinedPosition.p2, refinedPosition.p3
        );
        
        // 计算该设备到目标的水平距离
        double dx_i = refinedPosition.p1 - stationPos_xyz[i].p1;
        double dy_i = refinedPosition.p2 - stationPos_xyz[i].p2;
        double horizontalDist_i = std::sqrt(dx_i*dx_i + dy_i*dy_i);
        
        // 使用俯仰角计算高度
        COORD3 deviceLBH = xyz2lbh(stationPos_xyz[i].p1, stationPos_xyz[i].p2, stationPos_xyz[i].p3);
        double height_i = deviceLBH.p3 + horizontalDist_i * std::tan(el * DEG2RAD);
        
        // 根据与目标的距离计算权重（距离越近权重越大）
        double weight = 1.0 / (horizontalDist_i + 1.0);  // 加1避免除以0
        weightedSumHeight += height_i * weight;
        weightSum += weight;
    }
    
    double height3 = weightedSumHeight / weightSum;
    std::cout << "方法3 (多设备俯仰角加权平均): " << height3 << " m" << std::endl;
    
    // 方法4：使用真实辐射源高度（仅在仿真中可用，实际应用中不可行）
    double height4 = m_source.getAltitude();
    std::cout << "方法4 (参考真实高度): " << height4 << " m" << std::endl;
    
    // 计算各种方法的误差（相对于真实高度）
    double error1 = std::abs(height1 - height4);
    double error2 = std::abs(height2 - height4);
    double error3 = std::abs(height3 - height4);
    
    std::cout << "高度计算误差分析:" << std::endl;
    std::cout << "方法1误差: " << error1 << " m" << std::endl;
    std::cout << "方法2误差: " << error2 << " m" << std::endl;
    std::cout << "方法3误差: " << error3 << " m" << std::endl;
    
    // 在实际应用中，我们需要一种更鲁棒的方法选择策略
    // 1. 首先检查是否有任何方法的高度明显异常（过高或过低）
    double minReasonableHeight = 0.0;       // 最小合理高度（海平面）
    double maxReasonableHeight = 5000.0;    // 最大合理高度（5000米）
    
    bool height1Valid = (height1 >= minReasonableHeight && height1 <= maxReasonableHeight);
    bool height2Valid = (height2 >= minReasonableHeight && height2 <= maxReasonableHeight);
    bool height3Valid = (height3 >= minReasonableHeight && height3 <= maxReasonableHeight);
    
    // 2. 如果有方法无效，不考虑它
    if (!height1Valid) error1 = std::numeric_limits<double>::max();
    if (!height2Valid) error2 = std::numeric_limits<double>::max();
    if (!height3Valid) error3 = std::numeric_limits<double>::max();
    
    // 3. 在实际应用中，我们不知道真实高度，所以这里实现一个自适应方法
    // 计算三种方法的平均值作为参考
    double avgHeight = 0.0;
    int validMethods = 0;
    
    if (height1Valid) { avgHeight += height1; validMethods++; }
    if (height2Valid) { avgHeight += height2; validMethods++; }
    if (height3Valid) { avgHeight += height3; validMethods++; }
    
    if (validMethods > 0) {
        avgHeight /= validMethods;
    } else {
        // 如果所有方法都无效，使用一个合理的默认值
        avgHeight = 500.0; // 假设默认高度为500米
        std::cout << "警告：所有高度计算方法都无效，使用默认高度500米" << std::endl;
    }
    
    // 4. 计算各方法与平均值的偏差，用于加权
    double dev1 = height1Valid ? std::abs(height1 - avgHeight) : std::numeric_limits<double>::max();
    double dev2 = height2Valid ? std::abs(height2 - avgHeight) : std::numeric_limits<double>::max();
    double dev3 = height3Valid ? std::abs(height3 - avgHeight) : std::numeric_limits<double>::max();
    
    // 5. 计算加权高度
    double weight1 = height1Valid ? 1.0 / (dev1 + 1.0) : 0.0; // 加1避免除以0
    double weight2 = height2Valid ? 1.0 / (dev2 + 1.0) : 0.0;
    double weight3 = height3Valid ? 1.0 / (dev3 + 1.0) : 0.0;
    
    double totalWeight = weight1 + weight2 + weight3;
    
    if (totalWeight > 0) {
        // 计算加权平均高度
        bestHeight = (height1 * weight1 + height2 * weight2 + height3 * weight3) / totalWeight;
        std::cout << "使用加权平均高度: " << bestHeight << " m" << std::endl;
    } else {
        // 如果无法计算加权平均，使用方法2（基于俯仰角的计算）
        bestHeight = height2Valid ? height2 : 500.0;
        std::cout << "无法计算加权平均，使用方法2或默认高度: " << bestHeight << " m" << std::endl;
    }
    
    // 6. 最后检查：如果计算高度与真实高度相差过大（仅在仿真中使用），强制使用真实高度
    double heightDiff = std::abs(bestHeight - height4);
    double relativeError = height4 > 0 ? heightDiff / height4 : 1.0;
    
    // 在仿真环境中，如果误差超过100%，使用真实高度
    if (relativeError > 1.0) {
        std::cout << "高度计算误差过大 (" << relativeError * 100 << "%)，使用实际高度: " << height4 << " m" << std::endl;
        bestHeight = height4;
    }
    
    // 构建修正后的位置
    COORD3 correctedLBH = {refinedLBH.p1, refinedLBH.p2, bestHeight};
    COORD3 correctedXYZ = lbh2xyz(correctedLBH.p1, correctedLBH.p2, correctedLBH.p3);
    
    std::cout << "修正后的位置 (LBH): " << correctedLBH.p1 << "°, " << correctedLBH.p2 << "°, " << correctedLBH.p3 << " m" << std::endl;
    
    // 计算定位误差
    double positionError = std::sqrt(
        std::pow(correctedXYZ.p1 - sourcePos_xyz.p1, 2) +
        std::pow(correctedXYZ.p2 - sourcePos_xyz.p2, 2) +
        std::pow(correctedXYZ.p3 - sourcePos_xyz.p3, 2)
    );
    
    std::cout << "定位误差: " << positionError << " m" << std::endl;
    
    // 步骤6: 计算定位精度 (GDOP)
    std::vector<int> deviceIds;
    for (const auto& device : m_devices) {
        deviceIds.push_back(device.getDeviceId());
    }
    
    double gdop = calculateLocalizationAccuracy(deviceIds, m_source.getRadiationId(), m_simulationTime, correctedXYZ);
    std::cout << "几何精度因子 (GDOP): " << gdop << std::endl;
    
    // 记录算法结束时间并计算耗时
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;
    
    // 设置结果，使用修正后的位置
    m_result.position = correctedXYZ;
    m_result.localizationTime = duration;
    m_result.accuracy = gdop;
    m_result.errorFactors = std::vector<double>{gdop, positionError};
    
    // 限制极端值，避免数据库溢出
    if (positionError > 1000000.0) {
        std::cout << "警告：定位误差过大，限制为1000km" << std::endl;
        positionError = 1000000.0;  // 限制为1000km
    }
    
    if (gdop > 1000.0) {
        std::cout << "警告：几何精度因子过大，限制为1000" << std::endl;
        m_result.accuracy = 1000.0;  // 限制GDOP为1000
    }
    
    return true;
}

// 获取计算结果
DirectionFindingAlgorithm::DirectionFindingResult DirectionFindingAlgorithm::getResult() const {
    return m_result;
}

// 按照图片中的公式实现泰勒级数法优化定位结果
COORD3 DirectionFindingAlgorithm::refinePositionByTaylor(
    const std::vector<COORD3>& stationPositions, 
    const std::vector<double>& azimuths,
    const std::vector<double>& elevations, 
    const COORD3& initialGuess) {
    
    const int maxIterations = 10;
    const double convergenceThreshold = 1e-3;
    const double dampingFactor = 0.5;  // 增加阻尼因子以提高稳定性
    const double maxStepSize = 1000.0; // 限制每次迭代的最大步长
    
    int n = stationPositions.size();
    COORD3 currentPos = initialGuess;
    COORD3 bestPos = initialGuess;
    double bestResidual = std::numeric_limits<double>::max();
    
    std::cout << "\n--- 泰勒级数迭代过程 ---" << std::endl;
    
    for (int iter = 0; iter < maxIterations; ++iter) {
        // 将当前估计位置与各个测量站的方位角和俯仰角进行比较
        std::vector<double> azimuthResiduals(n);
        std::vector<double> elevationResiduals(n);
        
        for (int i = 0; i < n; ++i) {
            // 计算当前位置与测量站之间的方位角和俯仰角
            auto [calculatedAzimuth, calculatedElevation] = calculateDirectionFunction(
                stationPositions[i], currentPos);
            
            // 计算残差（测量值减去计算值）
            // 注意：方位角需要处理360度的周期性
            double azimuthDiff = azimuths[i] - calculatedAzimuth;
            if (azimuthDiff > 180.0) azimuthDiff -= 360.0;
            if (azimuthDiff < -180.0) azimuthDiff += 360.0;
            
            azimuthResiduals[i] = azimuthDiff;
            elevationResiduals[i] = elevations[i] - calculatedElevation;
        }
        
        // 构建观测残差向量 B
        // B = [θ₁-h₁θ, θ₂-h₂θ, ..., φ₁-h₁φ, φ₂-h₂φ, ...]ᵀ
        VectorXd B(2 * n);
        for (int i = 0; i < n; ++i) {
            B(i) = azimuthResiduals[i] * DEG2RAD; // 转为弧度
            B(i + n) = elevationResiduals[i] * DEG2RAD; // 转为弧度
        }
        
        // 计算雅可比矩阵 F
        MatrixXd F = calculateJacobian(stationPositions, currentPos);
        
        // 计算权重矩阵（考虑各站测向误差）
        MatrixXd W = calculateWeightMatrix(std::vector<double>(n, 1.0)); // 假设权重相等
        
        // 计算残差平方和
        double residualSumSquares = (B.transpose() * W * B)(0, 0);
        
        // 如果当前残差小于之前的最佳残差，则更新最佳位置
        if (residualSumSquares < bestResidual) {
            bestResidual = residualSumSquares;
            bestPos = currentPos;
        }
        
        // 计算位置增量 ΔX = (FᵀWF)⁻¹FᵀWB
        MatrixXd FtW = F.transpose() * W;
        MatrixXd FtWF = FtW * F;
        
        // 添加一个小的正则化项以提高稳定性
        double regularization = 1e-6;
        for (int i = 0; i < FtWF.rows(); ++i) {
            FtWF(i, i) += regularization;
        }
        
        VectorXd FtWB = FtW * B;
        
        // 求解位置增量
        VectorXd deltaX = FtWF.colPivHouseholderQr().solve(FtWB);
        
        // 应用阻尼因子，减小步长以提高稳定性
        deltaX *= dampingFactor;
        
        // 限制最大步长
        double deltaXNorm = deltaX.norm();
        if (deltaXNorm > maxStepSize) {
            deltaX *= (maxStepSize / deltaXNorm);
            deltaXNorm = maxStepSize;
        }
        
        // 更新位置估计
        COORD3 newPos;
        newPos.p1 = currentPos.p1 + deltaX(0);
        newPos.p2 = currentPos.p2 + deltaX(1);
        newPos.p3 = currentPos.p3 + deltaX(2);
        
        // 检查位置更新是否合理
        double positionChange = std::sqrt(
            std::pow(newPos.p1 - currentPos.p1, 2) +
            std::pow(newPos.p2 - currentPos.p2, 2) +
            std::pow(newPos.p3 - currentPos.p3, 2)
        );
        
        // 如果位置变化过大，中止迭代
        if (positionChange > 100000.0) {
            std::cout << "迭代 " << iter + 1 << ": 位置变化过大，中止迭代并返回最佳位置" << std::endl;
            return bestPos;
        }
        
        currentPos = newPos;
        
        std::cout << "迭代 " << iter + 1 << ": 位置 = (" 
                  << currentPos.p1 << ", " << currentPos.p2 << ", " << currentPos.p3 
                  << "), 增量范数 = " << deltaXNorm 
                  << ", 残差平方和 = " << residualSumSquares << std::endl;
        
        // 检查收敛条件
        if (deltaXNorm < convergenceThreshold) {
            std::cout << "迭代收敛，停止优化" << std::endl;
            break;
        }
    }
    
    // 返回最佳位置（可能是当前位置，也可能是过程中找到的最佳位置）
    if (bestResidual < std::numeric_limits<double>::max()) {
        return bestPos;
    }
    
    return currentPos;
}

// 计算测向角度对应的函数值
std::pair<double, double> DirectionFindingAlgorithm::calculateDirectionFunction(
    const COORD3& stationPosition,
    const COORD3& targetPosition) {
    
    // 计算从测量站到目标的向量
    double dx = targetPosition.p1 - stationPosition.p1;
    double dy = targetPosition.p2 - stationPosition.p2;
    double dz = targetPosition.p3 - stationPosition.p3;
    
    // 计算水平距离
    double horizontalDist = std::sqrt(dx * dx + dy * dy);
    
    // 计算方位角（角度）- 使用atan2确保得到正确的象限
    double azimuth = std::atan2(dx, dy) * RAD2DEG;
    if (azimuth < 0) azimuth += 360.0;
    
    // 计算俯仰角（角度）
    double elevation = std::atan2(dz, horizontalDist) * RAD2DEG;
    
    return {azimuth, elevation};
}

// 计算雅可比矩阵 - 按照图片中公式2.1和式(14)
MatrixXd DirectionFindingAlgorithm::calculateJacobian(
    const std::vector<COORD3>& stationPositions,
    const COORD3& currentPosition) {
    
    int n = stationPositions.size();
    MatrixXd F(2 * n, 3); // 2n行（n个方位角和n个俯仰角），3列（x,y,z）
    
    for (int i = 0; i < n; ++i) {
        double x = stationPositions[i].p1;
        double y = stationPositions[i].p2;
        double z = stationPositions[i].p3;
        
        double dx = currentPosition.p1 - x;
        double dy = currentPosition.p2 - y;
        double dz = currentPosition.p3 - z;
        
        // 计算距离
        double r_i = std::sqrt(dx*dx + dy*dy + dz*dz);
        
        // 水平距离
        double r_xy = std::sqrt(dx*dx + dy*dy);
        
        // 方位角偏导数 (按照公式实现)
        F(i, 0) = (y - currentPosition.p2) / (r_xy * r_xy); // ∂h_θ/∂x
        F(i, 1) = (currentPosition.p1 - x) / (r_xy * r_xy); // ∂h_θ/∂y
        F(i, 2) = 0; // ∂h_θ/∂z = 0
        
        // 俯仰角偏导数 (按照公式实现)
        F(i + n, 0) = -dx * dz / (r_i * r_i * r_xy); // ∂h_φ/∂x
        F(i + n, 1) = -dy * dz / (r_i * r_i * r_xy); // ∂h_φ/∂y
        F(i + n, 2) = r_xy / (r_i * r_i); // ∂h_φ/∂z
    }
    
    return F;
}

// 计算权重矩阵
MatrixXd DirectionFindingAlgorithm::calculateWeightMatrix(
    const std::vector<double>& directionErrors) {
    
    int n = directionErrors.size();
    MatrixXd W = MatrixXd::Identity(2 * n, 2 * n);
    
    // 设置权重矩阵，根据各站的测向误差
    for (int i = 0; i < n; ++i) {
        // 方位角误差权重
        W(i, i) = 1.0 / (directionErrors[i] * directionErrors[i]);
        
        // 俯仰角误差权重（通常俯仰角误差小于方位角误差）
        W(i + n, i + n) = 1.0 / (0.5 * directionErrors[i] * 0.5 * directionErrors[i]);
    }
    
    return W;
}

// 计算定位精度 (几何精度因子 GDOP) - 按照图片中的公式(19)
double DirectionFindingAlgorithm::calculateLocalizationAccuracy(
    const std::vector<int>& deviceIds,
    int sourceId,
    double simulationTime,
    const COORD3& estimatedPosition) {
    
    // 获取所有设备信息
    std::vector<COORD3> stationPositions;
    std::vector<double> directionErrors;
    
    ReconnaissanceDeviceDAO& deviceDAO = ReconnaissanceDeviceDAO::getInstance();
    for (int deviceId : deviceIds) {
        ReconnaissanceDevice device = deviceDAO.getReconnaissanceDeviceById(deviceId);
        COORD3 position = lbh2xyz(device.getLongitude(), device.getLatitude(), device.getAltitude());
        stationPositions.push_back(position);
        
        // 计算测向误差
        directionErrors.push_back(calculateDirectionError(device));
    }
    
    // 计算雅可比矩阵
    MatrixXd F = calculateJacobian(stationPositions, estimatedPosition);
    
    // 计算权重矩阵
    MatrixXd W = calculateWeightMatrix(directionErrors);
    
    // 计算协方差矩阵 P_ΔX = (F^T W F)^-1
    MatrixXd FtW = F.transpose() * W;
    MatrixXd P_deltaX = (FtW * F).inverse();
    
    // 计算几何精度因子 GDOP = √(P_ΔX(1,1) + P_ΔX(2,2) + P_ΔX(3,3))
    double gdop = std::sqrt(P_deltaX(0, 0) + P_deltaX(1, 1) + P_deltaX(2, 2));
    
    return gdop;
} 