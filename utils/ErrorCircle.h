#pragma once
#include <vector>
#include <string>
#include "CoordinateTransform.h"
#include "../constants/PhysicsConstants.h"

// 测向定位结果结构体
struct DFResult {
    std::vector<COORD3> estimatedPoints;  // 100个误差点(空间直角坐标)
    double cepRadius;                     // 误差圆半径(米)
    DFResult() : cepRadius(0.0) {}
    DFResult(const std::vector<COORD3>& points, double radius)
        : estimatedPoints(points), cepRadius(radius) {}
};

// 时差定位结果结构体
struct TDOAResult {
    std::vector<COORD3> estimatedPoints;  // 100个误差点(空间直角坐标)
    double cepRadius;                     // 误差圆半径(米)
    TDOAResult() : cepRadius(0.0) {}
    TDOAResult(const std::vector<COORD3>& points, double radius)
        : estimatedPoints(points), cepRadius(radius) {}
};

// 测向体制误差圆计算函数
DFResult calculateDFErrorCircle(
    const std::vector<std::string>& deviceNames,
    const std::string& sourceName,
    double esm1ErrorMean,
    double esm1ErrorSigma,
    double esm2ErrorMean,
    double esm2ErrorSigma,
    unsigned int seed = 0
);

// 时差体制误差圆计算函数
TDOAResult calculateTDOAErrorCircle(
    const std::vector<std::string>& deviceNames,
    const std::string& sourceName,
    double tdoaRmsError,
    double esmToaError,
    unsigned int seed = 0
);
