#pragma once

#include <gtk/gtk.h>
#include <vector>
#include <string>
#include "../models/ReconnaissanceDeviceModel.h"
#include "../models/RadiationSourceModel.h"
#include "../utils/DirectionErrorLines.h"

class MapView;

class MultiPlatformController;

// 多平台定位视图类
class MultiPlatformView {
public:
    MultiPlatformView();
    ~MultiPlatformView();
    
    // 创建多平台仿真UI
    GtkWidget* createView();
    
    // 获取视图控件
    GtkWidget* getView() const;

    // 设置控制器
    void setController(MultiPlatformController* controller);
    
    // 更新仿真结果显示
    void updateResult(const std::string& result);
    
    // 更新误差显示
    void updateError(const std::string& error);
    
    // 获取地图视图
    MapView* getMapView() const { return m_mapView; }
    
    // 显示测向误差线 - 从设备到目标位置，带误差角度
    void showDirectionErrorLines(int deviceIndex, 
                               double targetLongitude, double targetLatitude, double targetAltitude,
                               double errorAngle,
                               const std::string& lineColor = "#FF0000");
    
    // 显示多设备测向误差线 - 从所有设备到目标位置
    void showMultipleDeviceErrorLines(const std::vector<int>& deviceIndices,
                                    double targetLongitude, double targetLatitude, double targetAltitude,
                                    double errorAngle);
    
    // 清除测向误差线
    void clearDirectionErrorLines();
    
    // 获取测向误差参数
    double getDFMeanError(int deviceIndex) const;
    double getDFStdDev(int deviceIndex) const;
    
    // 获取TDOA误差参数
    double getTDOARmsError() const;
    double getESMToaError() const;

private:
    MultiPlatformController* m_controller;
    GtkWidget* m_view;//主视图
    GtkWidget* m_algoCombo;//技术体制
    GtkWidget* m_radarCombo[4]; // 4个侦察设备下拉框
    GtkWidget* m_radarFrame[4]; // 4个侦察设备Frame
    GtkWidget* m_sourceCombo;   // 辐射源下拉框
    GtkWidget* m_resultLabel;//仿真结果
    GtkWidget* m_errorLabel;//误差结果
    GtkWidget* m_timeEntry;    // 仿真时间输入框
    
    // 测向定位误差参数
    GtkWidget* m_dfParamsFrame;    // 测向误差参数框架
    GtkWidget* m_dfMeanError[2];   // 均值误差输入框
    GtkWidget* m_dfStdDev[2];      // 标准差输入框
    
    // TDOA定位误差参数
    GtkWidget* m_tdoaParamsFrame;  // TDOA误差参数框架
    GtkWidget* m_tdoaRmsError;     // TDOA rms Error输入框
    GtkWidget* m_esmToaError;      // ESM toa Error输入框
    
    std::vector<ReconnaissanceDevice> m_devices; // 设备数据
    std::vector<RadiationSource> m_sources;      // 辐射源数据
    MapView* m_mapView = nullptr; // 地图对象
    int m_radarMarkers[4] = {-1,-1,-1,-1}; // 侦察设备标记点ID
    int m_sourceMarker = -1; // 辐射源标记点ID
    
    // 测向误差线工具类
    DirectionErrorLines m_directionErrorLines;
    
    void updateDeviceCombos(); // 刷新设备下拉框内容
    void updateSourceCombo();  // 刷新辐射源下拉框内容
    static void onTechSystemChangedCallback(GtkWidget* widget, gpointer data); // 技术体制切换回调
    void onTechSystemChanged(); // 技术体制切换处理
    static void onDeviceComboChangedCallback(GtkComboBox* combo, gpointer user_data); // 侦察设备下拉框回调
    void onDeviceComboChanged(int idx); // 侦察设备下拉框处理
    static void onSourceComboChangedCallback(GtkComboBox* combo, gpointer user_data); // 辐射源下拉框回调
    void onSourceComboChanged(); // 辐射源下拉框处理
    void updateRadarMarker(int idx); // 更新单个侦察设备标记
    void updateSourceMarker(); // 更新辐射源标记
    std::string formatSimulationResult(const std::string& result); // 格式化仿真结果
    
    // 开始仿真按钮回调
    static void onStartSimulationCallback(GtkWidget* widget, gpointer data);
    void onStartSimulation(); // 开始仿真处理
    bool checkRadarModels(); // 检查雷达侦察模型是否有效
    
    // 创建测向误差参数UI
    void createDFParamsUI(GtkWidget* parent);
    
    // 显示/隐藏测向误差参数UI
    void toggleDFParamsUI(bool show);
    
    // 创建TDOA误差参数UI
    void createTDOAParamsUI(GtkWidget* parent);
    // 显示/隐藏TDOA误差参数UI
    void toggleTDOAParamsUI(bool show);
}; 