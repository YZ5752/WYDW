#include "HyperbolaLines.h"
#include "../views/components/MapView.h"
#include "../constants/PhysicsConstants.h"
#include <sstream>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <random>

// 绘制TDOA双曲线
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
    
    // 清除之前的双曲线
    clearHyperbolaLines(mapView);
    
    if (stationPositions.size() < 2 || tdoas.size() < stationPositions.size() - 1) {
        std::cerr << "侦察站数量或时差数量不足，无法绘制双曲线" << std::endl;
        return false;
    }
    
    // 将纳秒转换为秒
    double tdoaRmsError = tdoaRmsErrorNs * 1e-9;
    double esmToaError = esmToaErrorNs * 1e-9;
    
    std::cout << "绘制双曲线，TDOA RMS误差: " << tdoaRmsError << " s, ESM TOA误差: " << esmToaError << " s" << std::endl;
    
    // 对每两个相邻的侦察站绘制一条双曲线
    for (size_t i = 0; i < stationPositions.size() - 1; ++i) {
        // 获取两个侦察站的位置
        const COORD3& station1 = stationPositions[0]; // 参考站始终是第一个站
        const COORD3& station2 = stationPositions[i + 1];
        
        // 计算两站之间的TDOA值
        double tdoa = tdoas[i + 1];  // 相对于参考站的TDOA
        
        // 考虑误差的影响
        // 1. ESM TOA误差 (系统误差) - 导致双曲线平移
        // 根据公式: Δt^meas = Δt + δA
        double tdoa_with_system_error = tdoa;
        if (esmToaError != 0.0) {
            // 参考站的系统误差会影响所有TDOA值
            tdoa_with_system_error = tdoa + esmToaError;
            std::cout << "应用系统误差后的TDOA: " << tdoa_with_system_error << " s (原始: " << tdoa << " s)" << std::endl;
        }
        
        // 2. TDOA RMS误差 (随机误差) - 导致双曲线变为"带"
        // 根据公式: Δd = c·(Δt + ε)，我们需要绘制误差带
        // 我们绘制三条线：中心线、+1σ线和-1σ线
        
        // 计算中心双曲线上的点
        std::vector<COORD3> centerPoints = calculateHyperbolaPoints(
            station1, station2, tdoa_with_system_error, 0.0);
            
        // 计算+1σ双曲线上的点
        std::vector<COORD3> upperPoints;
        if (tdoaRmsError > 0.0) {
            upperPoints = calculateHyperbolaPoints(
                station1, station2, tdoa_with_system_error + tdoaRmsError, 0.0);
        }
        
        // 计算-1σ双曲线上的点
        std::vector<COORD3> lowerPoints;
        if (tdoaRmsError > 0.0) {
            lowerPoints = calculateHyperbolaPoints(
                station1, station2, tdoa_with_system_error - tdoaRmsError, 0.0);
        }
        
        // 选择颜色
        std::string color = (i < colors.size()) ? colors[i] : "#FF0000";
        
        // 绘制中心双曲线
        bool success = drawHyperbolaLine(mapView, centerPoints, color, 2.0);
        if (!success) {
            std::cerr << "绘制第 " << i+1 << " 条中心双曲线失败" << std::endl;
            return false;
        }
        
        // 如果有误差，绘制误差带
        if (tdoaRmsError > 0.0 && !upperPoints.empty() && !lowerPoints.empty()) {
            // 使用半透明颜色绘制误差边界线
            std::string errorColor = color + "80"; // 添加50%透明度
            drawHyperbolaLine(mapView, upperPoints, errorColor, 1.0);
            drawHyperbolaLine(mapView, lowerPoints, errorColor, 1.0);
            
            std::cout << "已绘制第 " << i+1 << " 条双曲线的误差带 (±" << tdoaRmsError * 1e9 << " ns)" << std::endl;
        }
    }
    
    return true;
}

// 清除地图上的所有双曲线
void HyperbolaLines::clearHyperbolaLines(MapView* mapView) {
    if (!mapView) return;
    
    std::string script = "var existingHyperbolas = viewer.entities.getById('tdoa-hyperbolas');\n"
                         "if (existingHyperbolas) {\n"
                         "  viewer.entities.removeById('tdoa-hyperbolas');\n"
                         "}\n";
    
    mapView->executeScript(script);
}

