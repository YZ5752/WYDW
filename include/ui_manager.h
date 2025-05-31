#pragma once

#include <gtk/gtk.h>
#include "simulation.h"
#include "data_processor.h"

// UI管理器
class UIManager {
public:
    static UIManager& getInstance();
    
    // 初始化UI
    bool initUI(int argc, char** argv);
    
    // 运行UI主循环
    void run();
    
    // 创建主窗口
    GtkWidget* createMainWindow();
    
    // 创建单平台仿真UI
    GtkWidget* createSinglePlatformUI();
    
    // 创建多平台仿真UI
    GtkWidget* createMultiPlatformUI();
    
    // 创建数据显示UI
    GtkWidget* createDataDisplayUI();
    
    // 创建数据分选UI
    GtkWidget* createDataSelectionUI();
    
    // 创建协同定位评估UI
    GtkWidget* createEvaluationUI();
    
    // 显示地图
    void showMap(GtkWidget* container);
    
    // 在地图上显示定位结果
    void showLocationResults(GtkWidget* map, const std::vector<LocationResult>& results);
    
    // 在地图上显示基准点
    void showReferencePoints(GtkWidget* map, const std::map<std::string, Coordinate>& points);
    
    // 显示雷达侦察设备参数设置对话框
    bool showRadarDeviceDialog(RadarDevice& device);
    
    // 显示辐射源参数设置对话框
    bool showRadiationSourceDialog(RadiationSource& source);
    
    // 显示定位精度图表
    void showAccuracyChart(GtkWidget* container, const std::map<double, double>& data);
    
    // 显示统计结果表格
    void showResultsTable(GtkWidget* container, 
                        const std::vector<std::pair<std::string, double>>& results);

    // 单平台仿真UI回调
    static void onSinglePlatformSimulation(GtkWidget* widget, gpointer data);
    
    // 多平台仿真UI回调
    static void onMultiPlatformSimulation(GtkWidget* widget, gpointer data);
    
    // 雷达设备参数更改回调
    static void onRadarDeviceChanged(GtkWidget* widget, gpointer data);
    
    // 辐射源参数更改回调
    static void onRadiationSourceChanged(GtkWidget* widget, gpointer data);
    
    // 定位算法选择回调
    static void onAlgorithmSelected(GtkWidget* widget, gpointer data);
    
    // 数据分选回调
    static void onDataSelectionChanged(GtkWidget* widget, gpointer data);
    
    // 保存结果回调
    static void onSaveResults(GtkWidget* widget, gpointer data);

private:
    UIManager();
    ~UIManager();
    
    GtkWidget* m_mainWindow;
    GtkWidget* m_singlePlatformPage;
    GtkWidget* m_multiPlatformPage;
    GtkWidget* m_dataDisplayPage;
    GtkWidget* m_evaluationPage;
}; 