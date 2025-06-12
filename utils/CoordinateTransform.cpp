/**
 * @brief 功能：
 * - 大地坐标转换为空间直角坐标
 * - 空间直角坐标转换为大地坐标
 * - 大地坐标系中的速度转换为空间直角坐标系中的速度分量
 * - 空间直角坐标系中的速度分量转换为大地坐标系中的速度参数
 * - 计算两个大地坐标点之间的距离
 */

#include "CoordinateTransform.h"

using namespace Constants;

// 大地坐标转换为空间直角坐标
COORD3 lbh2xyz(double l, double b, double h) {
    double N; // 卯酉圈曲率半径
    COORD3 xyz(0, 0, 0);
    l = l * DE
    G2RAD; // 转为弧度
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

// 大地坐标系中的速度转换为空间直角坐标系中的速度分量
COORD3 velocity_lbh2xyz(double l, double b, double v, double azimuth, double elevation) {
    COORD3 velocity(0, 0, 0);
    
    // 将角度转换为弧度
    l = l * DEG2RAD;
    b = b * DEG2RAD;
    azimuth = azimuth * DEG2RAD;
    elevation = elevation * DEG2RAD;
    
    // 计算在局部坐标系（东北天）中的速度分量
    double vE = v * cos(elevation) * sin(azimuth);  // 东向分量
    double vN = v * cos(elevation) * cos(azimuth);  // 北向分量
    double vU = v * sin(elevation);                 // 天向分量
    
    // 计算局部坐标系到空间直角坐标系的旋转矩阵
    // 旋转矩阵的三个列向量分别表示东、北、天方向在XYZ坐标系中的投影
    double R11 = -sin(l);
    double R12 = -sin(b) * cos(l);
    double R13 = cos(b) * cos(l);
    
    double R21 = cos(l);
    double R22 = -sin(b) * sin(l);
    double R23 = cos(b) * sin(l);
    
    double R31 = 0;
    double R32 = cos(b);
    double R33 = sin(b);
    
    // 将局部坐标系（东北天）中的速度分量转换到空间直角坐标系（XYZ）
    velocity.p1 = R11 * vE + R12 * vN + R13 * vU;  // vx
    velocity.p2 = R21 * vE + R22 * vN + R23 * vU;  // vy
    velocity.p3 = R31 * vE + R32 * vN + R33 * vU;  // vz
    
    return velocity;
}

// 空间直角坐标系中的速度分量转换为大地坐标系中的速度参数
COORD3 velocity_xyz2lbh(double l, double b, double vx, double vy, double vz) {
    COORD3 velocity(0, 0, 0);
    
    // 将角度转换为弧度
    l = l * DEG2RAD;
    b = b * DEG2RAD;
    
    // 计算局部坐标系到空间直角坐标系的旋转矩阵
    double R11 = -sin(l);
    double R12 = -sin(b) * cos(l);
    double R13 = cos(b) * cos(l);
    
    double R21 = cos(l);
    double R22 = -sin(b) * sin(l);
    double R23 = cos(b) * sin(l);
    
    double R31 = 0;
    double R32 = cos(b);
    double R33 = sin(b);
    
    // 将空间直角坐标系（XYZ）中的速度分量转换到局部坐标系（东北天）
    // 使用旋转矩阵的转置（即逆矩阵）
    double vE = R11 * vx + R21 * vy + R31 * vz;  // 东向分量
    double vN = R12 * vx + R22 * vy + R32 * vz;  // 北向分量
    double vU = R13 * vx + R23 * vy + R33 * vz;  // 天向分量
    
    // 计算速度大小
    double v = sqrt(vE * vE + vN * vN + vU * vU);
    velocity.p1 = v;  // 速度大小
    
    // 计算方位角（正北为0度，顺时针为正）
    double azimuth = atan2(vE, vN);
    if (azimuth < 0) {
        azimuth += 2 * PI;  // 确保方位角在[0, 2π)范围内
    }
    velocity.p2 = azimuth * RAD2DEG;  // 方位角（度）
    
    // 计算俯仰角（水平为0度，向上为正）
    double elevation = 0;
    if (v > 1e-10) {  // 避免除以零
        elevation = asin(vU / v);
    }
    velocity.p3 = elevation * RAD2DEG;  // 俯仰角（度）
    
    return velocity;
}

/**
 * @brief 计算两个大地坐标点之间的距离
 * @param l1 第一个点的经度(度)
 * @param b1 第一个点的纬度(度)
 * @param h1 第一个点的高程(米)
 * @param l2 第二个点的经度(度)
 * @param b2 第二个点的纬度(度)
 * @param h2 第二个点的高程(米)
 * @return 两点之间的空间距离(米)
 */
double calculateDistance(double l1, double b1, double h1, double l2, double b2, double h2) {
    // 将大地坐标转换为空间直角坐标
    COORD3 xyz1 = lbh2xyz(l1, b1, h1);
    COORD3 xyz2 = lbh2xyz(l2, b2, h2);
    
    // 计算欧式距离
    double dx = xyz2.p1 - xyz1.p1;
    double dy = xyz2.p2 - xyz1.p2;
    double dz = xyz2.p3 - xyz1.p3;
    
    // 返回两点之间的距离
    return sqrt(dx * dx + dy * dy + dz * dz);
} 