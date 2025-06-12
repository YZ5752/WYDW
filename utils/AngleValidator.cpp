/**
 * @file AngleValidator.cpp
 * @brief 辐射源和侦察设备之间角度验证的函数实现
 */
#include "AngleValidator.h"
#include "CoordinateTransform.h"
#include "../constants/PhysicsConstants.h"
#include "../models/RadiationSourceDAO.h"
#include "../models/ReconnaissanceDeviceDAO.h"
#include <cmath>
#include <sstream>
#include <iomanip>

// 计算从A点到B点的方位角和俯仰角
std::tuple<double, double> calculateAzimuthElevation(double ax, double ay, double az, 
                                                     double bx, double by, double bz) {
    // 计算相对坐标
    double dx = bx - ax;
    double dy = by - ay;
    double dz = bz - az;
    
    // 计算水平距离
    double horizontalDistance = std::sqrt(dx * dx + dy * dy);
    
    // 计算方位角（弧度），然后转换为度
    double azimuthRad = std::atan2(dx, dy);
    double azimuthDeg = azimuthRad * Constants::RAD2DEG;
    
    // 确保方位角在[0, 360]范围内
    if (azimuthDeg < 0) {
        azimuthDeg += 360.0;
    }
    
    // 计算俯仰角（弧度），然后转换为度
    double elevationRad = std::atan2(dz, horizontalDistance);
    double elevationDeg = elevationRad * Constants::RAD2DEG;
    
    return std::make_tuple(azimuthDeg, elevationDeg);
}

// 检查角度是否在给定范围内
bool isAngleInRange(double angle, double minAngle, double maxAngle) {
    // 处理方位角跨越0度的情况
    if (minAngle > maxAngle) {
        return angle >= minAngle || angle <= maxAngle;
    }
    
    // 常规情况
    return angle >= minAngle && angle <= maxAngle;
}

// 检查侦察站是否能接收到辐射源的信号
bool canReceiveSignal(double receiverX, double receiverY, double receiverZ,
                     double receiverAzimuthMin, double receiverAzimuthMax,
                     double receiverElevationMin, double receiverElevationMax,
                     double emitterX, double emitterY, double emitterZ,
                     double emitterAzimuthMin, double emitterAzimuthMax,
                     double emitterElevationMin, double emitterElevationMax) {
    
    // 计算侦察站到辐射源的方位角和俯仰角
    auto [azimuthToEmitter, elevationToEmitter] = calculateAzimuthElevation(
        receiverX, receiverY, receiverZ,
        emitterX, emitterY, emitterZ
    );
    
    // 计算辐射源到侦察站的方位角和俯仰角
    auto [azimuthToReceiver, elevationToReceiver] = calculateAzimuthElevation(
        emitterX, emitterY, emitterZ,
        receiverX, receiverY, receiverZ
    );
    
    // 检查侦察站的侦收范围是否覆盖辐射源
    bool receiverCanHear = isAngleInRange(azimuthToEmitter, receiverAzimuthMin, receiverAzimuthMax) &&
                           isAngleInRange(elevationToEmitter, receiverElevationMin, receiverElevationMax);
    
    // 检查辐射源的工作扇区是否覆盖侦察站
    bool emitterCanTransmit = isAngleInRange(azimuthToReceiver, emitterAzimuthMin, emitterAzimuthMax) &&
                              isAngleInRange(elevationToReceiver, emitterElevationMin, emitterElevationMax);
    
    // 只有当两个条件都满足时，侦察站才能接收到辐射源的信号
    return receiverCanHear && emitterCanTransmit;
}

