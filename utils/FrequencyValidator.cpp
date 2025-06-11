#include "FrequencyValidator.h"
#include "../models/RadiationSourceDAO.h"
#include "../models/ReconnaissanceDeviceDAO.h"
#include <iostream>

/**
 * @brief 验证辐射源的载波频率是否在侦察设备的侦收频率范围内
 * 
 * @param deviceId 侦察设备ID
 * @param sourceId 辐射源ID
 * @return bool 如果载波频率在侦收频率范围内返回true，否则返回false
 */
bool validateFrequency(int deviceId, int sourceId) {
    // 获取辐射源的载波频率
    double carrierFrequency = getSourceCarrierFrequency(sourceId);
    if (carrierFrequency <= 0) {
        std::cerr << "无法获取辐射源ID " << sourceId << " 的载波频率" << std::endl;
        return false;
    }
    
    // 获取侦察设备的侦收频率范围
    double minFreq, maxFreq;
    if (!getDeviceFrequencyRange(deviceId, minFreq, maxFreq)) {
        std::cerr << "无法获取侦察设备ID " << deviceId << " 的侦收频率范围" << std::endl;
        return false;
    }
    
    // 判断载波频率是否在侦收频率范围内
    bool isValid = (carrierFrequency >= minFreq && carrierFrequency <= maxFreq);
    
    std::cout << "辐射源ID " << sourceId << " 的载波频率: " << carrierFrequency << " GHz" << std::endl;
    std::cout << "侦察设备ID " << deviceId << " 的侦收频率范围: " << minFreq << " - " << maxFreq << " GHz" << std::endl;
    std::cout << "频率验证结果: " << (isValid ? "有效" : "无效") << std::endl;
    
    return isValid;
}

/**
 * @brief 获取辐射源的载波频率
 * 
 * @param sourceId 辐射源ID
 * @return double 载波频率(GHz)，如果获取失败返回0
 */
double getSourceCarrierFrequency(int sourceId) {
    // 从数据库获取辐射源信息
    RadiationSourceDAO& sourceDAO = RadiationSourceDAO::getInstance();
    RadiationSource source = sourceDAO.getRadiationSourceById(sourceId);
    
    // 检查辐射源ID是否有效
    if (source.getRadiationId() == 0) {
        std::cerr << "辐射源ID " << sourceId << " 不存在" << std::endl;
        return 0;
    }
    
    // 返回载波频率
    return source.getCarrierFrequency();
}

/**
 * @brief 获取侦察设备的侦收频率范围
 * 
 * @param deviceId 侦察设备ID
 * @param minFreq 输出参数，侦收频率范围下限(GHz)
 * @param maxFreq 输出参数，侦收频率范围上限(GHz)
 * @return bool 如果获取成功返回true，否则返回false
 */
bool getDeviceFrequencyRange(int deviceId, double& minFreq, double& maxFreq) {
    // 从数据库获取侦察设备信息
    ReconnaissanceDeviceDAO& deviceDAO = ReconnaissanceDeviceDAO::getInstance();
    ReconnaissanceDevice device = deviceDAO.getReconnaissanceDeviceById(deviceId);
    
    // 检查侦察设备ID是否有效
    if (device.getDeviceId() == 0) {
        std::cerr << "侦察设备ID " << deviceId << " 不存在" << std::endl;
        return false;
    }
    
    // 获取侦收频率范围
    minFreq = device.getFreqRangeMin();
    maxFreq = device.getFreqRangeMax();
    
    return true;
} 