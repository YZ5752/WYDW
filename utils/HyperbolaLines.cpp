#include "HyperbolaLines.h"
#include "../views/components/MapView.h"
#include "../constants/PhysicsConstants.h"
#include <sstream>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <random>
#include <limits> // 添加用于std::numeric_limits

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
    
    // 如果没有设置误差，使用默认值确保显示误差带
    if (tdoaRmsError <= 0.0) {
        tdoaRmsError = 5e-9; // 默认5纳秒的误差
        std::cout << "使用默认TDOA RMS误差: " << tdoaRmsError * 1e9 << " ns" << std::endl;
    }
    
    std::cout << "绘制双曲线，TDOA RMS误差: " << tdoaRmsError * 1e9 << " ns, ESM TOA误差: " << esmToaError * 1e9 << " ns" << std::endl;
    std::cout << "侦察站数量: " << stationPositions.size() << std::endl;
    std::cout << "TDOA数量: " << tdoas.size() << std::endl;
    
    // 确定绘制平面的高度（使用辐射源的高度）
    // 将空间直角坐标转换为大地坐标
    COORD3 sourcePos_lbh = xyz2lbh(sourcePos.p1, sourcePos.p2, sourcePos.p3);
    double planeHeight = sourcePos_lbh.p3; // 使用转换后的大地高度
    if (planeHeight < 0) planeHeight = 0; // 确保高度不为负
    
    std::cout << "双曲线绘制平面高度: " << planeHeight << " m" << std::endl;
    std::cout << "辐射源空间直角坐标: (" << sourcePos.p1 << ", " << sourcePos.p2 << ", " << sourcePos.p3 << ") m" << std::endl;
    std::cout << "辐射源大地坐标: (" << sourcePos_lbh.p1 << "°, " << sourcePos_lbh.p2 << "°, " << sourcePos_lbh.p3 << " m)" << std::endl;
    
    // 使用与图片中相似的颜色（黄色、青色、洋红色）
    std::vector<std::string> defaultColors = {
        "#FFFF00", // 黄色
        "#00FFFF", // 青色
        "#FF00FF", // 洋红色
        "#0000FF", // 蓝色
        "#FF0000"  // 红色
    };
    
    std::cout << "========= 开始绘制双曲线 =========" << std::endl;
    
    // 参考站始终是第一个站
    const COORD3& referenceStation = stationPositions[0];
    
    // 将所有站点投影到辐射源高度平面上
    std::vector<COORD3> stationsOnPlane;
    for (const auto& station : stationPositions) {
        COORD3 station_lbh = xyz2lbh(station.p1, station.p2, station.p3);
        COORD3 station_plane = lbh2xyz(station_lbh.p1, station_lbh.p2, planeHeight);
        stationsOnPlane.push_back(station_plane);
        
        std::cout << "站点原始坐标: (" << station_lbh.p1 << "°, " << station_lbh.p2 << "°, " << station_lbh.p3 << " m)" << std::endl;
        std::cout << "站点投影坐标: (" << station_lbh.p1 << "°, " << station_lbh.p2 << "°, " << planeHeight << " m)" << std::endl;
    }
    
    // 对每个非参考站与参考站绘制一条双曲线
    for (size_t i = 1; i < stationsOnPlane.size(); ++i) {
        // 获取侦察站的位置（已投影到同一平面）
        const COORD3& refStation = stationsOnPlane[0];
        const COORD3& otherStation = stationsOnPlane[i];
        
        // 计算两站之间的TDOA值
        double tdoa = tdoas[i - 1];  // 相对于参考站的TDOA
        
        std::cout << "绘制双曲线 " << i << "，参考站与站点" << i << "之间的TDOA: " << tdoa << " 秒" << std::endl;
        
        // 考虑系统误差的影响 - ESM TOA误差
        double tdoa_with_system_error = tdoa;
        
        // 根据实际情况应用ESM TOA误差
        // 1. 系统误差导致双曲线整体平移
        if (esmToaError != 0.0) {
            // 参考站的系统误差会影响所有TDOA值
            tdoa_with_system_error = tdoa + esmToaError;
            std::cout << "应用ESM TOA系统误差后的TDOA: " << tdoa_with_system_error << " s (原始: " << tdoa << " s)" << std::endl;
        }
        
        // 选择颜色 - 使用预定义的颜色数组
        std::string color;
        if (i - 1 < colors.size()) {
            color = colors[i - 1];
        } else if ((i - 1) % defaultColors.size() < defaultColors.size()) {
            color = defaultColors[(i - 1) % defaultColors.size()];
        } else {
            color = "#FFFF00"; // 默认黄色
        }
        
        std::cout << "双曲线" << i << "颜色: " << color << std::endl;
        
        // 使用较粗的线宽，增强可见性
        double lineWidth = 3.0;
        
        // 考虑随机误差的影响 - TDOA RMS误差
        // 创建多个双曲线来表示误差范围
        
        // 绘制中心双曲线 - 系统误差已考虑
        std::vector<COORD3> centerPoints = calculateHyperbolaPoints(
            refStation, otherStation, tdoa_with_system_error, 0.0, 300, 100000.0);
        
        // 绘制误差带
        // 考虑TDOA RMS误差 - 这是随机误差，导致形成误差带
        if (tdoaRmsError > 0.0) {
            // 计算+1σ双曲线上的点
            std::vector<COORD3> upperPoints = calculateHyperbolaPoints(
                refStation, otherStation, tdoa_with_system_error + tdoaRmsError, 0.0, 300, 100000.0);
            
            // 计算-1σ双曲线上的点
            std::vector<COORD3> lowerPoints = calculateHyperbolaPoints(
                refStation, otherStation, tdoa_with_system_error - tdoaRmsError, 0.0, 300, 100000.0);
            
            // 使用相同颜色绘制误差带
            std::string errorColor = color; // 与中心线完全相同的颜色
            // 将线宽设置为中心线的80%，以便区分但保持一致性
            double errorLineWidth = lineWidth * 0.8;
            
            // 绘制误差带的上边界线
            drawHyperbolaLine(mapView, upperPoints, errorColor, errorLineWidth, planeHeight);
            
            // 绘制误差带的下边界线
            drawHyperbolaLine(mapView, lowerPoints, errorColor, errorLineWidth, planeHeight);
            
            // 添加误差带填充区域
            if (upperPoints.size() > 10 && lowerPoints.size() > 10) {
                // 创建一个新的点集合，包含上边界和下边界的点
                std::stringstream fillScript;
                fillScript << "// 创建误差带填充区域\n";
                fillScript << "var fillPositions = [];\n";
                
                // 选择适当数量的点（避免过多点导致性能问题）
                int upperSkip = upperPoints.size() / 20;
                int lowerSkip = lowerPoints.size() / 20;
                if (upperSkip < 1) upperSkip = 1;
                if (lowerSkip < 1) lowerSkip = 1;
                
                // 添加上边界点
                for (size_t j = 0; j < upperPoints.size(); j += upperSkip) {
                    COORD3 lbh = xyz2lbh(upperPoints[j].p1, upperPoints[j].p2, upperPoints[j].p3);
                    fillScript << "fillPositions.push(Cesium.Cartesian3.fromDegrees("
                        << lbh.p1 << ", " << lbh.p2 << ", " << planeHeight << "));\n";
                }
                
                // 添加下边界点（反向）
                for (int j = lowerPoints.size() - 1; j >= 0; j -= lowerSkip) {
                    COORD3 lbh = xyz2lbh(lowerPoints[j].p1, lowerPoints[j].p2, lowerPoints[j].p3);
                    fillScript << "fillPositions.push(Cesium.Cartesian3.fromDegrees("
                        << lbh.p1 << ", " << lbh.p2 << ", " << planeHeight << "));\n";
                }
                
                // 闭合区域
                if (!upperPoints.empty()) {
                    COORD3 lbh = xyz2lbh(upperPoints[0].p1, upperPoints[0].p2, upperPoints[0].p3);
                    fillScript << "fillPositions.push(Cesium.Cartesian3.fromDegrees("
                        << lbh.p1 << ", " << lbh.p2 << ", " << planeHeight << "));\n";
                }
                
                // 创建填充区域实体
                fillScript << "viewer.entities.add({\n";
                fillScript << "  parent: hyperbolaContainer,\n";
                fillScript << "  polygon: {\n";
                fillScript << "    hierarchy: new Cesium.PolygonHierarchy(fillPositions),\n";
                fillScript << "    material: Cesium.Color.fromCssColorString('" << color << "').withAlpha(0.15),\n";
                fillScript << "    height: " << planeHeight << ",\n";
                fillScript << "    perPositionHeight: false\n";
                fillScript << "  }\n";
                fillScript << "});\n";
                
                // 执行填充区域脚本
                mapView->executeScript(fillScript.str());
                std::cout << "已添加误差带填充区域，误差范围为 ±" << tdoaRmsError * 1e9 << " ns" << std::endl;
            }
            
            std::cout << "已绘制第 " << i << " 条双曲线的误差带 (±" << tdoaRmsError * 1e9 << " ns)" << std::endl;
        }
        
        // 绘制中心双曲线 - 最后绘制，确保可见性
        bool success = drawHyperbolaLine(mapView, centerPoints, color, lineWidth, planeHeight);
        if (!success) {
            std::cerr << "绘制第 " << i << " 条中心双曲线失败" << std::endl;
            return false;
        }
    }
    
    // 添加一个标记表示辐射源位置（交点）
    std::string script = "// 添加辐射源标记\n";
    script += "viewer.entities.add({\n";
    script += "  parent: hyperbolaContainer,\n";
    script += "  position: Cesium.Cartesian3.fromDegrees(" + 
              std::to_string(sourcePos_lbh.p1) + ", " + 
              std::to_string(sourcePos_lbh.p2) + ", " + 
              std::to_string(planeHeight) + "),\n";
    script += "  point: {\n";
    script += "    pixelSize: 10,\n";
    script += "    color: Cesium.Color.RED,\n";
    script += "    outlineColor: Cesium.Color.WHITE,\n";
    script += "    outlineWidth: 2\n";
    script += "  },\n";
    script += "  label: {\n";
    script += "    text: '目标辐射源',\n";
    script += "    font: '14pt sans-serif',\n";
    script += "    horizontalOrigin: Cesium.HorizontalOrigin.CENTER,\n";
    script += "    verticalOrigin: Cesium.VerticalOrigin.BOTTOM,\n";
    script += "    pixelOffset: new Cesium.Cartesian2(0, -10),\n";
    script += "    fillColor: Cesium.Color.WHITE,\n";
    script += "    outlineWidth: 2,\n";
    script += "    style: Cesium.LabelStyle.FILL_AND_OUTLINE\n";
    script += "  }\n";
    script += "});\n";
    
    mapView->executeScript(script);
    
    std::cout << "双曲线绘制结果: 成功" << std::endl;
    std::cout << "========= 结束绘制双曲线 =========" << std::endl;
    
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
        // 强制调整TDOA值，使其能够形成双曲线
        // 将距离差设为焦距的80%
        constantA = focusDistance * 0.8;
        std::cout << "已自动调整TDOA距离差为焦距的80%: " << constantA << " 米" << std::endl;
    }
    
    // 首先将焦点转换为大地坐标
    COORD3 focus1_lbh = xyz2lbh(focus1.p1, focus1.p2, focus1.p3);
    COORD3 focus2_lbh = xyz2lbh(focus2.p1, focus2.p2, focus2.p3);
    
    std::cout << "焦点1大地坐标: (" << focus1_lbh.p1 << "°, " << focus1_lbh.p2 << "°, " << focus1_lbh.p3 << " m)" << std::endl;
    std::cout << "焦点2大地坐标: (" << focus2_lbh.p1 << "°, " << focus2_lbh.p2 << "°, " << focus2_lbh.p3 << " m)" << std::endl;
    
    // 使用相同的高度平面，确保双曲线在同一平面上
    double planeHeight = (focus1_lbh.p3 + focus2_lbh.p3) / 2.0;
    if (planeHeight < 0) planeHeight = 0;
    
    // 重新转换回直角坐标系，但使用统一的高度
    COORD3 focus1_plane = lbh2xyz(focus1_lbh.p1, focus1_lbh.p2, planeHeight);
    COORD3 focus2_plane = lbh2xyz(focus2_lbh.p1, focus2_lbh.p2, planeHeight);
    
    // 计算新焦点之间的距离
    double focusDistancePlane = std::sqrt(
        std::pow(focus2_plane.p1 - focus1_plane.p1, 2) +
        std::pow(focus2_plane.p2 - focus1_plane.p2, 2) +
        std::pow(focus2_plane.p3 - focus1_plane.p3, 2)
    );
    
    // 再次检查是否可以形成双曲线
    if (constantA >= focusDistancePlane) {
        constantA = focusDistancePlane * 0.8;
        std::cout << "在平面上调整TDOA距离差为焦距的80%: " << constantA << " 米" << std::endl;
    }
    
    // 计算双曲线的参数
    double c = focusDistancePlane / 2.0;  // 半焦距
    double a = constantA / 2.0;      // 半距离差
    
    // 检查是否可以形成双曲线
    if (a >= c) {
        std::cerr << "无法形成双曲线，半距离差大于等于半焦距" << std::endl;
        a = c * 0.8;
        std::cout << "已自动调整半距离差为半焦距的80%: " << a << " 米" << std::endl;
    }
    
    double b = std::sqrt(c*c - a*a); // 半轴长b
    
    // 计算中点
    COORD3 midPoint = {
        (focus1_plane.p1 + focus2_plane.p1) / 2.0,
        (focus1_plane.p2 + focus2_plane.p2) / 2.0,
        (focus1_plane.p3 + focus2_plane.p3) / 2.0
    };
    
    // 计算焦点之间的单位向量，从焦点1指向焦点2
    double dirX = (focus2_plane.p1 - focus1_plane.p1) / focusDistancePlane;
    double dirY = (focus2_plane.p2 - focus1_plane.p2) / focusDistancePlane;
    double dirZ = (focus2_plane.p3 - focus1_plane.p3) / focusDistancePlane;
    
    // 找到垂直于焦点连线的单位向量（在平面内）
    double perpX = -dirY;
    double perpY = dirX;
    double perpZ = 0.0; // Z分量设为0，确保在水平面内
    
    // 归一化垂直向量
    double perpLen = std::sqrt(perpX*perpX + perpY*perpY + perpZ*perpZ);
    if (perpLen > 1e-10) {
        perpX /= perpLen;
        perpY /= perpLen;
        perpZ /= perpLen;
    } else {
        // 如果垂直向量太小，选择另一个垂直方向
        perpX = 0.0;
        perpY = 0.0;
        perpZ = 1.0;
    }
    
    // 根据TDOA确定双曲线应该在哪个焦点附近更接近
    // TDOA > 0 表示信号先到达焦点1，双曲线应该更接近焦点1
    // TDOA < 0 表示信号先到达焦点2，双曲线应该更接近焦点2
    bool closerToFocus1 = (tdoa > 0);
    
    // 调整双曲线方向，保证其朝向参考站
    // 注意：因为参考站总是作为焦点1，所以我们要确保双曲线朝向焦点1弯曲
    // 对于TDOA > 0的情况，双曲线自然朝向焦点1
    // 对于TDOA < 0的情况，我们需要翻转方向使其朝向焦点1
    
    std::cout << "TDOA: " << tdoa << " 秒，双曲线" << (closerToFocus1 ? "更接近焦点1(参考站)" : "更接近焦点2") << std::endl;
    
    // 计算双曲线上的点
    points.reserve(numPoints);
    
    // 均匀分布的角度参数
    std::vector<double> angles;
    angles.reserve(numPoints);
    
    // 生成从-π/2到π/2的角度范围，以覆盖双曲线的一个分支
    double angleStep = M_PI / (numPoints - 1);
    for (int i = 0; i < numPoints; ++i) {
        double angle = -M_PI/2 + i * angleStep;
        angles.push_back(angle);
    }
    
    // 使用参数方程生成双曲线上的点
    for (double angle : angles) {
        // 参数方程: x = a*sec(θ), y = b*tan(θ)
        double paramX = a / std::cos(angle);
        double paramY = b * std::tan(angle);
        
        // 在平面内计算点的坐标
        COORD3 point;
        
        if (closerToFocus1) {
            // 双曲线靠近焦点1，标准方向
            point.p1 = midPoint.p1 - paramX * dirX + paramY * perpX;
            point.p2 = midPoint.p2 - paramX * dirY + paramY * perpY;
            point.p3 = midPoint.p3 - paramX * dirZ + paramY * perpZ;
        } else {
            // 双曲线靠近焦点2，但我们要使其朝向焦点1，所以翻转x方向
            point.p1 = midPoint.p1 + paramX * dirX + paramY * perpX;
            point.p2 = midPoint.p2 + paramX * dirY + paramY * perpY;
            point.p3 = midPoint.p3 + paramX * dirZ + paramY * perpZ;
        }
        
        points.push_back(point);
    }
    
    // 打印双曲线参数和方向
    std::cout << "双曲线参数:" << std::endl;
    std::cout << "  半焦距 (c): " << c << " 米" << std::endl;
    std::cout << "  半距离差 (a): " << a << " 米" << std::endl;
    std::cout << "  半轴长 (b): " << b << " 米" << std::endl;
    std::cout << "  偏心率 (e): " << c/a << std::endl;
    std::cout << "  生成点数: " << points.size() << std::endl;
    std::cout << "  弯曲方向: " << (closerToFocus1 ? "朝向焦点1(参考站)" : "朝向焦点2") << std::endl;
    
    return points;
}

