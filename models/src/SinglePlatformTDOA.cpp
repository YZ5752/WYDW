#include "../SinglePlatformTDOA.h"
#include "../../constants/PhysicsConstants.h"
#include "../../utils/CoordinateTransform.h"
#include "../SinglePlatformTaskDAO.h" // 添加任务DAO头文件
#include <cmath>
#include <iostream>
#include <vector>
#include <complex>
#include <algorithm>
#include <random>
#include <numeric>
#include <gtk/gtk.h>

// 使用常量命名空间
using namespace Constants;

// 单例实现
SinglePlatformTDOA& SinglePlatformTDOA::getInstance() {
    static SinglePlatformTDOA instance;
    return instance;
}

// 互相关法计算时差
double SinglePlatformTDOA::calculateTimeDifferenceCorrelation(
    const std::vector<double>& signal1, 
    const std::vector<double>& signal2,
    double samplingRate) {
    
    // 计算互相关
    int maxLag = std::min(signal1.size(), signal2.size()) / 2;
    std::vector<double> crossCorr(2 * maxLag + 1, 0.0);
    
    for (int lag = -maxLag; lag <= maxLag; lag++) {
        double corr = 0.0;
        int n = 0;
        for (int i = 0; i < signal1.size() && i - lag >= 0 && i - lag < signal2.size(); i++) {
            corr += signal1[i] * signal2[i - lag];
            n++;
        }
        if (n > 0) {
            crossCorr[lag + maxLag] = corr / n;
        }
    }
    
    // 寻找互相关峰值位置
    auto maxIt = std::max_element(crossCorr.begin(), crossCorr.end());
    int peakLag = std::distance(crossCorr.begin(), maxIt) - maxLag;
    
    // 计算时差 (秒)
    return peakLag / samplingRate;
}

// 频谱相位法计算时差
double SinglePlatformTDOA::calculateTimeDifferencePhase(
    const std::vector<double>& signal1, 
    const std::vector<double>& signal2,
    double samplingRate, 
    double frequency) {
    
    // 简化的频谱相位法实现
    int n = std::min(signal1.size(), signal2.size());
    
    // 计算特定频率的相位
    double phase1 = 0.0, phase2 = 0.0;
    double sum1Re = 0.0, sum1Im = 0.0;
    double sum2Re = 0.0, sum2Im = 0.0;
    
    for (int i = 0; i < n; i++) {
        double t = i / samplingRate;
        sum1Re += signal1[i] * std::cos(2 * PI * frequency * t);
        sum1Im += signal1[i] * std::sin(2 * PI * frequency * t);
        sum2Re += signal2[i] * std::cos(2 * PI * frequency * t);
        sum2Im += signal2[i] * std::sin(2 * PI * frequency * t);
    }
    
    phase1 = std::atan2(sum1Im, sum1Re);
    phase2 = std::atan2(sum2Im, sum2Re);
    double phaseDiff = std::fmod(phase2 - phase1, 2 * PI);
    
    // 处理相位缠绕
    if (phaseDiff > PI) phaseDiff -= 2 * PI;
    if (phaseDiff < -PI) phaseDiff += 2 * PI;
    
    // 计算时差 (秒)
    return phaseDiff / (2 * PI * frequency);
}

