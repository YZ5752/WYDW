#include "../InterferometerPositioning.h"
#include "../../constants/PhysicsConstants.h"
#include "../../utils/CoordinateTransform.h"
#include "../../utils/SNRValidator.h"
#include "../SinglePlatformTaskDAO.h"
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
    
    //获取辐射源位置（固定）
    COORD3 sourceXYZ = lbh2xyz(source.getLongitude(), source.getLatitude(), source.getAltitude());
    double X_T = sourceXYZ.p1;
    double Y_T = sourceXYZ.p2;
    double Z_T = sourceXYZ.p3;
    g_print("辐射源位置: %.6f°, %.6f°, %.2fm\n", source.getLongitude(), source.getLatitude(), source.getAltitude());
    g_print("辐射源位置: %.6f, %.6f, %.2f\n", X_T, Y_T, Z_T);

    // //假设
    //     double X_T = 1000;
    // double Y_T = 500;
    // double Z_T = 300;
    //     g_print("假设辐射源位置: %.6f, %.6f, %.2f\n", X_T, Y_T, Z_T);
    
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

    // //假设
    //  double v_x = 50;
    // double v_y = 0;
    // double v_z = 0;
    // g_print("假设观测站速度: %.2fm/s, %.2fm/s, %.2fm/s\n", v_x, v_y, v_z);

    // 获取基线长度
    double d = device.getBaselineLength();
    g_print("基线长度: %.2fm\n", d);
    
    // 获取辐射源频率（GHz转换为Hz）
    double f_T = source.getCarrierFrequency() * 1e9;
    g_print("辐射源频率: %.2fHz\n", f_T);
    
    // 计算观测站运动后的位置
    // 假设仿真时间内匀速运动
    double X_0_moved = X_0 + v_x * simulationTime;
    double Y_0_moved = Y_0 + v_y * simulationTime;
    double Z_0_moved = Z_0 + v_z * simulationTime;
    g_print("观测站运动后位置: %.6f, %.6f, %.2f\n", X_0_moved, Y_0_moved, Z_0_moved);

