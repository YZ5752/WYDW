#pragma once

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include <map>
#include "../models/RadiationSourceModel.h"

// 前向声明
class EvaluationController;  // 添加前向声明

class EvaluationView {
public:
    EvaluationView();
    ~EvaluationView();
    
    // 创建评估UI
    GtkWidget* createView();
    
    // 更新评估结果表格
    void updateResultsTable(const std::vector<std::pair<std::string, double>>& results);
    
    // 显示定位精度图表
    void showAccuracyChart(const std::map<double, double>& data);
    
    // 获取选择的评估模式
    std::string getSelectedMode() const;
    
    // 获取选择的评估指标
    std::vector<std::string> getSelectedMetrics() const;
    
    // 获取视图控件
    GtkWidget* getView() const;
    
    // 新增方法
    void onRadiationSourceSelected();
    void startEvaluation();

private:
    // 加载辐射源数据的辅助方法
    void loadRadiationSources();
    
    GtkWidget* m_view;
    GtkWidget* m_modeCombo;
    GtkWidget* m_metricsCheckList;
    GtkWidget* m_resultsTable;
    GtkWidget* m_chartArea;
    GtkWidget* m_targetCombo;  // 辐射源下拉框
    std::vector<RadiationSource> m_radiationSources;  // 保存辐射源数据
    std::map<double, double> m_chartData;  // 保存图表数据
}; 