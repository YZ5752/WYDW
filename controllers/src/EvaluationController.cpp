#include "../EvaluationController.h"
#include <fstream>
#include <iostream>
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