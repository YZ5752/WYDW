#ifndef DIRECTION_ERROR_LINES_H
#define DIRECTION_ERROR_LINES_H

#include <string>
#include "../views/components/MapView.h"
#include "../models/ReconnaissanceDeviceModel.h"

/**
 * @brief 测向误差线显示工具类
 * 
 * 该类用于在地图上显示从侦察设备发散出的测向误差线
 */
class DirectionErrorLines {
public:
    /**
     * @brief 构造函数
     */
    DirectionErrorLines();

    /**
     * @brief 析构函数
     */
    ~DirectionErrorLines();

    /**
     * @brief 在地图上显示测向误差线
     * 
     * @param mapView 地图视图对象
     * @param device 侦察设备对象
     * @param targetLongitude 目标辐射源经度
     * @param targetLatitude 目标辐射源纬度
     * @param targetAltitude 目标辐射源高度
     * @param errorAngle 测向误差角度（度）
     * @param lineColor 线条颜色
     * @param lineLength 线条长度（米）
     * @return 是否成功显示
     */
    bool showDirectionErrorLines(
        MapView* mapView,
        const ReconnaissanceDevice& device,
        double targetLongitude,
        double targetLatitude,
        double targetAltitude,
        double errorAngle,
        const std::string& lineColor = "#FF0000",
        double lineLength = 10000.0
    );
    
    /**
     * @brief 显示测向模拟的误差线
     * 
     * 该方法根据均值误差和标准差绘制3条线:
     * 1. 中心线(方位角 + 均值误差)
     * 2. 左边界线(中心线 - 标准差)
     * 3. 右边界线(中心线 + 标准差)
     * 
     * @param mapView 地图视图对象
     * @param device 侦察设备对象
     * @param targetLongitude 目标辐射源经度
     * @param targetLatitude 目标辐射源纬度
     * @param targetAltitude 目标辐射源高度
     * @param meanErrorDeg 均值误差(度)
     * @param stdDevDeg 标准差(度)
     * @param lineColor 线条颜色
     * @param lineLength 线条长度(米)
     * @return 是否成功显示
     */
    bool showDirectionSimulationLines(
        MapView* mapView,
        const ReconnaissanceDevice& device,
        double targetLongitude,
        double targetLatitude,
        double targetAltitude,
        double meanErrorDeg,
        double stdDevDeg,
        const std::string& lineColor = "#FF0000",
        double lineLength = 20000.0
    );

    /**
     * @brief 清除测向误差线
     * 
     * @param mapView 地图视图对象
     */
    void clearDirectionErrorLines(MapView* mapView);

private:
    /**
     * @brief 计算从起点沿指定方位角和俯仰角延伸指定距离后的终点坐标
     * 
     * @param startLongitude 起点经度
     * @param startLatitude 起点纬度
     * @param startAltitude 起点高度
     * @param azimuth 方位角（度）
     * @param elevation 俯仰角（度）
     * @param distance 距离（米）
     * @return 终点坐标(经度,纬度,高度)
     */
    std::tuple<double, double, double> calculateEndPoint(
        double startLongitude,
        double startLatitude,
        double startAltitude,
        double azimuth,
        double elevation,
        double distance
    );
};

#endif // DIRECTION_ERROR_LINES_H 