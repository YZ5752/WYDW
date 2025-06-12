/**
 * @file SNRValidator.h
 * @brief 信噪比(SNR)计算及验证相关函数
 */

#ifndef SNR_VALIDATOR_H
#define SNR_VALIDATOR_H

#include <vector>
#include <string>
#include <cmath>
#include "../constants/PhysicsConstants.h"
#include "../models/RadiationSourceDAO.h"
#include "../models/ReconnaissanceDeviceDAO.h"

/**
 * @brief 计算辐射源到侦察站的信噪比(SNR)
 * 
 * @param di 辐射源与侦察站之间的距离(米)
 * @param pt 辐射源发射功率(千瓦)
 * @param fc 辐射源载波频率(GHz)
 * @param N0_dBm 噪声功率谱密度(dBm/Hz)
 * @param B_GHz 侦察设备接收频率范围带宽(GHz)
 * @return double 信噪比(dB)
 */
double calculateSNR(double di, double pt, double fc, double N0_dBm, double B_GHz);

/**
 * @brief 根据信噪比计算最大可探测距离
 * 使用Constants::SNR_THRESHOLD作为最小可接收信噪比阈值
 * 
 * @param pt 辐射源发射功率(千瓦)
 * @param fc 辐射源载波频率(GHz)
 * @param N0_dBm 噪声功率谱密度(dBm/Hz)
 * @param B_GHz 侦察设备接收频率范围带宽(GHz)
 * @return double 最大可探测距离(米)
 */
double calculateMaxDetectionRange(double pt, double fc, double N0_dBm, double B_GHz);

/**
 * @brief 验证信噪比是否满足要求
 * 
 * 此函数会通过辐射源ID和侦察设备ID列表，从数据库获取相关信息，
 * 然后计算所有侦察设备与辐射源之间的信噪比，并验证是否满足要求。
 * @param deviceIds 侦察设备ID列表
 * @param sourceId 辐射源ID
 * @param failMessage 失败信息输出参数
 * @return bool 验证结果，成功返回true，失败返回false
 */
bool validateSNR(const std::vector<int>& deviceIds, int sourceId, std::string& failMessage);

#endif // SNR_VALIDATOR_H 