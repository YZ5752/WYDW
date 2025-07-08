#pragma once

#include "../models/RadiationSourceDAO.h"
#include "../models/ReconnaissanceDeviceDAO.h"
#include "../utils/SimulationValidator.h"
#include "../utils/CoordinateTransform.h"
#include "../utils/Vector3.h"

#include <vector>
#include <string>
#include <memory>
#include <random>
#include <algorithm>

class TDOAalgorithm {
public:
    // 定位结果结构体
    struct LocationResult {
        double longitude;        
        double latitude;         
        double altitude;         
        double azimuth;          
        double elevation;        
        double velocity;         
        double locationTime;     
        double distance;         
        double accuracy;         
    };

    // 单例模式
    static TDOAalgorithm& getInstance();

    // 初始化算法
    void init(const std::vector<std::string>& deviceNames, 
             const std::string& sourceName,
             const std::string& systemType,
             double simulationTime,
             double tdoaRmsError = 0.0,
             double esmToaError = 0.0);

    // 执行定位算法
    bool calculate();

    // 获取定位结果
    LocationResult getResult() const;
    
    // 设置误差参数
    void setErrorParams(double tdoaRmsError, double esmToaError) {
        m_tdoaRmsError = tdoaRmsError;
        m_esmToaError = esmToaError;
    }

private:
    TDOAalgorithm();
    ~TDOAalgorithm();

    TDOAalgorithm(const TDOAalgorithm&) = delete;
    TDOAalgorithm& operator=(const TDOAalgorithm&) = delete;

    // --- 核心算法私有辅助函数 ---
    // 定位解算函数保持不变
    COORD3 tdoaLocate_chan(const std::vector<COORD3>& stationPositions, const std::vector<double>& tdoas);

    // --- 数据加载辅助函数 ---
    bool loadDeviceInfo();
    bool loadSourceInfo();

    // 成员变量
    std::vector<std::string> m_deviceNames;    
    std::string m_sourceName;                  
    std::string m_systemType;                  
    double m_simulationTime;                   
    std::vector<ReconnaissanceDevice> m_devices;  
    RadiationSource m_source;                  
    LocationResult m_result;
    
    // 误差参数
    double m_tdoaRmsError;  // TDOA均方根误差（秒）
    double m_esmToaError;   // ESM TOA误差（秒）                   
};
