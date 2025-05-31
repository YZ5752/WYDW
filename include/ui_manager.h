#pragma once

#include <gtk/gtk.h>
#include "simulation.h"
#include "data_processor.h"

// 辅助函数，获取容器中指定索引的子控件
GtkWidget* get_child_at_index(GtkContainer* container, gint index);

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
    
    // 创建雷达设备模型UI
    GtkWidget* createRadarDeviceModelUI();
    
    // 创建辐射源模型UI
    GtkWidget* createRadiationSourceModelUI();
    
    // 创建单平台仿真UI
    GtkWidget* createSinglePlatformUI();
    
    // 创建多平台仿真UI
    GtkWidget* createMultiPlatformUI();
    
    // 创建数据显示UI (仅用于兼容性)
    GtkWidget* createDataDisplayUI();
    
    // 创建数据分选UI
    GtkWidget* createDataSelectionUI();
    
    // 创建仿真评估UI
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

    // 雷达设备模型UI回调
    static void onAddRadarDevice(GtkWidget* widget, gpointer data);
    static void onEditRadarDevice(GtkWidget* widget, gpointer data);
    static void onDeleteRadarDevice(GtkWidget* widget, gpointer data);
    
    // 辐射源模型UI回调
    static void onAddRadiationSource(GtkWidget* widget, gpointer data);
    static void onEditRadiationSource(GtkWidget* widget, gpointer data);
    static void onDeleteRadiationSource(GtkWidget* widget, gpointer data);
    
    // 单平台仿真UI回调
    static void onSinglePlatformSimulation(GtkWidget* widget, gpointer data);
    
    // 多平台仿真UI回调
    static void onMultiPlatformSimulation(GtkWidget* widget, gpointer data);
    
    // 数据分选回调
    static void onDataSelectionChanged(GtkWidget* widget, gpointer data);
    static void onDataImport(GtkWidget* widget, gpointer data);
    
    // 仿真评估回调
    static void onEvaluationStart(GtkWidget* widget, gpointer data);
    static void onExportResults(GtkWidget* widget, gpointer data);

private:
    UIManager();
    ~UIManager();
    
    GtkWidget* m_mainWindow;
    GtkWidget* m_radarDeviceModelPage;
    GtkWidget* m_radiationSourceModelPage;
    GtkWidget* m_singlePlatformPage;
    GtkWidget* m_multiPlatformPage;
    GtkWidget* m_dataSelectionPage;
    GtkWidget* m_evaluationPage;
    
    // 更新模型列表
    void updateRadarDeviceList(GtkWidget* list);
    void updateRadiationSourceList(GtkWidget* list);
    
    // 创建通用的表格
    GtkWidget* createModelList(const std::vector<std::string>& headers);
}; 