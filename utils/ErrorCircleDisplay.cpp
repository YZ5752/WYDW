#include "ErrorCircleDisplay.h"
#include "../views/components/MapView.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iostream>
// 在地图上显示误差点
void showErrorPointsOnMap(MapView* mapView, const std::vector<COORD3>& points) {
    if (!mapView) return;
   for (size_t i = 0; i < points.size(); ++i) {
    //for (size_t i = 0; i < 1; ++i) {
        COORD3 lbh = xyz2lbh(points[i].p1, points[i].p2, points[i].p3);
        double lon = lbh.p1;
        double lat = lbh.p2;
        double alt = lbh.p3;
        
        // 使用自定义脚本添加只有红色点的标记
        std::stringstream script;
        script << "viewer.entities.add({";
        script << "position: Cesium.Cartesian3.fromDegrees(" << lon << ", " << lat << ", " << alt << "),";
        script << "point: {";
        script << "pixelSize: 5,"; // 点的大小
        script << "color: Cesium.Color.RED,"; // 红色点
        script << "outlineWidth: 0"; // 无轮廓
        script << "}";
        script << "});";
        mapView->executeScript(script.str());
    }
}

// 在地图上显示误差圆
void showErrorCircleOnMap(MapView* mapView, const COORD3& center, double radius) {
    if (!mapView) return;
    // 直接使用预测目标的大地坐标
    double lon = center.p1;
    double lat = center.p2;
    double alt = center.p3;
    // Cesium绘制圆的JS脚本
    std::stringstream script;
    script << "viewer.entities.add({";
    script << "position: Cesium.Cartesian3.fromDegrees(" << lon << ", " << lat << ", " << alt << "),";
    script << "ellipse: {";
    script << "semiMajorAxis: " << radius << ",";
    script << "semiMinorAxis: " << radius << ","; 
    script << "height: " << alt << ",";
    script << "material: Cesium.Color.TRANSPARENT,"; // 无填充
    script << "outline: true, outlineColor: Cesium.Color.BLACK";
    script << "}";
    script << "});";
    mapView->executeScript(script.str());
} 