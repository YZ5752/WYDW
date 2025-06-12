#include "../SinglePlatformController.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/RadiationSourceDAO.h"
#include "../../constants/PhysicsConstants.h"
#include "../../utils/CoordinateTransform.h"
#include <iostream>
#include <cmath>    // 添加数学函数头文件
#include <ctime>    // 添加时间函数头文件
#include <cstdlib>  // 添加随机数函数头文件
#include <sstream>  // 添加字符串流头文件

// 使用常量命名空间
using namespace Constants;

// 单例实现
SinglePlatformController& SinglePlatformController::getInstance() {
    static SinglePlatformController instance;
    return instance;
}

// 构造函数
SinglePlatformController::SinglePlatformController() : m_view(nullptr) {
    // 初始化随机数种子
    srand(time(NULL));
}

// 析构函数
SinglePlatformController::~SinglePlatformController() {
}

// 初始化控制器
void SinglePlatformController::init(SinglePlatformView* view) {
    m_view = view;
    loadModelData();
}

// 启动仿真
void SinglePlatformController::startSimulation() {
    g_print("\n====== 单平台仿真按钮被点击 ======\n");
    
    if (!m_view) {
        g_print("错误：SinglePlatformController的m_view为空，控制器未正确初始化\n");
        return;
    }
    
    // 获取视图中选择的参数
    std::string techSystem = m_view->getSelectedTechSystem();
    std::string deviceName = m_view->getSelectedDevice();
    std::string sourceName = m_view->getSelectedSource();
    int simulationTime = m_view->getSimulationTime();
    
    g_print("开始单平台仿真...\n");
    g_print("技术体制: %s\n", techSystem.c_str());
    g_print("侦察设备: %s\n", deviceName.c_str());
    g_print("辐射源: %s\n", sourceName.c_str());
    g_print("仿真时间: %d秒\n", simulationTime);
    
    // 根据选择的设备名称获取对应的模型对象
    ReconnaissanceDevice device;
    bool deviceFound = false;
    
    // 从DAO获取设备
    std::vector<ReconnaissanceDevice> devices = ReconnaissanceDeviceDAO::getInstance().getAllReconnaissanceDevices();
    for (const auto& d : devices) {
        if (d.getDeviceName() == deviceName) {
            device = d;
            deviceFound = true;
            break;
        }
    }
    
    if (!deviceFound) {
        std::string errorMsg = "错误：未找到侦察设备 '" + deviceName + "'";
        g_print("%s\n", errorMsg.c_str());
        if (m_view) m_view->showErrorMessage(errorMsg);
        return;
    }
    
    // 验证侦察设备必须是移动设备
    if (device.getIsStationary()) {
        std::string errorMsg = "错误：单平台仿真要求侦察设备必须是移动设备";
        g_print("%s\n", errorMsg.c_str());
        if (m_view) m_view->showErrorMessage(errorMsg);
        return;
    }
    
    // 获取辐射源对象
    RadiationSource source;
    bool sourceFound = false;
    
    // 从DAO获取辐射源
    std::vector<RadiationSource> sources = RadiationSourceDAO::getInstance().getAllRadiationSources();
    for (const auto& s : sources) {
        if (s.getRadiationName() == sourceName) {
            source = s;
            sourceFound = true;
            break;
        }
    }
    
    if (!sourceFound) {
        std::string errorMsg = "错误：未找到辐射源 '" + sourceName + "'";
        g_print("%s\n", errorMsg.c_str());
        if (m_view) m_view->showErrorMessage(errorMsg);
        return;
    }
    
    // 验证辐射源必须是固定的
    if (!source.getIsStationary()) {
        std::string errorMsg = "错误：单平台仿真要求辐射源必须是固定的";
        g_print("%s\n", errorMsg.c_str());
        if (m_view) m_view->showErrorMessage(errorMsg);
        return;
    }
    
    g_print("使用侦察设备 ID: %d, 名称: %s\n", device.getDeviceId(), device.getDeviceName().c_str());
    g_print("使用辐射源 ID: %d, 名称: %s\n", source.getRadiationId(), source.getRadiationName().c_str());
    
    // 添加仿真前验证条件
    // 1. 时间验证：扫描周期必须小于等于仿真时间
    if (source.getScanPeriod() > simulationTime) {
        std::string errorMsg = "错误：仿真时间(" + std::to_string(simulationTime) + "秒)必须大于等于辐射源扫描周期(" 
            + std::to_string(source.getScanPeriod()) + "秒)";
        g_print("%s\n", errorMsg.c_str());
        if (m_view) m_view->showErrorMessage(errorMsg);
        return;
    }
    
    // 2. 频率验证：辐射源频率必须落在侦察设备的频率范围内
    double carrierFreq = source.getCarrierFrequency();
    if (carrierFreq < device.getFreqRangeMin() || carrierFreq > device.getFreqRangeMax()) {
        std::string errorMsg = "错误：辐射源载波频率(" + std::to_string(carrierFreq) + " GHz)不在侦察设备的频率范围(" 
            + std::to_string(device.getFreqRangeMin()) + "-" + std::to_string(device.getFreqRangeMax()) + " GHz)内";
        g_print("%s\n", errorMsg.c_str());
        if (m_view) m_view->showErrorMessage(errorMsg);
        return;
    }
    
    // 3. 角度验证：方位角和俯仰角的交集必须非空
    // 检查方位角交集
    bool azimuthOverlap = !(device.getAngleAzimuthMax() <= source.getAzimuthStart() || 
                           device.getAngleAzimuthMin() >= source.getAzimuthEnd());
    
    // 检查俯仰角交集
    bool elevationOverlap = !(device.getAngleElevationMax() <= source.getElevationStart() || 
                             device.getAngleElevationMin() >= source.getElevationEnd());
    
    if (!azimuthOverlap || !elevationOverlap) {
        std::string errorMsg = "错误：侦察设备与辐射源的工作角度范围没有交集\n";
        errorMsg += "侦察设备方位角范围: " + std::to_string(device.getAngleAzimuthMin()) + "-" 
            + std::to_string(device.getAngleAzimuthMax()) + "度, 辐射源方位角范围: " 
            + std::to_string(source.getAzimuthStart()) + "-" + std::to_string(source.getAzimuthEnd()) + "度\n";
        errorMsg += "侦察设备俯仰角范围: " + std::to_string(device.getAngleElevationMin()) + "-" 
            + std::to_string(device.getAngleElevationMax()) + "度, 辐射源俯仰角范围: " 
            + std::to_string(source.getElevationStart()) + "-" + std::to_string(source.getElevationEnd()) + "度";
        g_print("%s\n", errorMsg.c_str());
        if (m_view) m_view->showErrorMessage(errorMsg);
        return;
    }
    
    // 4. SNR验证：信噪比必须大于10dB
    // 计算设备和辐射源之间的距离（单位：米）
    // 使用坐标转换函数计算设备和辐射源的空间直角坐标
    COORD3 deviceXYZ = lbh2xyz(device.getLongitude(), device.getLatitude(), device.getAltitude());
    COORD3 sourceXYZ = lbh2xyz(source.getLongitude(), source.getLatitude(), source.getAltitude());
    
    // 计算两点之间的距离
    double dx = sourceXYZ.p1 - deviceXYZ.p1;
    double dy = sourceXYZ.p2 - deviceXYZ.p2;
    double dz = sourceXYZ.p3 - deviceXYZ.p3;
    double distance = sqrt(dx*dx + dy*dy + dz*dz);
    
    // 简化的自由空间损耗计算（不考虑波长和其他因素）
    // 信号强度 = 发射功率 - 路径损耗
    // 路径损耗 ~ 20*log10(distance)
    // 信噪比 = 信号强度 - 噪声功率
    
    // 转换发射功率从千瓦到dBm: P(dBm) = 10*log10(P(W)) + 30
    double transmitPowerDBm = 10 * log10(source.getTransmitPower() * 1000) + 30;
    
    // 简化的路径损耗计算（仅考虑距离）
    double pathLoss = 20 * log10(distance);
    
    // 接收信号强度（dBm）
    double receivedPower = transmitPowerDBm - pathLoss;
    
    // 噪声功率（dBm）- 使用设备的噪声功率谱密度
    double noisePower = device.getNoisePsd();
    
    // 信噪比（dB）
    double snr = receivedPower - noisePower;
    
    // 验证信噪比是否大于10dB
    if (snr <= 10.0) {
        std::string errorMsg = "错误：信噪比(" + std::to_string(snr) + " dB)小于最低要求的10 dB\n";
        errorMsg += "设备与辐射源距离: " + std::to_string(distance) + " 米\n";
        errorMsg += "接收信号强度: " + std::to_string(receivedPower) + " dBm, 噪声功率: " + std::to_string(noisePower) + " dBm";
        g_print("%s\n", errorMsg.c_str());
        if (m_view) m_view->showErrorMessage(errorMsg);
        return;
    }
    
    g_print("仿真条件验证通过！开始执行仿真...\n");
    
    // 设置初始地图视角
    MapView* mapView = m_view->getMapView();
    if (mapView) {
        // 不再主动设置地图视角，而是保持当前视角不变
        
        // 构建设备描述信息
        std::stringstream deviceDesc;
        deviceDesc << "侦察设备: " << device.getDeviceName() << "\n"
                   << "高度: " << device.getAltitude() << "米\n"
                   << "速度: " << device.getMovementSpeed() << "米/秒\n"
                   << "方位角: " << device.getMovementAzimuth() << "度";
        
        // 构建辐射源描述信息
        std::stringstream sourceDesc;
        sourceDesc << "辐射源: " << source.getRadiationName() << "\n"
                   << "高度: " << source.getAltitude() << "米\n"
                   << "频率: " << source.getCarrierFrequency() << "GHz";
        
        // 确保清除之前的标记
        mapView->clearMarkers();
        
        // 直接添加标记点 - 而不是调用updateRadarMarker和updateSourceMarker
        int radarMarker = mapView->addMarker(
            device.getLongitude(), 
            device.getLatitude(), 
            device.getDeviceName(), 
            deviceDesc.str(), 
            "red"
        );
        
        int sourceMarker = mapView->addMarker(
            source.getLongitude(), 
            source.getLatitude(), 
            source.getRadiationName(), 
            sourceDesc.str(), 
            "blue"
        );
        
        // 保存当前视角位置，确保仿真过程中不会改变
        std::string saveCameraScript = 
            "window.savedCameraPosition = viewer.camera.position.clone();"
            "window.savedCameraHeading = viewer.camera.heading;"
            "window.savedCameraPitch = viewer.camera.pitch;"
            "window.savedCameraRoll = viewer.camera.roll;"
            "console.log('Saved current camera position for simulation');";
        mapView->executeScript(saveCameraScript);
        
        // 等待地图加载完成
        std::string waitScript = "setTimeout(function() { console.log('Map ready for simulation'); }, 1000);";
        mapView->executeScript(waitScript);
    }
    
    // 执行设备移动仿真
    simulateDeviceMovement(device, source, simulationTime);
    
    // 执行仿真
    LocationResult result;
    
    // 根据选择的技术体制执行不同的算法
    if (techSystem == "干涉仪体制") {
        g_print("执行干涉仪体制定位算法...\n");
        result = runInterferometerSimulation(device, source, simulationTime);
    } else if (techSystem == "时差体制") {
        g_print("执行时差体制定位算法...\n");
        result = runTDOASimulation(device, source, simulationTime);
    } 
    
    // 更新视图显示结果
    char directionBuffer[100];
    sprintf(directionBuffer, "方位角: %.2f°, 俯仰角: %.2f°", result.azimuth, result.elevation);
    std::string directionData(directionBuffer);
    
    char locationBuffer[200];
    sprintf(locationBuffer, "经度: %.6f°, 纬度: %.6f°, 高度: %.2fm, 精度: %.2fm", 
            result.longitude, result.latitude, result.altitude, result.accuracy);
    std::string locationData(locationBuffer);
    
    m_view->updateDirectionData(directionData);
    m_view->updateLocationData(locationData);
    
    // 更新误差表格
    if (techSystem == "干涉仪体制") {
        // 更新干涉仪体制误差表格
        const char* errorNames[] = {
            "对中误差", "姿态测量误差", "圆锥效应误差", "天线阵测向误差", "测向误差"
        };
        
        GtkWidget* errorTable = m_view->getErrorTable();
        if (errorTable && result.errorFactors.size() >= 5) {
            for (int i = 0; i < 5; i++) {
                char errorBuffer[50];
                sprintf(errorBuffer, "%.4f", result.errorFactors[i]);
                
                GtkWidget* errorValue = gtk_label_new(errorBuffer);
                gtk_widget_set_halign(errorValue, GTK_ALIGN_END);
                gtk_grid_attach(GTK_GRID(errorTable), errorValue, 1, i, 1, 1);
            }
            
            // 显示所有控件
            gtk_widget_show_all(errorTable);
        }
    }
}

