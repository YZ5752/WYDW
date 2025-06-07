#pragma once

#include "reconnaissance_device_model.h"
#include "radiation_source_model.h"
#include <vector>
#include <string>
#include <map>

// 坐标点
struct Coordinate {
    double x;
    double y;
    double z;
};

// 定位结果
struct LocationResult {
    Coordinate position;      // 定位坐标
    double power;            // 威力
    double directionError;   // 测向误差
    double parameterError;   // 参数测量误差
    double time;             // 定位时间
};

// 单平台雷达侦察仿真
class SinglePlatformSimulation {
public:
    SinglePlatformSimulation();
    ~SinglePlatformSimulation();

    // 设置雷达设备
    void setRadarDevice(const ReconnaissanceDevice& device);
    
    // 设置辐射源
    void setRadiationSource(const RadiationSource& source);
    
    // 设置平台位置
    void setPlatformPosition(const Coordinate& position);
    
    // 执行仿真
    LocationResult runSimulation();
    
    // 计算定位结果
    LocationResult calculateLocation();

private:
    ReconnaissanceDevice m_device;
    RadiationSource m_source;
    Coordinate m_platformPosition;
    std::string m_algorithm;  // 定位算法
};

// 多平台协同侦察仿真
class MultiPlatformSimulation {
public:
    MultiPlatformSimulation();
    ~MultiPlatformSimulation();

    // 添加雷达平台
    void addPlatform(const Coordinate& position, const ReconnaissanceDevice& device);
    
    // 设置辐射源
    void setRadiationSource(const RadiationSource& source);
    
    // 设置算法 (时差定位/频差定位/测向定位)
    void setAlgorithm(const std::string& algorithm);
    
    // 执行仿真
    LocationResult runSimulation();
    
    // 计算协同定位结果
    LocationResult calculateCooperativeLocation();

private:
    std::vector<std::pair<Coordinate, ReconnaissanceDevice>> m_platforms;
    RadiationSource m_source;
    std::string m_algorithm;  // 定位算法
};

// 仿真评估模式
enum class SimulationMode {
    PRE_MISSION,     // 任务前仿真评估
    REAL_TIME,       // 实时仿真评估
    POST_MISSION     // 任务后仿真评估
};

// 仿真管理器
class SimulationManager {
public:
    static SimulationManager& getInstance();
    
    // 设置仿真模式
    void setSimulationMode(SimulationMode mode);
    
    // 获取单平台仿真对象
    SinglePlatformSimulation& getSinglePlatformSimulation();
    
    // 获取多平台仿真对象
    MultiPlatformSimulation& getMultiPlatformSimulation();
    
    // 保存仿真结果到数据库
    bool saveResults(const LocationResult& result);
    
    // 从数据库加载仿真结果
    std::vector<LocationResult> loadResults();

private:
    SimulationManager();
    ~SimulationManager();
    
    SimulationMode m_mode;
    SinglePlatformSimulation m_singleSim;
    MultiPlatformSimulation m_multiSim;
}; 