#pragma once

#include <gtk/gtk.h>
#include <vector>
#include <string>
#include "../models/CoordinateModel.h"

class MultiPlatformView {
public:
    MultiPlatformView();
    ~MultiPlatformView();
    
    // 创建多平台仿真UI
    GtkWidget* createView();
    
    // 添加平台到列表
    void addPlatformToList(const std::string& deviceName, const Coordinate& position);
    
    // 删除平台从列表
    void removePlatformFromList(int index);
    
    // 更新定位结果显示
    void updateLocationResult(const Coordinate& position, double error);
    
    // 显示协同定位精度图表
    void showAccuracyChart(const std::vector<double>& errors);
    
    // 获取选择的算法
    std::string getSelectedAlgorithm() const;
    
    // 获取平台列表
    std::vector<std::pair<std::string, Coordinate>> getPlatformList() const;
    
    // 获取视图控件
    GtkWidget* getView() const;

private:
    GtkWidget* m_view;
    GtkWidget* m_algoCombo;
    GtkWidget* m_platformList;
    GtkWidget* m_resultLabel;
    GtkWidget* m_errorLabel;
    GtkWidget* m_chartArea;
}; 