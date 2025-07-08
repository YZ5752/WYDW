#include "HyperbolaLines.h"
#include "../models/ReconnaissanceDeviceDAO.h"
#include "../models/RadiationSourceDAO.h"
#include <random>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>

using namespace Constants;

// 计算两点之间的距离
double HyperbolaLines::calculateDistance(const COORD3& p1, const COORD3& p2) {
    return std::sqrt(std::pow(p1.p1 - p2.p1, 2) + 
                     std::pow(p1.p2 - p2.p2, 2) + 
                     std::pow(p1.p3 - p2.p3, 2));
}

// 生成双曲线上的点
std::vector<COORD3> HyperbolaLines::generateHyperbolaPoints(
    const COORD3& station1, 
    const COORD3& station2, 
    double tdoa, 
    const COORD3& center,
    double range) {
    
    std::vector<COORD3> points;
    const int numPoints = 100;  // 每条双曲线上的点数
    
    // 将时差转换为距离差
    double distanceDiff = tdoa * c;
    
    // 计算两个站点之间的距离
    double stationDistance = calculateDistance(station1, station2);
    
    // 计算双曲线的参数
    double a = std::abs(distanceDiff) / 2.0;
    double c_hyperbola = stationDistance / 2.0;
    double b = std::sqrt(c_hyperbola * c_hyperbola - a * a);
    
    // 确定双曲线的方向向量(从station1指向station2)
    double dx = station2.p1 - station1.p1;
    double dy = station2.p2 - station1.p2;
    double dz = station2.p3 - station1.p3;
    
    // 归一化方向向量
    double norm = std::sqrt(dx*dx + dy*dy + dz*dz);
    dx /= norm;
    dy /= norm;
    dz /= norm;
    
    // 计算双曲线的中点(两站点的中点)
    double midX = (station1.p1 + station2.p1) / 2.0;
    double midY = (station1.p2 + station2.p2) / 2.0;
    double midZ = (station1.p3 + station2.p3) / 2.0;
    
    // 生成垂直于主轴的两个单位向量
    // 首先找一个不与主轴平行的向量
    double perpX, perpY, perpZ;
    if (std::abs(dx) < 0.9) {
        perpX = 1.0;
        perpY = 0.0;
        perpZ = 0.0;
    } else {
        perpX = 0.0;
        perpY = 1.0;
        perpZ = 0.0;
    }
    
    // 计算第一个垂直向量(叉积)
    double v1X = dy * perpZ - dz * perpY;
    double v1Y = dz * perpX - dx * perpZ;
    double v1Z = dx * perpY - dy * perpX;
    
    // 归一化
    norm = std::sqrt(v1X*v1X + v1Y*v1Y + v1Z*v1Z);
    v1X /= norm;
    v1Y /= norm;
    v1Z /= norm;
    
    // 计算第二个垂直向量(叉积)
    double v2X = dy * v1Z - dz * v1Y;
    double v2Y = dz * v1X - dx * v1Z;
    double v2Z = dx * v1Y - dy * v1X;
    
    // 生成双曲线上的点
    for (int i = 0; i < numPoints; i++) {
        // 参数方程的参数t
        double t = -M_PI/2 + i * M_PI / (numPoints - 1);
        
        // 双曲线参数方程: (a*sec(t), b*tan(t))
        double secT = 1.0 / std::cos(t);
        double tanT = std::tan(t);
        
        // 计算双曲线上的点(在主轴-垂直轴坐标系中)
        double x_local = a * secT;
        double y_local = b * tanT;
        
        // 将局部坐标转换到全局坐标
        double x = midX + x_local * dx + y_local * v1X;
        double y = midY + x_local * dy + y_local * v1Y;
        double z = midZ + x_local * dz + y_local * v1Z;
        
        // 检查点是否在有效范围内
        COORD3 point(x, y, z);
        if (calculateDistance(point, center) <= range) {
            points.push_back(point);
        }
        
        // 对称点(双曲线的另一支)
        x = midX + x_local * dx - y_local * v1X;
        y = midY + x_local * dy - y_local * v1Y;
        z = midZ + x_local * dz - y_local * v1Z;
        
        point = COORD3(x, y, z);
        if (calculateDistance(point, center) <= range) {
            points.push_back(point);
        }
    }
    
    return points;
}

