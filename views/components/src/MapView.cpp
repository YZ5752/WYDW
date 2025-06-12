#include "../MapView.h"
#include <filesystem>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;
// 构造函数
MapView::MapView() 
    : m_webView(nullptr), 
      m_use3DMap(true) {
    // 默认使用Cesium 3D地图
    m_htmlPath = getResourcePath() + "/cesium.html";
}

// 析构函数
MapView::~MapView() {
    // WebKit控件会被GTK自动销毁，不需要手动释放
}

// 创建地图视图控件
GtkWidget* MapView::create() {
    // 创建WebKit视图
    m_webView = WEBKIT_WEB_VIEW(webkit_web_view_new());
    
    // 启用WebGL和JavaScript
    WebKitSettings* settings = webkit_web_view_get_settings(m_webView);
    webkit_settings_set_enable_webgl(settings, TRUE);
    webkit_settings_set_enable_javascript(settings, TRUE);
    webkit_settings_set_javascript_can_access_clipboard(settings, TRUE);

        // 添加以下安全设置
    webkit_settings_set_allow_file_access_from_file_urls(settings, TRUE);
    webkit_settings_set_allow_universal_access_from_file_urls(settings, TRUE);
    webkit_settings_set_enable_xss_auditor(settings, FALSE);
    
    // 加载地图
    loadMap();
    
    // 返回控件
    return GTK_WIDGET(m_webView);
}

// 设置地图中心点
void MapView::setCenter(double longitude, double latitude, double height) {
    if (!m_webView) return;
    
    std::stringstream script;
    
    if (m_use3DMap) {
        // Cesium地图的设置中心点方法
        script << "viewer.camera.flyTo({" 
               << "destination: Cesium.Cartesian3.fromDegrees(" << longitude << ", " << latitude << ", " << height << "),"
               << "orientation: {heading: 0.0, pitch: 0, roll: 0.0}});";
    } else {
        // 2D地图的设置中心点方法
        script << "map.setView([" << latitude << ", " << longitude << "], " << height << ");";
    }
    
    executeScript(script.str());
}

// // 设置地图类型
// void MapView::setMapType(const std::string& mapType) {
//     if (!m_webView) return;
    
//     std::stringstream script;
    
//     if (mapType == "3d") {
//         m_use3DMap = true;
//         // 加载3D地图
//         m_htmlPath = getResourcePath() + "/cesium.html";
//         loadMap();
//     } else if (mapType == "2d") {
//         m_use3DMap = false;
//         // 加载2D地图
//         m_htmlPath = getResourcePath() + "/index.html";
//         loadMap();
//     }
// }

// 添加标记点
int MapView::addMarker(double longitude, double latitude, const std::string& title, 
                      const std::string& description, const std::string& color) {
    if (!m_webView) return -1;
    
    // 生成唯一的标记ID
    static int nextMarkerId = 1;
    int markerId = nextMarkerId++;
    
    std::stringstream script;
    
    if (m_use3DMap) {
        // 从描述中提取高度信息（如果有）
        double height = 0.0;
        size_t heightPos = description.find("高度:");
        if (heightPos != std::string::npos) {
            size_t valueStart = description.find_first_of("0123456789.", heightPos);
            if (valueStart != std::string::npos) {
                size_t valueEnd = description.find_first_not_of("0123456789.", valueStart);
                if (valueEnd != std::string::npos) {
                    std::string heightStr = description.substr(valueStart, valueEnd - valueStart);
                    try {
                        height = std::stod(heightStr);
                    } catch (...) {
                        // 转换失败，使用默认高度0
                    }
                }
            }
        }
        
        // Cesium地图添加标记点
        script << "var entity = viewer.entities.add({" 
               << "id: '" << markerId << "',"
               << "position: Cesium.Cartesian3.fromDegrees(" << longitude << ", " << latitude << ", " << height << "),"
               << "point: {pixelSize: 10, color: Cesium.Color.fromCssColorString('" << color << "'), outlineColor: Cesium.Color.BLACK, outlineWidth: 2},"
               << "label: {text: '" << title << "', font: '14pt sans-serif', style: Cesium.LabelStyle.FILL_AND_OUTLINE,"
               << "        outlineWidth: 2, verticalOrigin: Cesium.VerticalOrigin.BOTTOM, pixelOffset: new Cesium.Cartesian2(0, -9),"
               << "        showBackground: true, backgroundColor: new Cesium.Color(0.165, 0.165, 0.165, 0.7)},"
               << "billboard: {image: 'data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAyNCAyNCIgd2lkdGg9IjI0IiBoZWlnaHQ9IjI0Ij48Y2lyY2xlIGN4PSIxMiIgY3k9IjEyIiByPSIxMCIgZmlsbD0iIzAwMCIvPjxjaXJjbGUgY3g9IjEyIiBjeT0iMTIiIHI9IjgiIGZpbGw9IiIgLz48L3N2Zz4=',"
               << "        width: 32, height: 32, verticalOrigin: Cesium.VerticalOrigin.BOTTOM, color: Cesium.Color.fromCssColorString('" << color << "')},"
               << "description: '" << description << "',"
               << "});"
               
               // 添加垂直线连接标记点和地面
               << "viewer.entities.add({"
               << "  polyline: {"
               << "    positions: [Cesium.Cartesian3.fromDegrees(" << longitude << ", " << latitude << ", 0), "
               << "                Cesium.Cartesian3.fromDegrees(" << longitude << ", " << latitude << ", " << height << ")],"
               << "    width: 1,"
               << "    material: new Cesium.PolylineDashMaterialProperty({"
               << "      color: Cesium.Color.fromCssColorString('" << color << "')"
               << "    })"
               << "  }"
               << "});";
    } else {
        // 2D地图添加标记点
        script << "var marker = L.marker([" << latitude << ", " << longitude << "]).addTo(map);"
               << "marker.bindPopup('<b>" << title << "</b><br>" << description << "');"
               << "window.markers = window.markers || {};"
               << "window.markers[" << markerId << "] = marker;";
    }
    
    executeScript(script.str());
    return markerId;
}

