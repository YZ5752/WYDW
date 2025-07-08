#pragma once

#include "../views/MultiPlatformView.h"
#include "../models/MultiPlatformTaskDAO.h"
#include "../models/FDOAalgorithm.h"
#include "../utils/CoordinateTransform.h"
#include "../models/RadiationSourceModel.h"
#include "../models/ReconnaissanceDeviceModel.h"
#include "../models/TDOAalgorithm.h"
#include "../models/DirectionFinding.h"
#include "../utils/ErrorCircleDisplay.h"
#include "../utils/ErrorCircle.h"
#include "../utils/HyperbolaLines.h"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <ctime>

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
    
    // 设置TDOA误差参数
    void setTDOAErrorParams(double tdoaRmsError, double esmToaError);

private:
    MultiPlatformController();
    ~MultiPlatformController();
    
    // 禁止拷贝
    MultiPlatformController(const MultiPlatformController&) = delete;
    MultiPlatformController& operator=(const MultiPlatformController&) = delete;
    
    // 计算两点之间的距离
    double calculateDistance(const COORD3& p1, const COORD3& p2);
    
    MultiPlatformView* m_view;
    
    // TDOA误差参数
    double m_tdoaRmsError;
    double m_esmToaError;
}; 