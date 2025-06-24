#pragma once

#include "../views/MultiPlatformView.h"
#include "../models/RadiationSourceModel.h"
#include "../models/ReconnaissanceDeviceModel.h"
#include "../models/FDOAalgorithm.h"
#include "../models/TDOAalgorithm.h"
#include <string>
#include <vector>

class ApplicationController;  // 前向声明

class MultiPlatformController {
public:
    friend class ApplicationController;  // 声明ApplicationController为友元类
    
    static MultiPlatformController& getInstance();
    
    // 初始化控制器
    void init(MultiPlatformView* view);
    
    
    // 获取视图
    MultiPlatformView* getView() const;

    // 开始仿真
    void startSimulation(const std::vector<std::string>& deviceNames,
                        const std::string& sourceName,
                        const std::string& systemType,
                        double simulationTime);

private:
    MultiPlatformController();
    ~MultiPlatformController();
    
    // 禁止拷贝
    MultiPlatformController(const MultiPlatformController&) = delete;
    MultiPlatformController& operator=(const MultiPlatformController&) = delete;
    
    MultiPlatformView* m_view;
}; 