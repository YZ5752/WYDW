#pragma once

#include <vector>
#include <string>
#include "../views/components/MapView.h"
#include "CoordinateTransform.h"
#include "../constants/PhysicsConstants.h"

// 时差定位双曲线绘制类
class HyperbolaLines {
public:
    // 绘制TDOA双曲线
    static bool drawTDOAHyperbolas(
        MapView* mapView,
        const std::vector<COORD3>& stationPositions,  // 侦察站位置
        const std::vector<double>& tdoas,             // 时差数据
        const COORD3& targetPosition,                 // 目标位置
        const std::vector<std::string>& colors = {"blue", "red", "green"}  // 每条双曲线的颜色
    );
    
    // 清除双曲线
    static void clearHyperbolaLines(MapView* mapView);
    
    // 计算两点之间的距离
    static double calculateDistance(const COORD3& p1, const COORD3& p2);
    
private:
    // 生成双曲线上的点
    static std::vector<COORD3> generateHyperbolaPoints(
        const COORD3& station1,  // 参考站
        const COORD3& station2,  // 从站
        double tdoa,             // 时差值
        const COORD3& center,    // 中心点(用于确定绘制范围)
        double range             // 绘制范围(米)
    );
};

// TDOA误差结果结构体
struct TDOAResult {
    std::vector<COORD3> estimatedPoints;  // 误差点(空间直角坐标)
    double cepRadius;                     // 误差圆半径(米)
    COORD3 centerPoint;                   // 误差圆圆心
    
    TDOAResult() : cepRadius(0.0) {}
    TDOAResult(const std::vector<COORD3>& points, double radius)
        : estimatedPoints(points), cepRadius(radius) {}
    TDOAResult(const std::vector<COORD3>& points, double radius, const COORD3& center)
        : estimatedPoints(points), cepRadius(radius), centerPoint(center) {}
};

// 计算TDOA误差圆
TDOAResult calculateTDOAErrorCircle(
    const std::vector<std::string>& deviceNames,
    const std::string& sourceName,
    double tdoaErrorMean,    // 时差误差均值(秒)
    double tdoaErrorSigma,   // 时差误差标准差(秒)
    unsigned int seed = 0    // 随机种子
); 