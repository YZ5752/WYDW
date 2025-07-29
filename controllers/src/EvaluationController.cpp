#include "../EvaluationController.h"
#include "../../views/EvaluationView.h"  // 在实现文件中包含完整的头文件
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm> 
#include "../../models/SinglePlatformTaskDAO.h"
#include "../../models/MultiPlatformTaskDAO.h"

// 单例实现
EvaluationController& EvaluationController::getInstance() {
    static EvaluationController instance;
    return instance;
}

// 构造函数
EvaluationController::EvaluationController() : m_view(nullptr) {
}

// 析构函数
EvaluationController::~EvaluationController() {
}

// 初始化控制器
void EvaluationController::init(EvaluationView* view) {
    m_view = view;
}

// 获取视图
EvaluationView* EvaluationController::getView() const {
    return m_view;
}

// 获取所有辐射源
std::vector<RadiationSource> EvaluationController::getAllRadiationSources() {
    // 调用DAO层获取所有辐射源
    return RadiationSourceDAO::getInstance().getAllRadiationSources();
}

// 评估指定的辐射源
std::vector<std::pair<std::string, double>> EvaluationController::evaluateRadiationSource(int sourceId, bool isSinglePlatform) {
    std::vector<std::pair<std::string, double>> results;
    if (isSinglePlatform) {
        // 单平台：查询 single_platform_task
        std::vector<SinglePlatformTask> tasks = SinglePlatformTaskDAO::getInstance().getTasksBySourceId(sourceId);
        double maxDistanceSum = 0.0, positioningTimeSum = 0.0, positioningAccuracySum = 0.0, directionFindingAccuracySum = 0.0;
        int count = tasks.size();
        for (const auto& task : tasks) {
            maxDistanceSum += task.positioningDistance;
            positioningTimeSum += task.positioningTime;
            positioningAccuracySum += task.positioningAccuracy;
            directionFindingAccuracySum += task.directionFindingAccuracy;
        }
        results.push_back({"最远定位距离", count ? maxDistanceSum / count : 0.0});
        results.push_back({"定位时间", count ? positioningTimeSum / count : 0.0});
        results.push_back({"定位误差", count ? positioningAccuracySum / count : 0.0});
        results.push_back({"测向误差", count ? directionFindingAccuracySum / count : 0.0});
    } else {
        // 多平台：查询 multi_platform_task
        std::vector<MultiPlatformTask> tasks = MultiPlatformTaskDAO::getInstance().getMultiPlatformTasksByRadiationId(sourceId);
        double maxDistanceSum = 0.0, positioningTimeSum = 0.0, positioningAccuracySum = 0.0;
        int count = tasks.size();
        for (const auto& task : tasks) {
            maxDistanceSum += task.positioningDistance;
            positioningTimeSum += task.positioningTime;
            positioningAccuracySum += task.positioningAccuracy;
        }
        results.push_back({"最远定位距离", count ? maxDistanceSum / count : 0.0});
        results.push_back({"定位时间", count ? positioningTimeSum / count : 0.0});
        results.push_back({"定位误差", count ? positioningAccuracySum / count : 0.0});
        // 多平台不添加"测向精度"
    }
    m_results = results;
    return results;
}

// 获取性能随时间变化的数据 - 从数据库获取真实数据
std::map<double, double> EvaluationController::getAccuracyTimeData(int sourceId, bool isSinglePlatform) {
    std::map<double, double> timeData;
    
    try {
        if (isSinglePlatform) {
            // 单平台：从 single_platform_task 表获取数据
            std::vector<SinglePlatformTask> tasks = SinglePlatformTaskDAO::getInstance().getSinglePlatformTasksByRadiationId(sourceId);
            
            for (const auto& task : tasks) {
                // 使用定位时间作为X轴，定位精度作为Y轴
                if (task.positioningTime > 0 && task.positioningAccuracy > 0) {
                    timeData[task.positioningTime] = task.positioningAccuracy;
                }
            }
        } else {
            // 多平台：从 multi_platform_task 表获取数据
            std::vector<MultiPlatformTask> tasks = MultiPlatformTaskDAO::getInstance().getMultiPlatformTasksByRadiationId(sourceId);
            
            for (const auto& task : tasks) {
                // 使用定位时间作为X轴，定位精度作为Y轴
                if (task.positioningTime > 0 && task.positioningAccuracy > 0) {
                    timeData[task.positioningTime] = task.positioningAccuracy;
                }
            }
        }
        
        // 如果没有数据，返回空的map
        if (timeData.empty()) {
            std::cout << "警告：未找到辐射源ID " << sourceId << " 的历史数据" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "获取精度时间数据时出错: " << e.what() << std::endl;
    }
    
    return timeData;
}




