/**
 * @file CoordinateTransform.h
 * @brief 坐标系统转换函数
 */

#ifndef COORDINATE_TRANSFORM_H
#define COORDINATE_TRANSFORM_H

#include <cmath>
#include "../constants/PhysicsConstants.h"

/**
 * @brief 三维坐标类
 */
class COORD3 {
public:
    double p1, p2, p3;
    
    COORD3(double a1 = 0, double b1 = 0, double c1 = 0) {
        this->p1 = a1;
        this->p2 = b1;
        this->p3 = c1;
    }
};

/**
 * @brief 大地坐标转换为空间直角坐标
 * @param l 经度(度)
 * @param b 纬度(度)
 * @param h 高程(米)
 * @return 空间直角坐标(x,y,z)
 */
COORD3 lbh2xyz(double l, double b, double h);

/**
 * @brief 空间直角坐标转换为大地坐标
 * @param x X坐标(米)
 * @param y Y坐标(米)
 * @param z Z坐标(米)
 * @return 大地坐标(经度,纬度,高程)，单位为(度,度,米)
 */
COORD3 xyz2lbh(double x, double y, double z);

/**
 * @brief 大地坐标系中的速度转换为空间直角坐标系中的速度分量
 * @param l 经度(度)
 * @param b 纬度(度)
 * @param v 速度大小(米/秒)
 * @param azimuth 方位角(度)，正北为0度，顺时针为正
 * @param elevation 俯仰角(度)，水平为0度，向上为正
 * @return 空间直角坐标系中的速度分量(vx,vy,vz)，单位为(米/秒)
 */
COORD3 velocity_lbh2xyz(double l, double b, double v, double azimuth, double elevation);

/**
 * @brief 空间直角坐标系中的速度分量转换为大地坐标系中的速度参数
 * @param l 经度(度)
 * @param b 纬度(度)
 * @param vx X方向速度分量(米/秒)
 * @param vy Y方向速度分量(米/秒)
 * @param vz Z方向速度分量(米/秒)
 * @return 大地坐标系中的速度参数(速度大小,方位角,俯仰角)，单位为(米/秒,度,度)
 */
COORD3 velocity_xyz2lbh(double l, double b, double vx, double vy, double vz);

#endif // COORDINATE_TRANSFORM_H 