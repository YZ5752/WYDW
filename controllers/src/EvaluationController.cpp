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
    // 生成模拟数据：初始阶段精度大，中期逐渐减小，后期趋于稳定
    // 时间单位：秒，精度单位：米
    // 例如：0~100s，初始30米，逐步减小到5米，最后趋于3米
    const int totalPoints = 30;
    double startTime = 0.0;
    double endTime = 100.0;
    double stableAccuracy = 3.0;
    double initialAccuracy = 30.0;
    double midAccuracy = 5.0;
    for (int i = 0; i < totalPoints; ++i) {
        double t = startTime + (endTime - startTime) * i / (totalPoints - 1);
        double accuracy;
        if (t < 20) {
            // 初始阶段，精度较大
            accuracy = initialAccuracy - (initialAccuracy - midAccuracy) * (t / 20.0) * 0.7; // 逐步下降
        } else if (t < 60) {
            // 中期，精度快速减小
            accuracy = midAccuracy - (midAccuracy - stableAccuracy) * ((t - 20) / 40.0) * 0.8;
        } else {
            // 稳定阶段，趋于稳定值
            accuracy = stableAccuracy + 0.2 * ::sin(t / 10.0); // 稳定值附近小幅波动
        }
        timeData[t] = accuracy;
    }
    return timeData;
}