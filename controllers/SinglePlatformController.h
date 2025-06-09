#pragma once

#include "../views/SinglePlatformView.h"
#include "../models/RadiationSourceModel.h"
#include "../models/ReconnaissanceDeviceModel.h"
#include <string>

class ApplicationController;  // 前向声明

class SinglePlatformController {
public:
    friend class ApplicationController;  // 声明ApplicationController为友元类
    
    static SinglePlatformController& getInstance();
    
    // 初始化控制器
    void init(SinglePlatformView* view);
    
    // 启动仿真
    void startSimulation();
    
    // 加载模型数据
    void loadModelData();
    
    // 技术体制变化处理
    void handleTechSystemChange(const std::string& techSystem);
    
    // 获取视图
    SinglePlatformView* getView() const;

private:
    SinglePlatformController();
    ~SinglePlatformController();
    
    // 禁止拷贝
    SinglePlatformController(const SinglePlatformController&) = delete;
    SinglePlatformController& operator=(const SinglePlatformController&) = delete;
    
    SinglePlatformView* m_view;
}; 