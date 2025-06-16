#include "../InterferometerPositioning.h"
#include "../../constants/PhysicsConstants.h"
#include "../../utils/CoordinateTransform.h"
#include <cmath>
#include <iostream>
#include <gtk/gtk.h>

// 使用常量命名空间
using namespace Constants;

// 单例实现
InterferometerPositioning& InterferometerPositioning::getInstance() {
    static InterferometerPositioning instance;
    return instance;
}

// 干涉仪体制定位算法实现
LocationResult InterferometerPositioning::runSimulation(const ReconnaissanceDevice& device, 
                                                      const RadiationSource& source,
                                                      int simulationTime) {
    LocationResult result;
    
    // 使用相位差变化率定位法
    
    // 获取观测站初始位置（设备当前位置）
    COORD3 deviceXYZ = lbh2xyz(device.getLongitude(), device.getLatitude(), device.getAltitude());
    double X_0 = deviceXYZ.p1;
    double Y_0 = deviceXYZ.p2;
    double Z_0 = deviceXYZ.p3;
    g_print("观测站初始位置: %.6f°, %.6f°, %.2fm\n", device.getLongitude(), device.getLatitude(), device.getAltitude());
    g_print("观测站初始位置: %.6f, %.6f, %.2f\n", X_0, Y_0, Z_0);
    
    // 获取辐射源位置（固定）
    COORD3 sourceXYZ = lbh2xyz(source.getLongitude(), source.getLatitude(), source.getAltitude());
    double X_T = sourceXYZ.p1;
    double Y_T = sourceXYZ.p2;
    double Z_T = sourceXYZ.p3;
    g_print("辐射源位置: %.6f°, %.6f°, %.2fm\n", source.getLongitude(), source.getLatitude(), source.getAltitude());
    g_print("辐射源位置: %.6f, %.6f, %.2f\n", X_T, Y_T, Z_T);
    
    // 获取观测站速度
    // 将设备的运动速度和方向转换为笛卡尔坐标系中的速度分量
    COORD3 velocityXYZ = velocity_lbh2xyz(
        device.getLongitude(), 
        device.getLatitude(), 
        device.getMovementSpeed(), 
        device.getMovementAzimuth(), 
        device.getMovementElevation()
    );
    double v_x = velocityXYZ.p1;
    double v_y = velocityXYZ.p2;
    double v_z = velocityXYZ.p3;
    g_print("观测站速度: %.2fm/s, %.2fm/s, %.2fm/s\n", v_x, v_y, v_z);

    // 获取基线长度
    double d = device.getBaselineLength();
    g_print("基线长度: %.2fm\n", d);
    
    // 获取辐射源频率（GHz转换为Hz）
    double f_T = source.getCarrierFrequency() * 1e9;
    g_print("辐射源频率: %.2fHz\n", f_T);
    
    // 计算方位角 θ(t) = tg^(-1)((X_T - X_0)/(Y_T - Y_0)) (公式4.2.3)
    double theta_t = atan2(X_T - X_0, Y_T - Y_0);
    g_print("方位角: %.2f°\n", theta_t * RAD2DEG);
    
    // 计算俯仰角 ε(t) = tg^(-1)((Z_T - Z_0)/sqrt((X_T - X_0)^2 + (Y_T - Y_0)^2)) (公式4.2.4)
    double r_pt = sqrt(pow(X_T - X_0, 2) + pow(Y_T - Y_0, 2));
    double epsilon_t = atan2(Z_T - Z_0, r_pt);
    g_print("俯仰角: %.2f°\n", epsilon_t * RAD2DEG);
    
    // 计算方位角变化率 θ'(t) = (v_y*sin(θ(t)) - v_x*cos(θ(t)))/r_pt (公式4.2.5)
    double theta_dot_t = (v_y * sin(theta_t) - v_x * cos(theta_t)) / r_pt;
    g_print("方位角变化率: %.2f°/s\n", theta_dot_t * RAD2DEG);
    
    // 计算俯仰角变化率 ε'(t) = (-v_z*cos(ε(t)) + r_pt_dot*sin(ε(t)))/r (公式4.2.6)
    // 其中 r_pt_dot = (X_T - X_0)*sin(θ(t)) + (Y_T - Y_0)*cos(θ(t))
    double r_pt_dot = (X_T - X_0) * sin(theta_t) + (Y_T - Y_0) * cos(theta_t);
    double r_t = sqrt(pow(X_T - X_0, 2) + pow(Y_T - Y_0, 2) + pow(Z_T - Z_0, 2));
    double epsilon_dot_t = (-v_z * cos(epsilon_t) + r_pt_dot * sin(epsilon_t)) / r_t;
    g_print("俯仰角变化率: %.2f°/s\n", epsilon_dot_t * RAD2DEG);
    
    // 计算相位差变化率 (公式4.2.7)
    double delta_phi_dot_t = (2 * M_PI * d * f_T / c) * (
        (v_y * sin(theta_t) - v_x * cos(theta_t)) * cos(theta_t) * cos(epsilon_t) / (r_t * cos(epsilon_t)) -
        (r_pt_dot * sin(epsilon_t) - v_z * cos(epsilon_t)) * sin(theta_t) * sin(epsilon_t) / r_t
    );
    g_print("相位差变化率: %.2f°/s\n", delta_phi_dot_t * RAD2DEG);
    
    // 计算距离 (公式4.2.8)
    // 根据公式4.2.8，r_hat = (Δφ'(t)/fT * c/(2πd))^(-1) * { [y'O sin(θ) - x'O cos(θ)]cos(θ)cos(ε) - [r'pt sin(ε) - z'O cos(ε)]sin(θ)sin(ε) }
    double numerator = (v_y * sin(theta_t) - v_x * cos(theta_t)) * cos(theta_t) * cos(epsilon_t) -
                       (r_pt_dot * sin(epsilon_t) - v_z * cos(epsilon_t)) * sin(theta_t) * sin(epsilon_t);
    double denominator = delta_phi_dot_t * c / (2 * M_PI * d * f_T);
    double r_hat = numerator / denominator;
    g_print("距离: %.2fm\n", r_hat);
    
    // 计算辐射源坐标 (公式4.2.9)
    double X_T_calculated = X_0 + r_hat * cos(epsilon_t) * sin(theta_t);
    double Y_T_calculated = Y_0 + r_hat * cos(epsilon_t) * cos(theta_t);
    double Z_T_calculated = Z_0 + r_hat * sin(epsilon_t);
    g_print("计算得到的辐射源坐标: %.6f, %.6f, %.2f\n", X_T_calculated, Y_T_calculated, Z_T_calculated);
    
    // 将计算得到的辐射源笛卡尔坐标转换回经纬度高度
    COORD3 sourceLBH = xyz2lbh(X_T_calculated, Y_T_calculated, Z_T_calculated);
    g_print("计算得到的辐射源经纬度高度: %.6f°, %.6f°, %.2fm\n", sourceLBH.p1, sourceLBH.p2, sourceLBH.p3);
    
    // 设置结果
    result.longitude = sourceLBH.p1;
    result.latitude = sourceLBH.p2;
    result.altitude = sourceLBH.p3;
    
    // 将弧度转换为角度
    result.azimuth = theta_t * RAD2DEG;
    if (result.azimuth < 0) result.azimuth += 360.0;
    result.elevation = epsilon_t * RAD2DEG;
    
    // 计算误差因素
    result.errorFactors = calculateErrors(device, source, r_hat);
    
    g_print("干涉仪体制定位结果：\n");
    g_print("  方位角: %.2f°, 俯仰角: %.2f°\n", result.azimuth, result.elevation);
    g_print("  经度: %.6f°, 纬度: %.6f°, 高度: %.2fm\n", result.longitude, result.latitude, result.altitude);
    
    return result;
}