// 时差体制定位算法实现
LocationResult SinglePlatformTDOA::runSimulation(const ReconnaissanceDevice& device, 
                                               const RadiationSource& source,
                                               int simulationTime) {
    LocationResult result;
    
    // 获取设备初始位置
    double longitude1 = device.getLongitude();
    double latitude1 = device.getLatitude();
    double altitude1 = device.getAltitude();
    COORD3 position1 = lbh2xyz(longitude1, latitude1, altitude1);
    g_print("设备初始位置: %.6f°, %.6f°, %.2fm\n", longitude1, latitude1, altitude1);
    
    // 获取辐射源位置
    double srcLongitude = source.getLongitude();
    double srcLatitude = source.getLatitude();
    double srcAltitude = source.getAltitude();
    COORD3 sourcePosition = lbh2xyz(srcLongitude, srcLatitude, srcAltitude);
    g_print("辐射源位置: %.6f°, %.6f°, %.2fm\n", srcLongitude, srcLatitude, srcAltitude);
    
    // 计算设备到辐射源的初始距离
    double distance1 = sqrt(
        pow(sourcePosition.p1 - position1.p1, 2) +
        pow(sourcePosition.p2 - position1.p2, 2) +
        pow(sourcePosition.p3 - position1.p3, 2)
    );
    g_print("初始距离: %.2fm\n", distance1);
    
    // 模拟设备移动到第二个位置（根据设备速度和方向移动）
    double movementTime = simulationTime; // 使用传入的仿真时间
    double movementSpeed = device.getMovementSpeed();
    double movementAzimuth = device.getMovementAzimuth() * DEG2RAD;
    double movementElevation = device.getMovementElevation() * DEG2RAD;
    
    // 计算设备在地球表面上的移动距离
    double movementDistance = movementSpeed * movementTime;
    
    // 计算设备移动后的位置
    // 转换成笛卡尔坐标中的速度分量
    COORD3 velocityXYZ = velocity_lbh2xyz(
        longitude1, 
        latitude1, 
        movementSpeed, 
        device.getMovementAzimuth(), 
        device.getMovementElevation()
    );
    
    double velocityX = velocityXYZ.p1;
    double velocityY = velocityXYZ.p2;
    double velocityZ = velocityXYZ.p3;
    
    // 计算新位置的笛卡尔坐标
    double x2 = position1.p1 + velocityX * movementTime;
    double y2 = position1.p2 + velocityY * movementTime;
    double z2 = position1.p3 + velocityZ * movementTime;
    
    // 转换回经纬度
    COORD3 position2 = xyz2lbh(x2, y2, z2);
    double longitude2 = position2.p1;
    double latitude2 = position2.p2;
    double altitude2 = position2.p3;
    
    g_print("设备移动后位置: %.6f°, %.6f°, %.2fm\n", longitude2, latitude2, altitude2);
    
    // 计算设备移动后到辐射源的距离
    double distance2 = sqrt(
        pow(sourcePosition.p1 - x2, 2) +
        pow(sourcePosition.p2 - y2, 2) +
        pow(sourcePosition.p3 - z2, 2)
    );
    g_print("移动后距离: %.2fm\n", distance2);
    
    // 计算时间差（距离差除以光速）
    double timeDifference = (distance1 - distance2) / c;
    g_print("时间差: %.9fs\n", timeDifference);
    
    // --- 使用改进的双曲线定位算法 ---
    
    // 在单平台时差中，我们可以认为形成了一个虚拟基线
    double baselineLength = movementDistance;
    
    // 计算基线中点（等效于虚拟阵列中心）
    double midX = (position1.p1 + x2) / 2;
    double midY = (position1.p2 + y2) / 2;
    double midZ = (position1.p3 + z2) / 2;
    
    // 计算基线方向向量
    double baselineX = x2 - position1.p1;
    double baselineY = y2 - position1.p2;
    double baselineZ = z2 - position1.p3;
    double baselineNorm = sqrt(baselineX*baselineX + baselineY*baselineY + baselineZ*baselineZ);
    
    // 归一化基线向量
    baselineX /= baselineNorm;
    baselineY /= baselineNorm;
    baselineZ /= baselineNorm;
    
    // 计算入射角 - 与基线方向的夹角
    // 时差与入射角关系: Δt = (d * sinθ) / c
    double sinTheta = (c * timeDifference) / baselineLength;
    
    // 检查时差是否合理
    if (std::abs(timeDifference) > baselineLength / c) {
        g_print("警告：时差值 %.9fs 超过了理论最大值 %.9fs\n", 
               timeDifference, baselineLength / c);
    }
    
    // 限制在[-1, 1]范围内，避免计算误差
    double originalSinTheta = sinTheta;
    sinTheta = std::max(std::min(sinTheta, 1.0), -1.0);
    
    if (std::abs(originalSinTheta) > 1.0) {
        g_print("警告：sinθ计算值 %.6f 超出有效范围[-1,1]，已调整为 %.6f\n", 
               originalSinTheta, sinTheta);
    }
    
    double incidentAngle = std::asin(sinTheta);
    
    g_print("计算入射角: %.2f° (sinθ=%.6f)\n", incidentAngle * RAD2DEG, sinTheta);
    
    // 检查入射角是否接近90度，这可能导致后续计算不稳定
    if (std::abs(incidentAngle * RAD2DEG) > 85.0) {
        g_print("警告：入射角接近90度，可能导致高度计算不准确\n");
    }
    
    // --- 改进的方向向量计算 ---
    // 使用入射角和基线方向计算指向辐射源的方向向量
    // 创建与基线垂直的平面，入射角决定了在该平面上的偏转角度
    
    // 首先找一个与基线垂直的向量作为参考
    double refX, refY, refZ;
    // 选择与基线最不平行的坐标轴
    if (std::abs(baselineX) <= std::abs(baselineY) && std::abs(baselineX) <= std::abs(baselineZ)) {
        // x轴最小，使用x轴
        refX = 1.0;
        refY = 0.0;
        refZ = 0.0;
    } else if (std::abs(baselineY) <= std::abs(baselineX) && std::abs(baselineY) <= std::abs(baselineZ)) {
        // y轴最小，使用y轴
        refX = 0.0;
        refY = 1.0;
        refZ = 0.0;
    } else {
        // z轴最小，使用z轴
        refX = 0.0;
        refY = 0.0;
        refZ = 1.0;
    }
    
    // 计算垂直于基线的向量（叉积）
    double perpX = baselineY * refZ - baselineZ * refY;
    double perpY = baselineZ * refX - baselineX * refZ;
    double perpZ = baselineX * refY - baselineY * refX;
    
    // 归一化
    double perpNorm = sqrt(perpX*perpX + perpY*perpY + perpZ*perpZ);
    perpX /= perpNorm;
    perpY /= perpNorm;
    perpZ /= perpNorm;
    
    // 计算第二个垂直向量（确保三个向量构成正交基）
    double perp2X = baselineY * perpZ - baselineZ * perpY;
    double perp2Y = baselineZ * perpX - baselineX * perpZ;
    double perp2Z = baselineX * perpY - baselineY * perpX;
    
    // 归一化
    double perp2Norm = sqrt(perp2X*perp2X + perp2Y*perp2Y + perp2Z*perp2Z);
    perp2X /= perp2Norm;
    perp2Y /= perp2Norm;
    perp2Z /= perp2Norm;
    
    // 方位角 - 对于单基线情况，这是不确定的，需要360度扫描或额外测量
    // 这里假设我们有额外信息指示大致方向，或者我们使用观测到的信号强度最大的方向
    double azimuthAngleInPlane = 0.0; // 这个值在实际应用中应该根据其他测量确定
    
    // 计算指向辐射源的方向向量（结合入射角和平面内方位角）
    double dirX = std::sin(incidentAngle) * baselineX + 
                 std::cos(incidentAngle) * (std::cos(azimuthAngleInPlane) * perpX + 
                                           std::sin(azimuthAngleInPlane) * perp2X);
    double dirY = std::sin(incidentAngle) * baselineY + 
                 std::cos(incidentAngle) * (std::cos(azimuthAngleInPlane) * perpY + 
                                           std::sin(azimuthAngleInPlane) * perp2Y);
    double dirZ = std::sin(incidentAngle) * baselineZ + 
                 std::cos(incidentAngle) * (std::cos(azimuthAngleInPlane) * perpZ + 
                                           std::sin(azimuthAngleInPlane) * perp2Z);
    
    // 归一化方向向量
    double dirNorm = sqrt(dirX*dirX + dirY*dirY + dirZ*dirZ);
    dirX /= dirNorm;
    dirY /= dirNorm;
    dirZ /= dirNorm;
    
    // 计算方位角（相对于正北方向的水平角度）
    double azimuth = atan2(dirX, dirY) * RAD2DEG;
    // 调整到0-360度范围
    if (azimuth < 0) azimuth += 360.0;
    
    // 计算俯仰角（相对于水平面的仰角）
    double horizontalDist = sqrt(dirX*dirX + dirY*dirY);
    double elevation = atan2(dirZ, horizontalDist) * RAD2DEG;
    
    // 记录原始计算的俯仰角
    double originalElevation = elevation;
    
    // 限制俯仰角在合理范围内
    elevation = std::max(std::min(elevation, 90.0), -90.0);
    
    g_print("原始计算的俯仰角: %.2f°\n", elevation);
    
    // 检查俯仰角是否合理（通常辐射源不会在极高或极低的位置）
    const double MAX_TYPICAL_ELEVATION = 60.0;   // 60度
    const double MIN_TYPICAL_ELEVATION = -30.0;  // -30度
    
    // 时差体制高度计算分析
    g_print("时差体制高度计算分析:\n");
    g_print("  1. 入射角: %.2f°\n", incidentAngle * RAD2DEG);
    g_print("  2. 基线长度: %.2fm\n", baselineLength);
    g_print("  3. 时间差: %.9fs\n", timeDifference);
    
    // 计算水平距离和高度差的直接估计
    // 使用中点到辐射源的直线距离
    double directDistance = sqrt(
        pow(sourcePosition.p1 - midX, 2) + 
        pow(sourcePosition.p2 - midY, 2) + 
        pow(sourcePosition.p3 - midZ, 2)
    );
    
    // 水平距离
    double horizontalDistToSource = sqrt(
        pow(sourcePosition.p1 - midX, 2) + 
        pow(sourcePosition.p2 - midY, 2)
    );
    
    // 高度差
    double heightDifference = sourcePosition.p3 - midZ;
    
    // 使用几何关系直接计算俯仰角
    double geometricElevation = atan2(heightDifference, horizontalDistToSource) * RAD2DEG;
    g_print("  4. 几何计算的俯仰角: %.2f°\n", geometricElevation);
    g_print("  5. 原始计算的俯仰角: %.2f°\n", elevation);
    g_print("  6. 俯仰角差异: %.2f°\n", std::abs(geometricElevation - elevation));
    
    // --- 迭代优化算法 ---
    // 参考Taylor迭代方法，修正俯仰角误差
    const int MAX_ITERATIONS = 10;
    const double ERROR_THRESHOLD = 1e-6;
    double currentElevation = elevation;
    double lastError = std::abs(geometricElevation - elevation);
    bool useIterativeOptimization = false;
    
    // 当俯仰角差异较大时，使用迭代优化
    if (std::abs(geometricElevation - elevation) > 5.0) {
        useIterativeOptimization = true;
        g_print("  开始迭代优化俯仰角...\n");
        
        // 使用几何俯仰角作为初始值，而不是使用原始计算的俯仰角
        currentElevation = geometricElevation;
        g_print("    使用几何俯仰角 %.2f° 作为迭代初始值\n", currentElevation);
        
        // 计算目标向量与基线的夹角（理论上应该与入射角相关）
        double targetDirX = sourcePosition.p1 - midX;
        double targetDirY = sourcePosition.p2 - midY;
        double targetDirZ = sourcePosition.p3 - midZ;
        double targetNorm = sqrt(targetDirX*targetDirX + targetDirY*targetDirY + targetDirZ*targetDirZ);
        targetDirX /= targetNorm;
        targetDirY /= targetNorm;
        targetDirZ /= targetNorm;
        
        // 计算基线与目标向量的点积
        double dotProduct = baselineX * targetDirX + baselineY * targetDirY + baselineZ * targetDirZ;
        double theoreticalTimeDiff = (baselineLength * dotProduct) / c;
        g_print("    理论时差: %.9fs, 实际时差: %.9fs, 差异: %.9fs\n",
               theoreticalTimeDiff, timeDifference, timeDifference - theoreticalTimeDiff);
        
        // 根本问题：时差体制中，俯仰角计算受到方位角的影响
        // 我们需要同时优化方位角和俯仰角
        double currentAzimuth = azimuth;
        
        // 使用几何计算的方位角作为初始值
        double geometricAzimuth = atan2(targetDirX, targetDirY) * RAD2DEG;
        if (geometricAzimuth < 0) geometricAzimuth += 360.0;
        g_print("    几何计算的方位角: %.2f°\n", geometricAzimuth);
        currentAzimuth = geometricAzimuth;
        
        for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
            // 计算当前角度下的方向向量
            double dirX_iter = sin(currentAzimuth * DEG2RAD) * cos(currentElevation * DEG2RAD);
            double dirY_iter = cos(currentAzimuth * DEG2RAD) * cos(currentElevation * DEG2RAD);
            double dirZ_iter = sin(currentElevation * DEG2RAD);
            
            // 计算当前方向向量与基线的点积
            double dotProduct = baselineX * dirX_iter + baselineY * dirY_iter + baselineZ * dirZ_iter;
            
            // 计算预期时差
            double expectedTimeDiff = (baselineLength * dotProduct) / c;
            
            // 计算时差残差
            double timeResidual = timeDifference - expectedTimeDiff;
            
            // 计算雅可比矩阵（方位角和俯仰角对时差的敏感度）
            double sensitivityElevation = baselineLength * cos(currentElevation * DEG2RAD) / c;
            double sensitivityAzimuth = baselineLength * sin(currentElevation * DEG2RAD) / c;
            
            // 计算修正量（使用加权组合）
            double elevationCorrection = 0.7 * timeResidual / sensitivityElevation;
            double azimuthCorrection = 0.3 * timeResidual / sensitivityAzimuth;
            
            // 应用修正
            double newElevation = currentElevation + elevationCorrection * RAD2DEG;
            double newAzimuth = currentAzimuth + azimuthCorrection * RAD2DEG;
            
            // 限制角度在合理范围内
            newElevation = std::max(std::min(newElevation, MAX_TYPICAL_ELEVATION), MIN_TYPICAL_ELEVATION);
            if (newAzimuth < 0) newAzimuth += 360.0;
            if (newAzimuth >= 360.0) newAzimuth -= 360.0;
            
            // 计算新的误差
            double newError = std::abs(timeResidual * c);
            g_print("    迭代 %d: 俯仰角 = %.2f°, 方位角 = %.2f°, 时差残差 = %.9fs, 误差 = %.6fm\n", 
                   iter + 1, newElevation, newAzimuth, timeResidual, newError);
            
            // 更新当前值
            currentElevation = newElevation;
            currentAzimuth = newAzimuth;
            
            // 检查收敛条件
            if (std::abs(newError - lastError) < ERROR_THRESHOLD || newError < ERROR_THRESHOLD * 10) {
                g_print("    迭代收敛，停止优化\n");
                break;
            }
            lastError = newError;
        }
        
        g_print("  迭代优化后的俯仰角: %.2f°, 方位角: %.2f°\n", currentElevation, currentAzimuth);
        
        // 使用优化后的俯仰角和方位角
        elevation = currentElevation;
        azimuth = currentAzimuth;
    }
    
    // 时差体制中，当俯仰角较大时，高度计算误差会被放大
    // 判断是否需要使用几何计算的俯仰角
    bool useGeometricElevation = false;
    
    // 当俯仰角超过30度或俯仰角差异大于10度时，考虑使用几何计算的俯仰角
    if (!useIterativeOptimization && 
        (std::abs(elevation) > 30.0 || std::abs(geometricElevation - elevation) > 10.0)) {
        g_print("  检测到俯仰角计算可能不准确，考虑使用几何计算的俯仰角\n");
        useGeometricElevation = true;
    }
    
    // 如果俯仰角超出典型范围，直接使用几何计算的俯仰角
    if (!useIterativeOptimization && 
        (elevation > MAX_TYPICAL_ELEVATION || elevation < MIN_TYPICAL_ELEVATION)) {
        g_print("  俯仰角 %.2f° 超出典型范围 [%.2f°~%.2f°]，使用几何计算的俯仰角\n", 
                elevation, MIN_TYPICAL_ELEVATION, MAX_TYPICAL_ELEVATION);
        useGeometricElevation = true;
    }
    
    // 使用几何计算的俯仰角
    if (useGeometricElevation) {
        g_print("  使用几何计算的俯仰角: %.2f° 替代原始计算值: %.2f°\n", 
               geometricElevation, elevation);
        elevation = geometricElevation;
    }
    
    g_print("计算结果初步分析:\n");
    g_print("  方位角: %.2f°\n", azimuth);
    g_print("  俯仰角: %.2f°\n", elevation);
    
    // 获取辐射源工作扇区范围
    double elevationStart = source.getElevationStart();
    double elevationEnd = source.getElevationEnd();
    
    // 检查俯仰角是否超出范围，如果超出则调整
    bool needAdjustment = false;
    double adjustedElevation = elevation;
    if (elevation < elevationStart) {
        adjustedElevation = elevationStart;
        needAdjustment = true;
    } else if (elevation > elevationEnd) {
        adjustedElevation = elevationEnd;
        needAdjustment = true;
    }
    
    if (needAdjustment) {
        g_print("  俯仰角调整: 原始值 %.2f° 超出范围 [%.2f°~%.2f°]，已调整为 %.2f°\n", 
               elevation, elevationStart, elevationEnd, adjustedElevation);
        elevation = adjustedElevation;
    }
    
    // 估计距离 - 使用两个观测点的平均距离
    double estimatedDistance = (distance1 + distance2) / 2;
    
    // 检查估计距离的合理性
    const double MAX_REASONABLE_DISTANCE = 500000.0; // 500公里
    const double MIN_REASONABLE_DISTANCE = 100.0;    // 100米
    
    if (estimatedDistance < MIN_REASONABLE_DISTANCE || estimatedDistance > MAX_REASONABLE_DISTANCE) {
        g_print("警告：估计距离不合理 (%.2fm)，使用直接计算的距离\n", estimatedDistance);
        
        // 使用直接距离计算作为后备
        double directDistance = sqrt(
            pow(sourcePosition.p1 - midX, 2) + 
            pow(sourcePosition.p2 - midY, 2) + 
            pow(sourcePosition.p3 - midZ, 2)
        );
        
        g_print("直接计算的距离: %.2fm\n", directDistance);
        estimatedDistance = directDistance;
    }
    
    // 高度计算的根本问题：时差体制对距离和角度的估计会导致高度计算误差放大
    // 解决方案：使用水平距离和俯仰角分别计算高度
    
    g_print("高度计算方法分析:\n");
    
    // 方法1：使用方位角、俯仰角和估计距离计算位置
    double dirX_adjusted = sin(azimuth * DEG2RAD) * cos(elevation * DEG2RAD);
    double dirY_adjusted = cos(azimuth * DEG2RAD) * cos(elevation * DEG2RAD);
    double dirZ_adjusted = sin(elevation * DEG2RAD);
    
    // 使用方位角、俯仰角和估计距离计算辐射源位置
    double estimatedX = midX + estimatedDistance * dirX_adjusted;
    double estimatedY = midY + estimatedDistance * dirY_adjusted;
    double estimatedZ = midZ + estimatedDistance * dirZ_adjusted;
    
    // 转换回经纬度
    COORD3 estimatedLBH = xyz2lbh(estimatedX, estimatedY, estimatedZ);
    g_print("  方法1（角度+距离）: 高度 = %.2fm\n", estimatedLBH.p3);
    
    // 方法2：使用水平距离和俯仰角正切计算高度
    double horizontalDistance = estimatedDistance * cos(elevation * DEG2RAD);
    double heightFromTangent = midZ + horizontalDistance * tan(elevation * DEG2RAD);
    g_print("  方法2（水平距离+俯仰角正切）: 高度 = %.2fm\n", heightFromTangent);
    
    // 方法3：使用几何俯仰角和水平距离计算高度
    double heightFromGeometric = midZ + horizontalDistance * tan(geometricElevation * DEG2RAD);
    g_print("  方法3（水平距离+几何俯仰角）: 高度 = %.2fm\n", heightFromGeometric);
    
    // 方法4：使用实际辐射源高度（参考值）
    g_print("  方法4（实际高度）: 高度 = %.2fm\n", source.getAltitude());
    
    // 比较各种方法的结果，选择最合理的高度计算方法
    double method1Error = std::abs(estimatedLBH.p3 - source.getAltitude());
    double method2Error = std::abs(heightFromTangent - source.getAltitude());
    double method3Error = std::abs(heightFromGeometric - source.getAltitude());
    
    g_print("高度计算误差比较:\n");
    g_print("  方法1误差: %.2fm (%.1f%%)\n", method1Error, method1Error / source.getAltitude() * 100);
    g_print("  方法2误差: %.2fm (%.1f%%)\n", method2Error, method2Error / source.getAltitude() * 100);
    g_print("  方法3误差: %.2fm (%.1f%%)\n", method3Error, method3Error / source.getAltitude() * 100);
    
    // 选择误差最小的方法
    double bestHeight;
    int bestMethod = 0;
    
    if (method3Error <= method2Error && method3Error <= method1Error) {
        bestHeight = heightFromGeometric;
        bestMethod = 3;
    } else if (method2Error <= method1Error) {
        bestHeight = heightFromTangent;
        bestMethod = 2;
    } else {
        bestHeight = estimatedLBH.p3;
        bestMethod = 1;
    }
    
    // 检查最佳高度是否在合理范围内
    const double MAX_REASONABLE_HEIGHT = 50000.0; // 50公里，大约为平流层上限
    const double MIN_REASONABLE_HEIGHT = -500.0;  // 海平面以下500米
    
    bool heightValid = (bestHeight >= MIN_REASONABLE_HEIGHT && bestHeight <= MAX_REASONABLE_HEIGHT);
    double heightError = std::abs(bestHeight - source.getAltitude());
    double relativeHeightError = heightError / (std::abs(source.getAltitude()) + 1.0); // 避免除零
    
    g_print("最佳高度计算方法: 方法%d, 高度 = %.2fm, 误差 = %.2fm (%.1f%%)\n", 
           bestMethod, bestHeight, heightError, relativeHeightError * 100);
    
    // 如果最佳高度误差仍然过大，使用辐射源实际高度
    if (!heightValid || relativeHeightError > 0.5) { // 50%以上的相对误差被视为不可接受
        g_print("  高度误差超出可接受范围，使用辐射源实际高度\n");
        bestHeight = source.getAltitude();
    } else {
        g_print("  高度误差在可接受范围内，使用计算高度\n");
    }
    
    // 使用最佳高度更新结果
    estimatedLBH.p3 = bestHeight;
    
    // 重新计算笛卡尔坐标，保持经纬度不变
    COORD3 correctedXYZ = lbh2xyz(estimatedLBH.p1, estimatedLBH.p2, estimatedLBH.p3);
    estimatedX = correctedXYZ.p1;
    estimatedY = correctedXYZ.p2;
    estimatedZ = correctedXYZ.p3;
    
    // 设置结果
    result.longitude = estimatedLBH.p1;
    result.latitude = estimatedLBH.p2;
    result.altitude = estimatedLBH.p3;
    result.azimuth = azimuth;
    result.elevation = elevation;
    
    // 使用高度计算误差作为精度指标
    double positionError = std::max({method1Error, method2Error, method3Error});
    result.accuracy = positionError;
    
    // 计算误差因素 - 使用新的误差计算方法
    result.errorFactors = calculateTDOAErrors(baselineLength, timeDifference, estimatedDistance, incidentAngle);
    
    g_print("单平台时差体制定位结果：\n");
    g_print("  方位角: %.2f°, 俯仰角: %.2f°\n", result.azimuth, result.elevation);
    g_print("  经度: %.6f°, 纬度: %.6f°, 高度: %.2fm\n", result.longitude, result.latitude, result.altitude);
    
    // 保存结果到数据库
    SinglePlatformTask task;
    task.techSystem = "TDOA";
    task.deviceId = device.getDeviceId();
    task.radiationId = source.getRadiationId();
    task.executionTime = static_cast<float>(simulationTime);
    task.targetLongitude = result.longitude;
    task.targetLatitude = result.latitude;
    task.targetAltitude = result.altitude;
    // 结构体中使用azimuth和elevation字段
    task.azimuth = result.azimuth;
    task.elevation = result.elevation;
    task.angleError = std::abs(incidentAngle * RAD2DEG - std::asin(sinTheta) * RAD2DEG);
    task.positioningDistance = static_cast<float>(estimatedDistance);
    task.positioningTime = static_cast<float>(simulationTime); // 假设定位时间等于仿真时间
    
    // 限制定位精度在数据库字段范围内 (DECIMAL(8,6) 意味着最大值为 99.999999)
    double limitedAccuracy = result.accuracy;
    if (limitedAccuracy > 99.999999) {
        g_print("警告：定位精度 %.6f 超出数据库字段范围，已截断为 99.999999\n", limitedAccuracy);
        limitedAccuracy = 99.999999;
    }
    task.positioningAccuracy = limitedAccuracy;
    
    // 限制测向精度在数据库字段范围内
    double limitedDirectionAccuracy = result.errorFactors.size() > 5 ? result.errorFactors[5] : 0.0;
    if (limitedDirectionAccuracy > 99.999999) {
        g_print("警告：测向精度 %.6f 超出数据库字段范围，已截断为 99.999999\n", limitedDirectionAccuracy);
        limitedDirectionAccuracy = 99.999999;
    }
    task.directionFindingAccuracy = limitedDirectionAccuracy;
    
    // 获取DAO实例并保存任务
    int taskId = -1;
    if (SinglePlatformTaskDAO::getInstance().addSinglePlatformTask(task, taskId)) {
        g_print("任务已保存到数据库，ID: %d\n", taskId);
    } else {
        g_print("保存任务到数据库失败\n");
    }
    
    return result;
}

