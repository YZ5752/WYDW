/**
 * @file PhysicsConstants.h
 * @brief 定义项目中使用的物理常量
 */

#ifndef PHYSICS_CONSTANTS_H
#define PHYSICS_CONSTANTS_H

namespace Constants {
    // 物理常数
    const double c = 299792458.0;  // 光速 (m/s)
    const double PI = 3.14159265358979323846;

     // 坐标转换常量
    const double DEG2RAD = PI/180.0;
    const double RAD2DEG = 180.0/PI;
    const double a = 6378137.0000;  // CGCS2000长半轴
    const double EARTH_RADIUS = a;  // 地球半径（米），使用长半轴作为近似值
    const double e_squared = 0.00669438002290;  // CGCS2000第一偏心率平方

    // SNR阈值信号,如果辐射源与侦察站之间的信噪比小于5dB，则无法接收信号
    const double SNR_THRESHOLD = 1;  // SNR阈值 (dB)
    //频差分辨力
    const double FREQUENCY_RESOLUTION = 100;  // 频差分辨力 (Hz)
}

#endif // PHYSICS_CONSTANTS_H 