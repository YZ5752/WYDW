/**
 * @file FrequencyValidator.h
 * @brief 频率验证相关函数
 */

#ifndef FREQUENCY_VALIDATOR_H
#define FREQUENCY_VALIDATOR_H

#include <vector>
#include <string>
#include "../models/RadiationSourceDAO.h"
#include "../models/ReconnaissanceDeviceDAO.h"

/**
 * @brief 验证频率是否满足要求
 * 
 * 此函数会通过辐射源ID和侦察设备ID列表，从数据库获取相关信息，
 * 然后验证辐射源的频率是否在每个侦察设备的接收频率范围内。
 * 
 * @param deviceIds 侦察设备ID列表
 * @param sourceId 辐射源ID
 * @param failMessage 失败信息输出参数
 * @return bool 验证结果，成功返回true，失败返回false
 */
bool validateFrequency(const std::vector<int>& deviceIds, int sourceId, std::string& failMessage);

#endif // FREQUENCY_VALIDATOR_H 