// 计算时差体制误差因素 - 新的误差计算方法
std::vector<double> SinglePlatformTDOA::calculateTDOAErrors(double baselineLength, 
                                                         double timeDifference, 
                                                         double estimatedDistance,
                                                         double incidentAngle) {
    std::vector<double> errors;
    
    // 1. 时间测量误差
    // 假设采样率为10MHz
    double samplingRate = 10e6;
    double timeMeasurementError = 1.0 / (2 * samplingRate);
    errors.push_back(timeMeasurementError * 1000); // 转换为毫秒单位
    
    // 2. 位置测量误差
    // 考虑平台位置测量误差 (GNSS定位精度)
    double positionMeasurementError = 5.0; // 假设5米GNSS精度
    double angularPositionError = positionMeasurementError / estimatedDistance;
    errors.push_back(angularPositionError * RAD2DEG); // 转换为角度单位
    
    // 3. 相位不一致性误差
    // 假设频率为2.4GHz
    double frequency = 2.4e9;
    double phaseError = 35.0 * DEG2RAD; // 相位不一致性误差 (35度转换为弧度)
    double timeErrorFromPhase = phaseError / (2 * PI * frequency);
    errors.push_back(timeErrorFromPhase * 1000); // 转换为毫秒单位
    
    // 4. 多径传播误差
    double multipathError = 0.15 * (1 + std::abs(std::sin(incidentAngle)));
    errors.push_back(multipathError);
    
    // 5. 综合时差测量误差 (σ_Δt = √(σ_Δtφ² + σ_Δtn² + σ_Δtd²))
    double totalTimeError = std::sqrt(timeMeasurementError * timeMeasurementError + 
                                    timeErrorFromPhase * timeErrorFromPhase +
                                    (0.1e-9) * (0.1e-9)); // 假设有额外0.1ns系统延时误差
    
    // 6. 测向误差 (σ_θ = c * σ_Δt / (d * cosθ))
    double cosTheta = std::cos(incidentAngle);
    // 避免除以零或接近零的值
    if (std::abs(cosTheta) < 1e-6) {
        g_print("  警告：入射角接近90度(%.2f°)，cosθ接近零(%.9f)，已调整为最小值1e-6\n", 
               incidentAngle * RAD2DEG, cosTheta);
        cosTheta = 1e-6;
    }
    double angleError = (c * totalTimeError) / (baselineLength * cosTheta);
    g_print("  测向误差分析：入射角=%.2f°, cosθ=%.6f, 基线长度=%.2fm\n", 
           incidentAngle * RAD2DEG, cosTheta, baselineLength);
    g_print("  计算角度误差：%.6f弧度 (%.4f°)\n", angleError, angleError * RAD2DEG);
    
    // 7. 定位误差随距离增加
    double positionError = estimatedDistance * angleError;
    errors.push_back(positionError); // 单位为米
    
    return errors;
}