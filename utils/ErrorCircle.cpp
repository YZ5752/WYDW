#include "ErrorCircle.h"
#include "../models/ReconnaissanceDeviceDAO.h"
#include "../models/RadiationSourceDAO.h"
#include "CoordinateTransform.h"
#include <random>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <iostream>

using namespace Constants;

// 查询设备信息
bool loadDeviceInfo(const std::vector<std::string>& deviceNames, std::vector<ReconnaissanceDevice>& selectedDevices) {
    ReconnaissanceDeviceDAO& deviceDAO = ReconnaissanceDeviceDAO::getInstance();
    std::vector<ReconnaissanceDevice> allDevices = deviceDAO.getAllReconnaissanceDevices();
    selectedDevices.clear();
    for (const auto& name : deviceNames) {
        auto it = std::find_if(allDevices.begin(), allDevices.end(), [&](const ReconnaissanceDevice& d) {
            return d.getDeviceName() == name;
        });
        if (it != allDevices.end()) {
            selectedDevices.push_back(*it);
        }
    }
    return selectedDevices.size() >= 2;
}

// 查询辐射源信息
bool loadSourceInfo(const std::string& sourceName, RadiationSource& source) {
    RadiationSourceDAO& sourceDAO = RadiationSourceDAO::getInstance();
    std::vector<RadiationSource> allSources = sourceDAO.getAllRadiationSources();
    auto it = std::find_if(allSources.begin(), allSources.end(), [&](const RadiationSource& s) {
        return s.getRadiationName() == sourceName;
    });
    if (it != allSources.end()) {
        source = *it;
        return true;
    }
    return false;
}

// 2x2协方差矩阵计算
static void computeCovariance2x2(const std::vector<COORD3>& deviations, double& cov00, double& cov01, double& cov11) {
    double meanX = 0, meanY = 0;
    int n = deviations.size();
    for (const auto& dev : deviations) {
        meanX += dev.p1;
        meanY += dev.p2;
    }
    meanX /= n;
    meanY /= n;
    cov00 = cov01 = cov11 = 0;
    for (const auto& dev : deviations) {
        double dx = dev.p1 - meanX;
        double dy = dev.p2 - meanY;
        cov00 += dx * dx;
        cov11 += dy * dy;
        cov01 += dx * dy;
    }
    if (n > 1) {
        cov00 /= (n - 1);
        cov11 /= (n - 1);
        cov01 /= (n - 1);
    }
}

// 2x2矩阵特征值分解
static void eigenDecomposition2x2(double cov00, double cov01, double cov11, double& eig1, double& eig2) {
    double a = cov00, b = cov01, c = cov11;
    double trace = a + c;
    double det = a * c - b * b;
    double temp = std::sqrt(trace * trace - 4 * det);
    eig1 = (trace + temp) / 2.0;
    eig2 = (trace - temp) / 2.0;
}

