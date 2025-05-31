#pragma once

#include "simulation.h"
#include <vector>
#include <string>
#include <map>

// 目标情报数据
struct TargetIntelligence {
    int id;                     // 目标ID
    std::string name;           // 目标名称
    std::vector<LocationResult> locations; // 定位结果
    Coordinate referencePosition; // 基准位置
    bool isSelected;            // 是否被选中
    bool isHighlighted;         // 是否被高亮
};

// 数据处理类
class DataProcessor {
public:
    static DataProcessor& getInstance();
    
    // 从数据库加载情报数据
    std::vector<LocationResult> loadIntelligenceData();
    
    // 从文件加载基准数据
    bool loadReferenceData(const std::string& filename);
    
    // 手动添加基准点
    void addReferencePoint(const std::string& name, const Coordinate& position);
    
    // 分选数据
    TargetIntelligence selectData(const std::vector<LocationResult>& data, 
                               const Coordinate& referencePosition);
    
    // 整编数据
    void organizeData(TargetIntelligence& target);
    
    // 删除数据点
    void deleteDataPoint(TargetIntelligence& target, int index);
    
    // 高亮数据点
    void highlightDataPoint(TargetIntelligence& target, int index);
    
    // 剔除异常点
    void removeAnomalies(TargetIntelligence& target, double threshold);
    
    // 计算单平台定位距离和时间
    std::pair<double, double> calculateSinglePlatformMetrics(const TargetIntelligence& target);
    
    // 计算多平台协同定位距离和时间
    std::pair<double, double> calculateMultiPlatformMetrics(const std::vector<TargetIntelligence>& targets);
    
    // 计算单平台定位精度和测向精度
    std::pair<double, double> calculateSinglePlatformAccuracy(const TargetIntelligence& target);
    
    // 计算多平台协同定位精度
    double calculateMultiPlatformAccuracy(const std::vector<TargetIntelligence>& targets);
    
    // 保存统计结果到文本文件
    bool saveResultsToFile(const std::string& filename, 
                         const std::vector<std::pair<std::string, double>>& results);
    
    // 获取定位精度随时间变化的数据
    std::map<double, double> getAccuracyOverTime(const TargetIntelligence& target);

private:
    DataProcessor();
    ~DataProcessor();
    
    std::vector<TargetIntelligence> m_targets;
    std::map<std::string, Coordinate> m_referencePoints;
}; 