// 绘制TDOA双曲线
bool HyperbolaLines::drawTDOAHyperbolas(
    MapView* mapView,
    const std::vector<COORD3>& stationPositions,
    const std::vector<double>& tdoas,
    const COORD3& targetPosition,
    const std::vector<std::string>& colors) {
    
    if (!mapView || stationPositions.size() < 2 || tdoas.size() < stationPositions.size() - 1) {
        return false;
    }
    
    // 清除之前可能存在的双曲线
    clearHyperbolaLines(mapView);
    
    // 构建JavaScript代码绘制双曲线
    std::stringstream script;
    
    // 创建双曲线容器实体
    script << "// 创建TDOA双曲线容器\n"
           << "var hyperbolaContainer = viewer.entities.add({\n"
           << "  id: 'tdoa-hyperbola-lines',\n"
           << "  name: 'TDOA双曲线'\n"
           << "});\n";
    
    // 绘制范围(以目标为中心的圆形区域)
    double range = 60000.0;  // 60公里
    
    // 参考站(第一个站)
    const COORD3& referenceStation = stationPositions[0];
    
    // 为每个从站生成并绘制双曲线
    for (size_t i = 1; i < stationPositions.size() && i <= tdoas.size(); i++) {
        // 获取当前从站
        const COORD3& currentStation = stationPositions[i];
        
        // 获取时差值
        double tdoa = tdoas[i];
        
        // 选择颜色
        std::string color = (i <= colors.size()) ? colors[i-1] : "white";
        
        // 生成双曲线上的点
        std::vector<COORD3> hyperbolaPoints = generateHyperbolaPoints(
            referenceStation, currentStation, tdoa, targetPosition, range);
        
        // 如果没有生成点，跳过
        if (hyperbolaPoints.empty()) {
            continue;
        }
        
        // 将点转换为大地坐标
        std::vector<COORD3> hyperbolaPointsLBH;
        for (const auto& point : hyperbolaPoints) {
            COORD3 lbh = xyz2lbh(point.p1, point.p2, point.p3);
            hyperbolaPointsLBH.push_back(lbh);
        }
        
        // 构建点数组字符串
        script << "// 双曲线" << i << "的点\n"
               << "var hyperbolaPoints" << i << " = [";
        
        for (size_t j = 0; j < hyperbolaPointsLBH.size(); j++) {
            if (j > 0) script << ", ";
            script << "Cesium.Cartesian3.fromDegrees(" 
                   << hyperbolaPointsLBH[j].p1 << ", " 
                   << hyperbolaPointsLBH[j].p2 << ", " 
                   << hyperbolaPointsLBH[j].p3 << ")";
        }
        
        script << "];\n";
        
        // 创建双曲线实体
        script << "// 创建双曲线" << i << "\n"
               << "viewer.entities.add({\n"
               << "  parent: hyperbolaContainer,\n"
               << "  polyline: {\n"
               << "    positions: hyperbolaPoints" << i << ",\n"
               << "    width: 2,\n"
               << "    material: new Cesium.ColorMaterialProperty(Cesium.Color.fromCssColorString('" << color << "'))\n"
               << "  }\n"
               << "});\n";
    }
    
    // 执行JavaScript代码
    mapView->executeScript(script.str());
    
    return true;
}

// 清除双曲线
void HyperbolaLines::clearHyperbolaLines(MapView* mapView) {
    if (!mapView) return;
    
    std::string script = "var existingHyperbolas = viewer.entities.getById('tdoa-hyperbola-lines');\n"
                         "if (existingHyperbolas) {\n"
                         "  viewer.entities.removeById('tdoa-hyperbola-lines');\n"
                         "}\n";
    
    mapView->executeScript(script);
}

// 查询设备信息
static bool loadDeviceInfo(const std::vector<std::string>& deviceNames, std::vector<ReconnaissanceDevice>& selectedDevices) {
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
    return selectedDevices.size() >= 3;  // TDOA至少需要3个站
}