// 加载模型数据
void SinglePlatformController::loadModelData() {
    if (!m_view) return;
    
    // 从DAO加载设备和辐射源数据
    std::vector<ReconnaissanceDevice> devices = ReconnaissanceDeviceDAO::getInstance().getAllReconnaissanceDevices();
    std::vector<RadiationSource> sources = RadiationSourceDAO::getInstance().getAllRadiationSources();
    
    // 更新视图中的设备列表
    if (m_view) {
        m_view->updateDeviceList(devices);
    }
}

// 技术体制变化处理
void SinglePlatformController::handleTechSystemChange(const std::string& techSystem) {
    if (!m_view) return;
    
    // 更新误差表格
    m_view->updateErrorTable(techSystem);
}

// 获取视图
SinglePlatformView* SinglePlatformController::getView() const {
    return m_view;
}

// 干涉仪体制定位算法实现
LocationResult SinglePlatformController::runInterferometerSimulation(const ReconnaissanceDevice& device, 
                                                                   const RadiationSource& source,
                                                                   int simulationTime) {
    LocationResult result;
    
    // 根据图片中的算法，使用相位差变化率定位法
    
    // 获取观测站初始位置（设备当前位置）
    COORD3 deviceXYZ = lbh2xyz(device.getLongitude(), device.getLatitude(), device.getAltitude());
    double X_0 = deviceXYZ.p1;
    double Y_0 = deviceXYZ.p2;
    double Z_0 = deviceXYZ.p3;
    
    // 获取辐射源位置（固定）
    COORD3 sourceXYZ = lbh2xyz(source.getLongitude(), source.getLatitude(), source.getAltitude());
    double X_T = sourceXYZ.p1;
    double Y_T = sourceXYZ.p2;
    double Z_T = sourceXYZ.p3;
    
    // 获取观测站速度
    // 将设备的运动速度和方向转换为笛卡尔坐标系中的速度分量
    COORD3 velocityXYZ = velocity_lbh2xyz(
        device.getLongitude(), 
        device.getLatitude(), 
        device.getMovementSpeed(), 
        device.getMovementAzimuth(), 
        device.getMovementElevation()
    );
    double v_x = velocityXYZ.p1;
    double v_y = velocityXYZ.p2;
    double v_z = velocityXYZ.p3;
    
    // 获取基线长度
    double d = device.getBaselineLength();
    
    // 获取辐射源频率（GHz转换为Hz）
    double f_T = source.getCarrierFrequency() * 1e9;
    
    // 计算方位角 θ(t) = tg^(-1)((X_T - X_0)/(Y_T - Y_0)) (公式4.2.3)
    double theta_t = atan2(X_T - X_0, Y_T - Y_0);
    
    // 计算俯仰角 ε(t) = tg^(-1)((Z_T - Z_0)/sqrt((X_T - X_0)^2 + (Y_T - Y_0)^2)) (公式4.2.4)
    double r_pt = sqrt(pow(X_T - X_0, 2) + pow(Y_T - Y_0, 2));
    double epsilon_t = atan2(Z_T - Z_0, r_pt);
    
    // 计算方位角变化率 θ'(t) = (v_y*sin(θ(t)) - v_x*cos(θ(t)))/r_pt (公式4.2.5)
    double theta_dot_t = (v_y * sin(theta_t) - v_x * cos(theta_t)) / r_pt;
    
    // 计算俯仰角变化率 ε'(t) = (-v_z*cos(ε(t)) + r_pt_dot*sin(ε(t)))/r (公式4.2.6)
    // 其中 r_pt_dot = (X_T - X_0)*sin(θ(t)) + (Y_T - Y_0)*cos(θ(t))
    double r_pt_dot = (X_T - X_0) * sin(theta_t) + (Y_T - Y_0) * cos(theta_t);
    double r_t = sqrt(pow(X_T - X_0, 2) + pow(Y_T - Y_0, 2) + pow(Z_T - Z_0, 2));
    double epsilon_dot_t = (-v_z * cos(epsilon_t) + r_pt_dot * sin(epsilon_t)) / r_t;
    
    // 计算相位差变化率 (公式4.2.7)
    double delta_phi_dot_t = (2 * M_PI * d * f_T / c) * (
        (v_y * sin(theta_t) - v_x * cos(theta_t)) * cos(theta_t) * cos(epsilon_t) / (r_t * cos(epsilon_t)) -
        (r_pt_dot * sin(epsilon_t) - v_z * cos(epsilon_t)) * sin(theta_t) * sin(epsilon_t) / r_t
    );
    
    // 计算距离 (公式4.2.8)
    // 根据公式4.2.8，r_hat = (Δφ'(t)/fT * c/(2πd))^(-1) * { [y'O sin(θ) - x'O cos(θ)]cos(θ)cos(ε) - [r'pt sin(ε) - z'O cos(ε)]sin(θ)sin(ε) }
    double numerator = (v_y * sin(theta_t) - v_x * cos(theta_t)) * cos(theta_t) * cos(epsilon_t) -
                       (r_pt_dot * sin(epsilon_t) - v_z * cos(epsilon_t)) * sin(theta_t) * sin(epsilon_t);
    double denominator = delta_phi_dot_t * c / (2 * M_PI * d * f_T);
    double r_hat = numerator / denominator;
    
    // 计算辐射源坐标 (公式4.2.9)
    double X_T_calculated = X_0 + r_hat * cos(epsilon_t) * sin(theta_t);
    double Y_T_calculated = Y_0 + r_hat * cos(epsilon_t) * cos(theta_t);
    double Z_T_calculated = Z_0 + r_hat * sin(epsilon_t);
    
    // 将计算得到的辐射源笛卡尔坐标转换回经纬度高度
    COORD3 sourceLBH = xyz2lbh(X_T_calculated, Y_T_calculated, Z_T_calculated);
    
    // 设置结果
    result.longitude = sourceLBH.p1;
    result.latitude = sourceLBH.p2;
    result.altitude = sourceLBH.p3;
    
    // 将弧度转换为角度
    result.azimuth = theta_t * RAD2DEG;
    if (result.azimuth < 0) result.azimuth += 360.0;
    result.elevation = epsilon_t * RAD2DEG;
    
    // 计算定位精度（实际辐射源位置与计算位置之间的距离）
    double dx = X_T - X_T_calculated;
    double dy = Y_T - Y_T_calculated;
    double dz = Z_T - Z_T_calculated;
    result.accuracy = sqrt(dx*dx + dy*dy + dz*dz);
    
    // 计算误差因素
    result.errorFactors = calculateInterferometerErrors(device, source, r_hat);
    
    g_print("干涉仪体制定位结果：\n");
    g_print("  方位角: %.2f°, 俯仰角: %.2f°\n", result.azimuth, result.elevation);
    g_print("  经度: %.6f°, 纬度: %.6f°, 高度: %.2fm\n", result.longitude, result.latitude, result.altitude);
    g_print("  定位精度: %.2fm\n", result.accuracy);
    
    return result;
}

