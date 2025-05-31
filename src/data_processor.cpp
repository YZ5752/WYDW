#include "../include/data_processor.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

// 单例实现
DataProcessor& DataProcessor::getInstance() {
    static DataProcessor instance;
    return instance;
}

DataProcessor::DataProcessor() {
}

DataProcessor::~DataProcessor() {
}

std::vector<LocationResult> DataProcessor::loadIntelligenceData() {
    std::cout << "Loading intelligence data from database..." << std::endl;
    // 简化实现
    return std::vector<LocationResult>();
}

bool DataProcessor::loadReferenceData(const std::string& filename) {
    std::cout << "Loading reference data from file: " << filename << std::endl;
    
    // 简化实现
    // 添加一些示例基准点
    m_referencePoints["Point1"] = {100.0, 100.0, 0.0};
    m_referencePoints["Point2"] = {200.0, 200.0, 0.0};
    m_referencePoints["Point3"] = {300.0, 300.0, 0.0};
    
    return true;
}

void DataProcessor::addReferencePoint(const std::string& name, const Coordinate& position) {
    std::cout << "Adding reference point: " << name << std::endl;
    m_referencePoints[name] = position;
}

TargetIntelligence DataProcessor::selectData(const std::vector<LocationResult>& data, 
                                         const Coordinate& referencePosition) {
    std::cout << "Selecting data for target..." << std::endl;
    
    // 简化实现
    TargetIntelligence target;
    target.id = 1;
    target.name = "Target1";
    target.locations = data;
    target.referencePosition = referencePosition;
    target.isSelected = true;
    target.isHighlighted = false;
    
    return target;
}

void DataProcessor::organizeData(TargetIntelligence& target) {
    std::cout << "Organizing data for target: " << target.name << std::endl;
    
    // 简化实现
    // 按时间排序定位结果
    std::sort(target.locations.begin(), target.locations.end(), 
              [](const LocationResult& a, const LocationResult& b) {
                  return a.time < b.time;
              });
}

void DataProcessor::deleteDataPoint(TargetIntelligence& target, int index) {
    std::cout << "Deleting data point at index " << index << " for target: " << target.name << std::endl;
    
    if (index >= 0 && index < static_cast<int>(target.locations.size())) {
        target.locations.erase(target.locations.begin() + index);
    }
}

void DataProcessor::highlightDataPoint(TargetIntelligence& target, int index) {
    std::cout << "Highlighting data point at index " << index << " for target: " << target.name << std::endl;
    
    // 简化实现
    target.isHighlighted = true;
}

void DataProcessor::removeAnomalies(TargetIntelligence& target, double threshold) {
    std::cout << "Removing anomalies for target: " << target.name << " with threshold: " << threshold << std::endl;
    
    // 简化实现
    // 使用距离阈值移除异常点
    auto it = std::remove_if(target.locations.begin(), target.locations.end(),
                            [&](const LocationResult& result) {
                                double dx = result.position.x - target.referencePosition.x;
                                double dy = result.position.y - target.referencePosition.y;
                                double dz = result.position.z - target.referencePosition.z;
                                double distance = std::sqrt(dx*dx + dy*dy + dz*dz);
                                return distance > threshold;
                            });
    
    target.locations.erase(it, target.locations.end());
}

std::pair<double, double> DataProcessor::calculateSinglePlatformMetrics(const TargetIntelligence& target) {
    std::cout << "Calculating single platform metrics for target: " << target.name << std::endl;
    
    // 简化实现
    // 返回最大定位距离和平均定位时间
    double maxDistance = 0.0;
    double totalTime = 0.0;
    
    for (const auto& location : target.locations) {
        double dx = location.position.x - target.referencePosition.x;
        double dy = location.position.y - target.referencePosition.y;
        double dz = location.position.z - target.referencePosition.z;
        double distance = std::sqrt(dx*dx + dy*dy + dz*dz);
        
        maxDistance = std::max(maxDistance, distance);
        totalTime += location.time;
    }
    
    double avgTime = target.locations.empty() ? 0.0 : totalTime / target.locations.size();
    
    return std::make_pair(maxDistance, avgTime);
}

std::pair<double, double> DataProcessor::calculateMultiPlatformMetrics(const std::vector<TargetIntelligence>& targets) {
    std::cout << "Calculating multi-platform metrics for " << targets.size() << " targets" << std::endl;
    
    // 简化实现
    // 返回多平台的最大定位距离和平均定位时间
    double maxDistance = 0.0;
    double totalTime = 0.0;
    int totalLocations = 0;
    
    for (const auto& target : targets) {
        auto metrics = calculateSinglePlatformMetrics(target);
        maxDistance = std::max(maxDistance, metrics.first);
        totalTime += metrics.second * target.locations.size();
        totalLocations += target.locations.size();
    }
    
    double avgTime = totalLocations > 0 ? totalTime / totalLocations : 0.0;
    
    return std::make_pair(maxDistance, avgTime);
}

std::pair<double, double> DataProcessor::calculateSinglePlatformAccuracy(const TargetIntelligence& target) {
    std::cout << "Calculating single platform accuracy for target: " << target.name << std::endl;
    
    // 简化实现
    // 返回定位精度和测向精度
    double positionError = 0.0;
    double directionError = 0.0;
    
    for (const auto& location : target.locations) {
        double dx = location.position.x - target.referencePosition.x;
        double dy = location.position.y - target.referencePosition.y;
        double dz = location.position.z - target.referencePosition.z;
        double distance = std::sqrt(dx*dx + dy*dy + dz*dz);
        
        positionError += distance;
        directionError += location.directionError;
    }
    
    double avgPositionError = target.locations.empty() ? 0.0 : positionError / target.locations.size();
    double avgDirectionError = target.locations.empty() ? 0.0 : directionError / target.locations.size();
    
    return std::make_pair(avgPositionError, avgDirectionError);
}

double DataProcessor::calculateMultiPlatformAccuracy(const std::vector<TargetIntelligence>& targets) {
    std::cout << "Calculating multi-platform accuracy for " << targets.size() << " targets" << std::endl;
    
    // 简化实现
    // 返回多平台定位精度
    double totalError = 0.0;
    int totalLocations = 0;
    
    for (const auto& target : targets) {
        auto accuracy = calculateSinglePlatformAccuracy(target);
        totalError += accuracy.first * target.locations.size();
        totalLocations += target.locations.size();
    }
    
    double avgError = totalLocations > 0 ? totalError / totalLocations : 0.0;
    
    return avgError;
}

bool DataProcessor::saveResultsToFile(const std::string& filename, 
                                  const std::vector<std::pair<std::string, double>>& results) {
    std::cout << "Saving results to file: " << filename << std::endl;
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }
    
    file << "Name,Value\n";
    for (const auto& result : results) {
        file << result.first << "," << result.second << "\n";
    }
    
    file.close();
    return true;
}

std::map<double, double> DataProcessor::getAccuracyOverTime(const TargetIntelligence& target) {
    std::cout << "Getting accuracy over time for target: " << target.name << std::endl;
    
    // 简化实现
    std::map<double, double> accuracyOverTime;
    
    for (const auto& location : target.locations) {
        double dx = location.position.x - target.referencePosition.x;
        double dy = location.position.y - target.referencePosition.y;
        double dz = location.position.z - target.referencePosition.z;
        double distance = std::sqrt(dx*dx + dy*dy + dz*dz);
        
        accuracyOverTime[location.time] = distance;
    }
    
    return accuracyOverTime;
} 