// 查询辐射源信息
static bool loadSourceInfo(const std::string& sourceName, RadiationSource& source) {
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

// 计算TDOA误差圆
TDOAResult calculateTDOAErrorCircle(
    const std::vector<std::string>& deviceNames,
    const std::string& sourceName,
    double tdoaErrorMean,
    double tdoaErrorSigma,
    unsigned int seed
) {
    std::vector<ReconnaissanceDevice> selectedDevices;
    RadiationSource source;
    
    // 查询设备信息
    if (!loadDeviceInfo(deviceNames, selectedDevices)) {
        return TDOAResult();
    }
    
    // 查询辐射源信息
    if (!loadSourceInfo(sourceName, source)) {
        return TDOAResult();
    }
    
    // 将设备和辐射源的经纬度转换为直角坐标
    std::vector<COORD3> stationPositions;
    for (const auto& device : selectedDevices) {
        COORD3 stationCart = lbh2xyz(device.getLongitude(), device.getLatitude(), device.getAltitude());
        stationPositions.push_back(stationCart);
    }
    
    COORD3 targetCart = lbh2xyz(source.getLongitude(), source.getLatitude(), source.getAltitude());
    
    // 生成随机误差
    std::mt19937 gen(seed ? seed : static_cast<unsigned int>(std::time(nullptr)));
    std::normal_distribution<double> normalDist(0.0, 1.0);
    
    // 计算理论时差
    std::vector<double> trueTdoas;
    for (size_t i = 1; i < stationPositions.size(); i++) {
        double dist1 = HyperbolaLines::calculateDistance(stationPositions[0], targetCart);
        double dist2 = HyperbolaLines::calculateDistance(stationPositions[i], targetCart);
        double tdoa = (dist2 - dist1) / c;  // 时差(秒)
        trueTdoas.push_back(tdoa);
    }
    
    // 生成100个误差点
    std::vector<COORD3> deviations;
    std::vector<COORD3> estimatedPoints;
    deviations.reserve(100);
    estimatedPoints.reserve(100);
    
    for (int i = 0; i < 100; i++) {
        // 为每个时差添加误差
        std::vector<double> noisyTdoas = trueTdoas;
        for (auto& tdoa : noisyTdoas) {
            // 生成截断高斯分布误差(范围: [-3σ, 3σ])
            double error;
            do {
                error = tdoaErrorMean + tdoaErrorSigma * normalDist(gen);
            } while (fabs(error - tdoaErrorMean) > 3.0 * tdoaErrorSigma);
            
            tdoa += error;
        }
        
        // 使用带误差的时差进行定位(简化版Chan算法)
        // 这里使用一个简化的定位算法，实际应用中应使用更复杂的算法
        int N = stationPositions.size();
        int M = N - 1;
        
        std::vector<std::vector<double>> A(M, std::vector<double>(3));
        std::vector<double> b(M);
        
        for (int j = 0; j < M; j++) {
            double tdoa = noisyTdoas[j];
            double d = c * tdoa;  // 距离差
            
            // 参考站
            double x1 = stationPositions[0].p1;
            double y1 = stationPositions[0].p2;
            double z1 = stationPositions[0].p3;
            
            // 从站
            double x2 = stationPositions[j+1].p1;
            double y2 = stationPositions[j+1].p2;
            double z2 = stationPositions[j+1].p3;
            
            // 构建方程组
            A[j][0] = 2 * (x1 - x2 + d * (x2 - x1) / HyperbolaLines::calculateDistance(stationPositions[0], stationPositions[j+1]));
            A[j][1] = 2 * (y1 - y2 + d * (y2 - y1) / HyperbolaLines::calculateDistance(stationPositions[0], stationPositions[j+1]));
            A[j][2] = 2 * (z1 - z2 + d * (z2 - z1) / HyperbolaLines::calculateDistance(stationPositions[0], stationPositions[j+1]));
            
            b[j] = x1*x1 - x2*x2 + y1*y1 - y2*y2 + z1*z1 - z2*z2 + 
                   d * (HyperbolaLines::calculateDistance(stationPositions[0], stationPositions[j+1]) - 
                        2 * ((x1*(x1-x2) + y1*(y1-y2) + z1*(z1-z2)) / 
                             HyperbolaLines::calculateDistance(stationPositions[0], stationPositions[j+1])));
        }
        
        // 求解最小二乘问题
        double AtA[3][3] = {{0}};
        double Atb[3] = {0};
        
        // 计算A^T * A
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                for (int l = 0; l < M; l++) {
                    AtA[j][k] += A[l][j] * A[l][k];
                }
            }
        }
        
        // 计算A^T * b
        for (int j = 0; j < 3; j++) {
            for (int l = 0; l < M; l++) {
                Atb[j] += A[l][j] * b[l];
            }
        }
        
        // 求解线性方程组 AtA * x = Atb
        double det = AtA[0][0] * (AtA[1][1] * AtA[2][2] - AtA[1][2] * AtA[2][1]) -
                     AtA[0][1] * (AtA[1][0] * AtA[2][2] - AtA[1][2] * AtA[2][0]) +
                     AtA[0][2] * (AtA[1][0] * AtA[2][1] - AtA[1][1] * AtA[2][0]);
        
        if (std::abs(det) < 1e-10) {
            // 矩阵接近奇异，跳过此次迭代
            continue;
        }
        
        double invAtA[3][3];
        invAtA[0][0] = (AtA[1][1] * AtA[2][2] - AtA[1][2] * AtA[2][1]) / det;
        invAtA[0][1] = (AtA[0][2] * AtA[2][1] - AtA[0][1] * AtA[2][2]) / det;
        invAtA[0][2] = (AtA[0][1] * AtA[1][2] - AtA[0][2] * AtA[1][1]) / det;
        invAtA[1][0] = (AtA[1][2] * AtA[2][0] - AtA[1][0] * AtA[2][2]) / det;
        invAtA[1][1] = (AtA[0][0] * AtA[2][2] - AtA[0][2] * AtA[2][0]) / det;
        invAtA[1][2] = (AtA[0][2] * AtA[1][0] - AtA[0][0] * AtA[1][2]) / det;
        invAtA[2][0] = (AtA[1][0] * AtA[2][1] - AtA[1][1] * AtA[2][0]) / det;
        invAtA[2][1] = (AtA[0][1] * AtA[2][0] - AtA[0][0] * AtA[2][1]) / det;
        invAtA[2][2] = (AtA[0][0] * AtA[1][1] - AtA[0][1] * AtA[1][0]) / det;
        
        // 计算解 x = invAtA * Atb
        double x = invAtA[0][0] * Atb[0] + invAtA[0][1] * Atb[1] + invAtA[0][2] * Atb[2];
        double y = invAtA[1][0] * Atb[0] + invAtA[1][1] * Atb[1] + invAtA[1][2] * Atb[2];
        double z = invAtA[2][0] * Atb[0] + invAtA[2][1] * Atb[1] + invAtA[2][2] * Atb[2];
        
        // 计算偏差
        double dx = x - targetCart.p1;
        double dy = y - targetCart.p2;
        double dz = z - targetCart.p3;
        
        deviations.push_back(COORD3(dx, dy, 0.0));  // 只考虑水平面误差
        estimatedPoints.push_back(COORD3(x, y, z));
    }
    
    // 计算协方差矩阵
    double cov00, cov01, cov11;
    computeCovariance2x2(deviations, cov00, cov01, cov11);
    
    // 计算特征值(误差椭圆参数)
    double eig1, eig2;
    eigenDecomposition2x2(cov00, cov01, cov11, eig1, eig2);
    
    double majorAxis = std::sqrt(std::max(eig1, eig2));
    double minorAxis = std::sqrt(std::min(eig1, eig2));
    
    // CEP(Circular Error Probable)半径
    double cepRadius = 0.59 * (majorAxis + minorAxis);
    
    return TDOAResult(estimatedPoints, cepRadius, targetCart);
} 