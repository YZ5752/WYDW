#include "../constants/PhysicsConstants.h"
#include "DirectionFinding.h"
#include "../utils/DirectionErrorUtils.h"
#include "../utils/SimulationValidator.h"
#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>
#include <string>
#include <gtk/gtk.h>
#include "ReconnaissanceDeviceDAO.h"
#include "RadiationSourceDAO.h"

    // double esm1MeanError = 3.0;
    // double esm1StdDev = 1.0;
    // double esm2MeanError = 3.0;
    // double esm2StdDev = 1.0;
    double error = 0.0;

DirectionFinding& DirectionFinding::getInstance() {
    static DirectionFinding instance;
    return instance;
}

DirectionFinding::DirectionFinding() : m_isInitialized(false), m_simulationTime(0) {}
DirectionFinding::~DirectionFinding() = default;

void DirectionFinding::init(const std::vector<std::string>& deviceNames, const std::string& sourceName, double simulationTime) {
    m_deviceNames = deviceNames;
    m_sourceName = sourceName;
    m_simulationTime = simulationTime;
    m_devices.clear();
    m_directionLines.clear();
    m_deviceErrors.clear();
    m_isInitialized = false;
    m_result = Result{};
}

bool DirectionFinding::loadDeviceInfo() {
    m_devices.clear();
    ReconnaissanceDeviceDAO& deviceDAO = ReconnaissanceDeviceDAO::getInstance();
    auto allDevices = deviceDAO.getAllReconnaissanceDevices();
    for (const auto& name : m_deviceNames) {
        auto it = std::find_if(allDevices.begin(), allDevices.end(), [&](const ReconnaissanceDevice& d) {
            return d.getDeviceName() == name;
        });
        if (it != allDevices.end()) {
            m_devices.push_back(*it);
        } else {
            std::cerr << "找不到侦察设备: " << name << std::endl;
            return false;
        }
    }
    return m_devices.size() >= 2;
}

bool DirectionFinding::loadSourceInfo() {
    RadiationSourceDAO& sourceDAO = RadiationSourceDAO::getInstance();
    auto allSources = sourceDAO.getAllRadiationSources();
    auto it = std::find_if(allSources.begin(), allSources.end(), [&](const RadiationSource& s) {
        return s.getRadiationName() == m_sourceName;
    });
    if (it != allSources.end()) {
        m_source = *it;
        return true;
    } else {
        std::cerr << "找不到辐射源: " << m_sourceName << std::endl;
        return false;
    }
}

std::vector<int> DirectionFinding::getDeviceIds() const {
    std::vector<int> ids;
    for (const auto& d : m_devices) ids.push_back(d.getDeviceId());
    return ids;
}

int DirectionFinding::getSourceId() const { return m_source.getRadiationId(); }