// 移除标记点
void MapView::removeMarker(int markerId) {
    if (!m_webView) return;
    
    std::stringstream script;
    
    if (m_use3DMap) {
        // Cesium地图移除标记点
        script << "viewer.entities.removeById('" << markerId << "');";
    } else {
        // 2D地图移除标记点
        script << "if(window.markers && window.markers[" << markerId << "]) {"
               << "  map.removeLayer(window.markers[" << markerId << "]);"
               << "  delete window.markers[" << markerId << "];"
               << "}";
    }
    
    executeScript(script.str());
}

// 清除所有标记点
void MapView::clearMarkers() {
    if (!m_webView) return;
    
    std::stringstream script;
    
    if (m_use3DMap) {
        // Cesium地图清除所有标记点
        script << "viewer.entities.removeAll();";
    } else {
        // 2D地图清除所有标记点
        script << "if(window.markers) {"
               << "  for(var id in window.markers) {"
               << "    map.removeLayer(window.markers[id]);"
               << "  }"
               << "  window.markers = {};"
               << "}";
    }
    
    executeScript(script.str());
}

// 设置是否使用3D地图
void MapView::setUse3DMap(bool use3D) {
    if (m_use3DMap == use3D) return;
    
    m_use3DMap = use3D;
    if (m_use3DMap) {
        m_htmlPath = getResourcePath() + "/cesium.html";
    } else {
        m_htmlPath = getResourcePath() + "/index.html";
    }
    
    loadMap();
}

// 执行JavaScript代码
void MapView::executeScript(const std::string& script) {
    if (!m_webView) return;
    
    webkit_web_view_run_javascript(m_webView, script.c_str(), nullptr, nullptr, nullptr);
}

// 加载地图HTML文件
void MapView::loadMap() {
    if (!m_webView) return;
    
    // 检查文件是否存在
    if (!fs::exists(m_htmlPath)) {
        std::cerr << "Error: Map HTML file not found at: " << m_htmlPath << std::endl;
        return;
    }
    
    // 使用file://协议加载本地HTML文件
    std::string fileUrl = "file://" + m_htmlPath;
    webkit_web_view_load_uri(m_webView, fileUrl.c_str());
}

// 获取资源路径
std::string MapView::getResourcePath() const {
    // 尝试多个可能的资源路径
    std::vector<std::string> possiblePaths = {
        "./res",
        "../res",
        "../../res",
        "./share/passivelocation/res",
        "/usr/local/share/passivelocation/res",
        "/usr/share/passivelocation/res"
    };
    
    // 检查wydw中的资源目录
    possiblePaths.push_back("./wydw/res");
    possiblePaths.push_back("../wydw/res");
    
    // 检查每个可能的路径
    for (const auto& path : possiblePaths) {
        if (fs::exists(path)) {
            return fs::absolute(path).string();
        }
    }
    
    // 如果找不到资源目录，返回默认路径
    std::cerr << "Warning: Resource directory not found, using default path: ./res" << std::endl;
    return "./res";
} 