// //假设
//         double X_0_moved = 50;
//     double Y_0_moved =0;
//     double Z_0_moved =0;
//         g_print("假设观测站运动后位置: %.6f, %.6f, %.2f\n", X_0_moved, Y_0_moved, Z_0_moved);
    
    // 计算方位角 θ(t) = tg^(-1)((X_T - X_0_moved)/(Y_T - Y_0_moved)) (公式4.2.3修改版)
    double theta_t = atan2(X_T - X_0_moved, Y_T - Y_0_moved);
    g_print("方位角(运动后): %.1f°\n", theta_t * RAD2DEG);
    
    // 计算俯仰角 ε(t) = tg^(-1)((Z_T - Z_0_moved)/sqrt((X_T - X_0_moved)^2 + (Y_T - Y_0_moved)^2)) (公式4.2.4修改版)
    double r_pt = sqrt(pow(X_T - X_0_moved, 2) + pow(Y_T - Y_0_moved, 2));
    double epsilon_t = atan2(Z_T - Z_0_moved, r_pt);
    g_print("俯仰角(运动后): %.1f°\n", epsilon_t * RAD2DEG);
    
    // 计算方位角变化率 θ'(t) = (v_y*sin(θ(t)) - v_x*cos(θ(t)))/r_pt (公式4.2.5)
    double theta_dot_t = (v_y * sin(theta_t) - v_x * cos(theta_t)) / r_pt;
    g_print("方位角变化率: %.2f°/s\n", theta_dot_t * RAD2DEG);
    
    // 计算俯仰角变化率 ε'(t) = (-v_z*cos(ε(t)) + r_pt_dot*sin(ε(t)))/r (公式4.2.6)
    // 其中 r_pt_dot = (X_T - X_0_moved)*sin(θ(t)) + (Y_T - Y_0_moved)*cos(θ(t))
    double r_pt_dot = (X_T - X_0_moved) * sin(theta_t) + (Y_T - Y_0_moved) * cos(theta_t);
    double r_t = sqrt(pow(X_T - X_0_moved, 2) + pow(Y_T - Y_0_moved, 2) + pow(Z_T - Z_0_moved, 2));
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
    
    // 计算辐射源坐标 (公式4.2.9，基于运动后的位置)
    double X_T_calculated = X_0_moved + r_hat * cos(epsilon_t) * sin(theta_t);
    double Y_T_calculated = Y_0_moved + r_hat * cos(epsilon_t) * cos(theta_t);
    double Z_T_calculated = Z_0_moved + r_hat * sin(epsilon_t);
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
    
    // 计算最大定位距离
    double maxDetectionRange = calculateMaxDetectionRange(
        source.getTransmitPower(),           // 发射功率（千瓦）
        source.getCarrierFrequency(),        // 载波频率（GHz）
        device.getNoisePsd(),                // 噪声功率谱密度（dBm/Hz）
        device.getFreqRangeMax() - device.getFreqRangeMin()  // 带宽（GHz）
    );
    
    // 计算定位精度（使用综合测向误差作为定位精度）
    double positioningAccuracy = 0.0;
    if (!result.errorFactors.empty()) {
        positioningAccuracy = result.errorFactors.back(); // 最后一个元素是综合测向误差
    }
    
    // 计算测向精度（使用综合测向误差）
    double directionFindingAccuracy = positioningAccuracy;
    
    // 保存结果到数据库
    SinglePlatformTask task;
    task.techSystem = "INTERFEROMETER";
    task.deviceId = device.getDeviceId();
    task.radiationId = source.getRadiationId();
    task.executionTime = static_cast<float>(simulationTime);
    task.targetLongitude = result.longitude;
    task.targetLatitude = result.latitude;
    task.targetAltitude = result.altitude;
    task.targetAngle = result.azimuth;
    task.angleError = directionFindingAccuracy;
    task.maxPositioningDistance = static_cast<float>(maxDetectionRange);
    task.positioningTime = static_cast<float>(simulationTime); // 假设定位时间等于仿真时间
    task.positioningAccuracy = positioningAccuracy;
    task.directionFindingAccuracy = directionFindingAccuracy;
    
    int taskId;
    if (SinglePlatformTaskDAO::getInstance().addSinglePlatformTask(task, taskId)) {
        g_print("单平台干涉仪定位结果已保存到数据库，任务ID: %d\n", taskId);
        g_print("最大定位距离: %.2fm\n", maxDetectionRange);
        g_print("定位精度: %.6fm\n", positioningAccuracy);
        g_print("测向精度: %.6f°\n", directionFindingAccuracy);
    } else {
        g_print("警告：保存单平台干涉仪定位结果到数据库失败\n");
    }
    
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
    
    // 获取观测站速度
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
    
    // 将笛卡尔坐标系中的速度分量转换回大地坐标系中的速度、方位角和俯仰角
    COORD3 velocityLBH = velocity_xyz2lbh(
        device.getLongitude(),
        device.getLatitude(),
        v_x, v_y, v_z
    );
    g_print("测向计算中 - 从笛卡尔坐标转换回大地坐标系的运动参数:\n");
    g_print("  - 速度: %.2fm/s, 方位角: %.2f°, 俯仰角: %.2f°\n", 
           velocityLBH.p1, velocityLBH.p2, velocityLBH.p3);
    
    // 计算观测站运动后的位置（假设运动1秒）
    double X_0_moved = X_0 + v_x * 1.0;
    double Y_0_moved = Y_0 + v_y * 1.0;
    double Z_0_moved = Z_0 + v_z * 1.0;
    
    // 计算方位角（基于运动后位置）
    double theta_t = atan2(X_T - X_0_moved, Y_T - Y_0_moved) * RAD2DEG;
    if (theta_t < 0) theta_t += 360.0;
    
    // 计算俯仰角（基于运动后位置）
    double r_pt = sqrt(pow(X_T - X_0_moved, 2) + pow(Y_T - Y_0_moved, 2));
    double epsilon_t = atan2(Z_T - Z_0_moved, r_pt) * RAD2DEG;
    
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
    
    // 获取观测站速度
    COORD3 velocityXYZ = velocity_lbh2xyz(
        longitude, 
        latitude, 
        device.getMovementSpeed(), 
        device.getMovementAzimuth(), 
        device.getMovementElevation()
    );
    double v_x = velocityXYZ.p1;
    double v_y = velocityXYZ.p2;
    double v_z = velocityXYZ.p3;
    
    // 将笛卡尔坐标系中的速度分量转换回大地坐标系中的速度、方位角和俯仰角
    COORD3 velocityLBH = velocity_xyz2lbh(
        longitude,
        latitude,
        v_x, v_y, v_z
    );
    g_print("定位计算中 - 从笛卡尔坐标转换回大地坐标系的运动参数:\n");
    g_print("  - 速度: %.2fm/s, 方位角: %.2f°, 俯仰角: %.2f°\n", 
           velocityLBH.p1, velocityLBH.p2, velocityLBH.p3);
    
    // 估计距离
    double distance = 100000.0; // 假设距离为100km
    
    // 计算观测站当前位置
    COORD3 deviceXYZ = lbh2xyz(longitude, latitude, altitude);
    double X_0 = deviceXYZ.p1;
    double Y_0 = deviceXYZ.p2;
    double Z_0 = deviceXYZ.p3;
    
    // 计算观测站运动后的位置（假设运动1秒）
    double X_0_moved = X_0 + v_x * 1.0;
    double Y_0_moved = Y_0 + v_y * 1.0;
    double Z_0_moved = Z_0 + v_z * 1.0;
    
    // 基于运动后的位置计算辐射源坐标
    double X_T = X_0_moved + distance * cos(elevation_rad) * sin(azimuth_rad);
    double Y_T = Y_0_moved + distance * cos(elevation_rad) * cos(azimuth_rad);
    double Z_T = Z_0_moved + distance * sin(elevation_rad);
    
    // 将笛卡尔坐标转换为经纬度高度
    COORD3 sourceLBH = xyz2lbh(X_T, Y_T, Z_T);
    
    return std::make_pair(std::make_pair(sourceLBH.p1, sourceLBH.p2), sourceLBH.p3);
}

