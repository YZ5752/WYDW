#ifndef FREQUENCY_VALIDATOR_H
#define FREQUENCY_VALIDATOR_H

/**
 * @brief 验证辐射源的载波频率是否在侦察设备的侦收频率范围内
 * 
 * @param deviceId 侦察设备ID
 * @param sourceId 辐射源ID
 * @return bool 如果载波频率在侦收频率范围内返回true，否则返回false
 */
bool validateFrequency(int deviceId, int sourceId);

/**
 * @brief 获取辐射源的载波频率
 * @param sourceId 辐射源ID
 * @return double 载波频率(GHz)
 */
double getSourceCarrierFrequency(int sourceId);

/**
 * @brief 获取侦察设备的侦收频率范围
 * 
 * @param deviceId 侦察设备ID
 * @param minFreq 输出参数，侦收频率范围下限(GHz)
 * @param maxFreq 输出参数，侦收频率范围上限(GHz)
 * @return bool 如果获取成功返回true，否则返回false
 */
bool getDeviceFrequencyRange(int deviceId, double& minFreq, double& maxFreq);

#endif // FREQUENCY_VALIDATOR_H 