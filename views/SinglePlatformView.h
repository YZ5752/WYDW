#pragma once

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include "../models/ReconnaissanceDeviceModel.h"

// 声明全局回调函数
extern "C" {
    void onTechSystemChangedCallback(GtkWidget* widget, gpointer data);
    void onSinglePlatformSimulationCallback(GtkWidget* widget, gpointer data);
}

class SinglePlatformView {
public:
    SinglePlatformView();
    ~SinglePlatformView();
    
    // 创建单平台仿真UI
    GtkWidget* createView();
    
    // 更新测向数据显示
    void updateDirectionData(const std::string& data);
    
    // 更新定位数据显示
    void updateLocationData(const std::string& data);

    // 技术体制变化回调
    static void onTechSystemChanged(GtkWidget* widget, gpointer data);
    
    // 开始仿真回调
    static void onSinglePlatformSimulation(GtkWidget* widget, gpointer data);
    
    // 更新误差表格
    void updateErrorTable(const std::string& techSystem);
    
    // 更新侦察设备下拉列表
    void updateDeviceList(const std::vector<ReconnaissanceDevice>& devices);
    
    // 获取技术体制选择
    std::string getSelectedTechSystem() const;
    
    // 获取侦察设备选择
    std::string getSelectedDevice() const;
    
    // 获取辐射源选择
    std::string getSelectedSource() const;
    
    // 获取仿真时间
    int getSimulationTime() const;
    
    // 获取视图控件
    GtkWidget* getView() const;
    
    // 获取误差表格控件
    GtkWidget* getErrorTable() const;

private:
    GtkWidget* m_view;
    GtkWidget* m_algoCombo;
    GtkWidget* m_radarCombo;
    GtkWidget* m_sourceCombo;
    GtkWidget* m_timeEntry;
    GtkWidget* m_dirDataValue;
    GtkWidget* m_locDataValue;
    GtkWidget* m_errorTable;
    
    // 存储侦察设备数据
    std::vector<ReconnaissanceDevice> m_devices;
}; 