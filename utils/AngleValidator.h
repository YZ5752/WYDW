/**
 * @file AngleValidator.h
 * @brief 辐射源和侦察设备之间角度验证的函数声明
 */

#ifndef ANGLE_VALIDATOR_H
#define ANGLE_VALIDATOR_H

#include <tuple>
#include <string>
#include <vector>

/**
 * @brief 计算从A点到B点的方位角和俯仰角
 * @param ax A点的x坐标
 * @param ay A点的y坐标
 * @param az A点的z坐标
 * @param bx B点的x坐标
 * @param by B点的y坐标
 * @param bz B点的z坐标
 * @return 方位角和俯仰角（度）的元组，方位角以北为0度，顺时针增加
 */
std::tuple<double, double> calculateAzimuthElevation(double ax, double ay, double az, 
                                                     double bx, double by, double bz);

/**
 * @brief 检查角度是否在给定范围内
 * @param angle 要检查的角度
 * @param minAngle 角度范围下限
 * @param maxAngle 角度范围上限
 * @return 如果角度在范围内，返回true；否则返回false
 */
bool isAngleInRange(double angle, double minAngle, double maxAngle);

/**
 * @brief 检查侦察站是否能接收到辐射源的信号
 * @param receiverX 侦察站x坐标
 * @param receiverY 侦察站y坐标
 * @param receiverZ 侦察站z坐标
 * @param receiverAzimuthMin 侦察站方位角最小值
 * @param receiverAzimuthMax 侦察站方位角最大值
 * @param receiverElevationMin 侦察站俯仰角最小值
 * @param receiverElevationMax 侦察站俯仰角最大值
 * @param emitterX 辐射源x坐标
 * @param emitterY 辐射源y坐标
 * @param emitterZ 辐射源z坐标
 * @param emitterAzimuthMin 辐射源方位角最小值
 * @param emitterAzimuthMax 辐射源方位角最大值
 * @param emitterElevationMin 辐射源俯仰角最小值
 * @param emitterElevationMax 辐射源俯仰角最大值
 * @return 如果侦察站能接收到辐射源信号，返回true；否则返回false
 */
bool canReceiveSignal(double receiverX, double receiverY, double receiverZ,
                     double receiverAzimuthMin, double receiverAzimuthMax,
                     double receiverElevationMin, double receiverElevationMax,
                     double emitterX, double emitterY, double emitterZ,
                     double emitterAzimuthMin, double emitterAzimuthMax,
                     double emitterElevationMin, double emitterElevationMax);

/**
 * @brief 验证侦察设备是否能接收到辐射源的信号
 * @param deviceIds 侦察设备ID列表
 * @param sourceId 辐射源ID
 * @param failMessage 验证失败时的错误信息
 * @return 如果所有侦察设备都能接收到辐射源信号，返回true；否则返回false
 */
bool validateAngle(const std::vector<int>& deviceIds, int sourceId, std::string& failMessage);

#endif // ANGLE_VALIDATOR_H 