#pragma once

#include <vector>
#include <string>
#include "CoordinateTransform.h"
#include "Vector3.h"

class MapView;

/**
 * @brief 双曲线绘制工具类
 * 用于在地图上绘制TDOA定位的双曲线
 */
class HyperbolaLines {
public:
    /**
     * @brief 绘制TDOA双曲线
     * @param mapView 地图视图指针
     * @param stationPositions 侦察站位置数组（空间直角坐标）
     * @param tdoas 时差数组（秒）
     * @param sourcePos 辐射源位置（空间直角坐标）
     * @param colors 颜色数组，用于区分不同双曲线
     * @param tdoaRmsErrorNs TDOA均方根误差（纳秒）
     * @param esmToaErrorNs ESM TOA误差（纳秒）
     * @return 绘制是否成功
     */
    static bool drawTDOAHyperbolas(
        MapView* mapView,
        const std::vector<COORD3>& stationPositions,
        const std::vector<double>& tdoas,
        const COORD3& sourcePos,
        const std::vector<std::string>& colors,
        double tdoaRmsErrorNs,
        double esmToaErrorNs
    );
    
    /**
     * @brief 清除地图上的所有双曲线
     * @param mapView 地图视图指针
     */
    static void clearHyperbolaLines(MapView* mapView);
    
    /**
     * @brief 计算双曲线上的点
     * @param focus1 焦点1（空间直角坐标）
     * @param focus2 焦点2（空间直角坐标）
     * @param tdoa 时差（秒）
     * @param tdoaError 时差误差（秒，用于误差带）
     * @param numPoints 生成的点数量
     * @param maxDistance 最大距离（米）
     * @return 双曲线上的点数组（空间直角坐标）
     */
    static std::vector<COORD3> calculateHyperbolaPoints(
        const COORD3& focus1,
        const COORD3& focus2,
        double tdoa,
        double tdoaError = 0.0,
        int numPoints = 200,
        double maxDistance = 500000.0
    );
    
    /**
     * @brief 在地图上绘制双曲线
     * @param mapView 地图视图指针
     * @param points 双曲线上的点数组（空间直角坐标）
     * @param color 双曲线颜色
     * @param lineWidth 线宽
     * @return 绘制是否成功
     */
    static bool drawHyperbolaLine(
        MapView* mapView,
        const std::vector<COORD3>& points,
        const std::string& color,
        double lineWidth,
        double planeHeight = 0.0);
};