// 计算误差因素
std::vector<double> InterferometerPositioning::calculateErrors(const ReconnaissanceDevice& device,
                                                             const RadiationSource& source,
                                                             double distance) {
    std::vector<double> errors;
    
    // 获取基线长度（单位：米）
    double d = device.getBaselineLength();
    if (d < 0.001) d = 0.001;  // 防止基线长度太小
    
    // 获取辐射源波长（单位：米）
    double lambda = c / (source.getCarrierFrequency() * 1e9);  // 频率从GHz转换为Hz
    
    // 获取入射角（方位角和俯仰角）
    auto [theta, elevation] = calculateDirectionData(device, source);
    double theta_rad = theta * DEG2RAD;  // 转换为弧度
    
    // 1. 对中误差 Δem（度）
    double delta_em = INTERFEROMETER_ALIGNMENT_ERROR;
    errors.push_back(delta_em);
    
    // 2. 惯导测量精度 σα（度）
    double sigma_alpha = INTERFEROMETER_ATTITUDE_ERROR;
    errors.push_back(sigma_alpha);
    
    // 3. 圆锥效应误差 σβ（度）
    double beta = std::abs(elevation);  // 使用仰角绝对值
    double alpha = std::abs(theta);     // 使用方位角绝对值
    
    // 将方位角归一化到[0, 90]度范围内（利用圆锥效应的对称性）
    alpha = fmod(alpha, 180.0);
    if (alpha > 90.0) {
        alpha = 180.0 - alpha;
    }
    
    // 查表计算圆锥效应误差
    double sigma_beta = 0.0;
    
    // 处理超出表格范围的情况
    // 如果仰角超出最大范围，使用最大仰角对应的值
    double beta_to_use = beta;
    if (beta > CONE_EFFECT_BETA_BOUNDS[CONE_EFFECT_BETA_LEVELS-1]) {
        beta_to_use = CONE_EFFECT_BETA_BOUNDS[CONE_EFFECT_BETA_LEVELS-1];
    }
    
    // 如果方位角超出最大范围，使用最大方位角对应的值
    double alpha_to_use = alpha;
    if (alpha > CONE_EFFECT_ALPHA_BOUNDS[CONE_EFFECT_ALPHA_LEVELS-1]) {
        alpha_to_use = CONE_EFFECT_ALPHA_BOUNDS[CONE_EFFECT_ALPHA_LEVELS-1];
    }
    
    // 找到对应的区间
    int beta_index = -1;
    for (int i = 0; i < CONE_EFFECT_BETA_LEVELS; i++) {
        if (beta_to_use <= CONE_EFFECT_BETA_BOUNDS[i]) {
            beta_index = i;
            break;
        }
    }
    
    int alpha_index = -1;
    for (int i = 0; i < CONE_EFFECT_ALPHA_LEVELS; i++) {
        if (alpha_to_use <= CONE_EFFECT_ALPHA_BOUNDS[i]) {
            alpha_index = i;
            break;
        }
    }
    
    // 获取误差值
    if (beta_index >= 0 && alpha_index >= 0) {
        sigma_beta = CONE_EFFECT_ERROR_TABLE[beta_index][alpha_index];
        
        // 对于超出范围的角度，根据角度大小按比例增加误差
        // 这是一种简单的近似处理方法
        if (beta > CONE_EFFECT_BETA_BOUNDS[CONE_EFFECT_BETA_LEVELS-1] || 
            alpha > CONE_EFFECT_ALPHA_BOUNDS[CONE_EFFECT_ALPHA_LEVELS-1]) {
            
            // 计算比例因子：实际角度与表格最大角度的比值
            double scale_factor = 1.0;
            
            if (beta > CONE_EFFECT_BETA_BOUNDS[CONE_EFFECT_BETA_LEVELS-1]) {
                double beta_ratio = beta / CONE_EFFECT_BETA_BOUNDS[CONE_EFFECT_BETA_LEVELS-1];
                // 限制比例因子增长
                beta_ratio = std::min(beta_ratio, 2.0);
                scale_factor *= beta_ratio;
            }
            
            if (alpha > CONE_EFFECT_ALPHA_BOUNDS[CONE_EFFECT_ALPHA_LEVELS-1]) {
                double alpha_ratio = alpha / CONE_EFFECT_ALPHA_BOUNDS[CONE_EFFECT_ALPHA_LEVELS-1];
                // 限制比例因子增长
                alpha_ratio = std::min(alpha_ratio, 2.0);
                scale_factor *= alpha_ratio;
            }
            
            // 应用比例因子
            sigma_beta *= scale_factor;
        }
    }
    errors.push_back(sigma_beta);
    
    // 4. 天线阵测向误差 σθ（度）
    // 使用公式：σθ = (λ/(2πd*cos(θ))) * σφ
    double cos_theta = cos(theta_rad);
    if (std::abs(cos_theta) < 1e-6) {  // 防止除以零
        cos_theta = 1e-6;
    }
    
    double sigma_phi_rad = INTERFEROMETER_PHASE_ERROR * DEG2RAD;  // 转换为弧度
    double sigma_theta = (lambda / (2 * M_PI * d * cos_theta)) * sigma_phi_rad;

    
    // 添加详细的中间计算值输出
    g_print("天线阵测向误差计算中间值:\n");
    g_print("  cos(theta): %.10f\n", cos_theta);
    g_print("  lambda: %.10f m\n", lambda);
    g_print("  基线长度(d): %.4f m\n", d);
    g_print("  sigma_phi_rad: %.10f rad\n", sigma_phi_rad);
    g_print("  分母(2*PI*d*cos_theta): %.10f\n", 2 * M_PI * d * cos_theta);
    g_print("  sigma_theta_rad: %.10f rad\n", sigma_theta);

    // 5. 综合测向误差 Δθ（度）
    double total_error = sqrt(pow(sigma_alpha, 2) + pow(sigma_beta, 2) + 
                            pow(sigma_theta, 2) + pow(delta_em, 2));
        sigma_theta *= RAD2DEG;  // 转换回度
    errors.push_back(sigma_theta);
    errors.push_back(total_error);
    
    // 打印调试信息
    g_print("误差计算结果：\n");
    g_print("  对中误差: %.4f°\n", delta_em);
    g_print("  惯导测量精度: %.4f°\n", sigma_alpha);
    g_print("  圆锥效应误差: %.4f°\n", sigma_beta);
    g_print("  天线阵测向误差: %.4f°\n", sigma_theta);
    g_print("  综合测向误差: %.4f°\n", total_error);
    g_print("计算参数：\n");
    g_print("  基线长度: %.4f m\n", d);
    g_print("  波长: %.4f m\n", lambda);
    g_print("  方位角: %.4f°\n", theta);
    g_print("  俯仰角: %.4f°\n", elevation);
    
    return errors;
} 