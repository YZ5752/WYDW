#include "../HyperbolaLines.h"
#include "../../views/components/MapView.h"
#include "../../constants/PhysicsConstants.h"
#include <sstream>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <random>
#include <limits>
#include <iomanip>

// 地球半径（米）
const double EARTH_RADIUS = 6378137.0;

// 辅助函数：将大地坐标（经纬度，度）转换为局部二维平面坐标（米）
// 以参考点（refLon, refLat）为原点，x轴向东，y轴向北
std::pair<double, double> lonLatToXY(double lon, double lat, double refLon, double refLat) {
    // 转换为弧度
    double lonRad = lon * M_PI / 180.0;
    double latRad = lat * M_PI / 180.0;
    double refLonRad = refLon * M_PI / 180.0;
    double refLatRad = refLat * M_PI / 180.0;

    // 计算局部平面坐标（米）
    double x = EARTH_RADIUS * (lonRad - refLonRad) * cos(refLatRad);
    double y = EARTH_RADIUS * (latRad - refLatRad);
    return {x, y};
}

// 辅助函数：将局部二维平面坐标（米）转换为大地坐标（经纬度，度）
std::pair<double, double> xyToLonLat(double x, double y, double refLon, double refLat) {
    // 转换为弧度
    double refLonRad = refLon * M_PI / 180.0;
    double refLatRad = refLat * M_PI / 180.0;

    // 计算经纬度（弧度）
    double lonRad = refLonRad + x / (EARTH_RADIUS * cos(refLatRad));
    double latRad = refLatRad + y / EARTH_RADIUS;

    // 转换为度
    return {lonRad * 180.0 / M_PI, latRad * 180.0 / M_PI};
}

