#include "../FastAalgorithm.h"
#include "../InterferometerPositioning.h" // for LocationResult struct
#include "../../utils/DirectionErrorUtils.h"
#include "../../utils/CoordinateTransform.h"
#include "../../constants/PhysicsConstants.h"
#include "../SinglePlatformTaskDAO.h"
#include "../../utils/SimulationValidator.h"
#include "../../utils/SNRValidator.h"
#include <cmath>
#include <algorithm>

using namespace Constants;

FastAalgorithm& FastAalgorithm::getInstance() {
    static FastAalgorithm instance;
    return instance;
}

LocationResult FastAalgorithm::runSimulation(const ReconnaissanceDevice& device,
                                             const RadiationSource& source,
                                             int simulationTime,
                                             const std::string& techSystem) {
    LocationResult result{};

    // 验证
    SimulationValidator validator;
    std::vector<int> deviceIds = {device.getDeviceId()};
    std::string failMessage;
    if (!validator.validateAll(deviceIds, source.getRadiationId(), failMessage)) {
        result.accuracy = -1.0;
        return result;
    }

    // 基线长度：使用设备的两个天线间距
    double d = std::max(1e-3, static_cast<double>(device.getBaselineLength()));

    // 计算设备-目标几何（使用当前时刻）
    COORD3 deviceXYZ = lbh2xyz(device.getLongitude(), device.getLatitude(), device.getAltitude());
    COORD3 sourceXYZ = lbh2xyz(source.getLongitude(), source.getLatitude(), source.getAltitude());

    // 理论方位、俯仰
    double dx = sourceXYZ.p1 - deviceXYZ.p1;
    double dy = sourceXYZ.p2 - deviceXYZ.p2;
    double dz = sourceXYZ.p3 - deviceXYZ.p3;
    double r_pt = std::sqrt(dx*dx + dy*dy);
    double r = std::sqrt(dx*dx + dy*dy + dz*dz);
    double theta_deg = std::atan2(dx, dy) * RAD2DEG; if (theta_deg < 0) theta_deg += 360.0;
    double eps_deg = std::atan2(dz, r_pt) * RAD2DEG;

   
    std::vector<double> errors;
    if (techSystem == "INTERFEROMETER") {
        errors = DirectionErrorUtils::calculateInterferometerErrors(device, source, theta_deg, eps_deg);
    } else {
        // TDOA体制：使用天线基线长度计算测向误差，而不是运动基线
        double antennaBaseline = std::max(1e-3, static_cast<double>(device.getBaselineLength()));

        // 统一入射角定义：与基线算法完全一致
        // 1) 根据速度和仿真时长构造运动基线两端点
        double v = device.getMovementSpeed();
        COORD3 vxyz = velocity_lbh2xyz(device.getLongitude(), device.getLatitude(), v,
                                       device.getMovementAzimuth(), device.getMovementElevation());
        COORD3 p1 = deviceXYZ;
        COORD3 p2_xyz{p1.p1 + vxyz.p1 * simulationTime,
                      p1.p2 + vxyz.p2 * simulationTime,
                      p1.p3 + vxyz.p3 * simulationTime};
        // 2) 基线单位向量
        double bx = p2_xyz.p1 - p1.p1;
        double by = p2_xyz.p2 - p1.p2;
        double bz = p2_xyz.p3 - p1.p3;
        double bnorm = std::sqrt(bx*bx + by*by + bz*bz); if (bnorm < 1e-9) bnorm = 1e-9;
        bx/=bnorm; by/=bnorm; bz/=bnorm;
        // 3) 中点与视线方向（从中点指向目标）
        double midX = (p1.p1 + p2_xyz.p1) / 2.0;
        double midY = (p1.p2 + p2_xyz.p2) / 2.0;
        double midZ = (p1.p3 + p2_xyz.p3) / 2.0;
        double lox = (sourceXYZ.p1 - midX);
        double loy = (sourceXYZ.p2 - midY);
        double loz = (sourceXYZ.p3 - midZ);
        double lonorm = std::sqrt(lox*lox + loy*loy + loz*loz); if (lonorm < 1e-9) lonorm = 1e-9;
        lox/=lonorm; loy/=lonorm; loz/=lonorm;
        // 4) 夹角
        double dotbl = std::max(-1.0, std::min(1.0, bx*lox + by*loy + bz*loz));
        double theta_rad = std::acos(dotbl);

        double estR = r; // 使用设备到目标的距离
        
        // 计算SNR
        double pt = source.getTransmitPower();  // 发射功率 (kW)
        double fc = source.getCarrierFrequency();  // 载波频率 (GHz)
        double N0_W = device.getNoisePsd();  // W/Hz
        double N0_dBm = 10.0 * std::log10(N0_W * 1000.0);  // 转换为dBm/Hz
        double B_GHz = device.getSampleRate() * 0.5;  // 设备采样速率单位为GHz
        double snr_dB = calculateSNR(estR, pt, fc, N0_dBm, B_GHz);
        double snr_linear = std::pow(10.0, snr_dB / 10.0);
        snr_linear = std::max(1.0, std::min(1000.0, snr_linear));
        
        errors = DirectionErrorUtils::calculateTDOAErrors(antennaBaseline, theta_rad, estR, 
                                                        source.getCarrierFrequency() * 1e9,  // 载波频率(Hz)
                                                        0.0,  // 占位，将被动态计算覆盖
                                                        0.0,  // 占位，将被动态计算覆盖
                                                        0.0,  // 占位，将被动态计算覆盖
                                                        device.getSampleRate() * 1e9);  // 采样率(Hz)
        
        // 动态计算TDOA参数，根据侦察设备和辐射源的实际参数
        auto tdoaParams = DirectionErrorUtils::calculateTDOAParams(device, source, estR);
        
        // 重新计算误差因素，使用动态计算的参数
        errors = DirectionErrorUtils::calculateTDOAErrors(antennaBaseline, theta_rad, estR, 
                                                        source.getCarrierFrequency() * 1e9,  // 载波频率(Hz)
                                                        tdoaParams.phaseErrorDeg,            // 动态计算的相位误差
                                                        tdoaParams.bandwidthHz,              // 动态计算的带宽
                                                        tdoaParams.snrLinear,                // 动态计算的SNR
                                                        tdoaParams.samplingRateHz);          // 动态计算的采样率
    }
    double dirErrorDeg = errors.size() >= 5 ? errors[4] : 0.0;

    // 应用测向误差
    double theta_est = theta_deg + 0.5 * dirErrorDeg;
    double eps_est = eps_deg + 0.5 * dirErrorDeg;

    // 距离估计：在仿真环境中使用几何真值作为量级（避免由角误差反推距离导致的不稳定）
    double r_est = r;

    // 构造方向向量
    double dirX = std::sin(theta_est * DEG2RAD) * std::cos(eps_est * DEG2RAD);
    double dirY = std::cos(theta_est * DEG2RAD) * std::cos(eps_est * DEG2RAD);
    double dirZ = std::sin(eps_est * DEG2RAD);

    // 估计目标坐标
    double X_T = deviceXYZ.p1 + r_est * dirX;
    double Y_T = deviceXYZ.p2 + r_est * dirY;
    double Z_T = deviceXYZ.p3 + r_est * dirZ;

    COORD3 lbh = xyz2lbh(X_T, Y_T, Z_T);

    result.azimuth = theta_est;
    result.elevation = eps_est;
    result.longitude = lbh.p1;
    result.latitude = lbh.p2;
    result.altitude = lbh.p3;
    result.errorFactors = errors;

    // 计算实际定位误差：计算位置与真实位置之间的距离
    COORD3 calculatedXYZ = lbh2xyz(result.longitude, result.latitude, result.altitude);
    COORD3 trueSourceXYZ = lbh2xyz(source.getLongitude(), source.getLatitude(), source.getAltitude());
    
    double actualPositionError = std::sqrt(
        std::pow(calculatedXYZ.p1 - trueSourceXYZ.p1, 2) +
        std::pow(calculatedXYZ.p2 - trueSourceXYZ.p2, 2) +
        std::pow(calculatedXYZ.p3 - trueSourceXYZ.p3, 2)
    );
    
    result.accuracy = actualPositionError;

    // 保存任务
    SinglePlatformTask task;
    task.positioningAlgorithm = "FAST";
    task.deviceId = device.getDeviceId();
    task.radiationId = source.getRadiationId();
    task.executionTime = static_cast<float>(simulationTime);
    task.targetLongitude = result.longitude;
    task.targetLatitude = result.latitude;
    task.targetAltitude = result.altitude;
    task.azimuth = result.azimuth;
    task.elevation = result.elevation;
    task.angleError = dirErrorDeg;
    task.positioningDistance = static_cast<float>(r_est);
    task.positioningTime = static_cast<float>(simulationTime);
    task.positioningAccuracy = result.accuracy;
    task.directionFindingAccuracy = dirErrorDeg;
    int taskId;
    SinglePlatformTaskDAO::getInstance().addSinglePlatformTask(task, taskId);

    return result;
}


