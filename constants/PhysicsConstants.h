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
    const double e_squared = 0.00669438002290;  // CGCS2000第一偏心率平方
    
}

#endif // PHYSICS_CONSTANTS_H 