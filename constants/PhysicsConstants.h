/**
 * @file PhysicsConstants.h
 * @brief 定义项目中使用的物理常量
 */

#ifndef PHYSICS_CONSTANTS_H
#define PHYSICS_CONSTANTS_H

namespace Constants {
    // 物理常数
    const double c = 299792458.0;  // 光速 (m/s)
    const double SPEED_OF_LIGHT = c;  // 光速别名，保持兼容性
    const double PI = 3.14159265358979323846;

     // 坐标转换常量
    const double DEG2RAD = PI/180.0;
    const double RAD2DEG = 180.0/PI;
    const double a = 6378137.0000;  // CGCS2000长半轴
    const double EARTH_RADIUS = a;  // 地球半径（米），使用长半轴作为近似值
    const double e_squared = 0.00669438002290;  // CGCS2000第一偏心率平方

    // SNR阈值信号,如果辐射源与侦察站之间的信噪比小于1dB，则无法接收信号
    const double SNR_THRESHOLD = 1;  // SNR阈值 (dB)Add commentMore actions
    //频差分辨力
    const double FREQUENCY_RESOLUTION = 100;  // 频差分辨力 (Hz)
    
    // 干涉仪误差计算相关常量
    const double INTERFEROMETER_ALIGNMENT_ERROR = 0.17;   // 对中误差（度）
    const double INTERFEROMETER_ATTITUDE_ERROR = 0.1;     // 惯导测量精度（度）
    const double INTERFEROMETER_PHASE_ERROR = 35.0;       // 相位不一致性误差（度）

    // 圆锥效应误差表格数据
    const int CONE_EFFECT_ALPHA_LEVELS = 4;  // 方位角等级数
    const int CONE_EFFECT_BETA_LEVELS = 3;   // 仰角等级数
    const double CONE_EFFECT_ALPHA_BOUNDS[] = {5.0, 15.0, 25.0, 35.0};  // 方位角边界值
    const double CONE_EFFECT_BETA_BOUNDS[] = {5.0, 10.0, 15.0};         // 仰角边界值
    const double CONE_EFFECT_ERROR_TABLE[3][4] = {
        {0.02, 0.06, 0.10, 0.15},  // β ≤ 5°
        {0.08, 0.23, 0.41, 0.61},  // 5° < β ≤ 10°
        {0.17, 0.52, 0.91, 1.36}   // 10° < β ≤ 15°
    };
}

#endif // PHYSICS_CONSTANTS_H 