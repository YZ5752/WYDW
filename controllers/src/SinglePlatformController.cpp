#include "../SinglePlatformController.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/RadiationSourceDAO.h"
#include "../../constants/PhysicsConstants.h"
#include "../../utils/CoordinateTransform.h"
#include "../../utils/SimulationValidator.h"
#include "../../models/InterferometerPositioning.h"
#include "../../models/SinglePlatformTDOA.h"
#include "../../models/TrajectorySimulator.h"
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
SinglePlatformController::SinglePlatformController() : m_view(nullptr), m_lastErrorFactors() {
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
    
    // // ====== 仿真前条件验证 ======
    // SimulationValidator validator;
    // std::vector<int> deviceIds = { device.getDeviceId() };
    // int sourceId = source.getRadiationId();
    // std::string failMessage;
    // if (!validator.validateAll(deviceIds, sourceId, failMessage)) {
    //     g_print("仿真前条件验证失败：%s\n", failMessage.c_str());
    //     if (m_view) m_view->showErrorMessage(failMessage);
    //     return;
    // }
    
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
        
        // 不再清除之前的标记
        // mapView->clearMarkers();
        
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
    
    // 执行设备移动仿真 - 使用轨迹仿真器
    ReconnaissanceDevice deviceCopy = device; // 使用副本，避免修改原始数据
    
    // 保存原始设备位置，用于定位计算
    ReconnaissanceDevice originalDevice = device; // 保存原始设备数据，位置不会被修改
    
    std::vector<std::pair<double, double>> trajectoryPoints = 
        TrajectorySimulator::getInstance().simulateDeviceMovement(deviceCopy, simulationTime);
    
    // 在地图上显示设备移动轨迹
    m_view->animateDeviceMovement(deviceCopy, trajectoryPoints, simulationTime);
    
    // 仿真开始前清空参数
    m_view->clearSimulationResult();
    
    // 执行仿真
    LocationResult result;
    
    // 根据选择的技术体制执行不同的算法
    // 使用原始设备位置进行定位计算，而不是移动后的位置
    if (techSystem == "干涉仪体制") {
        g_print("执行干涉仪体制定位算法...\n");
        g_print("使用原始设备位置进行定位计算：经度=%.6f°, 纬度=%.6f°, 高度=%.2fm\n", 
                originalDevice.getLongitude(), originalDevice.getLatitude(), originalDevice.getAltitude());
        result = InterferometerPositioning::getInstance().runSimulation(originalDevice, source, simulationTime);
    } else if (techSystem == "时差体制") {
        g_print("执行时差体制定位算法...\n");
        g_print("使用原始设备位置进行定位计算：经度=%.6f°, 纬度=%.6f°, 高度=%.2fm\n", 
                originalDevice.getLongitude(), originalDevice.getLatitude(), originalDevice.getAltitude());
        result = SinglePlatformTDOA::getInstance().runSimulation(originalDevice, source, simulationTime);
    }
    
    // 先设置仿真结果到缓存，确保animateDeviceMovement可以使用
    m_view->setSimulationResult(result.longitude, result.latitude, result.altitude, result.azimuth, result.elevation);
    g_print("已设置仿真结果到缓存：经度=%.6f°, 纬度=%.6f°, 高度=%.2fm, 方位角=%.2f°, 俯仰角=%.2f°\n",
            result.longitude, result.latitude, result.altitude, result.azimuth, result.elevation);
    
    // 更新视图显示结果文本
    char directionBuffer[100];
    sprintf(directionBuffer, "方位角: %.2f°, 俯仰角: %.2f°", result.azimuth, result.elevation);
    std::string directionData(directionBuffer);
    char locationBuffer[200];
    sprintf(locationBuffer, "经度: %.6f°, 纬度: %.6f°, 高度: %.2fm", 
            result.longitude, result.latitude, result.altitude);
    std::string locationData(locationBuffer);
    m_view->updateDirectionData(directionData);
    m_view->updateLocationData(locationData);
    
    // 保存误差因素以便在仿真结束后显示
    std::vector<double> errorFactors = result.errorFactors;
    std::string currentTechSystem = techSystem;
    
//  // 在动画结束后才显示结果参数和误差分析
//     g_timeout_add(simulationTime * 1000 + 1200, [](gpointer data) -> gboolean {
//         auto* controller = static_cast<SinglePlatformController*>(data);
//         if (!controller || !controller->getView()) {
//             return G_SOURCE_REMOVE;
//         }
        
//         SinglePlatformView* view = controller->getView();
        
//         // 显示仿真结果参数（这部分已在SinglePlatformView::animateDeviceMovement中实现）
        
//         // 更新误差表格
//         std::string techSystem = view->getSelectedTechSystem();
//         GtkWidget* errorTable = view->getErrorTable();
        
//         if (errorTable) {
//             // 获取保存的误差因素数据
//             std::vector<double> errorFactors = controller->getLastErrorFactors();
            
//             if (techSystem == "干涉仪体制" && errorFactors.size() >= 5) {
//                 // 查找已存在的误差值标签
//                 GList* children = gtk_container_get_children(GTK_CONTAINER(errorTable));
//                 GtkWidget* errorLabels[5] = {nullptr}; // 存储找到的标签引用
                
//                 // 找出所有值标签的引用
//                 for (GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
//                     GtkWidget* child = GTK_WIDGET(iter->data);
//                     int row, col;
//                     gtk_container_child_get(GTK_CONTAINER(errorTable), child, 
//                                            "top-attach", &row, 
//                                            "left-attach", &col, NULL);
//                     if (col == 1 && row >= 0 && row < 5) {
//                         errorLabels[row] = child;
//                     }
//                 }
//                 g_list_free(children);
                
//                 // 更新每个标签的文本，直接设置文本而不是添加新标签
//                 for (int i = 0; i < 5; i++) {
//                     if (errorLabels[i]) {
//                         char errorBuffer[50];
//                         sprintf(errorBuffer, "%.4f°", errorFactors[i]);
//                         gtk_label_set_text(GTK_LABEL(errorLabels[i]), errorBuffer);
//                     }
//                 }
//             } else if (techSystem == "时差体制" && errorFactors.size() >= 5) {
//                 // 查找已存在的误差值标签
//                 GList* children = gtk_container_get_children(GTK_CONTAINER(errorTable));
//                 GtkWidget* errorLabels[5] = {nullptr}; // 存储找到的标签引用
                
//                 // 找出所有值标签的引用
//                 for (GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
//                     GtkWidget* child = GTK_WIDGET(iter->data);
//                     int row, col;
//                     gtk_container_child_get(GTK_CONTAINER(errorTable), child, 
//                                            "top-attach", &row, 
//                                            "left-attach", &col, NULL);
//                     if (col == 1 && row >= 0 && row < 5) {
//                         errorLabels[row] = child;
//                     }
//                 }
//                 g_list_free(children);
                
//                 // 更新每个标签的文本，直接设置文本而不是添加新标签
//                 for (int i = 0; i < 5; i++) {
//                     if (errorLabels[i]) {
//                         char errorBuffer[50];
//                         sprintf(errorBuffer, "%.4f°", errorFactors[i]);
//                         gtk_label_set_text(GTK_LABEL(errorLabels[i]), errorBuffer);
//                     }
//                 }
//             }
            
//             // 显示所有控件
//             gtk_widget_show_all(errorTable);
//         }
        
//         return G_SOURCE_REMOVE;
//     }, this);

    // 立即显示结果参数和误差分析，不等待动画结束
    // 显示仿真结果参数
    m_view->showSimulationResult(result.longitude, result.latitude, result.altitude, result.azimuth, result.elevation);
    
    // 更新误差表格
    GtkWidget* errorTable = m_view->getErrorTable();
    
    if (errorTable) {
        if (techSystem == "干涉仪体制" && errorFactors.size() >= 5) {
            // 查找已存在的误差值标签
            GList* children = gtk_container_get_children(GTK_CONTAINER(errorTable));
            GtkWidget* errorLabels[5] = {nullptr}; // 存储找到的标签引用
            
            // 找出所有值标签的引用
            for (GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
                GtkWidget* child = GTK_WIDGET(iter->data);
                int row, col;
                gtk_container_child_get(GTK_CONTAINER(errorTable), child, 
                                       "top-attach", &row, 
                                       "left-attach", &col, NULL);
                if (col == 1 && row >= 0 && row < 5) {
                    errorLabels[row] = child;
                }
            }
            g_list_free(children);
            
            // 更新每个标签的文本，直接设置文本而不是添加新标签
            for (int i = 0; i < 5; i++) {
                if (errorLabels[i]) {
                    char errorBuffer[50];
                    sprintf(errorBuffer, "%.4f°", errorFactors[i]);
                    gtk_label_set_text(GTK_LABEL(errorLabels[i]), errorBuffer);
                }
            }
        } else if (techSystem == "时差体制" && errorFactors.size() >= 5) {
            // 查找已存在的误差值标签
            GList* children = gtk_container_get_children(GTK_CONTAINER(errorTable));
            GtkWidget* errorLabels[5] = {nullptr}; // 存储找到的标签引用
            
            // 找出所有值标签的引用
            for (GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
                GtkWidget* child = GTK_WIDGET(iter->data);
                int row, col;
                gtk_container_child_get(GTK_CONTAINER(errorTable), child, 
                                       "top-attach", &row, 
                                       "left-attach", &col, NULL);
                if (col == 1 && row >= 0 && row < 5) {
                    errorLabels[row] = child;
                }
            }
            g_list_free(children);
            
            // 更新每个标签的文本，直接设置文本而不是添加新标签
            for (int i = 0; i < 5; i++) {
                if (errorLabels[i]) {
                    char errorBuffer[50];
                    sprintf(errorBuffer, "%.4f°", errorFactors[i]);
                    gtk_label_set_text(GTK_LABEL(errorLabels[i]), errorBuffer);
                }
            }
        }
        
        // 显示所有控件
        gtk_widget_show_all(errorTable);
    }
    
    // 保存最后一次仿真的误差因素，供延时函数使用
    m_lastErrorFactors = result.errorFactors;
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

// 获取最后一次仿真的误差因素
const std::vector<double>& SinglePlatformController::getLastErrorFactors() const {
    return m_lastErrorFactors;
} 