bool HyperbolaLines::drawTDOAHyperbolas(
    MapView* mapView,
    const std::vector<COORD3>& stationPositions,
    const std::vector<double>& tdoas,
    const COORD3& sourcePos,
    const std::vector<std::string>& colors,
    double tdoaRmsErrorNs,
    double esmToaErrorNs) {
    
    if (!mapView) {
        std::cerr << "MapView对象为空，无法绘制双曲线" << std::endl;
        return false;
    }
    
    clearHyperbolaLines(mapView);
    
    if (stationPositions.size() < 2 || tdoas.size() < stationPositions.size() - 1) {
        std::cerr << "侦察站数量或时差数量不足，无法绘制双曲线" << std::endl;
        return false;
    }
    
    double tdoaRmsError = tdoaRmsErrorNs * 1e-9;
    double esmToaError = esmToaErrorNs * 1e-9;

    
    std::cout << "绘制双曲线，TDOA RMS误差: " << tdoaRmsError * 1e9 << " ns, ESM TOA误差: " << esmToaError * 1e9 << " ns" << std::endl;
    std::cout << "侦察站数量: " << stationPositions.size() << std::endl;
    std::cout << "TDOA数量: " << tdoas.size() << std::endl;
    
    // 确定绘制平面高度（辐射源高度）
    COORD3 sourcePos_lbh = xyz2lbh(sourcePos.p1, sourcePos.p2, sourcePos.p3);
    double planeHeight = sourcePos_lbh.p3;
    if (planeHeight < 0) planeHeight = 0;
    
    std::cout << "双曲线绘制平面高度: " << planeHeight << " m" << std::endl;
    std::cout << "辐射源空间直角坐标: (" << sourcePos.p1 << ", " << sourcePos.p2 << ", " << sourcePos.p3 << ") m" << std::endl;
    std::cout << "辐射源大地坐标: (" << sourcePos_lbh.p1 << "°, " << sourcePos_lbh.p2 << "°, " << sourcePos_lbh.p3 << " m)" << std::endl;
    
    // 转换所有站点到大地坐标并投影到平面
    std::vector<COORD3> stationsOnPlaneLbh;
    for (const auto& station : stationPositions) {
        COORD3 station_lbh = xyz2lbh(station.p1, station.p2, station.p3);
        station_lbh.p3 = planeHeight; // 固定高度
        stationsOnPlaneLbh.push_back(station_lbh);
        std::cout << "站点投影大地坐标: (" << station_lbh.p1 << "°, " << station_lbh.p2 << "°, " << planeHeight << " m)" << std::endl;
    }
    
    // 辐射源投影坐标
    COORD3 sourcePos_plane_lbh = sourcePos_lbh;
    sourcePos_plane_lbh.p3 = planeHeight;
    std::cout << "辐射源投影坐标: (" << sourcePos_plane_lbh.p1 << "°, " << sourcePos_plane_lbh.p2 << "°, " << planeHeight << " m)" << std::endl;
    
    // 计算站点到辐射源的平面距离（二维）
    std::vector<double> stationToSourceDistances;
    std::cout << "\n--- 站点到辐射源平面距离 ---" << std::endl;
    // 以辐射源为参考点转换二维坐标
    double refLon = sourcePos_plane_lbh.p1;
    double refLat = sourcePos_plane_lbh.p2;
    for (size_t i = 0; i < stationsOnPlaneLbh.size(); ++i) {
        auto [x, y] = lonLatToXY(stationsOnPlaneLbh[i].p1, stationsOnPlaneLbh[i].p2, refLon, refLat);
        // 辐射源在二维坐标中为(0,0)
        double distance = std::sqrt(x*x + y*y);
        stationToSourceDistances.push_back(distance);
        std::cout << "站点" << i << "到辐射源距离: " << std::fixed << std::setprecision(6) << distance << " 米" << std::endl;
    }
    
    // 绘制各双曲线
    for (size_t i = 1; i < stationsOnPlaneLbh.size(); ++i) {
        const COORD3& refStationLbh = stationsOnPlaneLbh[0];
        const COORD3& otherStationLbh = stationsOnPlaneLbh[i];
        double tdoa = tdoas[i - 1];
        
        // 理论距离差
        double distanceDiff = stationToSourceDistances[i] - stationToSourceDistances[0];
        double tdoaDistanceDiff = tdoa * Constants::c;
        
        std::cout << std::scientific << std::setprecision(6);
        std::cout << "绘制双曲线 " << i << "，参考站与站点" << i << "之间的TDOA: " << tdoa << " 秒" << std::endl;
        std::cout << "理论距离差: " << distanceDiff << " 米" << std::endl;
        std::cout << "TDOA距离差: " << tdoaDistanceDiff << " 米" << std::endl;
        std::cout << "距离差误差: " << (tdoaDistanceDiff - distanceDiff) << " 米" << std::endl;
        
        // 应用系统误差
        double tdoa_with_system_error = tdoa;
        if (esmToaError != 0.0) {
            tdoa_with_system_error = tdoa - esmToaError;
            std::cout << "应用ESM TOA系统误差后的TDOA: " << tdoa_with_system_error << " s" << std::endl;
        }
        
        // 颜色选择
        std::vector<std::string> defaultColors = {"#FFFF00", "#00FFFF", "#FF00FF", "#0000FF", "#FF0000"};
        std::string color = (i - 1 < colors.size()) ? colors[i - 1] : defaultColors[(i - 1) % defaultColors.size()];
        std::cout << "双曲线" << i << "颜色: " << color << std::endl;
        
        // 计算双曲线点（二维）
        std::vector<COORD3> centerPoints = calculateHyperbolaPoints(
            refStationLbh, otherStationLbh, tdoa_with_system_error, planeHeight, refLon, refLat);
        
        // 绘制（无误差带时直接绘制中心线）
        if (tdoaRmsError <= 0.0) {
            bool success = drawHyperbolaLine(mapView, centerPoints, color, 1.5, planeHeight);
            if (!success) {
                std::cerr << "绘制第 " << i << " 条中心双曲线失败" << std::endl;
                return false;
            }
        } else {
            // 误差带处理（保留原逻辑，仅中心计算改为二维）
            std::vector<COORD3> upperPoints = calculateHyperbolaPoints(
                refStationLbh, otherStationLbh, tdoa_with_system_error + tdoaRmsError, planeHeight, refLon, refLat);
            std::vector<COORD3> lowerPoints = calculateHyperbolaPoints(
                refStationLbh, otherStationLbh, tdoa_with_system_error - tdoaRmsError, planeHeight, refLon, refLat);
            
            drawHyperbolaLine(mapView, upperPoints, color, 1.5, planeHeight);
            drawHyperbolaLine(mapView, lowerPoints, color, 1.5, planeHeight);
            
            // 填充误差带
            if (upperPoints.size() > 10 && lowerPoints.size() > 10) {
                std::stringstream fillScript;
                fillScript << "var fillPositions = [];\n";
                int skip = std::max(1, (int)upperPoints.size() / 20);
                for (size_t j = 0; j < upperPoints.size(); j += skip) {
                    COORD3 lbh = xyz2lbh(upperPoints[j].p1, upperPoints[j].p2, upperPoints[j].p3);
                    fillScript << "fillPositions.push(Cesium.Cartesian3.fromDegrees("
                        << lbh.p1 << ", " << lbh.p2 << ", " << planeHeight << "));\n";
                }
                for (int j = lowerPoints.size() - 1; j >= 0; j -= skip) {
                    COORD3 lbh = xyz2lbh(lowerPoints[j].p1, lowerPoints[j].p2, lowerPoints[j].p3);
                    fillScript << "fillPositions.push(Cesium.Cartesian3.fromDegrees("
                        << lbh.p1 << ", " << lbh.p2 << ", " << planeHeight << "));\n";
                }
                if (!upperPoints.empty()) {
                    COORD3 lbh = xyz2lbh(upperPoints[0].p1, upperPoints[0].p2, upperPoints[0].p3);
                    fillScript << "fillPositions.push(Cesium.Cartesian3.fromDegrees("
                        << lbh.p1 << ", " << lbh.p2 << ", " << planeHeight << "));\n";
                }
                fillScript << "viewer.entities.add({\n"
                    << "  parent: hyperbolaContainer,\n"
                    << "  polygon: {\n"
                    << "    hierarchy: new Cesium.PolygonHierarchy(fillPositions),\n"
                    << "    material: Cesium.Color.fromCssColorString('" << color << "').withAlpha(0.15),\n"
                    << "    height: " << planeHeight << "\n"
                    << "  }\n"
                    << "});\n";
                mapView->executeScript(fillScript.str());
                std::cout << "已添加误差带填充区域" << std::endl;
            }
            std::cout << "已绘制第 " << i << " 条双曲线的误差带" << std::endl;
        }
    }
    
    // 添加辐射源标记（修正字符串拼接错误）
    std::stringstream scriptStream;
    scriptStream << "viewer.entities.add({\n"
                 << "  parent: hyperbolaContainer,\n"
                 << "  position: Cesium.Cartesian3.fromDegrees(" 
                 << sourcePos_lbh.p1 << ", " 
                 << sourcePos_lbh.p2 << ", " 
                 << planeHeight << "),\n"
                 << "  point: {\n"
                 << "    pixelSize: 10,\n"
                 << "    color: Cesium.Color.RED,\n"
                 << "    outlineColor: Cesium.Color.WHITE,\n"
                 << "    outlineWidth: 2\n"
                 << "  }\n"
                 << "});\n";
    std::string script = scriptStream.str();
    mapView->executeScript(script);
    
    std::cout << "双曲线绘制结果: 成功" << std::endl;
    std::cout << "========= 结束绘制双曲线 =========" << std::endl;
    
    return true;
}

