#include "DirectionErrorLines.h"
#include "AngleValidator.h"
#include "CoordinateTransform.h"
#include "../constants/PhysicsConstants.h"
#include <sstream>
#include <cmath>
#include <iostream>

// 构造函数
DirectionErrorLines::DirectionErrorLines() {
    // 暂无需初始化的成员
}

// 析构函数
DirectionErrorLines::~DirectionErrorLines() {
    // 暂无需释放的资源
}

// 在地图上显示测向误差线
bool DirectionErrorLines::showDirectionErrorLines(
    MapView* mapView,
    const ReconnaissanceDevice& device,
    double targetLongitude,
    double targetLatitude,
    double targetAltitude,
    double errorAngle,
    const std::string& lineColor,
    double lineLength) {
    
    if (!mapView) {
        std::cerr << "MapView对象为空，无法显示测向误差线" << std::endl;
        return false;
    }
    
    // 获取侦察设备的位置
    double deviceLongitude = device.getLongitude();
    double deviceLatitude = device.getLatitude();
    double deviceAltitude = device.getAltitude();
    
    // 计算设备到目标的方位角和俯仰角
    COORD3 deviceXYZ = lbh2xyz(deviceLongitude, deviceLatitude, deviceAltitude);
    COORD3 targetXYZ = lbh2xyz(targetLongitude, targetLatitude, targetAltitude);
    
    auto [azimuth, elevation] = calculateAzimuthElevation(
        deviceXYZ.p1, deviceXYZ.p2, deviceXYZ.p3,
        targetXYZ.p1, targetXYZ.p2, targetXYZ.p3
    );
    
    // 计算误差线的方位角
    double leftAzimuth = azimuth - errorAngle / 2.0;
    double rightAzimuth = azimuth + errorAngle / 2.0;
    
    // 确保方位角在[0, 360]范围内
    if (leftAzimuth < 0) leftAzimuth += 360.0;
    if (leftAzimuth >= 360.0) leftAzimuth -= 360.0;
    if (rightAzimuth < 0) rightAzimuth += 360.0;
    if (rightAzimuth >= 360.0) rightAzimuth -= 360.0;
    
    // 计算误差线的端点坐标
    auto [leftEndLongitude, leftEndLatitude, leftEndAltitude] = calculateEndPoint(
        deviceLongitude, deviceLatitude, deviceAltitude,
        leftAzimuth, elevation, lineLength
    );
    
    auto [rightEndLongitude, rightEndLatitude, rightEndAltitude] = calculateEndPoint(
        deviceLongitude, deviceLatitude, deviceAltitude,
        rightAzimuth, elevation, lineLength
    );
    
    // 构建JavaScript代码绘制测向误差线
    std::stringstream script;
    
    // 清除之前可能存在的测向误差线
    script << "// 清除已有的测向误差线\n"
           << "var existingErrorLines = viewer.entities.getById('direction-error-lines');\n"
           << "if (existingErrorLines) {\n"
           << "  viewer.entities.removeById('direction-error-lines');\n"
           << "}\n";
    
    // 创建测向误差线容器实体
    script << "// 创建测向误差线容器\n"
           << "var errorLinesContainer = viewer.entities.add({\n"
           << "  id: 'direction-error-lines',\n"
           << "  name: '测向误差线'\n"
           << "});\n";
    
    // 创建左侧误差线
    script << "// 创建左侧误差线\n"
           << "viewer.entities.add({\n"
           << "  parent: errorLinesContainer,\n"
           << "  polyline: {\n"
           << "    positions: [Cesium.Cartesian3.fromDegrees(" 
           << deviceLongitude << ", " << deviceLatitude << ", " << deviceAltitude << "), "
           << "Cesium.Cartesian3.fromDegrees(" 
           << leftEndLongitude << ", " << leftEndLatitude << ", " << leftEndAltitude << ")],\n"
           << "    width: 3,\n"
           << "    material: new Cesium.ColorMaterialProperty(Cesium.Color.fromCssColorString('" << lineColor << "'))\n"
           << "  }\n"
           << "});\n";
    
    // 创建右侧误差线
    script << "// 创建右侧误差线\n"
           << "viewer.entities.add({\n"
           << "  parent: errorLinesContainer,\n"
           << "  polyline: {\n"
           << "    positions: [Cesium.Cartesian3.fromDegrees(" 
           << deviceLongitude << ", " << deviceLatitude << ", " << deviceAltitude << "), "
           << "Cesium.Cartesian3.fromDegrees(" 
           << rightEndLongitude << ", " << rightEndLatitude << ", " << rightEndAltitude << ")],\n"
           << "    width:3,\n"
           << "    material: new Cesium.ColorMaterialProperty(Cesium.Color.fromCssColorString('" << lineColor << "'))\n"
           << "  }\n"
           << "});\n";
    
    
    // 执行JavaScript代码
    mapView->executeScript(script.str());
    
    return true;
}

