#include "../TrajectorySimulator.h"
#include "../../constants/PhysicsConstants.h"
#include "../../utils/CoordinateTransform.h"
#include "../../views/components/MapView.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <gtk/gtk.h>

// 使用常量命名空间
using namespace Constants;

// 单例实现
TrajectorySimulator& TrajectorySimulator::getInstance() {
    static TrajectorySimulator instance;
    return instance;
}

// 模拟设备移动轨迹
std::vector<std::pair<double, double>> TrajectorySimulator::simulateDeviceMovement(
    ReconnaissanceDevice& device, 
    int simulationTime) {
    
    std::vector<std::pair<double, double>> trajectoryPoints;
    
    // 获取设备初始位置和运动参数
    double initialLongitude = device.getLongitude();
    double initialLatitude = device.getLatitude();
    double initialAltitude = device.getAltitude();
    double speed = device.getMovementSpeed();
    double azimuth = device.getMovementAzimuth() * DEG2RAD; // 转换为弧度
    double elevation = device.getMovementElevation() * DEG2RAD; // 转换为弧度
    
    // 将设备初始位置转换为ECEF坐标
    COORD3 initialPositionXYZ = lbh2xyz(initialLongitude, initialLatitude, initialAltitude);
    double x0 = initialPositionXYZ.p1;
    double y0 = initialPositionXYZ.p2;
    double z0 = initialPositionXYZ.p3;
    
    // 将运动方向和速度转换为ECEF坐标系中的速度分量
    COORD3 velocityXYZ = velocity_lbh2xyz(
        initialLongitude, 
        initialLatitude, 
        speed, 
        device.getMovementAzimuth(), 
        device.getMovementElevation()
    );
    double vx = velocityXYZ.p1;
    double vy = velocityXYZ.p2;
    double vz = velocityXYZ.p3;
    
    // 记录初始位置
    trajectoryPoints.push_back(std::make_pair(initialLongitude, initialLatitude));
    
    // 轨迹点的时间间隔（秒）
    double timeStep = 1.0;
    int numSteps = simulationTime;
    
    // 逐步计算设备位置
    for (int i = 1; i <= numSteps; i++) {
        // 计算当前时刻t的位置
        double t = i * timeStep;
        
        // 线性运动模型
        double xt = x0 + vx * t;
        double yt = y0 + vy * t;
        double zt = z0 + vz * t;
        
        // 将ECEF坐标转换回经纬度
        COORD3 positionLBH = xyz2lbh(xt, yt, zt);
        double longitude = positionLBH.p1;
        double latitude = positionLBH.p2;
        double altitude = positionLBH.p3;
        
        // 更新设备的当前位置（用于外部访问）
        if (i == numSteps) {
            device.setLongitude(longitude);
            device.setLatitude(latitude);
            device.setAltitude(altitude);
        }
        
        // 存储轨迹点
        trajectoryPoints.push_back(std::make_pair(longitude, latitude));
    }
    
    g_print("设备移动轨迹计算完成，共%zu个点\n", trajectoryPoints.size());
    return trajectoryPoints;
}

