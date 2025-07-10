#include "../EvaluationController.h"
#include "../../views/EvaluationView.h"  // 在实现文件中包含完整的头文件
#include <fstream>
#include <iostream>
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
    loadEvaluationData();
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
        results.push_back({"定位精度", count ? positioningAccuracySum / count : 0.0});
        results.push_back({"测向精度", count ? directionFindingAccuracySum / count : 0.0});
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
        results.push_back({"定位精度", count ? positioningAccuracySum / count : 0.0});
        // 多平台不添加"测向精度"
    }
    m_results = results;
    return results;
}

// 获取性能随时间变化的数据
std::map<double, double> EvaluationController::getAccuracyTimeData(int sourceId, bool isSinglePlatform) {
    std::map<double, double> timeData;
    
    if (isSinglePlatform) {
        // 获取单平台任务数据
        std::vector<SinglePlatformTask> tasks = SinglePlatformTaskDAO::getInstance().getTasksBySourceId(sourceId);
        
        if (tasks.empty()) {
            g_print("EvaluationController: No tasks found for radiation source ID %d\n", sourceId);
            return timeData;
        }
        
        // 对任务按执行时间排序
        std::sort(tasks.begin(), tasks.end(), [](const SinglePlatformTask& a, const SinglePlatformTask& b) {
            return a.executionTime < b.executionTime;
        });
        
        // 创建时间-精度映射
        for (const auto& task : tasks) {
            timeData[task.executionTime] = task.positioningAccuracy;
        }
    } else {
        // 多平台：获取多平台任务数据
        std::vector<MultiPlatformTask> tasks = MultiPlatformTaskDAO::getInstance().getMultiPlatformTasksByRadiationId(sourceId);

        if (tasks.empty()) {
            g_print("EvaluationController: No multi-platform tasks found for radiation source ID %d\n", sourceId);
            return timeData;
        }

        // 假设 MultiPlatformTask 也有 executionTime 和 positioningAccuracy 字段
        std::sort(tasks.begin(), tasks.end(), [](const MultiPlatformTask& a, const MultiPlatformTask& b) {
            return a.executionTime < b.executionTime;
        });

        for (const auto& task : tasks) {
            timeData[task.executionTime] = task.positioningAccuracy;
        }
    }
    
    return timeData;
}

// 添加空实现，防止链接错误
void EvaluationController::loadEvaluationData() {
    // TODO: 实现数据加载逻辑
} 