/**
 * @file FrequencyValidator.cpp
 * @brief 频率验证相关函数的实现
 */

#include "FrequencyValidator.h"
#include <sstream>

/**
 * @brief 验证频率是否满足要求
 */
bool validateFrequency(const std::vector<int>& deviceIds, int sourceId, std::string& failMessage) {
    // 获取辐射源信息
    RadiationSourceDAO& radiationSourceDAO = RadiationSourceDAO::getInstance();
    RadiationSource source = radiationSourceDAO.getRadiationSourceById(sourceId);
    
    // 获取辐射源的载波频率
    double sourceFrequency = source.carrierFrequency;
    
    // 获取侦察设备DAO实例
    ReconnaissanceDeviceDAO& deviceDAO = ReconnaissanceDeviceDAO::getInstance();
    
    // 遍历所有侦察设备
    for (int deviceId : deviceIds) {
        // 获取设备信息
        ReconnaissanceDevice device = deviceDAO.getReconnaissanceDeviceById(deviceId);
        
        // 获取侦察设备的频率范围
        double minFreq = device.freqRangeMin;
        double maxFreq = device.freqRangeMax;
        
        // 判断辐射源频率是否在侦察设备的频率范围内
        if (sourceFrequency < minFreq || sourceFrequency > maxFreq) {
            std::stringstream ss;
            ss << "频率验证失败：侦察设备 "<< device.deviceName
               << "的接收频率范围为 " << minFreq << "~" << maxFreq << " GHz，"
               << "无法接收辐射源 " << source.radiationName
               << "的频率 " << sourceFrequency << " GHz";
            failMessage = ss.str();
            return false;
        }
    }
    
    // 所有设备都通过验证，返回true
    return true;
} 