// 计算测向数据
std::pair<double, double> InterferometerPositioning::calculateDirectionData(const ReconnaissanceDevice& device, 
                                                                          const RadiationSource& source) {
    // 获取观测站位置（设备当前位置）
    COORD3 deviceXYZ = lbh2xyz(device.getLongitude(), device.getLatitude(), device.getAltitude());
    double X_0 = deviceXYZ.p1;
    double Y_0 = deviceXYZ.p2;
    double Z_0 = deviceXYZ.p3;
    
    // 获取辐射源位置
    COORD3 sourceXYZ = lbh2xyz(source.getLongitude(), source.getLatitude(), source.getAltitude());
    double X_T = sourceXYZ.p1;
    double Y_T = sourceXYZ.p2;
    double Z_T = sourceXYZ.p3;
    
    // 计算方位角
    double theta_t = atan2(X_T - X_0, Y_T - Y_0) * RAD2DEG;
    if (theta_t < 0) theta_t += 360.0;
    
    // 计算俯仰角
    double r_pt = sqrt(pow(X_T - X_0, 2) + pow(Y_T - Y_0, 2));
    double epsilon_t = atan2(Z_T - Z_0, r_pt) * RAD2DEG;
    
    return std::make_pair(theta_t, epsilon_t);
}

// 计算定位数据
std::pair<std::pair<double, double>, double> InterferometerPositioning::calculateLocationData(const ReconnaissanceDevice& device,
                                                                                            double azimuth,
                                                                                            double elevation) {
    // 将角度转换为弧度
    double azimuth_rad = azimuth * DEG2RAD;
    double elevation_rad = elevation * DEG2RAD;
    
    // 获取观测站位置
    double longitude = device.getLongitude();
    double latitude = device.getLatitude();
    double altitude = device.getAltitude();
    
    // 估计距离
    double distance = 100000.0; // 假设距离为100km
    
    // 计算辐射源坐标
    COORD3 deviceXYZ = lbh2xyz(longitude, latitude, altitude);
    double X_0 = deviceXYZ.p1;
    double Y_0 = deviceXYZ.p2;
    double Z_0 = deviceXYZ.p3;
    
    double X_T = X_0 + distance * cos(elevation_rad) * sin(azimuth_rad);
    double Y_T = Y_0 + distance * cos(elevation_rad) * cos(azimuth_rad);
    double Z_T = Z_0 + distance * sin(elevation_rad);
    
    // 将笛卡尔坐标转换为经纬度高度
    COORD3 sourceLBH = xyz2lbh(X_T, Y_T, Z_T);
    
    return std::make_pair(std::make_pair(sourceLBH.p1, sourceLBH.p2), sourceLBH.p3);
}