// TODO 时差体制定位算法实现
LocationResult SinglePlatformController::runTDOASimulation(const ReconnaissanceDevice& device, 
                                                        const RadiationSource& source,
                                                        int simulationTime) {
    // 简化实现，返回与干涉仪体制类似的结果但精度略低
    LocationResult result = runInterferometerSimulation(device, source, simulationTime);
    
    // 修改误差因素为时差体制的误差项
    result.errorFactors.clear();
    result.errorFactors.push_back(0.0015); // 时延误差
    result.errorFactors.push_back(0.0025); // 通道热噪声误差
    result.errorFactors.push_back(0.0035); // 时间测量误差
    result.errorFactors.push_back(0.0045); // 时差测量误差
    result.errorFactors.push_back(0.0055); // 测向误差
    
    // 降低精度
    result.accuracy *= 1.2;
    
    return result;
}

// 计算测向数据 - 干涉仪体制
std::pair<double, double> SinglePlatformController::calculateDirectionData(const ReconnaissanceDevice& device, 
                                                                        const RadiationSource& source) {
    // 使用坐标转换函数计算设备和辐射源的空间直角坐标
    COORD3 deviceXYZ = lbh2xyz(device.getLongitude(), device.getLatitude(), device.getAltitude());
    COORD3 sourceXYZ = lbh2xyz(source.getLongitude(), source.getLatitude(), source.getAltitude());
    
    // 方向向量
    double dirX = sourceXYZ.p1 - deviceXYZ.p1;
    double dirY = sourceXYZ.p2 - deviceXYZ.p2;
    double dirZ = sourceXYZ.p3 - deviceXYZ.p3;
    
    // 计算方位角（相对于正北方向的水平角度）
    double azimuth = atan2(dirX, dirY) * RAD2DEG;
    // 调整到0-360度范围
    if (azimuth < 0) azimuth += 360.0;
    
    // 计算俯仰角（相对于水平面的仰角）
    double horizontalDist = sqrt(dirX*dirX + dirY*dirY);
    double elevation = atan2(dirZ, horizontalDist) * RAD2DEG;
    
    return std::make_pair(azimuth, elevation);
}

