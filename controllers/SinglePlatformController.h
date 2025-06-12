#pragma once

#include "../views/SinglePlatformView.h"
#include "../models/RadiationSourceModel.h"
#include "../models/ReconnaissanceDeviceModel.h"
#include <string>
#include <vector>
#include <utility> // 添加pair支持

// 定位结果结构体
struct LocationResult {
    double azimuth;        // 方位角（度）
    double elevation;      // 俯仰角（度）
    double longitude;      // 经度（度）
    double latitude;       // 纬度（度）
    double altitude;       // 高度（米）
    double accuracy;       // 精度（米）
    std::vector<double> errorFactors; // 各种误差因素
};

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
    
    // 模拟设备移动
    void simulateDeviceMovement(ReconnaissanceDevice& device, const RadiationSource& source, int simulationTime);
    
    // 干涉仪体制定位算法
    LocationResult runInterferometerSimulation(const ReconnaissanceDevice& device, 
                                             const RadiationSource& source,
                                             int simulationTime);
    
    // 时差体制定位算法
    LocationResult runTDOASimulation(const ReconnaissanceDevice& device, 
                                   const RadiationSource& source,
                                   int simulationTime);
    
    // 计算测向数据 - 干涉仪体制
    std::pair<double, double> calculateDirectionData(const ReconnaissanceDevice& device, 
                                                   const RadiationSource& source);
    
    // 计算定位数据 - 干涉仪体制
    std::pair<std::pair<double, double>, double> calculateLocationData(const ReconnaissanceDevice& device,
                                                                     double azimuth,
                                                                     double elevation);
    
    // 计算误差因素 - 干涉仪体制
    std::vector<double> calculateInterferometerErrors(const ReconnaissanceDevice& device,
                                                   const RadiationSource& source,
                                                   double distance);
    
    SinglePlatformView* m_view;
}; 