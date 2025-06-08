#pragma once

#include "../views/MultiPlatformView.h"
#include "../models/SimulationModel.h"
#include "../models/RadiationSourceModel.h"
#include "../models/ReconnaissanceDeviceModel.h"
#include <string>
#include <vector>

class ApplicationController;  // 前向声明

class MultiPlatformController {
public:
    friend class ApplicationController;  // 声明ApplicationController为友元类
    
    static MultiPlatformController& getInstance();
    
    // 初始化控制器
    void init(MultiPlatformView* view);
    
    // 启动仿真
    void startSimulation();
    
    // 加载模型数据
    void loadModelData();
    
    // 添加平台
    void addPlatform(const ReconnaissanceDevice& device, const Coordinate& position);
    
    // 删除平台
    void removePlatform(int index);
    
    // 算法变化处理
    void handleAlgorithmChange(const std::string& algorithm);
    
    // 获取视图
    MultiPlatformView* getView() const;

private:
    MultiPlatformController();
    ~MultiPlatformController();
    
    // 禁止拷贝
    MultiPlatformController(const MultiPlatformController&) = delete;
    MultiPlatformController& operator=(const MultiPlatformController&) = delete;
    
    MultiPlatformView* m_view;
    MultiPlatformSimulation m_simulation;
}; 