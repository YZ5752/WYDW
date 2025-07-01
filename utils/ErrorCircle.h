#pragma once
#include <vector>
#include "Vector3.h"
#include "CoordinateTransform.h"

class ErrorCircle {
public:
    // 计算误差点集（输入：定位点xyz，真实点l,b,h）
    static std::vector<std::pair<double, double>> CalculateErrorPoints(
        const std::vector<COORD3>& estimatedPoints,
        double trueLon, double trueLat, double trueAlt);

    // 计算二维协方差矩阵
    static std::vector<std::vector<double>> CalculateCovarianceMatrix(
        const std::vector<std::pair<double, double>>& errorPoints);

    // 计算协方差矩阵的特征值和特征向量
    static void EigenDecomposition(
        const std::vector<std::vector<double>>& covMatrix,
        double& eig1, double& eig2,
        std::pair<double, double>& vec1,
        std::pair<double, double>& vec2);

    // 计算CEP半径
    static double CalculateCEP(double sigma1, double sigma2);

    // 便于界面绘制：返回长轴、短轴、方向角
    static void GetEllipseParams(
        const std::vector<std::vector<double>>& covMatrix,
        double& majorAxis, double& minorAxis, double& angleRad);
}; 