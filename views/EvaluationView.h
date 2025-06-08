#pragma once

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include <map>

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

private:
    GtkWidget* m_view;
    GtkWidget* m_modeCombo;
    GtkWidget* m_metricsCheckList;
    GtkWidget* m_resultsTable;
    GtkWidget* m_chartArea;
}; 