// 计算定位数据 - 干涉仪体制
std::pair<std::pair<double, double>, double> SinglePlatformController::calculateLocationData(
                                                                                        const ReconnaissanceDevice& device,
                                                                                        double azimuth,
                                                                                        double elevation) {
    // 将角度转换为弧度
    double azimuthRad = azimuth * DEG2RAD;
    double elevationRad = elevation * DEG2RAD;
    
    // 获取设备的空间直角坐标
    COORD3 deviceXYZ = lbh2xyz(device.getLongitude(), device.getLatitude(), device.getAltitude());
    
    // 获取辐射源位置（固定）
    // 这里我们使用runInterferometerSimulation中计算的位置
    // 由于函数结构限制，这里只是返回一个估计值，实际定位在runInterferometerSimulation中完成
    
    // 估计辐射源距离
    double estimatedDistance = 10000.0;
    
    // 计算辐射源的3D位置（相对于设备）
    double dx = estimatedDistance * cos(elevationRad) * sin(azimuthRad);
    double dy = estimatedDistance * cos(elevationRad) * cos(azimuthRad);
    double dz = estimatedDistance * sin(elevationRad);
    
    // 辐射源的笛卡尔坐标
    double sourceX = deviceXYZ.p1 + dx;
    double sourceY = deviceXYZ.p2 + dy;
    double sourceZ = deviceXYZ.p3 + dz;
    
    // 将辐射源笛卡尔坐标转换回经纬度高度
    COORD3 sourceLBH = xyz2lbh(sourceX, sourceY, sourceZ);
    
    // 返回经度、纬度和高度
    return std::make_pair(std::make_pair(sourceLBH.p1, sourceLBH.p2), sourceLBH.p3);
}

