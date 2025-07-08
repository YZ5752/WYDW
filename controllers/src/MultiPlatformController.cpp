#include "../MultiPlatformController.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/RadiationSourceDAO.h"
#include "../../models/TrajectorySimulator.h"

#include "../../utils/ErrorCircle.h"
#include "../../utils/ErrorCircleDisplay.h"

#include "../../models/TDOAalgorithm.h"
#include "../../models/DirectionFinding.h"
#include "../../utils/DirectionErrorLines.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

// 声明外部函数
extern TDOAResult calculateTDOAErrorCircle(
    const std::vector<std::string>& deviceNames,
    const std::string& sourceName,
    double tdoaErrorMean,
    double tdoaErrorSigma,
    unsigned int seed
);

// 单例实现
MultiPlatformController& MultiPlatformController::getInstance() {
    static MultiPlatformController instance;
    return instance;
}

// 构造函数
MultiPlatformController::MultiPlatformController() : m_view(nullptr), m_tdoaRmsError(2.0e-9), m_esmToaError(-0.33e-9) {
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

// 设置TDOA误差参数
void MultiPlatformController::setTDOAErrorParams(double tdoaRmsError, double esmToaError) {
    m_tdoaRmsError = tdoaRmsError;
    m_esmToaError = esmToaError;
}

// 计算两点之间的距离
double MultiPlatformController::calculateDistance(const COORD3& p1, const COORD3& p2) {
    return std::sqrt(std::pow(p1.p1 - p2.p1, 2) + 
                     std::pow(p1.p2 - p2.p2, 2) + 
                     std::pow(p1.p3 - p2.p3, 2));
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
        // 获取TDOA算法实例
        TDOAalgorithm& algorithm = TDOAalgorithm::getInstance();

        // 初始化算法参数
        algorithm.init(deviceNames, sourceName, systemType, simulationTime);

        // 执行算法
        if (algorithm.calculate()) {
            // 获取定位结果
            auto result = algorithm.getResult();

            // 格式化结果
            std::stringstream ss;
            ss << std::fixed << std::setprecision(6);

            // 辐射源位置
            ss << "<span weight='bold'>辐射源位置</span>\n";
            ss << "经度：" << result.longitude << "°\n";
            ss << "纬度：" << result.latitude << "°\n";
            ss << "高度：" << result.altitude << " 米\n\n";

            // 运动参数
            ss << "<span weight='bold'>运动参数</span>\n";
            ss << "方位角：" << result.azimuth << "°\n";
            ss << "俯仰角：" << result.elevation << "°\n";
            ss << "速度：" << result.velocity << " 米/秒\n\n";

            // 定位参数
            ss << "<span weight='bold'>定位参数</span>\n";
            ss << "定位时间：" << result.locationTime << " 秒\n";
            ss << "定位距离：" << result.distance << " 米\n";
            ss << "定位精度：" << result.accuracy << " 米";

            // 更新界面显示
            if (m_view) {
                m_view->updateResult(ss.str());
            }
            
            // 使用成员变量获取TDOA误差参数
            double tdoaRmsError = m_tdoaRmsError;
            double esmToaError = m_esmToaError;
            
            // 查询辐射源真实位置
            RadiationSourceDAO& sourceDAO = RadiationSourceDAO::getInstance();
            std::vector<RadiationSource> sources = sourceDAO.getAllRadiationSources();
            RadiationSource targetSource;
            bool found = false;
            
            for (const auto& source : sources) {
                if (source.getRadiationName() == sourceName) {
                    targetSource = source;
                    found = true;
                    break;
                }
            }
            
            if (found) {
                // 计算并显示TDOA误差圆
                TDOAResult errorResult = calculateTDOAErrorCircle(
                    deviceNames, 
                    sourceName, 
                    esmToaError,  // 使用ESM toa Error作为均值误差
                    tdoaRmsError, // 使用TDOA rms Error作为标准差
                    static_cast<unsigned int>(time(nullptr))  // 使用当前时间作为随机种子
                );
                
                // 获取地图视图并显示误差点和误差圆
                MapView* mapView = m_view->getMapView();
                if (mapView && !errorResult.estimatedPoints.empty()) {
                    // 显示误差点
                    showErrorPointsOnMap(mapView, errorResult.estimatedPoints);
                    
                    // 显示误差圆
                    COORD3 centerLBH = xyz2lbh(errorResult.centerPoint.p1, errorResult.centerPoint.p2, errorResult.centerPoint.p3);
                    showErrorCircleOnMap(mapView, centerLBH, errorResult.cepRadius);
                    
                    // 绘制双曲线
                    std::vector<COORD3> stationPositions;
                    std::vector<double> tdoas;
                    
                    // 转换设备位置到空间直角坐标
                    for (const auto& device : selectedDevices) {
                        COORD3 pos = lbh2xyz(device.getLongitude(), device.getLatitude(), device.getAltitude());
                        stationPositions.push_back(pos);
                    }
                    
                    // 计算理论TDOA值
                    COORD3 sourcePos = lbh2xyz(targetSource.getLongitude(), targetSource.getLatitude(), targetSource.getAltitude());
                    tdoas.push_back(0.0); // 参考站的TDOA为0
                    
                    for (size_t i = 1; i < stationPositions.size(); i++) {
                        double dist1 = calculateDistance(stationPositions[0], sourcePos);
                        double dist2 = calculateDistance(stationPositions[i], sourcePos);
                        double tdoa = (dist2 - dist1) / Constants::c;
                        tdoas.push_back(tdoa);
                    }
                    
                    // 绘制双曲线
                    HyperbolaLines::drawTDOAHyperbolas(mapView, stationPositions, tdoas, sourcePos);
                    
                    // 显示误差统计信息
                    std::stringstream errorSS;
                    errorSS << std::fixed << std::setprecision(2);
                    errorSS << "TDOA误差统计:\n";
                    errorSS << "CEP半径: " << errorResult.cepRadius << " 米\n";
                    errorSS << "TDOA rms Error: " << tdoaRmsError * 1e9 << " ns\n";
                    errorSS << "ESM toa Error: " << esmToaError * 1e9 << " ns\n";
                    m_view->updateError(errorSS.str());
                }
            }
        } else {
            if (m_view) {
                m_view->updateResult("<span color='red'>仿真计算失败</span>");
            }
        }
        
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
    } else if (systemType == "测向体制") {
        // 使用测向定位算法实现
        DirectionFinding& algorithm = DirectionFinding::getInstance();
        
        // 初始化算法参数
        algorithm.init(deviceNames, sourceName, simulationTime);
        
        // 获取测向误差参数
        double dev1MeanError = m_view->getDFMeanError(0);
        double dev1StdDev = m_view->getDFStdDev(0);
        double dev2MeanError = m_view->getDFMeanError(1);
        double dev2StdDev = m_view->getDFStdDev(1);
        
        // 执行算法，传入误差参数
        bool success = algorithm.calculate(dev1MeanError, dev1StdDev, dev2MeanError, dev2StdDev);
        
        if (success) {
            auto result = algorithm.getResult();
            
            // 将空间直角坐标转换为大地坐标
            COORD3 resultLBH = result.position;
            
            // 计算方位角和俯仰角（以第一个设备为参考）
            ReconnaissanceDevice& device1 = selectedDevices[0];
            COORD3 device1Pos = lbh2xyz(device1.getLongitude(), device1.getLatitude(), device1.getAltitude());
            
            // 计算从设备到计算位置的向量
            COORD3 resultPos = lbh2xyz(resultLBH.p1, resultLBH.p2, resultLBH.p3);
            double dx = resultPos.p1 - device1Pos.p1;
            double dy = resultPos.p2 - device1Pos.p2;
            double dz = resultPos.p3 - device1Pos.p3;
            
            // 计算水平距离
            double horizontalDist = std::sqrt(dx*dx + dy*dy);
            
            // 计算方位角（相对北方向的水平角度）
            double azimuth = std::atan2(dx, dy) * Constants::RAD2DEG;
            if (azimuth < 0) azimuth += 360.0;
            
            // 计算俯仰角（相对水平面的仰角）
            double elevation = std::atan2(dz, horizontalDist) * Constants::RAD2DEG;
            
            // // 计算距离
            // double distance = std::sqrt(dx*dx + dy*dy + dz*dz);
            
            // 输出结果
            std::stringstream ss;
            ss << std::fixed << std::setprecision(6);
            ss << "定位结果：\n";
            ss << "经度: " << resultLBH.p1 << " 度\n";
            ss << "纬度: " << resultLBH.p2 << " 度\n";
            ss << std::setprecision(2);
            ss << "高度: " << selectedSource.getAltitude() << " 米\n";
            ss << "定位误差: " << result.error << " 米\n";
            
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
            // task.positioningDistance = distance;
            task.positioningTime = simulationTime;
            
            // 限制精度值，避免数据库溢出
            task.positioningAccuracy = result.error;
            task.deviceIds = deviceIds;
            
            int taskId;
            MultiPlatformTaskDAO::getInstance().addMultiPlatformTask(task, taskId);

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
                dev1MeanError, dev1StdDev, // 使用从视图获取的误差参数，而不是固定值
                dev2MeanError, dev2StdDev,
                0 // 随机种子,0表示系统当前时间      
            );
            showErrorPointsOnMap(mapView, dfResult.estimatedPoints);
            // 圆心用定位结果的空间直角坐标
            showErrorCircleOnMap(mapView, resultLBH, dfResult.cepRadius);

            // 显示测向误差线
            DirectionErrorLines directionErrorLines;
            m_view->clearDirectionErrorLines(); // 清除可能存在的旧线
            
            // 设置颜色
            const std::string colors[] = {"#FF0000", "#0000FF"};
            
            // 为每个设备绘制测向线 - 使用从视图获取的误差参数
            for (size_t i = 0; i < selectedDevices.size() && i < 2; ++i) {
                double meanError = (i == 0) ? dev1MeanError : dev2MeanError;
                double stdDev = (i == 0) ? dev1StdDev : dev2StdDev;
                
                // 使用计算的定位结果位置，而不是真实辐射源位置
                // 这样测向线会指向计算结果，而不是真实目标位置
                directionErrorLines.showDirectionSimulationLines(
                    mapView,
                    selectedDevices[i],
                    resultLBH.p1,   // 使用计算的定位结果经度
                    resultLBH.p2,   // 使用计算的定位结果纬度
                    resultLBH.p3,   // 使用计算的定位结果高度
                    meanError,      // 均值误差
                    stdDev,         // 标准差
                    colors[i],      // 不同设备使用不同颜色
                    40000.0         // 足够长的线
                );
                
            }
        }
    }
} 