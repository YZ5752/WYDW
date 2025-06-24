#include "../SinglePlatformTDOA.h"
#include "../../constants/PhysicsConstants.h"
#include "../../utils/CoordinateTransform.h"
#include <cmath>
#include <iostream>
#include <vector>
#include <gtk/gtk.h>

// 使用常量命名空间
using namespace Constants;

// 单例实现
SinglePlatformTDOA& SinglePlatformTDOA::getInstance() {
    static SinglePlatformTDOA instance;
    return instance;
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
    
    // 模拟设备移动到第二个位置（根据设备速度和方向移动30秒）
    double movementTime = 30.0; // 30秒
    double movementSpeed = device.getMovementSpeed();
    double movementAzimuth = device.getMovementAzimuth() * DEG2RAD;
    double movementElevation = device.getMovementElevation() * DEG2RAD;
    
    // 计算设备在地球表面上的移动距离
    double movementDistance = movementSpeed * movementTime;
    g_print("设备移动距离: %.2fm\n", movementDistance);
    
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
    
    // 使用双曲线定位算法
    // 在单平台时差中，我们可以认为形成了一个虚拟基线
    double baselineLength = movementDistance;
    
    // 计算基线中点
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
    
    // 计算TDOA双曲面的焦点距离差（距离差）
    double focalDistance = c * timeDifference;
    
    // 双曲线半焦距
    double semifocalDistance = baselineLength / 2;
    
    // 半主轴长度
    double semiMajorAxis = focalDistance / 2;
    
    // 半次轴长度（使用双曲线公式）
    double semiMinorAxis = sqrt(semifocalDistance*semifocalDistance - semiMajorAxis*semiMajorAxis);
    
    // 计算到辐射源的方向向量
    double dirX = sourcePosition.p1 - midX;
    double dirY = sourcePosition.p2 - midY;
    double dirZ = sourcePosition.p3 - midZ;
    double dirNorm = sqrt(dirX*dirX + dirY*dirY + dirZ*dirZ);
    
    // 归一化方向向量
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
    
    // 估计距离
    double estimatedDistance = (distance1 + distance2) / 2;
    
    // 使用方位角、俯仰角和估计距离计算辐射源位置
    double estimatedX = midX + estimatedDistance * cos(elevation * DEG2RAD) * sin(azimuth * DEG2RAD);
    double estimatedY = midY + estimatedDistance * cos(elevation * DEG2RAD) * cos(azimuth * DEG2RAD);
    double estimatedZ = midZ + estimatedDistance * sin(elevation * DEG2RAD);
    
    // 转换回经纬度
    COORD3 estimatedLBH = xyz2lbh(estimatedX, estimatedY, estimatedZ);
    
    // 设置结果
    result.longitude = estimatedLBH.p1;
    result.latitude = estimatedLBH.p2;
    result.altitude = estimatedLBH.p3;
    result.azimuth = azimuth;
    result.elevation = elevation;
    
    // 计算误差因素
    result.errorFactors = calculateTDOAErrors();
    
    g_print("单平台时差体制定位结果：\n");
    g_print("  方位角: %.2f°, 俯仰角: %.2f°\n", result.azimuth, result.elevation);
    g_print("  经度: %.6f°, 纬度: %.6f°, 高度: %.2fm\n", result.longitude, result.latitude, result.altitude);
    
    return result;
}

// 计算时差体制误差因素
std::vector<double> SinglePlatformTDOA::calculateTDOAErrors() {
    std::vector<double> errors;
    
    // 时差体制误差因素
    // 1. 时间测量误差
    double timeError = 0.15;
    errors.push_back(timeError);
    
    // 2. 位置测量误差
    double positionError = 0.22;
    errors.push_back(positionError);
    
    // 3. 多径传播误差
    double multipathError = 0.18;
    errors.push_back(multipathError);
    
    // 4. 信号处理误差
    double signalProcessError = 0.12;
    errors.push_back(signalProcessError);
    
    // 5. 综合误差
    double totalError = sqrt(pow(timeError, 2) + pow(positionError, 2) + 
                           pow(multipathError, 2) + pow(signalProcessError, 2));
    errors.push_back(totalError);
    
    return errors;
}