// 执行设备移动动画
void TrajectorySimulator::animateDeviceMovement(
    MapView* mapView,
    const ReconnaissanceDevice& device,
    const std::vector<std::pair<double, double>>& trajectoryPoints,
    int simulationTime,
    double calculatedLongitude,
    double calculatedLatitude,
    double calculatedAltitude,
    const std::string& sourceName,
    double radiationSourceLongitude,
    double radiationSourceLatitude,
    double radiationSourceAltitude) {
    
    if (!mapView || trajectoryPoints.empty()) return;
    
    g_print("开始设备移动动画，轨迹点数量: %zu\n", trajectoryPoints.size());
    
    // 清除之前的所有实体
    std::string cleanupScript = 
        "// 移除所有实体 \n"
        "viewer.entities.removeAll(); ";
    mapView->executeScript(cleanupScript);
    
    // 获取设备高度
    double deviceAltitude = device.getAltitude();
    
    // 构建新的轨迹动画JavaScript代码
    std::stringstream script;
    
    // 创建轨迹点数组
    script << "// 定义轨迹点数组\n"
           << "var trajectoryPoints = [";
    
    // 添加所有轨迹点
    for (size_t i = 0; i < trajectoryPoints.size(); i++) {
        if (i > 0) script << ", ";
        script << "[" << trajectoryPoints[i].first << ", " << trajectoryPoints[i].second << "]";
    }
    
    script << "];\n"
           << "console.log('轨迹点数量: ' + trajectoryPoints.length);\n"
           // 计算更新间隔（毫秒）
           << "var simulationTime = " << simulationTime << ";\n"
           << "var updateInterval = (simulationTime * 1000) / (trajectoryPoints.length - 1);\n" 
           << "console.log('模拟时间: ' + simulationTime + '秒, 更新间隔: ' + updateInterval + 'ms');\n"
           
           // 创建轨迹线
           << "// 创建轨迹线\n"
           << "var positions = [];\n"
           << "for (var i = 0; i < trajectoryPoints.length; i++) {\n"
           << "  positions.push(Cesium.Cartesian3.fromDegrees(trajectoryPoints[i][0], trajectoryPoints[i][1], " << deviceAltitude << "));\n"
           << "}\n"
           
           << "// 添加完整轨迹线\n"
           << "viewer.entities.add({\n"
           << "  id: 'trajectory-line',\n"
           << "  polyline: {\n"
           << "    positions: positions,\n"
           << "    width: 3,\n"
           << "    material: new Cesium.PolylineGlowMaterialProperty({\n"
           << "      glowPower: 0.2,\n"
           << "      color: Cesium.Color.YELLOW\n"
           << "    })\n"
           << "  }\n"
           << "});\n"
           
           // 添加起始点标记 - 只显示标识点，不显示具体信息
           << "// 添加起始位置标记 - 只显示标识点\n"
           << "viewer.entities.add({\n"
           << "  id: 'start-point',\n"
           << "  position: Cesium.Cartesian3.fromDegrees(trajectoryPoints[0][0], trajectoryPoints[0][1], " << deviceAltitude << "),\n"
           << "  point: {\n"
           << "    pixelSize: 10,\n"
           << "    color: Cesium.Color.RED,\n"
           << "    outlineColor: Cesium.Color.WHITE,\n"
           << "    outlineWidth: 2\n"
           << "  }\n"
           << "});\n"
           
           // 使用直接的定时器动画而不是SampledPositionProperty，以确保在所有浏览器环境下都能正常工作
           << "// 设备移动动画 - 使用直接定时器\n"
           << "var currentIndex = 0;\n"
           << "var deviceEntity = viewer.entities.add({\n"
           << "  id: 'device-entity',\n"
           << "  position: Cesium.Cartesian3.fromDegrees(trajectoryPoints[0][0], trajectoryPoints[0][1], " << deviceAltitude << "),\n"
           << "  point: {\n"
           << "    pixelSize: 15,\n"
           << "    color: Cesium.Color.RED,\n"
           << "    outlineColor: Cesium.Color.WHITE,\n"
           << "    outlineWidth: 2\n"
           << "  },\n"
           << "  label: {\n"
           << "    text: '" << device.getDeviceName() << "',\n"
           << "    font: '14pt sans-serif',\n"
           << "    style: Cesium.LabelStyle.FILL_AND_OUTLINE,\n"
           << "    outlineWidth: 2,\n"
           << "    verticalOrigin: Cesium.VerticalOrigin.BOTTOM,\n"
           << "    pixelOffset: new Cesium.Cartesian2(0, -20),\n"
           << "    showBackground: true,\n"
           << "    backgroundColor: new Cesium.Color(0.165, 0.165, 0.165, 0.7)\n"
           << "  },\n"
           << "  billboard: {\n"
           << "    image: 'data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAyNCAyNCIgd2lkdGg9IjI0IiBoZWlnaHQ9IjI0Ij48Y2lyY2xlIGN4PSIxMiIgY3k9IjEyIiByPSIxMCIgZmlsbD0icmVkIi8+PC9zdmc+',\n"
           << "    width: 32,\n"
           << "    height: 32,\n"
           << "    verticalOrigin: Cesium.VerticalOrigin.BOTTOM,\n"
           << "    color: Cesium.Color.RED\n"
           << "  }\n"
           << "});\n"
           
           // 垂直连接线
           << "// 添加垂直连接线\n"
           << "var verticalLineEntity = viewer.entities.add({\n"
           << "  id: 'vertical-line',\n"
           << "  polyline: {\n"
           << "    positions: [\n"
           << "      Cesium.Cartesian3.fromDegrees(trajectoryPoints[0][0], trajectoryPoints[0][1], 0),\n"
           << "      Cesium.Cartesian3.fromDegrees(trajectoryPoints[0][0], trajectoryPoints[0][1], " << deviceAltitude << ")\n"
           << "    ],\n"
           << "    width: 1,\n"
           << "    material: new Cesium.PolylineDashMaterialProperty({\n"
           << "      color: Cesium.Color.RED\n"
           << "    })\n"
           << "  }\n"
           << "});\n"
           
           // 预定义辐射源位置
           << "// 定义辐射源位置\n"
           << "var radiationSourceLongitude = " << radiationSourceLongitude << ";\n"
           << "var radiationSourceLatitude = " << radiationSourceLatitude << ";\n"
           << "var radiationSourceAltitude = " << radiationSourceAltitude << ";\n"
           
           // 简单的移动函数
           << "// 设备移动函数\n"
           << "function moveDevice() {\n"
           << "  if (currentIndex < trajectoryPoints.length) {\n"
           << "    // 获取当前位置\n"
           << "    var longitude = trajectoryPoints[currentIndex][0];\n"
           << "    var latitude = trajectoryPoints[currentIndex][1];\n"
           << "    \n"
           << "    // 更新设备位置\n"
           << "    deviceEntity.position = Cesium.Cartesian3.fromDegrees(longitude, latitude, " << deviceAltitude << ");\n"
           << "    \n"
           << "    // 更新垂直连接线\n"
           << "    verticalLineEntity.polyline.positions = [\n"
           << "      Cesium.Cartesian3.fromDegrees(longitude, latitude, 0),\n"
           << "      Cesium.Cartesian3.fromDegrees(longitude, latitude, " << deviceAltitude << ")\n"
           << "    ];\n"
           << "    \n"
           << "    // 移动到下一个点\n"
           << "    currentIndex++;\n"
           << "    \n"
           << "    // 如果还有点，继续移动\n"
           << "    if (currentIndex < trajectoryPoints.length) {\n"
           << "      setTimeout(moveDevice, updateInterval);\n"
           << "    } else {\n"
           << "      // 动画结束，显示最终结果\n"
           << "      console.log('动画完成');\n"
           << "      showFinalResults();\n"
           << "    }\n"
           << "  }\n"
           << "}\n"
           
           // 显示最终结果的函数
           << "// 显示最终结果\n"
           << "function showFinalResults() {\n"
           << "  // 最终位置索引\n"
           << "  var finalIndex = trajectoryPoints.length - 1;\n"
           << "  var finalLongitude = trajectoryPoints[finalIndex][0];\n"
           << "  var finalLatitude = trajectoryPoints[finalIndex][1];\n"
           << "  \n"
           << "  // 在最终位置添加绿色标记点\n"
           << "  viewer.entities.add({\n"
           << "    id: 'final-position',\n"
           << "    position: Cesium.Cartesian3.fromDegrees(finalLongitude, finalLatitude, " << deviceAltitude << "),\n"
           << "    point: {\n"
           << "      pixelSize: 12,\n"
           << "      color: Cesium.Color.GREEN,\n"
           << "      outlineColor: Cesium.Color.WHITE,\n"
           << "      outlineWidth: 2\n"
           << "    },\n"
           << "    label: {\n"
           << "      text: '" << device.getDeviceName() << "',\n"
           << "      font: '14pt sans-serif',\n"
           << "      style: Cesium.LabelStyle.FILL_AND_OUTLINE,\n"
           << "      outlineWidth: 2,\n"
           << "      verticalOrigin: Cesium.VerticalOrigin.BOTTOM,\n"
           << "      pixelOffset: new Cesium.Cartesian2(0, -9),\n"
           << "      showBackground: true,\n"
           << "      backgroundColor: new Cesium.Color(0.165, 0.165, 0.165, 0.7)\n"
           << "    }\n"
           << "  });\n"
           << "  \n"
           << "  // 隐藏动画过程中的设备实体\n"
           << "  deviceEntity.show = false;\n"
           << "  verticalLineEntity.show = false;\n"
           << "  \n"
           << "  // 使用算法计算的定位结果\n"
           << "  var calculatedLongitude = " << calculatedLongitude << ";\n"
           << "  var calculatedLatitude = " << calculatedLatitude << ";\n"
           << "  var calculatedAltitude = " << calculatedAltitude << ";\n"
           << "  \n"
           << "  // 添加计算定位位置标记\n"
           << "  viewer.entities.add({\n"
           << "    id: 'calculated-position',\n"
           << "    position: Cesium.Cartesian3.fromDegrees(calculatedLongitude, calculatedLatitude, calculatedAltitude),\n"
           << "    point: {\n"
           << "      pixelSize: 12,\n"
           << "      color: Cesium.Color.PURPLE,\n"
           << "      outlineColor: Cesium.Color.WHITE,\n"
           << "      outlineWidth: 2\n"
           << "    },\n"
           << "    label: {\n"
           << "      text: '" << sourceName << "',\n"
           << "      font: '14pt sans-serif',\n"
           << "      style: Cesium.LabelStyle.FILL_AND_OUTLINE,\n"
           << "      outlineWidth: 2,\n"
           << "      verticalOrigin: Cesium.VerticalOrigin.BOTTOM,\n"
           << "      pixelOffset: new Cesium.Cartesian2(0, -9),\n"
           << "      showBackground: true,\n"
           << "      backgroundColor: new Cesium.Color(0.165, 0.165, 0.165, 0.7)\n"
           << "    }\n"
           << "  });\n"
           << "}\n"
           
           // 启动动画
           << "// 延迟一下启动动画，确保地图已加载\n"
           << "console.log('准备启动动画...');\n"
           << "setTimeout(function() {\n"
           << "  console.log('开始动画');\n"
           << "  moveDevice();\n"
           << "}, 1000);\n";
    
    // 执行脚本
    mapView->executeScript(script.str());
    
    g_print("设备移动仿真已启动，仿真时间: %d秒\n", simulationTime);
} 