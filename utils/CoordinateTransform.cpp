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
    COORD3 xyz(0, 0, 0);
    
    // 将角度转换为弧度
    l = l * DEG2RAD;
    b = b * DEG2RAD;
    
    // 计算辅助参数
    double sin_b = sin(b);
    double cos_b = cos(b);
    double sin_l = sin(l);
    double cos_l = cos(l);
    
    // 计算卯酉圈曲率半径
    double N = a / sqrt(1.0 - e_squared * sin_b * sin_b);
    
    // 计算空间直角坐标
    xyz.p1 = (N + h) * cos_b * cos_l;  // X
    xyz.p2 = (N + h) * cos_b * sin_l;  // Y
    xyz.p3 = (N * (1 - e_squared) + h) * sin_b; // Z
    
    return xyz;
}

// 空间直角坐标转换为大地坐标
COORD3 xyz2lbh(double x, double y, double z) {
    COORD3 lbh;
    
    // 计算经度
    lbh.p1 = atan2(y, x) * RAD2DEG;
    
    // 计算辅助参数
    double p = sqrt(x * x + y * y);
    double e2 = e_squared;
    double a2 = a * a;
    double b2 = a2 * (1 - e2);
    
    // 初始纬度估计
    double b = atan2(z, p * (1 - e2));
    
    // 迭代计算纬度
    double N, h, b_old;
    int max_iter = 10;
    double tolerance = 1e-12;
    
    for (int i = 0; i < max_iter; i++) {
        b_old = b;
        double sin_b = sin(b);
        N = a / sqrt(1 - e2 * sin_b * sin_b);
        h = p / cos(b) - N;
        b = atan2(z, p * (1 - e2 * N / (N + h)));
        
        if (fabs(b - b_old) < tolerance) {
            break;
        }
    }
    
    // 设置纬度（转换为度）
    lbh.p2 = b * RAD2DEG;
    
    // 计算最终高度
    double sin_b = sin(b);
    N = a / sqrt(1 - e2 * sin_b * sin_b);
    lbh.p3 = p / cos(b) - N;
    
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
    double sin_l = sin(l);
    double cos_l = cos(l);
    double sin_b = sin(b);
    double cos_b = cos(b);
    
    // 旋转矩阵（从东北天到XYZ）
    double R11 = -sin_l;
    double R12 = -sin_b * cos_l;
    double R13 = cos_b * cos_l;
    
    double R21 = cos_l;
    double R22 = -sin_b * sin_l;
    double R23 = cos_b * sin_l;
    
    double R31 = 0;
    double R32 = cos_b;
    double R33 = sin_b;
    
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
    double sin_l = sin(l);
    double cos_l = cos(l);
    double sin_b = sin(b);
    double cos_b = cos(b);
    
    // 旋转矩阵（从XYZ到东北天）
    double R11 = -sin_l;
    double R12 = cos_l;
    double R13 = 0;
    
    double R21 = -sin_b * cos_l;
    double R22 = -sin_b * sin_l;
    double R23 = cos_b;
    
    double R31 = cos_b * cos_l;
    double R32 = cos_b * sin_l;
    double R33 = sin_b;
    
    // 将空间直角坐标系（XYZ）中的速度分量转换到局部坐标系（东北天）
    double vE = R11 * vx + R12 * vy + R13 * vz;  // 东向分量
    double vN = R21 * vx + R22 * vy + R23 * vz;  // 北向分量
    double vU = R31 * vx + R32 * vy + R33 * vz;  // 天向分量
    
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