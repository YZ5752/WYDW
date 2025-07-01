#include "../MultiPlatformController.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/RadiationSourceDAO.h"
#include "../../models/TrajectorySimulator.h"
#include "../../models/DirectionFindingAlgorithm.h"
#include "../../utils/ErrorCircle.h"
#include "../../utils/ErrorCircleDisplay.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

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
        m_view->updateResult("错误：无法获取地图视图");
        return;
    }
    
    // 获取所有设备和辐射源数据
    std::vector<ReconnaissanceDevice> allDevices = ReconnaissanceDeviceDAO::getInstance().getAllReconnaissanceDevices();
    std::vector<RadiationSource> allSources = RadiationSourceDAO::getInstance().getAllRadiationSources();
    
    // 根据名称查找选中的设备
    std::vector<ReconnaissanceDevice> selectedDevices;
    std::vector<int> deviceIds;  // 添加deviceIds变量
    for (const auto& deviceName : deviceNames) {
        for (const auto& device : allDevices) {
            if (device.getDeviceName() == deviceName) {
                selectedDevices.push_back(device);
                deviceIds.push_back(device.getDeviceId());  // 保存设备ID
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
        m_view->updateResult("错误：未找到选定的设备或辐射源");
        return;
    }
    
    
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
            
            // 计算速度的大地坐标表示
            COORD3 velocityResult = velocity_xyz2lbh(resultLBH.p1, resultLBH.p2, 
                                           result.velocity.x, result.velocity.y, result.velocity.z);
            
            //经过simulationTime时间后，辐射源移动的位置
            COORD3 movedPosition = algorithm.calculateSourcePositionAtTime(selectedSource, simulationTime);
            
            //经过simulationTime时间后，侦察站1的位置
            COORD3 device1Position = algorithm.calculateDevicePositionAtTime(selectedDevices[0], simulationTime);
            
            //计算侦察站1到辐射源移动后的位置的距离
            double distance = std::sqrt(
                (movedPosition.p1 - device1Position.p1) * (movedPosition.p1 - device1Position.p1) + 
                (movedPosition.p2 - device1Position.p2) * (movedPosition.p2 - device1Position.p2) + 
                (movedPosition.p3 - device1Position.p3) * (movedPosition.p3 - device1Position.p3)
            );
            
            //计算方位角
            double theta_t = atan2(movedPosition.p1 - device1Position.p1, 
                                 movedPosition.p2 - device1Position.p2);
            double azimuth = theta_t * Constants::RAD2DEG;  // 转换为角度
            
            //计算俯仰角
            double r_pt = sqrt(pow(movedPosition.p1 - device1Position.p1, 2) + 
                             pow(movedPosition.p2 - device1Position.p2, 2));
            double epsilon_t = atan2(movedPosition.p3 - device1Position.p3, r_pt);
            double elevation = epsilon_t * Constants::RAD2DEG;  // 转换为角度
            
            // 计算定位精度
            double localizationAccuracy = algorithm.calculateLocalizationAccuracy(
                deviceIds,
                selectedSource.getRadiationId(),
                simulationTime,
                result.position,
                result.velocity
            );
            
            // 输出结果
            std::stringstream ss;
            ss << "定位结果：\n";
            ss << "经度: " << resultLBH.p1 << " 度\n";
            ss << "纬度: " << resultLBH.p2 << " 度\n";
            ss << "高度: " << resultLBH.p3 << " 米\n";
            ss << "运动速度: " << velocityResult.p1 << " m/s\n";
            ss << "运动方位角: " << velocityResult.p2 << " 度\n";
            ss << "运动俯仰角: " << velocityResult.p3 << " 度\n";
            ss << "定位时间: " << simulationTime << " 秒\n";
            ss << "定位距离: " << distance << " 米\n";
            ss << "定位精度: " << localizationAccuracy << "\n";
            ss << "方位角: " << azimuth << " 度\n";
            ss << "俯仰角: " << elevation << " 度\n";

            // 同时输出到控制台
            std::cout << ss.str() << std::endl;
            
            // 更新视图
            m_view->updateResult(ss.str());
            
            // 保存多平台仿真任务信息到数据库
            MultiPlatformTask task;
            task.techSystem = "FDOA";  // 频差定位
            task.radiationId = selectedSource.getRadiationId();
            task.executionTime = simulationTime;
            task.targetLongitude = resultLBH.p1;
            task.targetLatitude = resultLBH.p2;
            task.targetAltitude = resultLBH.p3;
            task.movementSpeed = velocityResult.p1;
            task.movementAzimuth = velocityResult.p2;
            task.movementElevation = velocityResult.p3;
            task.azimuth = azimuth;
            task.elevation = elevation;
            task.positioningDistance = distance;
            task.positioningTime = result.locationTime;
            task.positioningAccuracy = localizationAccuracy;
            task.deviceIds = deviceIds;

            int taskId;
            if (!MultiPlatformTaskDAO::getInstance().addMultiPlatformTask(task, taskId)) {
                std::cerr << "多平台仿真任务保存失败" << std::endl;
                m_view->updateResult("定位结果（多平台仿真任务保存失败）");
            }
            
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
            
            // 显示测向误差线
            // 假设角度误差为5度（可以根据实际算法计算结果调整）
            double errorAngle = 5.0;
            std::vector<int> activeDeviceIndices;
            for (int i = 0; i < deviceNames.size(); ++i) {
                activeDeviceIndices.push_back(i);
            }
            
            // 清除可能存在的测向误差线
            m_view->clearDirectionErrorLines();
            
            // 添加0.5秒延迟，等待轨迹动画开始后再显示测向误差线
            // 注意：这里的延迟是为了避免视觉冲突，确保轨迹动画先开始
            std::string delayScript = "setTimeout(function() { console.log('Ready to show direction error lines'); }, 500);";
            mapView->executeScript(delayScript);
            
            // 显示所有设备的测向误差线
            m_view->showMultipleDeviceErrorLines(
                activeDeviceIndices,
                resultLBH.p1,  // 使用计算得到的位置
                resultLBH.p2,
                resultLBH.p3,
                errorAngle
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
        
        // 显示测向误差线
        double errorAngle = 8.0; // 时差体制通常误差略大
        std::vector<int> activeDeviceIndices;
        for (int i = 0; i < deviceNames.size(); ++i) {
            activeDeviceIndices.push_back(i);
        }
        
        // 添加延迟
        std::string delayScript = "setTimeout(function() { console.log('Ready to show direction error lines'); }, 500);";
        mapView->executeScript(delayScript);
        
        // 显示所有设备的测向误差线
        m_view->showMultipleDeviceErrorLines(
            activeDeviceIndices,
            calculatedLongitude,  // 使用真实位置（因为算法未实现）
            calculatedLatitude,
            calculatedAltitude,
            errorAngle
        );
    } else if (systemType == "测向体制") {
        // 使用测向定位算法实现
        DirectionFindingAlgorithm& algorithm = DirectionFindingAlgorithm::getInstance();
        
        // 初始化算法参数
        algorithm.init(deviceNames, sourceName, systemType, simulationTime);
        
        // 执行算法
        bool success = algorithm.calculate();
        
        if (success) {
            auto result = algorithm.getResult();
            
            // 将空间直角坐标转换为大地坐标
            COORD3 resultLBH = xyz2lbh(result.position.p1, result.position.p2, result.position.p3);
            
            // 计算方位角和俯仰角（以第一个设备为参考）
            ReconnaissanceDevice& device1 = selectedDevices[0];
            COORD3 device1Pos = lbh2xyz(device1.getLongitude(), device1.getLatitude(), device1.getAltitude());
            
            // 计算从设备到计算位置的向量
            double dx = result.position.p1 - device1Pos.p1;
            double dy = result.position.p2 - device1Pos.p2;
            double dz = result.position.p3 - device1Pos.p3;
            
            // 计算水平距离
            double horizontalDist = std::sqrt(dx*dx + dy*dy);
            
            // 计算方位角（相对北方向的水平角度）
            double azimuth = std::atan2(dx, dy) * Constants::RAD2DEG;
            if (azimuth < 0) azimuth += 360.0;
            
            // 计算俯仰角（相对水平面的仰角）
            double elevation = std::atan2(dz, horizontalDist) * Constants::RAD2DEG;
            
            // 计算距离
            double distance = std::sqrt(dx*dx + dy*dy + dz*dz);
            
            // 输出结果
            std::stringstream ss;
            ss << std::fixed << std::setprecision(6);
            ss << "定位结果：\n";
            ss << "经度: " << resultLBH.p1 << " 度\n";
            ss << "纬度: " << resultLBH.p2 << " 度\n";
            ss << std::setprecision(2);
            ss << "高度: " << resultLBH.p3 << " 米\n";
            ss << "定位时间: " << result.localizationTime << " 秒\n";
            ss << "定位距离: " << distance << " 米\n";
            ss << "定位精度 (GDOP): " << result.accuracy << "\n";
            ss << "方位角: " << azimuth << " 度\n";
            ss << "俯仰角: " << elevation << " 度\n";
            
            // 更新视图
            m_view->updateResult(ss.str());
            
            // 保存多平台仿真任务信息到数据库
            MultiPlatformTask task;
            task.techSystem = "TDOA"; // 使用数据库支持的值，将DF归入TDOA类别
            task.radiationId = selectedSource.getRadiationId();
            task.executionTime = simulationTime;
            task.targetLongitude = resultLBH.p1;
            task.targetLatitude = resultLBH.p2;
            task.targetAltitude = resultLBH.p3;
            
            // 添加默认的运动参数
            task.movementSpeed = 0.0f;  // 静止目标
            task.movementAzimuth = 0.0;
            task.movementElevation = 0.0;
            
            task.azimuth = azimuth;
            task.elevation = elevation;
            task.positioningDistance = distance;
            task.positioningTime = result.localizationTime;
            
            // 限制精度值，避免数据库溢出
            task.positioningAccuracy = std::min(1000.0, result.accuracy);
            task.deviceIds = deviceIds;
            
            int taskId;
            if (!MultiPlatformTaskDAO::getInstance().addMultiPlatformTask(task, taskId)) {
                std::cerr << "多平台仿真任务保存失败" << std::endl;
                // 不覆盖已显示的结果，只添加错误信息
                m_view->updateResult(ss.str() + "\n多平台仿真任务保存失败");
            }
            
            // 执行轨迹动画
            TrajectorySimulator::getInstance().animateMultipleDevicesMovement(
                mapView,
                selectedDevices,
                selectedSource,
                simulationTime,
                resultLBH.p1,  // 使用计算的位置
                resultLBH.p2,
                resultLBH.p3
            );
            
            // 误差圆计算与显示
            DFResult dfResult = calculateDFErrorCircle(
                deviceNames,
                sourceName,
                3.0, 2.0, // 可根据实际需求调整误差均值和标准差
                3.0, 1.0,
                0//随机种子,0表示系统当前时间      
            );
            std::cout << "误差点数量: " << dfResult.estimatedPoints.size() << ", 误差圆半径: " << dfResult.cepRadius << std::endl;
            showErrorPointsOnMap(mapView, dfResult.estimatedPoints);
            // 圆心用定位结果的空间直角坐标
            showErrorCircleOnMap(mapView, resultLBH, dfResult.cepRadius);




            // 显示测向误差线
            double errorAngle = 3.0; // 使用默认测向误差角度
            
            // 如果有测向误差因子，使用实际计算值
            if (!result.errorFactors.empty() && result.errorFactors.size() > 1) {
                // 假设第一个误差因子是角度误差（度）
                errorAngle = result.errorFactors[0];
            }
            
            std::vector<int> activeDeviceIndices;
            for (int i = 0; i < deviceNames.size(); ++i) {
                activeDeviceIndices.push_back(i);
            }
            
            // 添加延迟
            std::string delayScript = "setTimeout(function() { console.log('Ready to show direction error lines'); }, 500);";
            mapView->executeScript(delayScript);
            
            // 显示所有设备的测向误差线
            m_view->showMultipleDeviceErrorLines(
                activeDeviceIndices,
                resultLBH.p1,
                resultLBH.p2,
                resultLBH.p3,
                errorAngle
            );
        } else {
            m_view->updateResult("测向定位计算失败");
            
            // 即使算法失败，也执行轨迹动画（使用真实辐射源位置）
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
} 