// 清除测向误差线
void DirectionErrorLines::clearDirectionErrorLines(MapView* mapView) {
    if (!mapView) return;
    
    std::string script = "var existingErrorLines = viewer.entities.getById('direction-error-lines');\n"
                         "if (existingErrorLines) {\n"
                         "  viewer.entities.removeById('direction-error-lines');\n"
                         "}\n";
    
    mapView->executeScript(script);
}

// 计算从起点沿指定方位角和俯仰角延伸指定距离后的终点坐标
std::tuple<double, double, double> DirectionErrorLines::calculateEndPoint(
    double startLongitude,
    double startLatitude,
    double startAltitude,
    double azimuth,
    double elevation,
    double distance) {
    
    // 将角度转换为弧度
    double azimuthRad = azimuth * Constants::DEG2RAD;
    double elevationRad = elevation * Constants::DEG2RAD;
    
    // 将大地坐标转换为空间直角坐标
    COORD3 startXYZ = lbh2xyz(startLongitude, startLatitude, startAltitude);
    
    // 计算水平距离
    double horizontalDistance = distance * std::cos(elevationRad);
    
    // 计算高度差
    double heightDifference = distance * std::sin(elevationRad);
    
    // 计算在空间直角坐标系中的位移
    double dx = horizontalDistance * std::sin(azimuthRad);
    double dy = horizontalDistance * std::cos(azimuthRad);
    double dz = heightDifference;
    
    // 计算终点的空间直角坐标
    double endX = startXYZ.p1 + dx;
    double endY = startXYZ.p2 + dy;
    double endZ = startXYZ.p3 + dz;
    
    // 将终点空间直角坐标转换回大地坐标
    COORD3 endLBH = xyz2lbh(endX, endY, endZ);
    
    return std::make_tuple(endLBH.p1, endLBH.p2, endLBH.p3);
}