// 计算误差圆
DFResult calculateDFErrorCircle(
    const std::vector<std::string>& deviceNames,
    const std::string& sourceName,
    double esm1ErrorMean, double esm1ErrorSigma,
    double esm2ErrorMean, double esm2ErrorSigma,
    //随机种子
    unsigned int seed
) {
    std::vector<ReconnaissanceDevice> selectedDevices;
    RadiationSource source;
    // 查询设备信息
    if (!loadDeviceInfo(deviceNames, selectedDevices)) {
        return DFResult();
    }
    // 查询辐射源信息
    if (!loadSourceInfo(sourceName, source)) {
        return DFResult();
    }
    // 将设备和辐射源的经纬度转换为直角坐标
    COORD3 esm1Cart = lbh2xyz(selectedDevices[0].getLongitude(), selectedDevices[0].getLatitude(), selectedDevices[0].getAltitude());
    COORD3 esm2Cart = lbh2xyz(selectedDevices[1].getLongitude(), selectedDevices[1].getLatitude(), selectedDevices[1].getAltitude());
    COORD3 targetCart = lbh2xyz(source.getLongitude(), source.getLatitude(), source.getAltitude());
    // 生成随机误差
    std::mt19937 gen(seed ? seed : static_cast<unsigned int>(std::time(nullptr)));
    std::normal_distribution<double> normalDist(0.0, 1.0);
    std::vector<COORD3> deviations;
    deviations.reserve(100);
    // 生成100个误差点
    for (int i = 0; i < 100; i++) {
        //计算理论方位角
        double dx1 = targetCart.p1 - esm1Cart.p1;
        double dy1 = targetCart.p2 - esm1Cart.p2;
        double trueBearing1 = atan2(dy1, dx1);

        double dx2 = targetCart.p1 - esm2Cart.p1;
        double dy2 = targetCart.p2 - esm2Cart.p2;
        double trueBearing2 = atan2(dy2, dx2);
        // 转换误差单位为弧度
        double esm1MeanRad = esm1ErrorMean * DEG2RAD;
        double esm1SigmaRad = esm1ErrorSigma * DEG2RAD;
        double esm2MeanRad = esm2ErrorMean * DEG2RAD;
        double esm2SigmaRad = esm2ErrorSigma * DEG2RAD;
        //生成截断高斯分布误差(范围: [-3σ, 3σ])
        double error1, error2;
        //生成误差1
        do {
            error1 = esm1MeanRad + esm1SigmaRad * normalDist(gen);
        } while (fabs(error1 - esm1MeanRad) > 3.0 * esm1SigmaRad);
        //生成误差2
        do {
            error2 = esm2MeanRad + esm2SigmaRad * normalDist(gen);
        } while (fabs(error2 - esm2MeanRad) > 3.0 * esm2SigmaRad);
        //计算带误差的测量方位角
        double measuredBearing1 = trueBearing1 + error1;
        double measuredBearing2 = trueBearing2 + error2;
        //计算定位点
        double m1 = tan(measuredBearing1);
        double m2 = tan(measuredBearing2);
        double x, y;
        if (fabs(m1) > 1e6) {
            x = esm1Cart.p1;
            y = m2 * (x - esm2Cart.p1) + esm2Cart.p2;
        } else if (fabs(m2) > 1e6) {
            x = esm2Cart.p1;
            y = m1 * (x - esm1Cart.p1) + esm1Cart.p2;
        } else {
            x = (m1 * esm1Cart.p1 - m2 * esm2Cart.p1 + esm2Cart.p2 - esm1Cart.p2) / (m1 - m2);
            y = m1 * (x - esm1Cart.p1) + esm1Cart.p2;
        }
        //计算偏差
        double dx = x - targetCart.p1;
        double dy = y - targetCart.p2;
        deviations.push_back(COORD3(dx, dy, 0.0));
    }
    // 计算协方差矩阵
    double cov00, cov01, cov11;
    computeCovariance2x2(deviations, cov00, cov01, cov11);
    // 计算特征值(误差椭圆参数)
    double eig1, eig2;
    eigenDecomposition2x2(cov00, cov01, cov11, eig1, eig2);
    double majorAxis = std::sqrt(std::max(eig1, eig2));
    double minorAxis = std::sqrt(std::min(eig1, eig2));
    double cepRadius = 0.59 * (majorAxis + minorAxis);
    // 先得到目标点的大地高
    COORD3 targetLBH = xyz2lbh(targetCart.p1, targetCart.p2, targetCart.p3);
    double targetHeight = targetLBH.p3;
    // 将误差点转换为COORD3
    std::vector<COORD3> estimatedPoints;
    estimatedPoints.reserve(100);
    for (const auto& dev : deviations) {
        double estX = targetCart.p1 + dev.p1;
        double estY = targetCart.p2 + dev.p2;
        double estZ = targetCart.p3; // 保持高度一致
        COORD3 estLBH = xyz2lbh(estX, estY, estZ); // 这里必须传 X, Y, Z
        double estLon = estLBH.p1;
        double estLat = estLBH.p2;
        COORD3 estXYZ = lbh2xyz(estLon, estLat, targetHeight); // targetHeight 是目标点大地高
        estimatedPoints.push_back(estXYZ);
    }
    return {estimatedPoints, cepRadius};
}







