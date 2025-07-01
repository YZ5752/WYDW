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
        if (i < 10) {
            std::cout << "误差点" << i+1 << ": 经度=" << lon << ", 纬度=" << lat << ", 高度=" << alt << std::endl;
        }
       // 只用最简单的点实体，避免批量添加时 Cesium 脚本出错
        mapView->addMarker(lon, lat, "", "", "#FF0000");
    }
}

// 在地图上显示误差圆
void showErrorCircleOnMap(MapView* mapView, const COORD3& center, double radius) {
    if (!mapView) return;
    // 直接使用预测目标的大地坐标
    double lon = center.p1;
    double lat = center.p2;
    double alt = center.p3;
    std::cout <<"圆心位置：" << "经度: " << lon << ", 纬度: " << lat << ", 高度: " << alt << std::endl;
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