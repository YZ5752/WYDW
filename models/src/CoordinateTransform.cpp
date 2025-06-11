/**
 * @file CoordinateTransform.cpp
 * @brief 坐标系统转换函数实现
 */

#include "../CoordinateTransform.h"

using namespace Constants;

// 大地坐标转换为空间直角坐标
COORD3 lbh2xyz(double l, double b, double h) {
    double N; // 卯酉圈曲率半径
    COORD3 xyz(0, 0, 0);
    l = l * DEG2RAD; // 转为弧度
    b = b * DEG2RAD;
    N = a / sqrt(1.0 - e_squared * sin(b) * sin(b));
    xyz.p1 = (N + h) * cos(b) * cos(l);  // X
    xyz.p2 = (N + h) * cos(b) * sin(l);  // Y
    xyz.p3 = (N * (1 - e_squared) + h) * sin(b); // Z
    return xyz;
}

// 空间直角坐标转换为大地坐标
COORD3 xyz2lbh(double x, double y, double z) {
    double b0, b, N, R;
    double err;
    COORD3 lbh;
    R = sqrt(x * x + y * y);
    b0 = atan2(z, R);
    do {
        N = a / sqrt(1.0 - e_squared * sin(b0) * sin(b0));
        b = atan2(z + N * e_squared * sin(b0), R);
        err = b - b0;
        b0 = b;
    } while (fabs(err) > 0.0001 / 3600 * DEG2RAD);
    lbh.p2 = b * RAD2DEG;          // B
    lbh.p1 = atan2(y, x) * RAD2DEG; // L
    lbh.p3 = R / cos(b) - N;         // H
    return lbh;
} 