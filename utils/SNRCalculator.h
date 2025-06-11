#ifndef SNR_CALCULATOR_H
#define SNR_CALCULATOR_H

#include <cmath>
#include "../constants/PhysicsConstants.h"

/**
 * @brief 计算辐射源到侦察站的信噪比(SNR)
 * 
 * @param sourceId 辐射源ID
 * @param deviceId 侦察设备ID
 * @param B_GHz 侦察设备接收频率范围带宽(GHz)
 * @return double 信噪比(dB)
 */
double calculateSNR(int sourceId, int deviceId, double B_GHz);

/**
 * @brief 根据信噪比计算最大可探测距离
 * 使用Constants::SNR_THRESHOLD作为最小可接收信噪比阈值
 * 
 * @param sourceId 辐射源ID
 * @param deviceId 侦察设备ID
 * @param B_GHz 侦察设备接收频率范围带宽(GHz)
 * @return double 最大可探测距离(米)
 */
double calculateMaxDetectionRange(int sourceId, int deviceId, double B_GHz);

#endif // SNR_CALCULATOR_H 