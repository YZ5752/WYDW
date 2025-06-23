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

// 模拟移动轨迹 - 通用方法，支持侦察设备和辐射源
std::vector<std::pair<double, double>> TrajectorySimulator::simulateMovement(
    double initialLongitude, 
    double initialLatitude, 
    double initialAltitude,
    double speed,
    double azimuth,
    double elevation,
    int simulationTime,
    bool updatePosition,
    void* objectPtr,
    bool isDevice) {
    
    std::vector<std::pair<double, double>> trajectoryPoints;
    
    // 转换方位角和俯仰角为弧度
    double azimuthRad = azimuth * DEG2RAD; 
    double elevationRad = elevation * DEG2RAD;
    
    // 将初始位置转换为ECEF坐标
    COORD3 initialPositionXYZ = lbh2xyz(initialLongitude, initialLatitude, initialAltitude);
    double x0 = initialPositionXYZ.p1;
    double y0 = initialPositionXYZ.p2;
    double z0 = initialPositionXYZ.p3;
    
    // 将运动方向和速度转换为ECEF坐标系中的速度分量
    COORD3 velocityXYZ = velocity_lbh2xyz(
        initialLongitude, 
        initialLatitude, 
        speed, 
        azimuth, 
        elevation
    );
    double vx = velocityXYZ.p1;
    double vy = velocityXYZ.p2;
    double vz = velocityXYZ.p3;
    
    g_print("模拟轨迹 - 初始位置: [%.6f, %.6f, %.2f], 速度: %.2f m/s, 方位角: %.2f°, 俯仰角: %.2f°\n",
            initialLongitude, initialLatitude, initialAltitude, speed, azimuth, elevation);
    g_print("速度向量 (ECEF): [%.2f, %.2f, %.2f] m/s\n", vx, vy, vz);
    
    // 记录初始位置
    trajectoryPoints.push_back(std::make_pair(initialLongitude, initialLatitude));
    
    // 轨迹点的时间间隔（秒）
    double timeStep = 1.0;
    int numSteps = simulationTime;
    
    // 逐步计算移动位置
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
        
        // 更新位置（仅在最后一步，且需要更新时）
        if (i == numSteps && updatePosition && objectPtr != nullptr) {
            if (isDevice) {
                // 对象是ReconnaissanceDevice
                ReconnaissanceDevice* device = static_cast<ReconnaissanceDevice*>(objectPtr);
                device->setLongitude(longitude);
                device->setLatitude(latitude);
                device->setAltitude(altitude);
                g_print("更新侦察设备位置: [%.6f, %.6f, %.2f]\n", longitude, latitude, altitude);
            } else {
                // 对象是RadiationSource
                RadiationSource* source = static_cast<RadiationSource*>(objectPtr);
                source->setLongitude(longitude);
                source->setLatitude(latitude);
                source->setAltitude(altitude);
                g_print("更新辐射源位置: [%.6f, %.6f, %.2f]\n", longitude, latitude, altitude);
            }
        }
        
        // 存储轨迹点
        trajectoryPoints.push_back(std::make_pair(longitude, latitude));
    }
    
    g_print("移动轨迹计算完成，共%zu个点\n", trajectoryPoints.size());
    return trajectoryPoints;
}

// 模拟设备移动轨迹 - 兼容旧方法，调用通用方法
std::vector<std::pair<double, double>> TrajectorySimulator::simulateDeviceMovement(
    ReconnaissanceDevice& device, 
    int simulationTime) {
    
    return simulateMovement(
        device.getLongitude(),
        device.getLatitude(), 
        device.getAltitude(),
        device.getMovementSpeed(),
        device.getMovementAzimuth(),
        device.getMovementElevation(),
        simulationTime,
        true,   // 更新位置
        &device,
        true    // 是设备
    );
}