// 计算误差因素 - 干涉仪体制
std::vector<double> SinglePlatformController::calculateInterferometerErrors(const ReconnaissanceDevice& device,
                                                                          const RadiationSource& source,
                                                                          double distance) {
    std::vector<double> errors;
    
    // 1. 对中误差（与基线长度和频率相关）
    double alignmentError = 0.001 * device.getBaselineLength() / 10.0;
    errors.push_back(alignmentError);
    
    // 2. 姿态测量误差（与设备移动速度相关）
    double attitudeError = 0.002 * device.getMovementSpeed() / 10.0;
    errors.push_back(attitudeError);
    
    // 3. 圆锥效应误差（与俯仰角相关）
    double coneEffectError = 0.003 * fabs(sin(source.getElevationStart() * DEG2RAD));
    errors.push_back(coneEffectError);
    
    // 4. 天线阵测向误差（与基线长度、波长和频率相关）
    // 波长 = 光速 / 频率
    double wavelength = c / (source.getCarrierFrequency() * 1e9); // 转换为米
    double antennaArrayError = 0.004 * (wavelength / device.getBaselineLength()) * (source.getCarrierFrequency() / 5.0);
    errors.push_back(antennaArrayError);
    
    // 5. 总测向误差（各误差项的平方和的平方根）
    double totalDirectionError = sqrt(
        alignmentError*alignmentError + 
        attitudeError*attitudeError + 
        coneEffectError*coneEffectError + 
        antennaArrayError*antennaArrayError
    );
    errors.push_back(totalDirectionError);
    
    return errors;
}

