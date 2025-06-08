#include "../MultiPlatformController.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/RadiationSourceDAO.h"
#include <iostream>

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
    loadModelData();
}

// 启动仿真
void MultiPlatformController::startSimulation() {
    if (!m_view) return;
    
    // 获取视图中选择的参数
    std::string algorithm = m_view->getSelectedAlgorithm();
    std::vector<std::pair<std::string, Coordinate>> platforms = m_view->getPlatformList();
    
    // 设置算法
    m_simulation.setAlgorithm(algorithm);
    
    // TODO: 根据平台列表添加平台
    // for (const auto& platform : platforms) {
    //     ReconnaissanceDevice device = ...;  // 根据名称获取设备
    //     m_simulation.addPlatform(platform.second, device);
    // }
    
    // TODO: 设置辐射源
    // RadiationSource source = ...;
    // m_simulation.setRadiationSource(source);
    
    // 执行仿真
    LocationResult result = m_simulation.runSimulation();
    
    // 更新视图显示结果
    // m_view->updateLocationResult(result.position, result.directionError);
    
    // TODO: 计算并显示精度图表
    // std::vector<double> errors = ...;
    // m_view->showAccuracyChart(errors);
}

// 加载模型数据
void MultiPlatformController::loadModelData() {
    if (!m_view) return;
    
    // TODO: 从DAO加载设备数据
    // std::vector<ReconnaissanceDevice> devices = ReconnaissanceDeviceDAO::getInstance().getAllReconnaissanceDevices();
    
    // 更新视图中的设备列表
    // ...
}

// 添加平台
void MultiPlatformController::addPlatform(const ReconnaissanceDevice& device, const Coordinate& position) {
    if (!m_view) return;
    
    // 添加平台到视图列表
    m_view->addPlatformToList(device.getDeviceName(), position);
}

// 删除平台
void MultiPlatformController::removePlatform(int index) {
    if (!m_view) return;
    
    // 从视图列表中删除平台
    m_view->removePlatformFromList(index);
}

// 算法变化处理
void MultiPlatformController::handleAlgorithmChange(const std::string& algorithm) {
    // 根据算法变化更新UI或模型
    // ...
}

// 获取视图
MultiPlatformView* MultiPlatformController::getView() const {
    return m_view;
} 