// 计算误差因素
std::vector<double> InterferometerPositioning::calculateErrors(const ReconnaissanceDevice& device,
                                                             const RadiationSource& source,
                                                             double distance) {
    std::vector<double> errors;
    
    // 1. 对中误差 - 基线垂直于信号到达方向时最小
    double baseline_error = 0.1 * (1 + 0.05 * distance / 1000.0);
    errors.push_back(baseline_error);
    
    // 2. 姿态测量误差 - 与设备的稳定性和姿态传感器精度有关
    double attitude_error = 0.2 * (1 + 0.03 * device.getMovementSpeed() / 10.0);
    errors.push_back(attitude_error);
    
    // 3. 圆锥效应误差 - 与俯仰角和基线夹角有关
    // 模拟圆锥效应误差
    double cone_error = 0.15 * (1 + fabs(sin(device.getMovementElevation() * DEG2RAD)));
    errors.push_back(cone_error);
    
    // 4. 天线阵测向误差 - 与天线数量、间距和频率有关
    double antenna_error = 0.25 * (1 - 0.8 * device.getBaselineLength() / 10.0);
    if (antenna_error < 0.05) antenna_error = 0.05;
    errors.push_back(antenna_error);
    
    // 5. 综合测向误差 - 以上各项误差的RSS（平方根和）
    double total_error = sqrt(pow(baseline_error, 2) + pow(attitude_error, 2) + 
                             pow(cone_error, 2) + pow(antenna_error, 2));
    errors.push_back(total_error);
    
    return errors;
} 