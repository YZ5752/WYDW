#pragma once

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include "../models/ReconnaissanceDeviceModel.h"
#include "../models/RadiationSourceModel.h"
#include "components/MapView.h"
#include "../models/TrajectorySimulator.h"
#include "../utils/DirectionErrorLines.h"

// 声明全局回调函数
extern "C" {
    void onTechSystemChangedCallback(GtkWidget* widget, gpointer data);
    void onSinglePlatformSimulationCallback(GtkWidget* widget, gpointer data);
    void onDeviceComboChangedCallback(GtkWidget* widget, gpointer data);
    void onSourceComboChangedCallback(GtkWidget* widget, gpointer data);
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
    
    // 更新侦察设备下拉框内容
    void updateDeviceCombo();
    
    // 更新辐射源下拉框内容
    void updateSourceCombo();
    
    // 更新侦察设备地图标记
    void updateRadarMarker();
    
    // 更新辐射源地图标记
    void updateSourceMarker();
    
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
    
    // 显示错误信息
    void showErrorMessage(const std::string& message);
    
    // 获取地图视图
    MapView* getMapView() const;
    
    // 设备移动动画
    void animateDeviceMovement(const ReconnaissanceDevice& device, 
                              const std::vector<std::pair<double, double>>& trajectoryPoints, 
                              int simulationTime);
    
    // 显示仿真结果参数
    void showSimulationResult(double lon, double lat, double alt, double az, double el);
    
    // 清空仿真结果参数
    void clearSimulationResult();
    
    // 设置仿真结果缓存
    void setSimulationResult(double lon, double lat, double alt, double az, double el);
    
    // 获取仿真结果缓存
    bool getSimulationResult(double& lon, double& lat, double& alt, double& az, double& el);
    
    // 从指定位置显示测向误差线
    void showDirectionErrorLines(double errorAngle, 
                               double deviceLongitude, double deviceLatitude, double deviceAltitude,
                               double targetLongitude, double targetLatitude, double targetAltitude);
    
    // 清除测向误差线
    void clearDirectionErrorLines();

private:
    GtkWidget* m_view;
    GtkWidget* m_algoCombo;
    GtkWidget* m_radarCombo;
    GtkWidget* m_sourceCombo;
    GtkWidget* m_timeEntry;
    GtkWidget* m_dirDataValue;
    GtkWidget* m_locDataValue;
    GtkWidget* m_errorTable;
    
    // 地图视图
    MapView* m_mapView;
    
    // 地图标记点ID
    int m_radarMarker;
    int m_sourceMarker;
    
    // 存储侦察设备数据
    std::vector<ReconnaissanceDevice> m_devices;
    
    // 存储辐射源数据
    std::vector<RadiationSource> m_sources;
    
    // 轨迹线ID
    int m_trajectoryLineId;

    // 仿真结果参数label
    GtkWidget* m_resultLon;
    GtkWidget* m_resultLat;
    GtkWidget* m_resultAlt;
    GtkWidget* m_resultAz;
    GtkWidget* m_resultEl;

    double m_lastLon = 0, m_lastLat = 0, m_lastAlt = 0, m_lastAz = 0, m_lastEl = 0;
    bool m_hasResult = false;

    // 测向误差线工具类
    DirectionErrorLines m_directionErrorLines;
}; 