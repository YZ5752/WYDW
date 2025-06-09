#pragma once

#include "../views/EvaluationView.h"
#include <string>
#include <vector>
#include <map>

class ApplicationController;  // 前向声明

class EvaluationController {
public:
    friend class ApplicationController;  // 声明ApplicationController为友元类
    
    static EvaluationController& getInstance();
    
    // 初始化控制器
    void init(EvaluationView* view);
    
    // 开始评估
    void startEvaluation();
    
    // 导出评估结果
    void exportResults(const std::string& filePath);
    
    // 加载评估数据
    void loadEvaluationData();
    
    // 模式变化处理
    void handleModeChange(const std::string& mode);
    
    // 获取视图
    EvaluationView* getView() const;

private:
    EvaluationController();
    ~EvaluationController();
    
    // 禁止拷贝
    EvaluationController(const EvaluationController&) = delete;
    EvaluationController& operator=(const EvaluationController&) = delete;
    
    EvaluationView* m_view;
    std::vector<std::pair<std::string, double>> m_results;
}; 