// 模拟辐射源移动轨迹
std::vector<std::pair<double, double>> TrajectorySimulator::simulateSourceMovement(
    RadiationSource& source, 
    int simulationTime) {
    
    return simulateMovement(
        source.getLongitude(),
        source.getLatitude(), 
        source.getAltitude(),
        source.getMovementSpeed(),
        source.getMovementAzimuth(),
        source.getMovementElevation(),
        simulationTime,
        true,   // 更新位置
        &source,
        false   // 是辐射源
    );
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
           << "console.log('模拟时间: ' + simulationTime + '秒, 更新间隔: ' + updateInterval + 'ms');\n";
    
    // 初始化轨迹线数组
    script << "// 初始化轨迹线数组\n"
           << "var trailPositions = [];\n"
           << "trailPositions.push(Cesium.Cartesian3.fromDegrees(trajectoryPoints[0][0], trajectoryPoints[0][1], " << deviceAltitude << "));\n";

    // 创建轨迹线实体
    script << "// 创建轨迹线实体\n"
           << "var trailEntity = viewer.entities.add({\n"
           << "  id: 'device-trail',\n"
           << "  polyline: {\n"
           << "    positions: trailPositions,\n"
           << "    width: 3,\n"
           << "    material: new Cesium.PolylineGlowMaterialProperty({\n"
           << "      glowPower: 0.2,\n"
           << "      color: Cesium.Color.YELLOW\n"
           << "    })\n"
           << "  }\n"
           << "});\n";
           
    // 添加起始点标记 - 只显示标识点，不显示具体信息
    script << "// 添加起始位置标记 - 只显示标识点\n"
           << "viewer.entities.add({\n"
           << "  id: 'start-point',\n"
           << "  position: Cesium.Cartesian3.fromDegrees(trajectoryPoints[0][0], trajectoryPoints[0][1], " << deviceAltitude << "),\n"
           << "  point: {\n"
           << "    pixelSize: 10,\n"
           << "    color: Cesium.Color.RED,\n"
           << "    outlineColor: Cesium.Color.WHITE,\n"
           << "    outlineWidth: 2\n"
           << "  }\n"
           << "});\n";
           
    // 使用直接的定时器动画而不是SampledPositionProperty，以确保在所有浏览器环境下都能正常工作
    script << "// 设备移动动画 - 使用直接定时器\n"
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
           << "});\n";
           
    // 预定义辐射源位置
    script << "// 定义辐射源位置\n"
           << "var radiationSourceLongitude = " << radiationSourceLongitude << ";\n"
           << "var radiationSourceLatitude = " << radiationSourceLatitude << ";\n"
           << "var radiationSourceAltitude = " << radiationSourceAltitude << ";\n";
           
    // 简单的移动函数，移除垂直连接线
    script << "// 设备移动函数\n"
           << "function moveDevice() {\n"
           << "  if (currentIndex < trajectoryPoints.length) {\n"
           << "    // 获取当前位置\n"
           << "    var longitude = trajectoryPoints[currentIndex][0];\n"
           << "    var latitude = trajectoryPoints[currentIndex][1];\n"
           << "    var position = Cesium.Cartesian3.fromDegrees(longitude, latitude, " << deviceAltitude << ");\n"
           << "    \n"
           << "    // 更新设备位置\n"
           << "    deviceEntity.position = position;\n"
           << "    \n"
           << "    // 更新轨迹线\n"
           << "    trailPositions.push(position);\n"
           << "    trailEntity.polyline.positions = trailPositions;\n"
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
           << "}\n";
           
    // 修改显示最终结果函数，移除垂直连接线相关代码
    script << "// 显示最终结果\n"
           << "function showFinalResults() {\n"
           << "  // 最终位置索引\n"
           << "  var finalIndex = trajectoryPoints.length - 1;\n"
           << "  var finalLongitude = trajectoryPoints[finalIndex][0];\n"
           << "  var finalLatitude = trajectoryPoints[finalIndex][1];\n"
           << "  \n"
           << "  // 隐藏动画过程中的设备实体\n"
           << "  deviceEntity.show = false;\n"
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
           << "}\n";
           
    // 启动动画
    script << "// 延迟一下启动动画，确保地图已加载\n"
           << "console.log('准备启动动画...');\n"
           << "setTimeout(function() {\n"
           << "  console.log('开始动画');\n"
           << "  moveDevice();\n"
           << "}, 1000);\n";
    
    // 执行脚本
    mapView->executeScript(script.str());
    
    g_print("设备移动仿真已启动，仿真时间: %d秒\n", simulationTime);
}

