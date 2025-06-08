#pragma once

#include "LocationResultModel.h"
#include "CoordinateModel.h"
#include <vector>
#include <map>
#include <string>
#include <utility> // 添加对std::pair的支持

// 目标情报数据结构
struct ProcessedTargetIntelligence {
    int id;
    std::string name;
    std::vector<LocationResult> locations;
    Coordinate referencePosition;
    bool isSelected;
    bool isHighlighted;
};

// 数据处理结果
struct ProcessedData {
    std::vector<double> directionData;  // 方向数据
    std::vector<double> frequencyData;  // 频率数据
    std::vector<double> powerData;      // 功率数据
    std::vector<double> timeData;       // 时间数据
};

// 数据处理器
class DataProcessor {
public:
    static DataProcessor& getInstance();
    
    // 预处理辐射源数据
    ProcessedData preprocessData(const std::vector<double>& rawData);
    
    // 过滤噪声
    std::vector<double> filterNoise(const std::vector<double>& data, double threshold);
    
    // 测向算法
    double directionFinding(const std::vector<double>& data);
    
    // 信号检测
    bool detectSignal(const std::vector<double>& data, double threshold);
    
    // 参数估计
    std::map<std::string, double> estimateParameters(const std::vector<double>& data);
    
    // 定位计算
    Coordinate calculateLocation(const std::vector<double>& directions, const std::vector<Coordinate>& positions);
    
    // 误差分析
    std::map<std::string, double> analyzeErrors(const std::vector<LocationResult>& results, const Coordinate& reference);
    
    // 定位精度评估
    double evaluateAccuracy(const LocationResult& result, const Coordinate& reference);
    
    // 生成评估报告
    std::string generateReport(const std::map<std::string, double>& metrics);
    
    // 从DataProcessorModel.cpp中添加的方法
    std::vector<LocationResult> loadIntelligenceData();
    bool loadReferenceData(const std::string& filename);
    void addReferencePoint(const std::string& name, const Coordinate& position);
    ProcessedTargetIntelligence selectData(const std::vector<LocationResult>& data, const Coordinate& referencePosition);
    void organizeData(ProcessedTargetIntelligence& target);
    void deleteDataPoint(ProcessedTargetIntelligence& target, int index);
    void highlightDataPoint(ProcessedTargetIntelligence& target, int index);
    void removeAnomalies(ProcessedTargetIntelligence& target, double threshold);
    std::pair<double, double> calculateSinglePlatformMetrics(const ProcessedTargetIntelligence& target);
    std::pair<double, double> calculateMultiPlatformMetrics(const std::vector<ProcessedTargetIntelligence>& targets);
    std::pair<double, double> calculateSinglePlatformAccuracy(const ProcessedTargetIntelligence& target);
    double calculateMultiPlatformAccuracy(const std::vector<ProcessedTargetIntelligence>& targets);
    bool saveResultsToFile(const std::string& filename, const std::vector<std::pair<std::string, double>>& results);
    std::map<double, double> getAccuracyOverTime(const ProcessedTargetIntelligence& target);

private:
    DataProcessor();
    ~DataProcessor();
    
    // 禁止拷贝
    DataProcessor(const DataProcessor&) = delete;
    DataProcessor& operator=(const DataProcessor&) = delete;
    
    // 添加成员变量
    std::map<std::string, Coordinate> m_referencePoints;
}; 