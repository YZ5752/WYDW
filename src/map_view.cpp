#include "map_view.h"
#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>  // 添加这个头文件以支持dirname函数

// 构造函数
MapView::MapView() : m_webView(nullptr), m_use3DMap(true) {
    m_resourcePath = findResourcePath();
}

// 析构函数
MapView::~MapView() {
    // WebKitWebView 会由 GTK 自动释放，不需要手动清理
}

// 创建地图控件
GtkWidget* MapView::create() {
    // 创建WebKit Web视图
    m_webView = WEBKIT_WEB_VIEW(webkit_web_view_new());
    
    // 构建HTML文件的绝对路径
    std::string mapHtmlPath = m_resourcePath + "/map_cesium.html";
    
    // 加载地图HTML文件
    std::string uri = "file://" + mapHtmlPath;
    webkit_web_view_load_uri(m_webView, uri.c_str());
    
    // 设置Web视图属性
    gtk_widget_set_size_request(GTK_WIDGET(m_webView), 600, 600);
    
    return GTK_WIDGET(m_webView);
}

// 设置是否使用3D地图
void MapView::setUse3DMap(bool use3D) {
    m_use3DMap = use3D;
}

// 获取是否使用3D地图
bool MapView::isUsing3DMap() const {
    return m_use3DMap;
}

// 设置地图中心点和缩放级别
void MapView::setCenter(double longitude, double latitude, int zoomLevel) {
    std::string script = "window.setCenter(" + 
                        std::to_string(longitude) + ", " + 
                        std::to_string(latitude) + ", " + 
                        std::to_string(zoomLevel) + ");";
    executeScript(script);
}

// 添加标记点
void MapView::addPoint(double longitude, double latitude, const std::string& info) {
    std::string script = "window.addPoint(" + 
                        std::to_string(longitude) + ", " + 
                        std::to_string(latitude) + ", '" + 
                        info + "');";
    executeScript(script);
}

// 添加多个标记点
void MapView::addPoints(const std::vector<MapCoordinate>& points) {
    for (const auto& point : points) {
        addPoint(point.longitude, point.latitude, point.info);
    }
}

// 清除所有标记点
void MapView::clearPoints() {
    executeScript("window.clearPoints();");
}

// 显示定位结果
void MapView::showLocationResults(const std::vector<MapLocationResult>& results) {
    clearPoints();
    
    for (const auto& result : results) {
        std::string script = "window.addPoint(" + 
                           std::to_string(result.longitude) + ", " + 
                           std::to_string(result.latitude) + ", '" + 
                           result.label + "');";
        executeScript(script);
    }
    
    if (!results.empty()) {
        // 将地图中心设置到第一个结果点
        setCenter(results[0].longitude, results[0].latitude, 12);
    }
}

// 设置地图类型
void MapView::setMapType(const std::string& type) {
    std::string script = "window.setMapType('" + type + "');";
    executeScript(script);
}

// 执行JavaScript代码
void MapView::executeScript(const std::string& script) {
    if (m_webView) {
        webkit_web_view_run_javascript(m_webView, script.c_str(), NULL, NULL, NULL);
    }
}

// 查找资源文件路径
std::string MapView::findResourcePath() {
    // 尝试获取当前可执行文件路径
    char exePath[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len != -1) {
        exePath[len] = '\0';
        std::string dirPath(dirname(exePath));
        
        // 尝试直接在可执行文件目录下的res目录
        std::string resPath = dirPath + "/res";
        if (access((resPath + "/map_gtk.html").c_str(), F_OK) != -1) {
            return resPath;
        }
        
        // 尝试上一级目录下的res目录（开发环境）
        resPath = dirPath + "/../res";
        if (access((resPath + "/map_gtk.html").c_str(), F_OK) != -1) {
            return resPath;
        }
        
        // 尝试系统安装路径
        resPath = "/usr/share/passivelocation/res";
        if (access((resPath + "/map_gtk.html").c_str(), F_OK) != -1) {
            return resPath;
        }
    }
    
    // 如果无法找到，返回相对路径
    return "res";
}