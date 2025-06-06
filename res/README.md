 # 地图实现说明

## 概述

本地图实现使用了OpenLayers作为基础库，通过WebKitGTK嵌入到GTK应用程序中。地图支持显示天地图的标准地图图层，可以添加标记点、显示定位结果等功能。

## 依赖项

- WebKitGTK 4.0 或更高版本
- GTK3
- OpenLayers (已包含在res目录中)

## 安装依赖项

在Ubuntu上安装依赖项：

```bash
sudo apt-get update
sudo apt-get install libwebkit2gtk-4.0-dev libgtk-3-dev
```

## 文件结构

- `map_gtk.html`: 地图的HTML实现，包含OpenLayers的初始化和基本功能
- `ol.js`: OpenLayers核心库
- `ol.css`: OpenLayers样式表
- `ol.js.map`: OpenLayers源码映射文件
- `location.png`: 位置标记图标
- `temp.png`: 临时标记图标
- `map/`: 地图瓦片文件夹，包含离线地图瓦片

## 使用方法

### 在GTK应用程序中显示地图

```c++
#include <webkit2/webkit2.h>

// 创建WebKit视图
WebKitWebView* webView = WEBKIT_WEB_VIEW(webkit_web_view_new());

// 加载地图HTML文件
webkit_web_view_load_uri(webView, "file:///path/to/map_gtk.html");

// 添加到GTK容器
gtk_container_add(GTK_CONTAINER(container), GTK_WIDGET(webView));
```

### JavaScript API

地图HTML文件提供了以下JavaScript API，可以通过WebKitGTK的JavaScript接口调用：

- `window.addPoint(longitude, latitude, info)`: 添加一个标记点
- `window.clearPoints()`: 清除所有标记点
- `window.setCenter(longitude, latitude, zoom)`: 设置地图中心和缩放级别
- `window.setMapType(type)`: 设置地图类型（如果实现了多种地图类型）

### 示例：添加一个标记点

```c++
// JavaScript回调函数
static void js_finished_callback(WebKitWebView* web_view, GAsyncResult* res, gpointer user_data) {
    // 处理JavaScript执行结果
    WebKitJavascriptResult* js_result = webkit_web_view_run_javascript_finish(web_view, res, NULL);
    if (js_result)
        webkit_javascript_result_unref(js_result);
}

// 添加标记点
void addPoint(WebKitWebView* webView, double lon, double lat, const char* info) {
    char script[256];
    snprintf(script, sizeof(script), "window.addPoint(%f, %f, '%s');", lon, lat, info);
    webkit_web_view_run_javascript(webView, script, NULL, js_finished_callback, NULL);
}
```

## 注意事项

1. 确保地图HTML文件及其依赖的资源文件能够被应用程序正确加载。
2. 天地图API需要使用有效的API密钥，请在实际应用中替换为您自己的密钥。
3. 离线地图瓦片需要放置在正确的位置，并确保HTML文件中的路径设置正确。

## 故障排除

如果地图无法显示，请检查：

1. WebKitGTK是否正确安装
2. 资源文件路径是否正确
3. 天地图API密钥是否有效
4. 网络连接是否正常（如果使用在线地图）

## 离线地图说明

如果需要使用离线地图，需要将地图瓦片放置在`map/`目录下，并修改`map_gtk.html`文件中的地图源设置。