// 在地图上绘制双曲线
bool HyperbolaLines::drawHyperbolaLine(
    MapView* mapView,
    const std::vector<COORD3>& points,
    const std::string& color,
    double lineWidth,
    double planeHeight) { // 新增参数：平面高度
    
    if (!mapView || points.empty()) {
        std::cerr << "绘制双曲线失败: " << (mapView ? "点为空" : "MapView为空") << std::endl;
        return false;
    }
    
    // 将空间直角坐标转换为大地坐标
    std::vector<COORD3> lbhPoints;
    lbhPoints.reserve(points.size());
    
    // 跟踪最小和最大高度，用于调试
    double minHeight = std::numeric_limits<double>::max();
    double maxHeight = std::numeric_limits<double>::lowest();
    
    for (const auto& point : points) {
        COORD3 lbh = xyz2lbh(point.p1, point.p2, point.p3);
        
        // 记录高度范围
        minHeight = std::min(minHeight, lbh.p3);
        maxHeight = std::max(maxHeight, lbh.p3);
        
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
    
    // 创建点位置数组
    script << "// 创建点位置数组\n"
           << "var positions = [];\n";
    
    // 只保留部分点，但保证曲线平滑
    int skipFactor = lbhPoints.size() > 200 ? lbhPoints.size() / 200 : 1;
    
    // 写入所有点的坐标，但使用固定高度
    for (size_t i = 0; i < lbhPoints.size(); i += skipFactor) {
        const auto& point = lbhPoints[i];
        script << "positions.push(Cesium.Cartesian3.fromDegrees("
               << point.p1 << ", " << point.p2 << ", " << planeHeight // 使用传入的平面高度
               << "));\n";
    }
    
    // 创建Cesium实体
    script << "// 创建双曲线实体\n"
           << "viewer.entities.add({\n"
           << "  parent: hyperbolaContainer,\n"
           << "  polyline: {\n"
           << "    positions: positions,\n"
           << "    width: " << lineWidth << ",\n"
           << "    material: Cesium.Color.fromCssColorString('" << color << "'),\n"
           << "    clampToGround: false,\n"
           << "    followSurface: false,\n"
           << "    arcType: Cesium.ArcType.NONE\n"
           << "  }\n"
           << "});\n";
    
    // 添加调试日志
    script << "console.log('绘制双曲线，颜色: " << color << "，点数: " << lbhPoints.size() / skipFactor << "');\n";
    
    // 输出JavaScript代码用于调试
    std::cout << "\n========= 双曲线绘制JavaScript代码 =========" << std::endl;
    std::cout << script.str() << std::endl;
    std::cout << "=======================================" << std::endl;
    
    // 添加详细调试信息
    std::cout << "双曲线点数量: " << points.size() << std::endl;
    std::cout << "转换后点数量: " << lbhPoints.size() << std::endl;
    std::cout << "实际绘制点数: " << lbhPoints.size() / skipFactor << std::endl;
    std::cout << "高度范围: " << minHeight << " 至 " << maxHeight << " 米" << std::endl;
    std::cout << "使用的平面高度: " << planeHeight << " 米" << std::endl;
    std::cout << "颜色: " << color << std::endl;
    std::cout << "线宽: " << lineWidth << std::endl;
    
    // 输出前10个点的坐标（如果有）
    int maxPoints = std::min(10, static_cast<int>(lbhPoints.size()));
    std::cout << "双曲线前" << maxPoints << "个点坐标:" << std::endl;
    for (int i = 0; i < maxPoints; ++i) {
        std::cout << "  点" << i << ": (" 
                 << lbhPoints[i].p1 << ", " 
                 << lbhPoints[i].p2 << ", " 
                 << planeHeight << ")" << std::endl; // 使用传入的平面高度
    }
    
    // 执行JavaScript代码
    mapView->executeScript(script.str());
    
    return true;
}