// 移除无参数的calculate方法，只保留带参数的版本
bool DirectionFinding::calculate(double dev1MeanError, double dev1StdDev,
                             double dev2MeanError, double dev2StdDev) {
    if (!loadDeviceInfo() || !loadSourceInfo()) return false;

    // 仿真前验证
    SimulationValidator validator;
    std::vector<int> deviceIds;
    for (const auto& device : m_devices) {
        deviceIds.push_back(device.getDeviceId());
    }
    std::string failMessage;

    if (!validator.validateAll(deviceIds, m_source.getRadiationId(), failMessage)) {
        // 验证失败，显示错误对话框
        GtkWidget* dialog = gtk_message_dialog_new(
            nullptr,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "仿真验证失败：%s", failMessage.c_str()
        );
        gtk_window_set_title(GTK_WINDOW(dialog), "测向定位仿真验证失败");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        return false;
    }
    
    // 清除之前的测向线信息
    m_directionLines.clear();
    m_deviceErrors.clear();
    
    // 只用前两个设备
    const auto& dev1 = m_devices[0];
    const auto& dev2 = m_devices[1];

    // 存储误差参数
    m_deviceErrors.push_back(std::make_tuple(dev1MeanError, dev1StdDev));
    m_deviceErrors.push_back(std::make_tuple(dev2MeanError, dev2StdDev));
    
    double error = 0.0;
    // COORD3->Vector3
    COORD3 esm1_coord = lbh2xyz(dev1.getLongitude(), dev1.getLatitude(), dev1.getAltitude());
    COORD3 esm2_coord = lbh2xyz(dev2.getLongitude(), dev2.getLatitude(), dev2.getAltitude());
    COORD3 target_coord = lbh2xyz(m_source.getLongitude(), m_source.getLatitude(), m_source.getAltitude());
    
    // 保存目标的真实高度值
    // double targetAlt = m_source.getAltitude();
    
    Vector3 esm1(esm1_coord.p1, esm1_coord.p2, esm1_coord.p3);
    Vector3 esm2(esm2_coord.p1, esm2_coord.p2, esm2_coord.p3);
    Vector3 target(target_coord.p1, target_coord.p2, target_coord.p3);
    
    // double commonHeight = esm1.z;
    // esm2.z = commonHeight;
    // target.z = commonHeight;
    
    Vector3 dir1 = calculateDirectionWithError(esm1, target, dev1MeanError, dev1StdDev);
    Vector3 dir2 = calculateDirectionWithError(esm2, target, dev2MeanError, dev2StdDev);
    
    // 存储测向线信息
    DirectionLine line1 = {0, esm1, dir1, dev1MeanError, dev1StdDev};
    DirectionLine line2 = {1, esm2, dir2, dev2MeanError, dev2StdDev};
    m_directionLines.push_back(line1);
    m_directionLines.push_back(line2);
    
    Vector3 estimatedPosition = intersectDirections2D(esm1, dir1, esm2, dir2);
    error = std::sqrt((estimatedPosition.x - target.x) * (estimatedPosition.x - target.x) +
                     (estimatedPosition.y - target.y) * (estimatedPosition.y - target.y));
    
    // 关键修复：直接使用目标真实高度进行坐标转换
    // 1. 先使用估计位置的XY坐标和真实目标高度创建新的空间直角坐标
    COORD3 correctedXYZ(estimatedPosition.x, estimatedPosition.y, target_coord.p3);
    
    // 2. 将修正后的空间直角坐标转换为大地坐标
    auto lbh = xyz2lbh(correctedXYZ.p1, correctedXYZ.p2, correctedXYZ.p3);
    
    m_result.position = {lbh.p1, lbh.p2, lbh.p3};
    m_result.error = error;
    m_isInitialized = true;
    return true;
}