// 验证侦察设备是否能接收到辐射源的信号
bool validateAngle(const std::vector<int>& deviceIds, int sourceId, std::string& failMessage) {
    // 获取辐射源信息
    RadiationSourceDAO& radiationSourceDAO = RadiationSourceDAO::getInstance();
    RadiationSource source = radiationSourceDAO.getRadiationSourceById(sourceId);
    
    // 获取辐射源的位置和工作扇区
    double sourceLongitude = source.longitude;
    double sourceLatitude = source.latitude;
    double sourceAltitude = source.altitude;
    double sourceAzimuthStart = source.azimuthStartAngle;
    double sourceAzimuthEnd = source.azimuthEndAngle;
    double sourceElevationStart = source.elevationStartAngle;
    double sourceElevationEnd = source.elevationEndAngle;
    
    // 获取辐射源空间直角坐标
    COORD3 sourceXYZ = lbh2xyz(sourceLongitude, sourceLatitude, sourceAltitude);
    
    // 获取侦察设备DAO实例
    ReconnaissanceDeviceDAO& deviceDAO = ReconnaissanceDeviceDAO::getInstance();
    
    // 遍历所有侦察设备
    for (int deviceId : deviceIds) {
        // 获取设备信息
        ReconnaissanceDevice device = deviceDAO.getReconnaissanceDeviceById(deviceId);
        
        // 获取设备的位置和接收角度范围
        double deviceLongitude = device.longitude;
        double deviceLatitude = device.latitude;
        double deviceAltitude = device.altitude;
        double deviceAzimuthMin = device.angleAzimuthMin;
        double deviceAzimuthMax = device.angleAzimuthMax;
        double deviceElevationMin = device.angleElevationMin;
        double deviceElevationMax = device.angleElevationMax;
        
        // 获取设备空间直角坐标
        COORD3 deviceXYZ = lbh2xyz(deviceLongitude, deviceLatitude, deviceAltitude);
        
        // 计算两个方向的方位角和俯仰角
        auto [azimuthToEmitter, elevationToEmitter] = calculateAzimuthElevation(
            deviceXYZ.p1, deviceXYZ.p2, deviceXYZ.p3,
            sourceXYZ.p1, sourceXYZ.p2, sourceXYZ.p3
        );
        
        auto [azimuthToReceiver, elevationToReceiver] = calculateAzimuthElevation(
            sourceXYZ.p1, sourceXYZ.p2, sourceXYZ.p3,
            deviceXYZ.p1, deviceXYZ.p2, deviceXYZ.p3
        );
        
        // 检查侦察站的侦收范围是否覆盖辐射源
        bool receiverCanHear = isAngleInRange(azimuthToEmitter, deviceAzimuthMin, deviceAzimuthMax) &&
                              isAngleInRange(elevationToEmitter, deviceElevationMin, deviceElevationMax);
        
        // 检查辐射源的工作扇区是否覆盖侦察站
        bool emitterCanTransmit = isAngleInRange(azimuthToReceiver, sourceAzimuthStart, sourceAzimuthEnd) &&
                                 isAngleInRange(elevationToReceiver, sourceElevationStart, sourceElevationEnd);
        
        // 判断角度验证是否通过
        if (!receiverCanHear) {
            std::stringstream ss;
            ss << "角度验证失败：设备 " << deviceId << " (" << device.deviceName << ") "
               << "的接收角度范围为 方位角[" << deviceAzimuthMin << "°~" << deviceAzimuthMax << "°], "
               << "俯仰角[" << deviceElevationMin << "°~" << deviceElevationMax << "°], "
               << "而辐射源 " << sourceId << " (" << source.radiationName << ") "
               << "相对于该设备的方位角为 " << std::fixed << std::setprecision(2) << azimuthToEmitter << "°, "
               << "俯仰角为 " << std::fixed << std::setprecision(2) << elevationToEmitter << "°";
            failMessage = ss.str();
            return false;
        }
        
        if (!emitterCanTransmit) {
            std::stringstream ss;
            ss << "角度验证失败：辐射源 " << sourceId << " (" << source.radiationName << ") "
               << "的工作扇区范围为 方位角[" << sourceAzimuthStart << "°~" << sourceAzimuthEnd << "°], "
               << "俯仰角[" << sourceElevationStart << "°~" << sourceElevationEnd << "°], "
               << "无法覆盖到设备 " << deviceId << " (" << device.deviceName << "), "
               << "设备相对于辐射源的方位角为 " << std::fixed << std::setprecision(2) << azimuthToReceiver << "°, "
               << "俯仰角为 " << std::fixed << std::setprecision(2) << elevationToReceiver << "°";
            failMessage = ss.str();
            return false;
        }
    }
    
    // 所有设备都通过验证，返回true
    return true;
} 