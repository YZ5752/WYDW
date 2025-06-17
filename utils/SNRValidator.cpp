/**
 * @file SNRValidator.cpp
 * @brief 信噪比(SNR)计算及验证相关函数的实现
 */

#include "SNRValidator.h"
#include "CoordinateTransform.h"
#include <sstream>
#include <iomanip>
#include <limits>
#include <algorithm>

/**
 * @brief 计算辐射源到侦察站的信噪比(SNR)
 */
double calculateSNR(double di, double pt, double fc, double N0_dBm, double B_GHz) {
    // 使用PhysicsConstants.h中定义的常量
    const double c = Constants::c;       // 光速(m/s)
    const double pi = Constants::PI;     // 圆周率
    
    // 单位转换
    double Pt = pt * 1000.0;            // 千瓦 -> 瓦
    double fc_Hz = fc * 1e9;            // GHz -> Hz
    double B_Hz = B_GHz * 1e9;          // GHz -> Hz
    
    // 计算波长λ (m)
    double lambda = c / fc_Hz;
    
    // 计算接收功率Pr (瓦)
    double Pr = Pt * std::pow(lambda / (4 * pi * di), 2);
    
    // 将噪声功率谱密度从dBm/Hz转换为瓦/Hz
    double N0_mW = std::pow(10.0, N0_dBm / 10.0); // dBm -> mW
    double N0 = N0_mW / 1000.0;                   // mW -> W
    
    // 计算噪声功率Pn (瓦)
    double Pn = N0 * B_Hz;
    
    // 计算线性信噪比
    double SNR_linear = Pr / Pn;
    
    // 计算并返回dB形式的信噪比
    return 10.0 * std::log10(SNR_linear);
}

/**
 * @brief 根据信噪比计算最大可探测距离
 * 使用Constants::SNR_THRESHOLD作为最小可接收信噪比阈值
 */
double calculateMaxDetectionRange(double pt, double fc, double N0_dBm, double B_GHz) {
    // 使用PhysicsConstants.h中定义的常量
    const double c = Constants::c;       // 光速(m/s)
    const double pi = Constants::PI;     // 圆周率
    // 使用SNR阈值常量
    const double min_SNR_dB = Constants::SNR_THRESHOLD;
    
    // 单位转换
    double Pt = pt * 1000.0;            // 千瓦 -> 瓦
    double fc_Hz = fc * 1e9;            // GHz -> Hz
    double B_Hz = B_GHz * 1e9;          // GHz -> Hz
    
    // 计算波长λ (m)
    double lambda = c / fc_Hz;
    
    // 将噪声功率谱密度从dBm/Hz转换为瓦/Hz
    double N0_mW = std::pow(10.0, N0_dBm / 10.0); // dBm -> mW
    double N0 = N0_mW / 1000.0;                   // mW -> W
    
    // 计算噪声功率Pn (瓦)
    double Pn = N0 * B_Hz;
    
    // 将最小信噪比从dB转换为线性值
    double min_SNR_linear = std::pow(10.0, min_SNR_dB / 10.0);
    
    // 根据SNR公式反推最大距离
    // SNR = Pt * (λ/(4πd))² / Pn
    // 解出d: d = (λ/(4π)) * sqrt(Pt/(SNR*Pn))
    double max_distance = (lambda / (4 * pi)) * std::sqrt(Pt / (min_SNR_linear * Pn));
    
    return max_distance;
}

/**
 * @brief 验证信噪比是否满足要求
 */
bool validateSNR(const std::vector<int>& deviceIds, int sourceId, std::string& failMessage) {
    // 获取辐射源信息
    RadiationSourceDAO& radiationSourceDAO = RadiationSourceDAO::getInstance();
    RadiationSource source = radiationSourceDAO.getRadiationSourceById(sourceId);
    
    // 获取辐射源的位置、发射功率和载波频率
    double sourceLongitude = source.getLongitude();
    double sourceLatitude = source.getLatitude();
    double sourceAltitude = source.getAltitude();
    double sourcePower = source.getTransmitPower();   
    double sourceFrequency = source.getCarrierFrequency(); 
    
    // 获取侦察设备DAO实例
    ReconnaissanceDeviceDAO& deviceDAO = ReconnaissanceDeviceDAO::getInstance();
    
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
        
        // 获取设备的位置，噪声功率谱密度
        double deviceLongitude = device.getLongitude();
        double deviceLatitude = device.getLatitude();
        double deviceAltitude = device.getAltitude();
        double deviceNoisePSD = device.getNoisePsd();  
        
        // 1. 计算两点之间的距离
        double di = calculateDistance(
            deviceLongitude, deviceLatitude, deviceAltitude,
            sourceLongitude, sourceLatitude, sourceAltitude
        );
        
        // 2. 计算最大可探测距离（使用公共带宽）
        double max_distance = calculateMaxDetectionRange(
            sourcePower, sourceFrequency, deviceNoisePSD, commonBandwidth
        );
        
        // 3. 计算信噪比（使用公共带宽）
        double snr = calculateSNR(di, sourcePower, sourceFrequency, deviceNoisePSD, commonBandwidth);
        
        // 4. 判断信噪比是否超过阈值
        if (snr < Constants::SNR_THRESHOLD) {
            std::stringstream ss;
            ss << "SNR验证失败：辐射源 " << source.getRadiationName()
               << "与侦察设备 " << device.getDeviceName()
               << "之间的距离不能超过 " << std::fixed << std::setprecision(2) << max_distance << " 米，"
               << "当前距离为 " << std::fixed << std::setprecision(2) << di << " 米，"
               << "信噪比为 " << std::fixed << std::setprecision(2) << snr << " dB，"
               << "低于阈值 " << Constants::SNR_THRESHOLD << " dB";
            failMessage = ss.str();
            return false;
        }
    }
    
    // 所有设备都通过验证，返回true
    return true;
} 