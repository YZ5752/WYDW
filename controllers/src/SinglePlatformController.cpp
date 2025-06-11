#include "../SinglePlatformController.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/RadiationSourceDAO.h"
#include <iostream>

// 单例实现
SinglePlatformController& SinglePlatformController::getInstance() {
    static SinglePlatformController instance;
    return instance;
}

// 构造函数
SinglePlatformController::SinglePlatformController() : m_view(nullptr) {
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
    if (!m_view) return;
    
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
        g_print("错误：未找到侦察设备 '%s'\n", deviceName.c_str());
        return;
    }
    
    // 验证侦察设备必须是移动设备
    if (device.getIsStationary()) {
        g_print("错误：单平台仿真要求侦察设备必须是移动设备\n");
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
        g_print("错误：未找到辐射源 '%s'\n", sourceName.c_str());
        return;
    }
    
    // 验证辐射源必须是固定的
    if (!source.getIsStationary()) {
        g_print("错误：单平台仿真要求辐射源必须是固定的\n");
        return;
    }
    
    g_print("使用侦察设备 ID: %d, 名称: %s\n", device.getDeviceId(), device.getDeviceName().c_str());
    g_print("使用辐射源 ID: %d, 名称: %s\n", source.getRadiationId(), source.getRadiationName().c_str());
    
    // 设置仿真参数
    // m_simulation.setRadarDevice(device);
    // m_simulation.setRadiationSource(source);
    
    // 执行仿真
    // LocationResult result = m_simulation.runSimulation();
    
    // 更新视图显示结果
    // 这里使用模拟数据
    std::string directionData = "方位角: 45.2°, 俯仰角: 12.8°";
    std::string locationData = "经度: " + std::to_string(source.getLongitude()) + 
                              "°, 纬度: " + std::to_string(source.getLatitude()) + 
                              "°, 高度: " + std::to_string(source.getAltitude()) + "m";
    
    m_view->updateDirectionData(directionData);
    m_view->updateLocationData(locationData);
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