void HyperbolaLines::clearHyperbolaLines(MapView* mapView) {
    if (!mapView) return;
    std::string script = "var hyperbolaContainer = viewer.entities.getById('tdoa-hyperbolas');\n"
                         "if (hyperbolaContainer) viewer.entities.remove(hyperbolaContainer);\n"
                         "viewer.entities.add({id: 'tdoa-hyperbolas', name: 'TDOA双曲线'});\n";
    mapView->executeScript(script);
}

// 二维双曲线点计算
std::vector<COORD3> HyperbolaLines::calculateHyperbolaPoints(
    const COORD3& focus1Lbh,  // 焦点1大地坐标（经纬度，度）
    const COORD3& focus2Lbh,  // 焦点2大地坐标（经纬度，度）
    double tdoa,              // TDOA（秒）
    double planeHeight,       // 平面高度（米）
    double refLon,            // 参考点经度（度）
    double refLat) {          // 参考点纬度（度）
    
    std::vector<COORD3> points;
    const double PRECISION_FACTOR = 1e-10;
    
    // 1. 将焦点转换为二维平面坐标（以参考点为原点）
    auto [x1, y1] = lonLatToXY(focus1Lbh.p1, focus1Lbh.p2, refLon, refLat);
    auto [x2, y2] = lonLatToXY(focus2Lbh.p1, focus2Lbh.p2, refLon, refLat);
    
    // 2. 计算二维焦距（焦点间距离）
    double focusDistance = std::sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
    std::cout << "焦点间平面距离: " << focusDistance << " 米" << std::endl;
    
    // 3. 距离差（常数）
    double constantA = std::abs(tdoa) * Constants::c;
    std::cout << "距离差: " << constantA << " 米" << std::endl;
    
    // 检查双曲线可行性
    if (constantA >= focusDistance - PRECISION_FACTOR) {
        std::cout << "调整距离差以形成双曲线: " << constantA << " -> " << (focusDistance - 1e-3) << std::endl;
        constantA = focusDistance - 1e-3;  // 确保小于焦距
    }
    
    // 4. 双曲线参数（二维）
    double c = focusDistance / 2.0;  // 半焦距
    double a = constantA / 2.0;      // 半距离差
    double b = std::sqrt(std::max(c*c - a*a, PRECISION_FACTOR));  // 避免负数
    
    // 5. 焦点中点（二维）
    double midX = (x1 + x2) / 2.0;
    double midY = (y1 + y2) / 2.0;
    
    // 6. 焦点连线方向向量（二维单位向量）
    double dirX = (x2 - x1) / focusDistance;
    double dirY = (y2 - y1) / focusDistance;
    
    // 7. 垂直方向向量（二维，垂直于焦点连线）
    double perpX = -dirY;  // 垂直向量
    double perpY = dirX;
    
    // 8. 确定双曲线分支（靠近哪个焦点）
    bool closerToFocus1 = (tdoa > 0);  // TDOA>0: 信号先到焦点1
    
    // 9. 生成双曲线上的点（参数方程）
    const int numPoints = 500;
    double angleRange = M_PI * 0.95;  // 角度范围（接近180度）
    double angleStep = angleRange / (numPoints - 1);
    
    for (int i = 0; i < numPoints; ++i) {
        double angle = -angleRange/2 + i * angleStep;
        double secTheta = 1.0 / std::cos(angle);
        double tanTheta = std::tan(angle);
        
        // 二维参数方程
        double paramX = a * secTheta;
        double paramY = b * tanTheta;
        
        // 计算点坐标（二维）
        double x, y;
        if (closerToFocus1) {
            // 靠近焦点1的分支
            x = midX - paramX * dirX + paramY * perpX;
            y = midY - paramX * dirY + paramY * perpY;
        } else {
            // 靠近焦点2的分支
            x = midX + paramX * dirX + paramY * perpX;
            y = midY + paramX * dirY + paramY * perpY;
        }
        
        // 转换回大地坐标（经纬度）
        auto [lon, lat] = xyToLonLat(x, y, refLon, refLat);
        // 转换为空间直角坐标（用于返回）
        COORD3 xyz = lbh2xyz(lon, lat, planeHeight);
        points.push_back(xyz);
    }
    
    // 输出参数
    std::cout << "双曲线参数:" << std::endl;
    std::cout << "  半焦距(c): " << c << " 米" << std::endl;
    std::cout << "  半距离差(a): " << a << " 米" << std::endl;
    std::cout << "  半轴长(b): " << b << " 米" << std::endl;
    std::cout << "  偏心率(e): " << c/a << std::endl;
    std::cout << "  生成点数: " << points.size() << std::endl;
    std::cout << "  弯曲方向: " << (closerToFocus1 ? "朝向焦点1(参考站)" : "朝向焦点2") << std::endl;
    
    return points;
}

