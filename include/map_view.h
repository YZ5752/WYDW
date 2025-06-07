#pragma once

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <string>

/**
 * @brief MapView类 - 封装地图视图的创建和操作
 * 
 * 此类用于创建和管理基于WebKit的Cesium地图视图
 */
class MapView {
public:
    /**
     * @brief 构造函数
     */
    MapView();

    /**
     * @brief 析构函数
     */
    ~MapView();

    /**
     * @brief 创建地图视图控件
     * @return 返回创建的WebKit视图控件
     */
    GtkWidget* create();

    /**
     * @brief 设置地图中心点
     * @param longitude 经度
     * @param latitude 纬度
     * @param height 高度或缩放级别
     */
    void setCenter(double longitude, double latitude, double height);

    // /**
    //  * @brief 设置地图类型
    //  * @param mapType 地图类型，可以是"2d"或"3d"
    //  */
    // void setMapType(const std::string& mapType);

    /**
     * @brief 添加标记点
     * @param longitude 经度
     * @param latitude 纬度
     * @param title 标记点标题
     * @param description 标记点描述
     * @param color 标记点颜色
     * @return 标记点ID
     */
    int addMarker(double longitude, double latitude, const std::string& title, 
                 const std::string& description, const std::string& color);

    /**
     * @brief 移除标记点
     * @param markerId 标记点ID
     */
    void removeMarker(int markerId);

    /**
     * @brief 清除所有标记点
     */
    void clearMarkers();

    /**
     * @brief 设置是否使用3D地图
     * @param use3D 是否使用3D地图
     */
    void setUse3DMap(bool use3D);

    /**
     * @brief 执行JavaScript代码
     * @param script JavaScript代码
     */
    void executeScript(const std::string& script);

private:
    WebKitWebView* m_webView;
    std::string m_htmlPath;
    bool m_use3DMap;
    
    /**
     * @brief 加载地图HTML文件
     */
    void loadMap();
    
    /**
     * @brief 获取资源路径
     * @return 资源路径
     */
    std::string getResourcePath() const;
}; 