// 计算双曲线上的点
std::vector<COORD3> HyperbolaLines::calculateHyperbolaPoints(
    const COORD3& focus1,
    const COORD3& focus2,
    double tdoa,
    double tdoaError,
    int numPoints,
    double maxDistance) {
    
    std::vector<COORD3> points;
    
    // 计算两个焦点之间的距离
    double focusDistance = std::sqrt(
        std::pow(focus2.p1 - focus1.p1, 2) +
        std::pow(focus2.p2 - focus1.p2, 2) +
        std::pow(focus2.p3 - focus1.p3, 2)
    );
    
    // 计算双曲线的常数2a (距离差)
    double constantA = std::abs(tdoa) * Constants::c;
    
    // 如果2a大于或等于焦距，则无法形成双曲线
    if (constantA >= focusDistance) {
        std::cerr << "TDOA值过大，无法形成双曲线: " << tdoa << " 秒" << std::endl;
        return points;
    }
    
    // 计算中点
    COORD3 midPoint = {
        (focus1.p1 + focus2.p1) / 2.0,
        (focus1.p2 + focus2.p2) / 2.0,
        (focus1.p3 + focus2.p3) / 2.0
    };
    
    // 计算焦点之间的单位向量
    double dirX = (focus2.p1 - focus1.p1) / focusDistance;
    double dirY = (focus2.p2 - focus1.p2) / focusDistance;
    double dirZ = (focus2.p3 - focus1.p3) / focusDistance;
    
    // 找到垂直于焦点连线的两个单位向量
    double perpX1, perpY1, perpZ1;
    double perpX2, perpY2, perpZ2;
    
    // 找到第一个垂直向量
    if (std::abs(dirX) < std::abs(dirY) && std::abs(dirX) < std::abs(dirZ)) {
        // 如果x分量最小，使用(1,0,0)与方向向量叉乘
        perpX1 = 0;
        perpY1 = -dirZ;
        perpZ1 = dirY;
    } else if (std::abs(dirY) < std::abs(dirZ)) {
        // 如果y分量最小，使用(0,1,0)与方向向量叉乘
        perpX1 = dirZ;
        perpY1 = 0;
        perpZ1 = -dirX;
    } else {
        // 如果z分量最小，使用(0,0,1)与方向向量叉乘
        perpX1 = -dirY;
        perpY1 = dirX;
        perpZ1 = 0;
    }
    
    // 归一化第一个垂直向量
    double perpLen1 = std::sqrt(perpX1*perpX1 + perpY1*perpY1 + perpZ1*perpZ1);
    perpX1 /= perpLen1;
    perpY1 /= perpLen1;
    perpZ1 /= perpLen1;
    
    // 计算第二个垂直向量（叉乘）
    perpX2 = dirY * perpZ1 - dirZ * perpY1;
    perpY2 = dirZ * perpX1 - dirX * perpZ1;
    perpZ2 = dirX * perpY1 - dirY * perpX1;
    
    // 计算双曲线的参数
    double c = focusDistance / 2.0;  // 半焦距
    double a = constantA / 2.0;      // 半距离差
    double b = std::sqrt(c*c - a*a); // 半轴长b
    
    // 生成双曲线上的点
    points.reserve(numPoints);
    
    // 计算参数范围
    double tMin = 0.01;  // 避免奇点
    double tMax = std::atan(maxDistance / b);
    
    // 沿双曲线生成点
    for (int i = 0; i < numPoints / 2; ++i) {
        // 参数t在[tMin, tMax]范围内均匀分布
        double t = tMin + (tMax - tMin) * i / (numPoints / 2 - 1);
        
        // 计算双曲线上的点 (正向和负向)
        double x = a * std::cosh(t);
        double y = b * std::sinh(t);
        
        // 在3D空间中旋转点
        // 正向点
        COORD3 point1;
        point1.p1 = midPoint.p1 + x * dirX + y * perpX1;
        point1.p2 = midPoint.p2 + x * dirY + y * perpY1;
        point1.p3 = midPoint.p3 + x * dirZ + y * perpZ1;
        points.push_back(point1);
        
        // 负向点
        COORD3 point2;
        point2.p1 = midPoint.p1 + x * dirX - y * perpX1;
        point2.p2 = midPoint.p2 + x * dirY - y * perpY1;
        point2.p3 = midPoint.p3 + x * dirZ - y * perpZ1;
        points.push_back(point2);
        
        // 如果需要考虑Z轴方向，可以添加更多点
        if (numPoints > 100) {  // 只有当点数足够多时才添加
            COORD3 point3;
            point3.p1 = midPoint.p1 + x * dirX + y * perpX2;
            point3.p2 = midPoint.p2 + x * dirY + y * perpY2;
            point3.p3 = midPoint.p3 + x * dirZ + y * perpZ2;
            points.push_back(point3);
            
            COORD3 point4;
            point4.p1 = midPoint.p1 + x * dirX - y * perpX2;
            point4.p2 = midPoint.p2 + x * dirY - y * perpY2;
            point4.p3 = midPoint.p3 + x * dirZ - y * perpZ2;
            points.push_back(point4);
        }
    }
    
    return points;
}

// 在地图上绘制双曲线
bool HyperbolaLines::drawHyperbolaLine(
    MapView* mapView,
    const std::vector<COORD3>& points,
    const std::string& color,
    double lineWidth) {
    
    if (!mapView || points.empty()) {
        return false;
    }
    
    // 将空间直角坐标转换为大地坐标
    std::vector<COORD3> lbhPoints;
    lbhPoints.reserve(points.size());
    
    for (const auto& point : points) {
        COORD3 lbh = xyz2lbh(point.p1, point.p2, point.p3);
        lbhPoints.push_back(lbh);
    }
    
    // 构建JavaScript代码绘制双曲线
    std::stringstream script;
    
    // 确保存在双曲线容器
    script << "// 确保存在双曲线容器\n"
           << "var hyperbolaContainer = viewer.entities.getById('tdoa-hyperbolas');\n"
           << "if (!hyperbolaContainer) {\n"
           << "  hyperbolaContainer = viewer.entities.add({\n"
           << "    id: 'tdoa-hyperbolas',\n"
           << "    name: 'TDOA双曲线'\n"
           << "  });\n"
           << "}\n";
    
    // 创建双曲线点位置数组
    script << "// 创建双曲线点位置数组\n"
           << "var positions = [];\n";
    
    // 添加所有点
    for (const auto& point : lbhPoints) {
        script << "positions.push(Cesium.Cartesian3.fromDegrees("
               << point.p1 << ", " << point.p2 << ", " << point.p3
               << "));\n";
    }
    
    // 创建双曲线实体
    script << "// 创建双曲线实体\n"
           << "viewer.entities.add({\n"
           << "  parent: hyperbolaContainer,\n"
           << "  polyline: {\n"
           << "    positions: positions,\n"
           << "    width: " << lineWidth << ",\n"
           << "    material: new Cesium.ColorMaterialProperty(Cesium.Color.fromCssColorString('" << color << "')),\n"
           << "    clampToGround: false\n"
           << "  }\n"
           << "});\n";
    
    // 执行JavaScript代码
    mapView->executeScript(script.str());
    
    return true;
}
