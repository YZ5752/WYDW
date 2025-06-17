#pragma once

#include <gtk/gtk.h>
#include <vector>
#include <string>
#include "../models/ReconnaissanceDeviceModel.h"
#include "../models/RadiationSourceModel.h"
#include "components/MapView.h"

class MultiPlatformView {
public:
    MultiPlatformView();
    ~MultiPlatformView();
    
    // 创建多平台仿真UI
    GtkWidget* createView();
    
    // 获取视图控件
    GtkWidget* getView() const;

    // 更新仿真结果显示
    void updateResult(const std::string& result);

private:
    GtkWidget* m_view;//主视图
    GtkWidget* m_algoCombo;//技术体制
    GtkWidget* m_radarCombo[4]; // 4个侦察设备下拉框
    GtkWidget* m_radarFrame[4]; // 4个侦察设备Frame
    GtkWidget* m_sourceCombo;   // 辐射源下拉框
    GtkWidget* m_resultLabel;//仿真结果
    GtkWidget* m_errorLabel;//误差结果
    GtkWidget* m_timeEntry;    // 仿真时间输入框
    std::vector<ReconnaissanceDevice> m_devices; // 设备数据
    std::vector<RadiationSource> m_sources;      // 辐射源数据
    MapView* m_mapView = nullptr; // 地图对象
    int m_radarMarkers[4] = {-1,-1,-1,-1}; // 侦察设备标记点ID
    int m_sourceMarker = -1; // 辐射源标记点ID
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
    
    // 开始仿真按钮回调
    static void onStartSimulationCallback(GtkWidget* widget, gpointer data);
    void onStartSimulation(); // 开始仿真处理
    bool checkRadarModels(); // 检查雷达侦察模型是否有效
}; 