#pragma once

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <string>
#include <vector>
#include <map>

// 坐标结构体
struct MapCoordinate {
    double longitude;
    double latitude;
    std::string info;
    
    MapCoordinate(double lon, double lat, const std::string& information = "")
        : longitude(lon), latitude(lat), info(information) {}
};

// 定位结果结构体
struct MapLocationResult {
    double longitude;
    double latitude;
    double accuracy;
    std::string label;
    
    MapLocationResult(double lon, double lat, double acc, const std::string& lbl = "")
        : longitude(lon), latitude(lat), accuracy(acc), label(lbl) {}
};

// 地图视图类
class MapView {
public:
    // 构造函数
    MapView();
    ~MapView();
    
    // 创建地图控件
    GtkWidget* create();
    
    // 设置是否使用3D地图
    void setUse3DMap(bool use3D);
    
    // 获取是否使用3D地图
    bool isUsing3DMap() const;
    
    // 设置地图中心点和缩放级别
    void setCenter(double longitude, double latitude, int zoomLevel = 5);
    
    // 添加标记点
    void addPoint(double longitude, double latitude, const std::string& info = "");
    
    // 添加多个标记点
    void addPoints(const std::vector<MapCoordinate>& points);
    
    // 清除所有标记点
    void clearPoints();
    
    // 显示定位结果
    void showLocationResults(const std::vector<MapLocationResult>& results);
    
    // 设置地图类型
    void setMapType(const std::string& type);
    
    // 获取WebView控件
    WebKitWebView* getWebView() const { return m_webView; }
    
private:
    // WebView控件
    WebKitWebView* m_webView;
    
    // 资源文件路径
    std::string m_resourcePath;
    
    // 是否使用3D地图
    bool m_use3DMap;
    
    // 执行JavaScript代码
    void executeScript(const std::string& script);
    
    // 查找资源文件路径
    std::string findResourcePath();
}; 