bool HyperbolaLines::drawHyperbolaLine(
    MapView* mapView,
    const std::vector<COORD3>& points,
    const std::string& color,
    double lineWidth,
    double planeHeight) {
    
    if (!mapView || points.empty()) {
        std::cerr << "绘制双曲线失败: " << (mapView ? "点为空" : "MapView为空") << std::endl;
        return false;
    }
    
    std::vector<COORD3> lbhPoints;
    for (const auto& point : points) {
        lbhPoints.push_back(xyz2lbh(point.p1, point.p2, point.p3));
    }
    
    std::stringstream script;
    script << "var hyperbolaContainer = viewer.entities.getById('tdoa-hyperbolas');\n"
           << "if (!hyperbolaContainer) {\n"
           << "  hyperbolaContainer = viewer.entities.add({id: 'tdoa-hyperbolas', name: 'TDOA双曲线'});\n"
           << "}\n";
    
    script << "var positions = [];\n";
    int skip = lbhPoints.size() > 200 ? lbhPoints.size() / 200 : 1;
    for (size_t i = 0; i < lbhPoints.size(); i += skip) {
        const auto& p = lbhPoints[i];
        script << "positions.push(Cesium.Cartesian3.fromDegrees("
               << p.p1 << ", " << p.p2 << ", " << planeHeight
               << "));\n";
    }
    
    script << "viewer.entities.add({\n"
           << "  parent: hyperbolaContainer,\n"
           << "  polyline: {\n"
           << "    positions: positions,\n"
           << "    width: " << lineWidth << ",\n"
           << "    material: Cesium.Color.fromCssColorString('" << color << "'),\n"
           << "    clampToGround: false,\n"
           << "    arcType: Cesium.ArcType.NONE\n"
           << "  }\n"
           << "});\n";
    
    mapView->executeScript(script.str());
    return true;
} 