// 在地图上显示测向定位误差线
bool DirectionErrorLines::showDirectionSimulationLines(
    MapView* mapView,
    const ReconnaissanceDevice& device,
    double targetLongitude,
    double targetLatitude,
    double targetAltitude,
    double meanErrorDeg,   // 均值误差
    double stdDevDeg,      // 标准差
    const std::string& lineColor,
    double lineLength) {
    
    if (!mapView) {
        std::cerr << "MapView对象为空，无法显示测向误差线" << std::endl;
        return false;
    }
    
    // 获取侦察设备的位置
    double deviceLongitude = device.getLongitude();
    double deviceLatitude = device.getLatitude();
    double deviceAltitude = device.getAltitude();
    
    // 计算设备到目标的方位角和俯仰角
    COORD3 deviceXYZ = lbh2xyz(deviceLongitude, deviceLatitude, deviceAltitude);
    COORD3 targetXYZ = lbh2xyz(targetLongitude, targetLatitude, targetAltitude);
    
    auto [azimuth, elevation] = calculateAzimuthElevation(
        deviceXYZ.p1, deviceXYZ.p2, deviceXYZ.p3,
        targetXYZ.p1, targetXYZ.p2, targetXYZ.p3
    );
    
    // 强制设置elevation为0，使测向线水平发出
    // elevation = 0.0;
    
    // 根据均值误差和标准差计算测向线的方位角
    // 修改：将方位角作为中心线，而不是"中心线+均值误差"
    double centralAzimuth = azimuth;
    
    // 左右两条线分别使用均值误差±标准差
    double leftAzimuth = azimuth + meanErrorDeg - stdDevDeg;   // 左误差线 = 基准方位角 + 均值误差 - 标准差
    double rightAzimuth = azimuth + meanErrorDeg + stdDevDeg;  // 右误差线 = 基准方位角 + 均值误差 + 标准差
    
    // 确保方位角在[0, 360]范围内
    if (centralAzimuth < 0) centralAzimuth += 360.0;
    if (centralAzimuth >= 360.0) centralAzimuth -= 360.0;
    if (leftAzimuth < 0) leftAzimuth += 360.0;
    if (leftAzimuth >= 360.0) leftAzimuth -= 360.0;
    if (rightAzimuth < 0) rightAzimuth += 360.0;
    if (rightAzimuth >= 360.0) rightAzimuth -= 360.0;
    
    // 计算各条线的端点坐标
    auto [centralEndLongitude, centralEndLatitude, centralEndAltitude] = calculateEndPoint(
        deviceLongitude, deviceLatitude, deviceAltitude,
        centralAzimuth, elevation, lineLength
    );
    
    auto [leftEndLongitude, leftEndLatitude, leftEndAltitude] = calculateEndPoint(
        deviceLongitude, deviceLatitude, deviceAltitude,
        leftAzimuth, elevation, lineLength
    );
    
    auto [rightEndLongitude, rightEndLatitude, rightEndAltitude] = calculateEndPoint(
        deviceLongitude, deviceLatitude, deviceAltitude,
        rightAzimuth, elevation, lineLength
    );
    
    // 确保终点和起点高度相同，使线完全水平
    centralEndAltitude = deviceAltitude;
    leftEndAltitude = deviceAltitude;
    rightEndAltitude = deviceAltitude;
    
    // 生成唯一ID，避免多个设备的线互相干扰
    std::string uniqueId = "df-" + std::to_string(device.getDeviceId());
    
    // 构建JavaScript代码绘制测向误差线
    std::stringstream script;
    
    // 清除之前可能存在的测向误差线
    script << "// 清除已有的测向误差线\n"
           << "var existingErrorLines = viewer.entities.getById('" << uniqueId << "');\n"
           << "if (existingErrorLines) {\n"
           << "  viewer.entities.removeById('" << uniqueId << "');\n"
           << "}\n";
    
    // 创建测向误差线容器实体
    script << "// 创建测向误差线容器\n"
           << "var errorLinesContainer = viewer.entities.add({\n"
           << "  id: '" << uniqueId << "',\n"
           << "  name: '测向误差线-设备" << device.getDeviceId() << "'\n"
           << "});\n";
    
    // 创建左侧误差线
    script << "// 创建左侧误差线\n"
           << "viewer.entities.add({\n"
           << "  parent: errorLinesContainer,\n"
           << "  polyline: {\n"
           << "    positions: [Cesium.Cartesian3.fromDegrees(" 
           << deviceLongitude << ", " << deviceLatitude << ", " << deviceAltitude << "), "
           << "Cesium.Cartesian3.fromDegrees(" 
           << leftEndLongitude << ", " << leftEndLatitude << ", " << leftEndAltitude << ")],\n"
           << "    width: 2,\n"
           << "    material: new Cesium.ColorMaterialProperty(Cesium.Color.fromCssColorString('" << lineColor << "'))\n"
           << "  }\n"
           << "});\n";
    
    // 创建右侧误差线
    script << "// 创建右侧误差线\n"
           << "viewer.entities.add({\n"
           << "  parent: errorLinesContainer,\n"
           << "  polyline: {\n"
           << "    positions: [Cesium.Cartesian3.fromDegrees(" 
           << deviceLongitude << ", " << deviceLatitude << ", " << deviceAltitude << "), "
           << "Cesium.Cartesian3.fromDegrees(" 
           << rightEndLongitude << ", " << rightEndLatitude << ", " << rightEndAltitude << ")],\n"
           << "    width: 2,\n"
           << "    material: new Cesium.ColorMaterialProperty(Cesium.Color.fromCssColorString('" << lineColor << "'))\n"
           << "  }\n"
           << "});\n";
    
    // 执行JavaScript代码
    mapView->executeScript(script.str());
    
    return true;
} 