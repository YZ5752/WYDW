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
    
    // TODO: 根据选择的设备和辐射源名称获取对应的模型对象
    // ReconnaissanceDevice device = ...
    // RadiationSource source = ...
    
    // 设置仿真参数
    // m_simulation.setRadarDevice(device);
    // m_simulation.setRadiationSource(source);
    
    // 执行仿真
    LocationResult result = m_simulation.runSimulation();
    
    // 更新视图显示结果
    // m_view->updateDirectionData(...);
    // m_view->updateLocationData(...);
}

// 加载模型数据
void SinglePlatformController::loadModelData() {
    if (!m_view) return;
    
    // TODO: 从DAO加载设备和辐射源数据
    // std::vector<ReconnaissanceDevice> devices = ReconnaissanceDeviceDAO::getInstance().getAllReconnaissanceDevices();
    // std::vector<RadiationSource> sources = RadiationSourceDAO::getInstance().getAllRadiationSources();
    
    // 更新视图中的设备和辐射源列表
    // ...
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