bool DirectionFinding::calculateAuto() {
    if (!loadDeviceInfo() || !loadSourceInfo()) return false;

    // 仿真前验证
    SimulationValidator validator;
    std::vector<int> deviceIds;
    for (const auto& device : m_devices) {
        deviceIds.push_back(device.getDeviceId());
    }
    std::string failMessage;
    if (!validator.validateAll(deviceIds, m_source.getRadiationId(), failMessage)) {
        GtkWidget* dialog = gtk_message_dialog_new(
            nullptr,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "仿真验证失败：%s", failMessage.c_str()
        );
        gtk_window_set_title(GTK_WINDOW(dialog), "测向定位仿真验证失败");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return false;
    }

    // 清除之前的测向线信息
    m_directionLines.clear();
    m_deviceErrors.clear();

    // 只用前两个固定设备
    const auto& dev1 = m_devices[0];
    const auto& dev2 = m_devices[1];

    // 计算两个设备到辐射源的方位角/俯仰角
    COORD3 dev1_xyz = lbh2xyz(dev1.getLongitude(), dev1.getLatitude(), dev1.getAltitude());
    COORD3 dev2_xyz = lbh2xyz(dev2.getLongitude(), dev2.getLatitude(), dev2.getAltitude());
    COORD3 src_xyz  = lbh2xyz(m_source.getLongitude(), m_source.getLatitude(), m_source.getAltitude());

    auto calcAzEl = [](const COORD3& obs, const COORD3& tgt){
        double dx = tgt.p1 - obs.p1;
        double dy = tgt.p2 - obs.p2;
        double dz = tgt.p3 - obs.p3;
        double az = std::atan2(dx, dy) * Constants::RAD2DEG; if (az < 0) az += 360.0;
        double r = std::sqrt(dx*dx + dy*dy);
        double el = std::atan2(dz, r) * Constants::RAD2DEG;
        return std::make_pair(az, el);
    };

    auto [az1, el1] = calcAzEl(dev1_xyz, src_xyz);
    auto [az2, el2] = calcAzEl(dev2_xyz, src_xyz);
    std::cout << "[DF] dev1 az(deg)=" << az1 << ", el(deg)=" << el1 << std::endl;
    std::cout << "[DF] dev2 az(deg)=" << az2 << ", el(deg)=" << el2 << std::endl;

    // 根据设备技术体制选择误差计算方法
    std::vector<double> err1, err2;
    double mean1, std1, mean2, std2;

    // 设备1误差计算
    std::cout << "[DF] 设备1=" << dev1.getDeviceName() << " 技术体制=" << dev1.getTechSystem() << std::endl;
    if (dev1.getTechSystem() == "时差体制" || dev1.getTechSystem() == "TDOA") {
        // 时差体制：使用TDOA误差计算方法
        double incidentAngleRad = az1 * Constants::DEG2RAD;
        double estimatedDistance = std::sqrt(
            (src_xyz.p1 - dev1_xyz.p1) * (src_xyz.p1 - dev1_xyz.p1) +
            (src_xyz.p2 - dev1_xyz.p2) * (src_xyz.p2 - dev1_xyz.p2) +
            (src_xyz.p3 - dev1_xyz.p3) * (src_xyz.p3 - dev1_xyz.p3)
        );
        
        // 动态计算TDOA参数，根据侦察设备和辐射源的实际参数
        auto tdoaParams = DirectionErrorUtils::calculateTDOAParams(dev1, m_source, estimatedDistance);
        
        std::cout << "[DF] 设备1 TDOA参数: phaseErr=" << tdoaParams.phaseErrorDeg 
                  << "°, bandwidth=" << tdoaParams.bandwidthHz/1e6 << "MHz, SNR=" << tdoaParams.snrLinear 
                  << ", fs=" << tdoaParams.samplingRateHz/1e6 << "MHz" << std::endl;
        
        err1 = DirectionErrorUtils::calculateTDOAErrors(
            dev1.getBaselineLength(),
            incidentAngleRad,
            estimatedDistance,
            m_source.getCarrierFrequency() * 1e9,  // 转换为Hz
            tdoaParams.phaseErrorDeg,
            tdoaParams.bandwidthHz,
            tdoaParams.snrLinear,
            tdoaParams.samplingRateHz
        );
        // TDOA返回: [σ_τ, 占位, σ_τφ, σ_τn, σ_θ]
        // 均值误差=0，标准差=σ_θ
        mean1 = 0.0;
        std1 = err1.size() >= 5 ? err1[4] : 0.0;  // σ_θ
        std::cout << "[DF] 设备1 TDOA误差: mean(deg)=" << mean1 << ", std(deg)=" << std1 << std::endl;
    } else {
        // 干涉仪体制：使用干涉仪误差计算方法
        err1 = DirectionErrorUtils::calculateInterferometerErrors(dev1, m_source, az1, el1);
        // 干涉仪返回: [Δem, σ_α, σ_β, σ_θ, total]
        // 均值误差=Δem，标准差=sqrt(σ_α^2+σ_β^2+σ_θ^2)
        auto calcMeanStd = [](const std::vector<double>& e){
            if (e.size() < 4) return std::make_pair(0.0, 0.0);
            double delta_em = e[0];
            double sigma_alpha = e[1];
            double sigma_beta  = e[2];
            double sigma_theta = e[3];
            double stddev = std::sqrt(sigma_alpha*sigma_alpha + sigma_beta*sigma_beta + sigma_theta*sigma_theta);
            return std::make_pair(delta_em, stddev);
        };
        auto [m1, s1] = calcMeanStd(err1);
        mean1 = m1;
        std1 = s1;
        std::cout << "[DF] 设备1 干涉仪误差: mean(deg)=" << mean1 << ", std(deg)=" << std1 << std::endl;
    }

    // 设备2误差计算
    std::cout << "[DF] 设备2=" << dev2.getDeviceName() << " 技术体制=" << dev2.getTechSystem() << std::endl;
    if (dev2.getTechSystem() == "时差体制" || dev2.getTechSystem() == "TDOA") {
        // 时差体制：使用TDOA误差计算方法
        double incidentAngleRad = az2 * Constants::DEG2RAD;
        double estimatedDistance = std::sqrt(
            (src_xyz.p1 - dev2_xyz.p1) * (src_xyz.p1 - dev2_xyz.p1) +
            (src_xyz.p2 - dev2_xyz.p2) * (src_xyz.p2 - dev2_xyz.p2) +
            (src_xyz.p3 - dev2_xyz.p3) * (src_xyz.p3 - dev2_xyz.p3)
        );
        
        // 动态计算TDOA参数，根据侦察设备和辐射源的实际参数
        auto tdoaParams = DirectionErrorUtils::calculateTDOAParams(dev2, m_source, estimatedDistance);
        
        std::cout << "[DF] 设备2 TDOA参数: phaseErr=" << tdoaParams.phaseErrorDeg 
                  << "°, bandwidth=" << tdoaParams.bandwidthHz/1e6 << "MHz, SNR=" << tdoaParams.snrLinear 
                  << ", fs=" << tdoaParams.samplingRateHz/1e6 << "MHz" << std::endl;
        
        err2 = DirectionErrorUtils::calculateTDOAErrors(
            dev2.getBaselineLength(),
            incidentAngleRad,
            estimatedDistance,
            m_source.getCarrierFrequency() * 1e9,  // 转换为Hz
            tdoaParams.phaseErrorDeg,
            tdoaParams.bandwidthHz,
            tdoaParams.snrLinear,
            tdoaParams.samplingRateHz
        );
        // TDOA返回: [σ_τ, 占位, σ_τφ, σ_τn, σ_θ]
        // 均值误差=0，标准差=σ_θ
        mean2 = 0.0;
        std2 = err2.size() >= 5 ? err2[4] : 0.0;  // σ_θ
        std::cout << "[DF] 设备2 TDOA误差: mean(deg)=" << mean2 << ", std(deg)=" << std2 << std::endl;
    } else {
        // 干涉仪体制：使用干涉仪误差计算方法
        err2 = DirectionErrorUtils::calculateInterferometerErrors(dev2, m_source, az2, el2);
        // 干涉仪返回: [Δem, σ_α, σ_β, σ_θ, total]
        // 均值误差=Δem，标准差=sqrt(σ_α^2+σ_β^2+σ_θ^2)
        auto calcMeanStd = [](const std::vector<double>& e){
            if (e.size() < 4) return std::make_pair(0.0, 0.0);
            double delta_em = e[0];
            double sigma_alpha = e[1];
            double sigma_beta  = e[2];
            double sigma_theta = e[3];
            double stddev = std::sqrt(sigma_alpha*sigma_alpha + sigma_beta*sigma_beta + sigma_theta*sigma_theta);
            return std::make_pair(delta_em, stddev);
        };
        auto [m2, s2] = calcMeanStd(err2);
        mean2 = m2;
        std2 = s2;
        std::cout << "[DF] 设备2 干涉仪误差: mean(deg)=" << mean2 << ", std(deg)=" << std2 << std::endl;
    }

    // 保存误差参数
    m_deviceErrors.push_back(std::make_tuple(mean1, std1));
    m_deviceErrors.push_back(std::make_tuple(mean2, std2));

    // COORD3->Vector3
    Vector3 esm1(dev1_xyz.p1, dev1_xyz.p2, dev1_xyz.p3);
    Vector3 esm2(dev2_xyz.p1, dev2_xyz.p2, dev2_xyz.p3);
    Vector3 target(src_xyz.p1, src_xyz.p2, src_xyz.p3);

    Vector3 dir1 = calculateDirectionWithError(esm1, target, mean1, std1);
    Vector3 dir2 = calculateDirectionWithError(esm2, target, mean2, std2);
    std::cout << "[DF] dir1=(" << dir1.x << "," << dir1.y << "," << dir1.z << ")" << std::endl;
    std::cout << "[DF] dir2=(" << dir2.x << "," << dir2.y << ")" << std::endl;

    // 存储测向线信息
    DirectionLine line1 = {0, esm1, dir1, mean1, std1};
    DirectionLine line2 = {1, esm2, dir2, mean2, std2};
    m_directionLines.push_back(line1);
    m_directionLines.push_back(line2);

    Vector3 estimatedPosition = intersectDirections2D(esm1, dir1, esm2, dir2);
    std::cout << "[DF] intersect XY=(" << estimatedPosition.x << "," << estimatedPosition.y << ")" << std::endl;
    double errorXY = std::sqrt((estimatedPosition.x - target.x) * (estimatedPosition.x - target.x) +
                               (estimatedPosition.y - target.y) * (estimatedPosition.y - target.y));
    std::cout << "[DF] errorXY(m)=" << errorXY << std::endl;

    // 修正高度后输出结果
    COORD3 correctedXYZ(estimatedPosition.x, estimatedPosition.y, src_xyz.p3);
    auto lbh = xyz2lbh(correctedXYZ.p1, correctedXYZ.p2, correctedXYZ.p3);
    m_result.position = {lbh.p1, lbh.p2, lbh.p3};
    std::cout << "[DF] result LBH=(lon=" << lbh.p1 << ", lat=" << lbh.p2 << ", h=" << lbh.p3 << ")" << std::endl;
    m_result.error = errorXY;
    m_isInitialized = true;
    return true;
}

