/**
 * @file CoordinateTransform.h
 * @brief 坐标系统转换函数
 */

#ifndef COORDINATE_TRANSFORM_H
#define COORDINATE_TRANSFORM_H

#include <cmath>
#include "PhysicsConstants.h"

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

#endif // COORDINATE_TRANSFORM_H 