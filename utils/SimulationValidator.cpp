/**
 * @file SimulationValidator.cpp
 * @brief 仿真前条件验证类的实现
 */
#include "SimulationValidator.h"
#include "SNRValidator.h"
#include "FrequencyValidator.h"
#include "CoordinateTransform.h"
#include "AngleValidator.h"
#include "../constants/PhysicsConstants.h"
#include "../models/RadiationSourceDAO.h"
#include "../models/ReconnaissanceDeviceDAO.h"
#include <sstream>
#include <iomanip>
#include <limits>
#include <algorithm>

SimulationValidator::SimulationValidator() {
}

SimulationValidator::~SimulationValidator() {
}
//所有验证
bool SimulationValidator::validateAll(const std::vector<int>& deviceIds, int sourceId, std::string& failMessage) {
    // 一次性获取所有需要的数据
    RadiationSourceDAO& radiationSourceDAO = RadiationSourceDAO::getInstance();
    ReconnaissanceDeviceDAO& deviceDAO = ReconnaissanceDeviceDAO::getInstance();
    
    // 获取辐射源信息
    RadiationSource source = radiationSourceDAO.getRadiationSourceById(sourceId);
    
    // 获取辐射源的各项参数
    double sourceFrequency = source.getCarrierFrequency();  // 载波频率(GHz)
    double sourceLongitude = source.getLongitude();         // 经度
    double sourceLatitude = source.getLatitude();           // 纬度
    double sourceAltitude = source.getAltitude();           // 高度
    double sourcePower = source.getTransmitPower();         // 发射功率
    double sourceAzimuthStart = source.getAzimuthStart();    // 工作扇区方位角下限
    double sourceAzimuthEnd = source.getAzimuthEnd();        // 工作扇区方位角上限
    double sourceElevationStart = source.getElevationStart(); // 工作扇区俯仰角下限
    double sourceElevationEnd = source.getElevationEnd();     // 工作扇区俯仰角上限
    
    // 获取辐射源空间直角坐标
    COORD3 sourceXYZ = lbh2xyz(sourceLongitude, sourceLatitude, sourceAltitude);
    
    // 计算所有侦察设备的频率范围交集
    double intersectFreqMin = -std::numeric_limits<double>::infinity();
    double intersectFreqMax = std::numeric_limits<double>::infinity();
    
    // 遍历所有侦察设备，计算侦收频率范围交集
    for (int deviceId : deviceIds) {
        // 获取设备信息
        ReconnaissanceDevice device = deviceDAO.getReconnaissanceDeviceById(deviceId);
        
        // 更新频率范围交集
        intersectFreqMin = std::max(intersectFreqMin, static_cast<double>(device.getFreqRangeMin()));
        intersectFreqMax = std::min(intersectFreqMax, static_cast<double>(device.getFreqRangeMax()));
    }
    
    // 检查交集是否有效
    if (intersectFreqMin > intersectFreqMax) {
        std::stringstream ss;
        ss << "频率验证失败：所有侦察设备的频率范围没有交集";
        failMessage = ss.str();
        return false;
    }
    
    // 计算实际带宽 (GHz)
    double commonBandwidth = intersectFreqMax - intersectFreqMin;
    
    // 遍历所有侦察设备
    for (int deviceId : deviceIds) {
        // 获取设备信息
        ReconnaissanceDevice device = deviceDAO.getReconnaissanceDeviceById(deviceId);
        
        // 获取设备的各项参数
        double minFreq = device.getFreqRangeMin();           // 侦收频率范围下限
        double maxFreq = device.getFreqRangeMax();           // 侦收频率范围上限
        double deviceLongitude = device.getLongitude();      // 经度
        double deviceLatitude = device.getLatitude();        // 纬度
        double deviceAltitude = device.getAltitude();        // 高度
        double deviceNoisePSD = device.getNoisePsd();        // 噪声功率谱密度(dBm/Hz)
        double deviceAzimuthMin = device.getAngleAzimuthMin();      // 方位角下限
        double deviceAzimuthMax = device.getAngleAzimuthMax();      // 方位角上限
        double deviceElevationMin = device.getAngleElevationMin();  // 俯仰角下限
        double deviceElevationMax = device.getAngleElevationMax();  // 俯仰角上限
        
        // ============ 1. 频率验证 ============
        if (sourceFrequency < minFreq || sourceFrequency > maxFreq) {
            std::stringstream ss;
            ss << "频率验证失败：侦察设备 "<< device.getDeviceName()
               << "的接收频率范围为 " << minFreq << "~" << maxFreq << " GHz，"
               << "无法接收辐射源 " << source.getRadiationName() 
               << "的频率 " << sourceFrequency << " GHz";
            failMessage = ss.str();
            return false;
        }
        
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
        
        if (!receiverCanHear) {
            std::stringstream ss;
            ss << "角度验证失败：设备 " << device.getDeviceName() 
               << "的接收角度范围为 方位角[" << deviceAzimuthMin << "°~" << deviceAzimuthMax << "°], "
               << "俯仰角[" << deviceElevationMin << "°~" << deviceElevationMax << "°], "
               << "而辐射源 " << source.getRadiationName()
               << "相对于该设备的方位角为 " << std::fixed << std::setprecision(2) << azimuthToEmitter << "°, "
               << "俯仰角为 " << std::fixed << std::setprecision(2) << elevationToEmitter << "°";
            failMessage = ss.str();
            return false;
        }
        
        if (!emitterCanTransmit) {
            std::stringstream ss;
            ss << "角度验证失败：辐射源 " << source.getRadiationName()
               << "的工作扇区范围为 方位角[" << sourceAzimuthStart << "°~" << sourceAzimuthEnd << "°], "
               << "俯仰角[" << sourceElevationStart << "°~" << sourceElevationEnd << "°], "
               << "而设备 " << device.getDeviceName()
               << "相对于辐射源的方位角为 " << std::fixed << std::setprecision(2) << azimuthToReceiver << "°, "
               << "俯仰角为 " << std::fixed << std::setprecision(2) << elevationToReceiver << "°";
            failMessage = ss.str();
            return false;
        }
        
        // ============ 3. SNR验证 ============
        // 计算两点之间的距离（使用两点坐标计算）
        double dx = sourceXYZ.p1 - deviceXYZ.p1;
        double dy = sourceXYZ.p2 - deviceXYZ.p2;
        double dz = sourceXYZ.p3 - deviceXYZ.p3;
        double distance = std::sqrt(dx*dx + dy*dy + dz*dz);
        
        // 计算SNR（使用函数计算）
        double snr = calculateSNR(distance, sourcePower, sourceFrequency, deviceNoisePSD, commonBandwidth);
        
        // 计算最大可探测距离
        double max_distance = calculateMaxDetectionRange(sourcePower, sourceFrequency, deviceNoisePSD, commonBandwidth);
        
        // 判断SNR是否超过阈值
        if (snr < Constants::SNR_THRESHOLD) {
            std::stringstream ss;
            ss << "SNR验证失败：辐射源 " << source.getRadiationName()
               << "与侦察设备 " << device.getDeviceName()
               << "之间的距离不能超过 " << std::fixed << std::setprecision(2) << max_distance << " 米，"
               << "当前距离为 " << std::fixed << std::setprecision(2) << distance << " 米，"
               << "信噪比为 " << std::fixed << std::setprecision(2) << snr << " dB，"
               << "低于阈值 " << Constants::SNR_THRESHOLD << " dB";
            failMessage = ss.str();
            return false;
        }
    }
    
    // 所有验证都通过，返回true
    return true;
}