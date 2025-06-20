#pragma once

#include <string>
#include <vector>
#include <map>
#include <gtk/gtk.h>
#include "../models/RadiationSourceModel.h"
#include "../models/RadiationSourceDAO.h"
#include "../models/SinglePlatformTaskDAO.h"

// 前向声明
class EvaluationView;
class ApplicationController;

class EvaluationController {
public:
    friend class ApplicationController;  // 声明ApplicationController为友元类
    
    static EvaluationController& getInstance();
    
    // 初始化控制器
    void init(EvaluationView* view);
    
    // 开始评估
    void startEvaluation();
    
    // 评估指定的辐射源
    std::vector<std::pair<std::string, double>> evaluateRadiationSource(int sourceId, bool isSinglePlatform);
    
    // 获取性能随时间变化的数据
    std::map<double, double> getAccuracyTimeData(int sourceId, bool isSinglePlatform);
    
    // 导出评估结果
    void exportResults(const std::string& filePath);
    
    // 加载评估数据
    void loadEvaluationData();
    
    // 模式变化处理
    void handleModeChange(const std::string& mode);
    
    // 获取视图
    EvaluationView* getView() const;
    
    // 获取所有辐射源数据
    std::vector<RadiationSource> getAllRadiationSources();

private:
    EvaluationController();
    ~EvaluationController();
    
    // 禁止拷贝
    EvaluationController(const EvaluationController&) = delete;
    EvaluationController& operator=(const EvaluationController&) = delete;
    
    EvaluationView* m_view;
    std::vector<std::pair<std::string, double>> m_results;
}; 