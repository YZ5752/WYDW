#include "../BaselineAalgorithm.h"
#include "../InterferometerPositioning.h" // for LocationResult
#include "../../utils/DirectionErrorUtils.h"
#include "../../utils/CoordinateTransform.h"
#include "../../constants/PhysicsConstants.h"
#include "../../utils/SimulationValidator.h"
#include "../../utils/SNRValidator.h"
#include "../SinglePlatformTaskDAO.h"
#include <cmath>
#include <algorithm>

using namespace Constants;

BaselineAalgorithm& BaselineAalgorithm::getInstance() {
    static BaselineAalgorithm instance;
    return instance;
}

LocationResult BaselineAalgorithm::runSimulation(const ReconnaissanceDevice& device,
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

    // 基线长度：使用运动距离 d = v * t
    double v = device.getMovementSpeed();
    double d = std::max(1e-3, static_cast<double>(v * simulationTime));

    // 计算设备移动后位置
    COORD3 p1 = lbh2xyz(device.getLongitude(), device.getLatitude(), device.getAltitude());
    COORD3 vxyz = velocity_lbh2xyz(device.getLongitude(), device.getLatitude(), v,
                                   device.getMovementAzimuth(), device.getMovementElevation());
    COORD3 p2_xyz{p1.p1 + vxyz.p1 * simulationTime,
                  p1.p2 + vxyz.p2 * simulationTime,
                  p1.p3 + vxyz.p3 * simulationTime};

    // 中点、基线方向
    double midX = (p1.p1 + p2_xyz.p1) / 2.0;
    double midY = (p1.p2 + p2_xyz.p2) / 2.0;
    double midZ = (p1.p3 + p2_xyz.p3) / 2.0;
    double bx = p2_xyz.p1 - p1.p1;
    double by = p2_xyz.p2 - p1.p2;
    double bz = p2_xyz.p3 - p1.p3;
    double bnorm = std::sqrt(bx*bx + by*by + bz*bz);
    bx /= bnorm; by /= bnorm; bz /= bnorm;

    // 理论几何定义：使用统一的 TDOA 入射角定义
    COORD3 src = lbh2xyz(source.getLongitude(), source.getLatitude(), source.getAltitude());
    // baseline unit
    double bxu = bx, byu = by, bzu = bz; // 已归一
    // mid LOS
    double lox = src.p1 - midX, loy = src.p2 - midY, loz = src.p3 - midZ;
    double lonorm = std::sqrt(lox*lox + loy*loy + loz*loz); if (lonorm < 1e-9) lonorm = 1e-9;
    lox/=lonorm; loy/=lonorm; loz/=lonorm;
    double theta = std::acos(std::max(-1.0, std::min(1.0, bxu*lox + byu*loy + bzu*loz)));

    // 测向误差计算：使用设备的天线基线长度，而不是运动基线
    // 运动基线只参与定位算法的几何计算，测向误差与天线基线长度相关
    double antennaBaseline = std::max(1e-3, static_cast<double>(device.getBaselineLength()));
    double estimatedDistance = lonorm; // 与 FastAalgorithm 统一：使用中点到目标的距离
    
    // 计算理论方位角和俯仰角（用于测向误差计算）
    double theta_deg = std::atan2(src.p1 - midX, src.p2 - midY) * RAD2DEG; if (theta_deg < 0) theta_deg += 360.0;
    double eps_deg = std::atan2(src.p3 - midZ, std::sqrt((src.p1 - midX)*(src.p1 - midX) + (src.p2 - midY)*(src.p2 - midY))) * RAD2DEG;
    
    std::vector<double> errors;
    if (techSystem == "INTERFEROMETER") {
        // 干涉仪体制：使用天线基线长度计算测向误差
        errors = DirectionErrorUtils::calculateInterferometerErrors(device, source, theta_deg, eps_deg);
    } else {
        // TDOA体制：使用天线基线长度计算测向误差
        // 入射角应为基线方向与视线方向的夹角 theta，而非方位角
        double theta_rad = theta;
        // 计算SNR
        double pt = source.getTransmitPower();  // 发射功率 (kW)
        double fc = source.getCarrierFrequency();  // 载波频率 (GHz)
        double N0_W = device.getNoisePsd();  // W/Hz
        double N0_dBm = 10.0 * std::log10(N0_W * 1000.0);  // 转换为dBm/Hz
        double B_GHz = device.getSampleRate() * 0.5;  // 设备采样速率单位为GHz，无需再除以1e9
        double snr_dB = calculateSNR(estimatedDistance, pt, fc, N0_dBm, B_GHz);
        double snr_linear = std::pow(10.0, snr_dB / 10.0);
        snr_linear = std::max(1.0, std::min(1000.0, snr_linear));
        
        errors = DirectionErrorUtils::calculateTDOAErrors(antennaBaseline, theta_rad, estimatedDistance, 
                                                        source.getCarrierFrequency() * 1e9,  // 载波频率(Hz)
                                                        0.0,  // 占位，将被动态计算覆盖
                                                        0.0,  // 占位，将被动态计算覆盖
                                                        0.0,  // 占位，将被动态计算覆盖
                                                        device.getSampleRate() * 1e9);  // 采样率(Hz)
        
        // 动态计算TDOA参数，根据侦察设备和辐射源的实际参数
        auto tdoaParams = DirectionErrorUtils::calculateTDOAParams(device, source, estimatedDistance);
        
        // 重新计算误差因素，使用动态计算的参数
        errors = DirectionErrorUtils::calculateTDOAErrors(antennaBaseline, theta_rad, estimatedDistance, 
                                                        source.getCarrierFrequency() * 1e9,  // 载波频率(Hz)
                                                        tdoaParams.phaseErrorDeg,            // 动态计算的相位误差
                                                        tdoaParams.bandwidthHz,              // 动态计算的带宽
                                                        tdoaParams.snrLinear,                // 动态计算的SNR
                                                        tdoaParams.samplingRateHz);          // 动态计算的采样率
    }
    double dirErrorDeg = errors.size() >= 5 ? errors[4] : 0.0;

    // 应用测向误差：与快速定位算法保持一致，使用简单的角度叠加
    double theta_est = theta_deg + 0.5 * dirErrorDeg;
    double eps_est = eps_deg + 0.5 * dirErrorDeg;

    // 距离估计：使用中点到目标的距离
    double r_est = estimatedDistance;

    // 构造方向向量
    double dirX = std::sin(theta_est * DEG2RAD) * std::cos(eps_est * DEG2RAD);
    double dirY = std::cos(theta_est * DEG2RAD) * std::cos(eps_est * DEG2RAD);
    double dirZ = std::sin(eps_est * DEG2RAD);

    // 估计目标坐标（以中点为参考沿 dir 前进 r_est）
    double X_T = midX + r_est * dirX;
    double Y_T = midY + r_est * dirY;
    double Z_T = midZ + r_est * dirZ;
    COORD3 lbh = xyz2lbh(X_T, Y_T, Z_T);

    LocationResult out{};
    out.azimuth = theta_est;
    out.elevation = eps_est;
    out.longitude = lbh.p1;
    out.latitude = lbh.p2;
    out.altitude = lbh.p3;
    out.errorFactors = errors;
    
    // 计算实际定位误差：计算位置与真实位置之间的距离
    COORD3 calculatedXYZ = lbh2xyz(out.longitude, out.latitude, out.altitude);
    COORD3 trueSourceXYZ = lbh2xyz(source.getLongitude(), source.getLatitude(), source.getAltitude());
    
    double actualPositionError = std::sqrt(
        std::pow(calculatedXYZ.p1 - trueSourceXYZ.p1, 2) +
        std::pow(calculatedXYZ.p2 - trueSourceXYZ.p2, 2) +
        std::pow(calculatedXYZ.p3 - trueSourceXYZ.p3, 2)
    );
    
    out.accuracy = actualPositionError;

    // 保存任务
    SinglePlatformTask task;
    task.positioningAlgorithm = "BASELINE";
    task.deviceId = device.getDeviceId();
    task.radiationId = source.getRadiationId();
    task.executionTime = static_cast<float>(simulationTime);
    task.targetLongitude = out.longitude;
    task.targetLatitude = out.latitude;
    task.targetAltitude = out.altitude;
    task.azimuth = out.azimuth;
    task.elevation = out.elevation;
    task.angleError = dirErrorDeg;
    task.positioningDistance = static_cast<float>(r_est);
    task.positioningTime = static_cast<float>(simulationTime);
    task.positioningAccuracy = out.accuracy;
    task.directionFindingAccuracy = dirErrorDeg;
    int taskId;
    SinglePlatformTaskDAO::getInstance().addSinglePlatformTask(task, taskId);

    return out;
}


