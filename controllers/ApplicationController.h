#pragma once

#include <gtk/gtk.h>
#include <string>
#include "../models/DBConnector.h"

// 前向声明
class SinglePlatformController;
class MultiPlatformController;
class DataSelectionController;
class EvaluationController;
class RadiationSourceModelController;
class ReconnaissanceDeviceModelController;
class ReconnaissanceDeviceModelView;
class RadiationSourceModelView;
class SinglePlatformView;
class MultiPlatformView;
class DataSelectionView;
class EvaluationView;

// 应用程序控制器
class ApplicationController {
public:
    static ApplicationController& getInstance();
    
    // 初始化应用程序
    bool init(int argc, char** argv);
    
    // 运行应用程序
    void run();
    
    // 创建主窗口
    GtkWidget* createMainWindow();
    
    // 切换到单平台仿真页面
    void switchToSinglePlatformPage();
    
    // 切换到多平台仿真页面
    void switchToMultiPlatformPage();
    
    // 切换到数据分选页面
    void switchToDataSelectionPage();
    
    // 切换到仿真评估页面
    void switchToEvaluationPage();
    
    // 切换到雷达设备模型页面
    void switchToRadarDeviceModelPage();
    
    // 切换到辐射源模型页面
    void switchToRadiationSourceModelPage();
    
    // 获取当前页面
    std::string getCurrentPage() const;

private:
    ApplicationController();
    ~ApplicationController();
    
    // 禁止拷贝
    ApplicationController(const ApplicationController&) = delete;
    ApplicationController& operator=(const ApplicationController&) = delete;
    
    GtkWidget* m_mainWindow;
    GtkWidget* m_notebook;
    std::string m_currentPage;
    
    // 控制器
    SinglePlatformController* m_singlePlatformController;
    MultiPlatformController* m_multiPlatformController;
    DataSelectionController* m_dataSelectionController;
    EvaluationController* m_evaluationController;
    RadiationSourceModelController* m_radiationSourceModelController;
    ReconnaissanceDeviceModelController* m_reconnaissanceDeviceModelController;
    
    // 视图
    ReconnaissanceDeviceModelView* m_reconDeviceView;
    RadiationSourceModelView* m_radiationSourceView;
    SinglePlatformView* m_singlePlatformView;
    MultiPlatformView* m_multiPlatformView;
    DataSelectionView* m_dataSelectionView;
    EvaluationView* m_evaluationView;
}; 