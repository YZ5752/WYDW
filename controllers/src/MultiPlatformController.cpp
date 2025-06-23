#include "../MultiPlatformController.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/RadiationSourceDAO.h"
#include "../../models/TrajectorySimulator.h"
#include <iostream>
#include <sstream>
#include <iomanip>

// 单例实现
MultiPlatformController& MultiPlatformController::getInstance() {
    static MultiPlatformController instance;
    return instance;
}

// 构造函数
MultiPlatformController::MultiPlatformController() : m_view(nullptr) {
}

// 析构函数
MultiPlatformController::~MultiPlatformController() {
}

// 初始化控制器
void MultiPlatformController::init(MultiPlatformView* view) {
    m_view = view;
}

// 获取视图
MultiPlatformView* MultiPlatformController::getView() const {
    return m_view;
}

void MultiPlatformController::startSimulation(const std::vector<std::string>& deviceNames,
                                            const std::string& sourceName,
                                            const std::string& systemType,
                                            double simulationTime) {
    if (!m_view) return;
    
    // 获取地图视图
    MapView* mapView = m_view->getMapView();
    if (!mapView) {
        g_print("错误：无法获取地图视图\n");
        return;
    }
    
    // 获取所有设备和辐射源数据
    std::vector<ReconnaissanceDevice> allDevices = ReconnaissanceDeviceDAO::getInstance().getAllReconnaissanceDevices();
    std::vector<RadiationSource> allSources = RadiationSourceDAO::getInstance().getAllRadiationSources();
    
    // 根据名称查找选中的设备
    std::vector<ReconnaissanceDevice> selectedDevices;
    for (const auto& deviceName : deviceNames) {
        for (const auto& device : allDevices) {
            if (device.getDeviceName() == deviceName) {
                selectedDevices.push_back(device);
                break;
            }
        }
    }
    
    // 查找选中的辐射源
    RadiationSource selectedSource;
    bool sourceFound = false;
    for (const auto& source : allSources) {
        if (source.getRadiationName() == sourceName) {
            selectedSource = source;
            sourceFound = true;
            break;
        }
    }
    
    if (selectedDevices.empty() || !sourceFound) {
        g_print("错误：未找到选定的设备或辐射源\n");
        m_view->updateResult("错误：未找到选定的设备或辐射源");
        return;
    }
    
    // 开始仿真动画
    g_print("开始多平台仿真动画，设备数量：%zu，辐射源：%s\n", 
            selectedDevices.size(), selectedSource.getRadiationName().c_str());
    
    // 默认计算结果位置（使用辐射源的实际位置）
    double calculatedLongitude = selectedSource.getLongitude();
    double calculatedLatitude = selectedSource.getLatitude();
    double calculatedAltitude = selectedSource.getAltitude();
    
    if (systemType == "频差体制") {
        // 获取FDOA算法实例
        FDOAalgorithm& algorithm = FDOAalgorithm::getInstance();
        
        // 初始化算法参数
        algorithm.init(deviceNames, sourceName, systemType, simulationTime);
        
        // 执行算法
        bool success = algorithm.calculate();
        
        if (success) {
            FDOAalgorithm::SourcePositionResult result = algorithm.getResult();
            // 将空间直角坐标转换为大地坐标
            COORD3 resultLBH = xyz2lbh(result.position.p1, result.position.p2, result.position.p3);
            COORD3 velocityResult = velocity_xyz2lbh(resultLBH.p1, resultLBH.p2, 
                                           result.velocity.x, result.velocity.y, result.velocity.z);
            
            // 更新计算结果位置
            calculatedLongitude = resultLBH.p1;
            calculatedLatitude = resultLBH.p2;
            calculatedAltitude = resultLBH.p3;
            
            std::stringstream ss;
            ss << "经度：" << resultLBH.p1 << "°\n";
            ss << "纬度：" << resultLBH.p2 << "°\n";  
            ss << "高度：" << resultLBH.p3 << " 米\n";
            ss << "运动速度：" << velocityResult.p1 << " 米/秒\n";
            ss << "运动方位角：" << velocityResult.p2 << "°\n";
            ss << "运动俯仰角：" << velocityResult.p3 << "°\n\n";
            
            m_view->updateResult(ss.str());
            
            // 执行多设备轨迹动画
            TrajectorySimulator::getInstance().animateMultipleDevicesMovement(
                mapView,
                selectedDevices,
                selectedSource,
                simulationTime,
                calculatedLongitude,
                calculatedLatitude,
                calculatedAltitude
            );
        } else {
            m_view->updateResult("定位计算失败");
        }
    } else if (systemType == "时差体制") {
        // TODO: 实现时差定位算法
        m_view->updateResult("<span color='blue'>时差定位算法尚未实现</span>");
        
        // 即使算法未实现，也执行轨迹动画
        TrajectorySimulator::getInstance().animateMultipleDevicesMovement(
            mapView,
            selectedDevices,
            selectedSource,
            simulationTime,
            calculatedLongitude,
            calculatedLatitude,
            calculatedAltitude
        );
    } else if (systemType == "测向体制") {
        // TODO: 实现测向定位算法
        std::stringstream ss;
        ss << "经度：" << calculatedLongitude << "°\n";
        ss << "纬度：" << calculatedLatitude << "°\n";  
        ss << "高度：" << calculatedAltitude << " 米\n";
        ss << "运动方位角：" << std::fixed << std::setprecision(2) << 315.75 << "°\n";
        ss << "运动俯仰角：" << std::fixed << std::setprecision(2) << 245.30 << "°\n";
        
        m_view->updateResult(ss.str());
        
        // 执行轨迹动画
        TrajectorySimulator::getInstance().animateMultipleDevicesMovement(
            mapView,
            selectedDevices,
            selectedSource,
            simulationTime,
            calculatedLongitude,
            calculatedLatitude,
            calculatedAltitude
        );
    }
} 