// 执行多设备移动动画
void TrajectorySimulator::animateMultipleDevicesMovement(
    MapView* mapView,
    const std::vector<ReconnaissanceDevice>& devices,
    const RadiationSource& source,
    int simulationTime,
    double calculatedLongitude,
    double calculatedLatitude,
    double calculatedAltitude) {
    
    if (!mapView || devices.empty()) return;
    
    g_print("开始多设备移动动画，设备数量: %zu\n", devices.size());
    
    // 清除之前的所有实体
    std::string cleanupScript = 
        "// 移除所有实体 \n"
        "viewer.entities.removeAll(); ";
    mapView->executeScript(cleanupScript);
    
    // 构建新的轨迹动画JavaScript代码
    std::stringstream script;
    
    // 定义颜色数组，用于区分不同设备
    script << "// 定义颜色数组\n"
           << "var deviceColors = [\n"
           << "  Cesium.Color.RED,\n"
           << "  Cesium.Color.YELLOW,\n"
           << "  Cesium.Color.GREEN,\n" 
           << "  Cesium.Color.ORANGE,\n"
           << "  Cesium.Color.MAGENTA,\n"
           << "  Cesium.Color.CYAN\n"
           << "];\n";
    
    // 模拟每个设备的轨迹并创建轨迹点数组
    script << "// 定义设备轨迹点数组\n"
           << "var allDeviceTrajectories = [];\n"
           << "var deviceTrailPositions = [];\n";
    
    for (size_t deviceIdx = 0; deviceIdx < devices.size(); deviceIdx++) {
        const ReconnaissanceDevice& device = devices[deviceIdx];
        
        // 模拟设备轨迹
        ReconnaissanceDevice deviceCopy = device; // 使用副本，避免修改原始数据
        std::vector<std::pair<double, double>> trajectoryPoints = 
            simulateDeviceMovement(deviceCopy, simulationTime);
        
        // 添加轨迹点数组
        script << "// 设备" << deviceIdx << "轨迹点\n"
               << "var trajectoryPoints" << deviceIdx << " = [";
        
        for (size_t i = 0; i < trajectoryPoints.size(); i++) {
            if (i > 0) script << ", ";
            script << "[" << trajectoryPoints[i].first << ", " << trajectoryPoints[i].second << "]";
        }
        script << "];\n"
               << "allDeviceTrajectories.push(trajectoryPoints" << deviceIdx << ");\n"
               << "console.log('设备" << deviceIdx << "轨迹点数量: ' + trajectoryPoints" << deviceIdx << ".length);\n";
        
        // 初始化轨迹线数组
        script << "// 创建设备" << deviceIdx << "轨迹线初始化\n"
               << "deviceTrailPositions[" << deviceIdx << "] = [];\n";
    }
    
    // 模拟辐射源轨迹
    RadiationSource sourceCopy = source; // 使用副本，避免修改原始数据
    std::vector<std::pair<double, double>> sourceTrajectoryPoints = 
        simulateSourceMovement(sourceCopy, simulationTime);
    
    // 添加辐射源轨迹点数组
    script << "// 辐射源轨迹点\n"
           << "var sourceTrajectoryPoints = [";
    
    for (size_t i = 0; i < sourceTrajectoryPoints.size(); i++) {
        if (i > 0) script << ", ";
        script << "[" << sourceTrajectoryPoints[i].first << ", " << sourceTrajectoryPoints[i].second << "]";
    }
    script << "];\n"
           << "console.log('辐射源轨迹点数量: ' + sourceTrajectoryPoints.length);\n"
           << "// 初始化辐射源轨迹线数组\n"
           << "var sourceTrailPositions = [];\n";
           
    // 计算更新间隔（毫秒）
    script << "// 计算更新间隔\n"
           << "var simulationTime = " << simulationTime << ";\n"
           << "var updateInterval = (simulationTime * 1000) / (sourceTrajectoryPoints.length - 1);\n" 
           << "console.log('模拟时间: ' + simulationTime + '秒, 更新间隔: ' + updateInterval + 'ms');\n";
           
    // 创建多个设备实体和辐射源实体
    script << "// 创建设备实体数组\n"
           << "var deviceEntities = [];\n"
           << "var deviceTrailEntities = [];\n";
           
    // 为每个设备创建实体
    for (size_t deviceIdx = 0; deviceIdx < devices.size(); deviceIdx++) {
        const ReconnaissanceDevice& device = devices[deviceIdx];
        
        script << "// 创建设备" << deviceIdx << "实体\n"
               << "var deviceEntity" << deviceIdx << " = viewer.entities.add({\n"
               << "  id: 'device-entity-" << deviceIdx << "',\n"
               << "  position: Cesium.Cartesian3.fromDegrees(trajectoryPoints" << deviceIdx << "[0][0], trajectoryPoints" << deviceIdx << "[0][1], " << device.getAltitude() << "),\n"
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
               << "deviceEntities.push(deviceEntity" << deviceIdx << ");\n";
               
        // 创建轨迹线实体
        script << "// 创建设备" << deviceIdx << "轨迹线实体\n"
               << "var deviceTrailEntity" << deviceIdx << " = viewer.entities.add({\n"
               << "  id: 'device-trail-" << deviceIdx << "',\n"
               << "  polyline: {\n"
               << "    positions: [],\n"
               << "    width: 3,\n"
               << "    material: new Cesium.PolylineGlowMaterialProperty({\n"
               << "      glowPower: 0.2,\n"
               << "      color: Cesium.Color.YELLOW\n"
               << "    })\n"
               << "  }\n"
               << "});\n"
               << "deviceTrailEntities.push(deviceTrailEntity" << deviceIdx << ");\n";
    }
    
    // 创建辐射源实体
    script << "// 创建辐射源实体\n"
           << "var sourceEntity = viewer.entities.add({\n"
           << "  id: 'source-entity',\n"
           << "  position: Cesium.Cartesian3.fromDegrees(sourceTrajectoryPoints[0][0], sourceTrajectoryPoints[0][1], " << source.getAltitude() << "),\n"
           << "  point: {\n"
           << "    pixelSize: 15,\n"
           << "    color: Cesium.Color.BLUE,\n"
           << "    outlineColor: Cesium.Color.WHITE,\n"
           << "    outlineWidth: 2\n"
           << "  },\n"
           << "  label: {\n"
           << "    text: '" << source.getRadiationName() << "',\n"
           << "    font: '14pt sans-serif',\n"
           << "    style: Cesium.LabelStyle.FILL_AND_OUTLINE,\n"
           << "    outlineWidth: 2,\n"
           << "    verticalOrigin: Cesium.VerticalOrigin.BOTTOM,\n"
           << "    pixelOffset: new Cesium.Cartesian2(0, -20),\n"
           << "    showBackground: true,\n"
           << "    backgroundColor: new Cesium.Color(0.165, 0.165, 0.165, 0.7)\n"
           << "  },\n"
           << "  billboard: {\n"
           << "    image: 'data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAyNCAyNCIgd2lkdGg9IjI0IiBoZWlnaHQ9IjI0Ij48Y2lyY2xlIGN4PSIxMiIgY3k9IjEyIiByPSIxMCIgZmlsbD0iYmx1ZSIvPjwvc3ZnPg==',\n"
           << "    width: 32,\n"
           << "    height: 32,\n"
           << "    verticalOrigin: Cesium.VerticalOrigin.BOTTOM,\n"
           << "    color: Cesium.Color.BLUE\n"
           << "  }\n"
           << "});\n";
           
    // 创建辐射源轨迹线实体
    script << "// 创建辐射源轨迹线实体\n"
           << "sourceTrailPositions.push(Cesium.Cartesian3.fromDegrees(sourceTrajectoryPoints[0][0], sourceTrajectoryPoints[0][1], " << source.getAltitude() << "));\n"
           << "var sourceTrailEntity = viewer.entities.add({\n"
           << "  id: 'source-trail',\n"
           << "  polyline: {\n"
           << "    positions: sourceTrailPositions,\n"
           << "    width: 3,\n"
           << "    material: new Cesium.PolylineGlowMaterialProperty({\n"
           << "      glowPower: 0.2,\n"
           << "      color: Cesium.Color.BLUE\n"
           << "    })\n"
           << "  }\n"
           << "});\n";
    
    // 修改移动函数，移除垂直连接线
    script << "// 移动函数\n"
           << "var currentIndex = 0;\n"
           << "function moveEntities() {\n"
           << "  if (currentIndex < sourceTrajectoryPoints.length) {\n"
           << "    // 获取当前位置\n"
           << "    var sourceLongitude = sourceTrajectoryPoints[currentIndex][0];\n"
           << "    var sourceLatitude = sourceTrajectoryPoints[currentIndex][1];\n"
           << "    var sourceAltitude = " << source.getAltitude() << ";\n"
           << "    var sourcePosition = Cesium.Cartesian3.fromDegrees(sourceLongitude, sourceLatitude, sourceAltitude);\n"
           << "    \n"
           << "    // 更新辐射源位置\n"
           << "    sourceEntity.position = sourcePosition;\n"
           << "    \n"
           << "    // 更新辐射源轨迹线\n"
           << "    sourceTrailPositions.push(sourcePosition);\n"
           << "    sourceTrailEntity.polyline.positions = sourceTrailPositions;\n"
           << "    \n"
           << "    // 更新每个设备位置\n"
           << "    for (var i = 0; i < deviceEntities.length; i++) {\n"
           << "      var deviceLongitude = allDeviceTrajectories[i][currentIndex][0];\n"
           << "      var deviceLatitude = allDeviceTrajectories[i][currentIndex][1];\n"
           << "      var deviceAltitude = " << (devices.size() > 0 ? std::to_string(devices[0].getAltitude()) : "1000") << ";\n"
           << "      var devicePosition = Cesium.Cartesian3.fromDegrees(deviceLongitude, deviceLatitude, deviceAltitude);\n"
           << "      \n"
           << "      // 更新设备位置\n"
           << "      deviceEntities[i].position = devicePosition;\n"
           << "      \n"
           << "      // 更新设备轨迹线 - 实时创建和更新\n"
           << "      deviceTrailPositions[i].push(devicePosition);\n"
           << "      deviceTrailEntities[i].polyline.positions = deviceTrailPositions[i];\n"
           << "    }\n"
           << "    \n"
           << "    // 移动到下一个点\n"
           << "    currentIndex++;\n"
           << "    \n"
           << "    // 如果还有点，继续移动\n"
           << "    if (currentIndex < sourceTrajectoryPoints.length) {\n"
           << "      setTimeout(moveEntities, updateInterval);\n"
           << "    } else {\n"
           << "      // 动画结束，显示最终结果\n"
           << "      console.log('动画完成');\n"
           << "      showFinalResults();\n"
           << "    }\n"
           << "  }\n"
           << "}\n";
    
    // 修改显示最终结果函数，移除垂直连接线相关代码
    script << "// 显示最终结果\n"
           << "function showFinalResults() {\n"
           << "  // 隐藏动画中的实体\n"
           << "  sourceEntity.show = false;\n"
           << "  \n"
           << "  for (var i = 0; i < deviceEntities.length; i++) {\n"
           << "    deviceEntities[i].show = false;\n"
           << "  }\n"
           << "  \n"
           << "  // 在最终位置添加设备标记点\n"
           << "  for (var i = 0; i < allDeviceTrajectories.length; i++) {\n"
           << "    var finalIndex = allDeviceTrajectories[i].length - 1;\n"
           << "    var finalLongitude = allDeviceTrajectories[i][finalIndex][0];\n"
           << "    var finalLatitude = allDeviceTrajectories[i][finalIndex][1];\n"
           << "    var deviceAltitude = " << (devices.size() > 0 ? std::to_string(devices[0].getAltitude()) : "1000") << ";\n"
           << "    \n"
           << "    viewer.entities.add({\n"
           << "      id: 'final-device-position-' + i,\n"
           << "      position: Cesium.Cartesian3.fromDegrees(finalLongitude, finalLatitude, deviceAltitude),\n"
           << "      point: {\n"
           << "        pixelSize: 12,\n"
           << "        color: Cesium.Color.RED,\n"
           << "        outlineColor: Cesium.Color.WHITE,\n"
           << "        outlineWidth: 2\n"
           << "      },\n"
           << "      label: {\n";
           
    for (size_t idx = 0; idx < devices.size(); idx++) {
        if (idx == 0) {
            script << "        text: i == " << idx << " ? '" << devices[idx].getDeviceName() << "' : ";
        } else if (idx == devices.size() - 1) {
            script << "i == " << idx << " ? '" << devices[idx].getDeviceName() << "' : '" << devices[0].getDeviceName() << "-' + i,\n";
        } else {
            script << "i == " << idx << " ? '" << devices[idx].getDeviceName() << "' : ";
        }
    }
           
    script << "        font: '14pt sans-serif',\n"
           << "        style: Cesium.LabelStyle.FILL_AND_OUTLINE,\n"
           << "        outlineWidth: 2,\n"
           << "        verticalOrigin: Cesium.VerticalOrigin.BOTTOM,\n"
           << "        pixelOffset: new Cesium.Cartesian2(0, -9),\n"
           << "        showBackground: true,\n"
           << "        backgroundColor: new Cesium.Color(0.165, 0.165, 0.165, 0.7)\n"
           << "      }\n"
           << "    });\n"
           << "  }\n"
           << "  \n"
           << "  // 在最终位置添加辐射源标记点\n"
           << "  var sourceFinalIndex = sourceTrajectoryPoints.length - 1;\n"
           << "  var sourceFinalLongitude = sourceTrajectoryPoints[sourceFinalIndex][0];\n"
           << "  var sourceFinalLatitude = sourceTrajectoryPoints[sourceFinalIndex][1];\n"
           << "  \n"
           << "  viewer.entities.add({\n"
           << "    id: 'final-source-position',\n"
           << "    position: Cesium.Cartesian3.fromDegrees(sourceFinalLongitude, sourceFinalLatitude, " << source.getAltitude() << "),\n"
           << "    point: {\n"
           << "      pixelSize: 12,\n"
           << "      color: Cesium.Color.GREEN,\n"
           << "      outlineColor: Cesium.Color.WHITE,\n"
           << "      outlineWidth: 2\n"
           << "    },\n"
           << "    label: {\n"
           << "      text: '" << source.getRadiationName() << "',\n"
           << "      font: '14pt sans-serif',\n"
           << "      style: Cesium.LabelStyle.FILL_AND_OUTLINE,\n"
           << "      outlineWidth: 2,\n"
           << "      verticalOrigin: Cesium.VerticalOrigin.BOTTOM,\n"
           << "      pixelOffset: new Cesium.Cartesian2(0, -9),\n"
           << "      showBackground: true,\n"
           << "      backgroundColor: new Cesium.Color(0.165, 0.165, 0.165, 0.7)\n"
           << "    }\n"
           << "  });\n"
           << "}\n";
           
    // 启动动画
    script << "// 延迟一下启动动画，确保地图已加载\n"
           << "console.log('准备启动动画...');\n"
           << "setTimeout(function() {\n"
           << "  console.log('开始动画');\n"
           << "  moveEntities();\n"
           << "}, 1000);\n";
    
    // 执行脚本
    mapView->executeScript(script.str());
    
    g_print("多设备移动仿真已启动，仿真时间: %d秒\n", simulationTime);
}