DirectionFinding::Result DirectionFinding::getResult() const { 
    return m_result; 
}

std::vector<DirectionFinding::DirectionLine> DirectionFinding::getDirectionLines() const {
    return m_directionLines;
}

std::tuple<double, double> DirectionFinding::getErrorAngles(int deviceIndex) const {
    if (deviceIndex >= 0 && deviceIndex < m_deviceErrors.size()) {
        return m_deviceErrors[deviceIndex];
    }
    return std::make_tuple(0.0, 0.0);
}

// 向量相关函数实现
Vector3 calculateDirectionWithError(
    const Vector3& observer, 
    const Vector3& target,
    double meanErrorDeg,  // 均值误差（度）
    double stdDevDeg      // 标准差（度）
) {
    double trueAzimuth = std::atan2(target.y - observer.y, target.x - observer.x);
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<> angleDist(meanErrorDeg, stdDevDeg);
    double angularErrorRad = angleDist(gen) * Constants::DEG2RAD;
    double measuredAzimuth = trueAzimuth + angularErrorRad;
    return Vector3(std::cos(measuredAzimuth), std::sin(measuredAzimuth), 0);
}

Vector3 intersectDirections2D(
    const Vector3& obs1, const Vector3& dir1, 
    const Vector3& obs2, const Vector3& dir2
) {
    Vector3 p1p2 = obs2 - obs1;
    double denominator = dir1.x * dir2.y - dir1.y * dir2.x;
    if (std::abs(denominator) < 1e-10) {
        return Vector3((obs1.x + obs2.x) / 2, (obs1.y + obs2.y) / 2, obs1.z);
    }
    double t = (p1p2.x * dir2.y - p1p2.y * dir2.x) / denominator;
    return Vector3(
        obs1.x + dir1.x * t,
        obs1.y + dir1.y * t,
        obs1.z
    );
} 