// 模拟设备移动
void SinglePlatformController::simulateDeviceMovement(ReconnaissanceDevice& device, const RadiationSource& source, int simulationTime) {
    if (!m_view) {
        g_print("错误：无法获取视图，无法显示设备移动\n");
        return;
    }
    
    MapView* mapView = m_view->getMapView();
    if (!mapView) {
        g_print("错误：无法获取地图视图，无法显示设备移动\n");
        return;
    }
    
    // 获取设备初始位置
    double initialLongitude = device.getLongitude();
    double initialLatitude = device.getLatitude();
    double initialAltitude = device.getAltitude();
    
    // 获取设备移动参数
    double speed = device.getMovementSpeed(); // 米/秒
    double azimuth = device.getMovementAzimuth(); // 度
    double elevation = device.getMovementElevation(); // 度
    
    g_print("设备初始位置: 经度=%.6f, 纬度=%.6f, 高度=%.2f\n", 
            initialLongitude, initialLatitude, initialAltitude);
    g_print("设备移动参数: 速度=%.2f米/秒, 方位角=%.2f度, 俯仰角=%.2f度\n", 
            speed, azimuth, elevation);
    
    // 计算每一帧的时间间隔（秒）
    // 每秒更新一次，时间间隔为1秒
    const double timeStep = 1.0; // 每秒一个轨迹点
    const int numPoints = static_cast<int>(simulationTime / timeStep) + 1;
    
    // 创建轨迹点数组
    std::vector<std::pair<double, double>> trajectoryPoints;
    trajectoryPoints.reserve(numPoints); // 预分配内存以提高性能
    
    // 添加初始位置
    trajectoryPoints.push_back(std::make_pair(initialLongitude, initialLatitude));
    
    // 方位角转换为弧度
    double azimuthRad = azimuth * DEG2RAD;
    double elevationRad = elevation * DEG2RAD;
    
    // 计算水平面速度分量
    double horizontalSpeed = speed * cos(elevationRad);
    
    // 计算东向和北向速度分量
    double eastSpeed = horizontalSpeed * sin(azimuthRad);  // 东向速度
    double northSpeed = horizontalSpeed * cos(azimuthRad); // 北向速度
    double upSpeed = speed * sin(elevationRad);           // 上向速度
    
    // 计算每一时间步长的位置
    for (int i = 1; i < numPoints; i++) {
        double t = i * timeStep;
        
        // 计算经度变化（基于东向速度）
        // 经度变化 = 东向距离 / (地球半径 * cos(纬度)) * 弧度到度的转换
        double longitudeChange = (eastSpeed * t) / (Constants::EARTH_RADIUS * cos(initialLatitude * DEG2RAD)) * RAD2DEG;
        
        // 计算纬度变化（基于北向速度）
        // 纬度变化 = 北向距离 / 地球半径 * 弧度到度的转换
        double latitudeChange = (northSpeed * t) / Constants::EARTH_RADIUS * RAD2DEG;
        
        // 计算高度变化
        double altitudeChange = upSpeed * t;
        
        // 计算新的经纬度和高度
        double newLongitude = initialLongitude + longitudeChange;
        double newLatitude = initialLatitude + latitudeChange;
        double newAltitude = initialAltitude + altitudeChange;
        
        // 添加到轨迹点数组
        trajectoryPoints.push_back(std::make_pair(newLongitude, newLatitude));
        
        // 更新设备位置（最后一个点作为最终位置）
        if (i == numPoints - 1) {
            device.setLongitude(newLongitude);
            device.setLatitude(newLatitude);
            device.setAltitude(newAltitude);
            
            g_print("设备最终位置: 经度=%.6f, 纬度=%.6f, 高度=%.2f\n", 
                    newLongitude, newLatitude, newAltitude);
        }
    }
    
    // 在地图上显示设备移动轨迹
    m_view->animateDeviceMovement(device, trajectoryPoints, simulationTime);
    
    g_print("设备移动轨迹计算完成，共%zu个点\n", trajectoryPoints.size());
} 