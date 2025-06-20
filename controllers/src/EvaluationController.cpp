#include "../EvaluationController.h"
#include "../../views/EvaluationView.h"  // 在实现文件中包含完整的头文件
#include <fstream>
#include <iostream>
#include <algorithm> 


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

// 开始评估
void EvaluationController::startEvaluation() {
    if (!m_view) return;
    
    // 获取视图中选择的参数
    std::string mode = m_view->getSelectedMode();
    std::vector<std::string> metrics = m_view->getSelectedMetrics();
    
    // TODO: 执行评估
    // ...
    
    // 更新结果表格
    // m_view->updateResultsTable(m_results);
    
    // TODO: 显示精度图表
    // std::map<double, double> accuracyData = ...;
    // m_view->showAccuracyChart(accuracyData);
}

// 导出评估结果
void EvaluationController::exportResults(const std::string& filePath) {
    if (m_results.empty()) return;
    
    std::ofstream outFile(filePath);
    if (!outFile.is_open()) {
        std::cerr << "无法打开文件: " << filePath << std::endl;
        return;
    }
    
    // 写入表头
    outFile << "指标,值\n";
    
    // 写入结果数据
    for (const auto& result : m_results) {
        outFile << result.first << "," << result.second << "\n";
    }
    
    outFile.close();
}

// 加载评估数据
void EvaluationController::loadEvaluationData() {
    if (!m_view) return;
    
    // TODO: 从数据库加载评估数据
    // m_results = ...;
    
    // 更新视图
    // m_view->updateResultsTable(m_results);
}

// 模式变化处理
void EvaluationController::handleModeChange(const std::string& mode) {
    // 根据模式变化更新UI或模型
    // ...
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
        // 获取单平台任务数据
        std::vector<SinglePlatformTask> tasks = SinglePlatformTaskDAO::getInstance().getTasksBySourceId(sourceId);
        
        if (tasks.empty()) {
            g_print("EvaluationController: No tasks found for radiation source ID %d\n", sourceId);
            // 返回空结果
            results.push_back(std::make_pair("最远定位距离", 0.0));
            results.push_back(std::make_pair("定位时间", 0.0));
            results.push_back(std::make_pair("定位精度", 0.0));
            results.push_back(std::make_pair("测向精度", 0.0));
            return results;
        }
        
        // 计算平均值
        double maxDistanceSum = 0.0;
        double positioningTimeSum = 0.0;
        double positioningAccuracySum = 0.0;
        double directionFindingAccuracySum = 0.0;
        
        for (const auto& task : tasks) {
            maxDistanceSum += task.maxPositioningDistance;
            positioningTimeSum += task.positioningTime;
            positioningAccuracySum += task.positioningAccuracy;
            directionFindingAccuracySum += task.directionFindingAccuracy;
        }
        
        int count = tasks.size();
        double avgMaxDistance = maxDistanceSum / count;
        double avgPositioningTime = positioningTimeSum / count;
        double avgPositioningAccuracy = positioningAccuracySum / count;
        double avgDirectionFindingAccuracy = directionFindingAccuracySum / count;
        
        // 添加结果
        results.push_back(std::make_pair("最远定位距离", avgMaxDistance));
        results.push_back(std::make_pair("定位时间", avgPositioningTime));
        results.push_back(std::make_pair("定位精度", avgPositioningAccuracy));
        results.push_back(std::make_pair("测向精度", avgDirectionFindingAccuracy));
        
        // 保存结果用于后续操作
        m_results = results;
    } else {
        // 多平台模式的处理，可以根据需要添加
        g_print("EvaluationController: Multi-platform evaluation not implemented yet\n");
    }
    
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
        // 多平台模式的处理，可以根据需要添加
        g_print("EvaluationController: Multi-platform time data not implemented yet\n");
    }
    
    return timeData;
} 