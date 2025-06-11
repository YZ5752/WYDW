#include "../utils/SNRCalculator.h"
#include "../constants/PhysicsConstants.h"
#include "../models/RadiationSourceDAO.h"
#include "../models/ReconnaissanceDeviceDAO.h"
#include "../utils/CoordinateTransform.h"
#include <cmath>

/**
 * @brief 计算辐射源到侦察站的信噪比(SNR)
 * @param sourceId 辐射源ID
 * @param deviceId 侦察设备ID
 * @param B_GHz 侦察设备接收频率范围带宽(GHz)
 * @return double 信噪比(dB)
 */
double calculateSNR(int sourceId, int deviceId, double B_GHz) {
    // 从数据库获取辐射源信息
    RadiationSourceDAO& sourceDAO = RadiationSourceDAO::getInstance();
    RadiationSource source = sourceDAO.getRadiationSourceById(sourceId);
    
    // 从数据库获取侦察设备信息
    ReconnaissanceDeviceDAO& deviceDAO = ReconnaissanceDeviceDAO::getInstance();
    ReconnaissanceDevice device = deviceDAO.getReconnaissanceDeviceById(deviceId);
    
    // 获取辐射源参数
    double pt = source.getTransmitPower();       // 发射功率(千瓦)
    double fc = source.getCarrierFrequency();    // 载波频率(GHz)
    double source_longitude = source.getLongitude(); // 辐射源经度
    double source_latitude = source.getLatitude();   // 辐射源纬度
    double source_altitude = source.getAltitude();   // 辐射源高度
    
    // 获取侦察设备参数
    double N0_dBm = device.getNoisePsd();        // 噪声功率谱密度(dBm/Hz)
    double device_longitude = device.getLongitude(); // 侦察设备经度
    double device_latitude = device.getLatitude();   // 侦察设备纬度
    double device_altitude = device.getAltitude();   // 侦察设备高度
    
    // 计算辐射源与侦察设备之间的距离
    double di = calculateDistance(
        source_longitude, source_latitude, source_altitude,
        device_longitude, device_latitude, device_altitude
    );
    
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
 * @param sourceId 辐射源ID
 * @param deviceId 侦察设备ID
 * @param B_GHz 侦察设备接收频率范围带宽(GHz)
 * @return double 最大可探测距离(米)
 */
double calculateMaxDetectionRange(int sourceId, int deviceId, double B_GHz) {
    // 从数据库获取辐射源信息
    RadiationSourceDAO& sourceDAO = RadiationSourceDAO::getInstance();
    RadiationSource source = sourceDAO.getRadiationSourceById(sourceId);
    
    // 从数据库获取侦察设备信息
    ReconnaissanceDeviceDAO& deviceDAO = ReconnaissanceDeviceDAO::getInstance();
    ReconnaissanceDevice device = deviceDAO.getReconnaissanceDeviceById(deviceId);
    
    // 获取辐射源参数
    double pt = source.getTransmitPower();       // 发射功率(千瓦)
    double fc = source.getCarrierFrequency();    // 载波频率(GHz)
    
    // 获取侦察设备参数
    double N0_dBm = device.getNoisePsd();        // 噪声功率谱密度(dBm/Hz)
    
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