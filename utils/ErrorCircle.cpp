#include "ErrorCircle.h"
#include <cmath>
#include <algorithm>

// 计算误差点集
std::vector<std::pair<double, double>> ErrorCircle::CalculateErrorPoints(
    const std::vector<COORD3>& estimatedPoints,
    double trueLon, double trueLat, double trueAlt) {
    COORD3 trueXYZ = lbh2xyz(trueLon, trueLat, trueAlt);
    std::vector<std::pair<double, double>> errors;
    for (const auto& pt : estimatedPoints) {
        errors.emplace_back(pt.p1 - trueXYZ.p1, pt.p2 - trueXYZ.p2);
        // pt.p3 与 trueXYZ.p3 相等，不参与误差椭圆统计
    }
    return errors;
}

// 计算二维协方差矩阵
std::vector<std::vector<double>> ErrorCircle::CalculateCovarianceMatrix(
    const std::vector<std::pair<double, double>>& errorPoints) {
    double meanX = 0, meanY = 0;
    for (const auto& e : errorPoints) {
        meanX += e.first;
        meanY += e.second;
    }
    meanX /= errorPoints.size();
    meanY /= errorPoints.size();
    double varXX = 0, varYY = 0, covXY = 0;
    for (const auto& e : errorPoints) {
        varXX += (e.first - meanX) * (e.first - meanX);
        varYY += (e.second - meanY) * (e.second - meanY);
        covXY += (e.first - meanX) * (e.second - meanY);
    }
    varXX /= (errorPoints.size() - 1);
    varYY /= (errorPoints.size() - 1);
    covXY /= (errorPoints.size() - 1);
    return {{varXX, covXY}, {covXY, varYY}};
}

// 协方差矩阵特征值分解（2x2）
void ErrorCircle::EigenDecomposition(
    const std::vector<std::vector<double>>& covMatrix,
    double& eig1, double& eig2,
    std::pair<double, double>& vec1,
    std::pair<double, double>& vec2) {
    double a = covMatrix[0][0], b = covMatrix[0][1], c = covMatrix[1][1];
    double trace = a + c;
    double det = a * c - b * b;
    double temp = std::sqrt(trace * trace - 4 * det);
    eig1 = (trace + temp) / 2.0;
    eig2 = (trace - temp) / 2.0;
    // 特征向量
    if (b != 0) {
        vec1 = {eig1 - c, b};
        vec2 = {eig2 - c, b};
    } else {
        vec1 = {1, 0};
        vec2 = {0, 1};
    }
    // 归一化
    double norm1 = std::sqrt(vec1.first * vec1.first + vec1.second * vec1.second);
    double norm2 = std::sqrt(vec2.first * vec2.first + vec2.second * vec2.second);
    if (norm1 > 0) { vec1.first /= norm1; vec1.second /= norm1; }
    if (norm2 > 0) { vec2.first /= norm2; vec2.second /= norm2; }
}

// 计算CEP
// CEP = 0.59 * (sigma1 + sigma2)
double ErrorCircle::CalculateCEP(double sigma1, double sigma2) {
    return 0.59 * (std::sqrt(sigma1) + std::sqrt(sigma2));
}

// 获取椭圆参数：长轴、短轴、方向角
void ErrorCircle::GetEllipseParams(
    const std::vector<std::vector<double>>& covMatrix,
    double& majorAxis, double& minorAxis, double& angleRad) {
    double eig1, eig2;
    std::pair<double, double> vec1, vec2;
    EigenDecomposition(covMatrix, eig1, eig2, vec1, vec2);
    // 长轴=较大特征值的均方根，短轴=较小特征值的均方根
    if (eig1 >= eig2) {
        majorAxis = std::sqrt(eig1);
        minorAxis = std::sqrt(eig2);
        angleRad = std::atan2(vec1.second, vec1.first);
    } else {
        majorAxis = std::sqrt(eig2);
        minorAxis = std::sqrt(eig1);
        angleRad = std::atan2(vec2.second, vec2.first);
    }
} 