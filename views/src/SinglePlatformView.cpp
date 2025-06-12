#include "../SinglePlatformView.h"
#include "../components/MapView.h"
#include "../../controllers/SinglePlatformController.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/RadiationSourceDAO.h"
#include <gtk/gtk.h>
#include <string>
#include <sstream>
#include <iomanip>

SinglePlatformView::SinglePlatformView() :
    m_view(nullptr),
    m_algoCombo(nullptr),
    m_radarCombo(nullptr),
    m_sourceCombo(nullptr),
    m_timeEntry(nullptr),
    m_dirDataValue(nullptr),
    m_locDataValue(nullptr),
    m_errorTable(nullptr),
    m_mapView(nullptr),
    m_radarMarker(-1),
    m_sourceMarker(-1),
    m_trajectoryLineId(-1) {
}

SinglePlatformView::~SinglePlatformView() {
    if (m_mapView) {
        delete m_mapView;
        m_mapView = nullptr;
    }
}

GtkWidget* SinglePlatformView::createView() {
// 创建页面的主容器
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(container), 15);
    
    // 保存视图引用
    m_view = container;
    
    // 左侧：地图显示区域
    GtkWidget* mapFrame = gtk_frame_new("地图");
    gtk_widget_set_size_request(mapFrame, 800, 700);
    gtk_box_pack_start(GTK_BOX(container), mapFrame, TRUE, TRUE, 0);
    
    // 显示地图
    m_mapView = new MapView();
    GtkWidget* mapWidget = m_mapView->create();
    gtk_container_add(GTK_CONTAINER(mapFrame), mapWidget);
    
    // 右侧：参数设置和结果区域
    GtkWidget* rightBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(container), rightBox, FALSE, FALSE, 0);

    // 技术体制选择
    GtkWidget* algoFrame = gtk_frame_new("技术体制");
    gtk_box_pack_start(GTK_BOX(rightBox), algoFrame, FALSE, FALSE, 0);
    
    GtkWidget* algoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(algoFrame), algoBox);
    gtk_container_set_border_width(GTK_CONTAINER(algoBox), 10);
    
    GtkWidget* algoCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algoCombo), "干涉仪体制");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algoCombo), "时差体制");
    gtk_combo_box_set_active(GTK_COMBO_BOX(algoCombo), 0);
    gtk_box_pack_start(GTK_BOX(algoBox), algoCombo, TRUE, TRUE, 5);
    g_signal_connect(algoCombo, "changed", G_CALLBACK(onTechSystemChangedCallback), this);
    
    // 保存技术体制下拉框引用
    m_algoCombo = algoCombo;
    
    // 雷达设备模型选择
    GtkWidget* radarFrame = gtk_frame_new("侦察设备模型");
    gtk_box_pack_start(GTK_BOX(rightBox), radarFrame, FALSE, FALSE, 0);
    
    GtkWidget* radarBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(radarFrame), radarBox);
    gtk_container_set_border_width(GTK_CONTAINER(radarBox), 10);
    
    GtkWidget* radarCombo = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(radarBox), radarCombo, TRUE, TRUE, 5);
    g_signal_connect(radarCombo, "changed", G_CALLBACK(onDeviceComboChangedCallback), this);
    
    // 保存雷达设备下拉框引用
    m_radarCombo = radarCombo;
    
    // 辐射源模型选择
    GtkWidget* sourceFrame = gtk_frame_new("辐射源模型");
    gtk_box_pack_start(GTK_BOX(rightBox), sourceFrame, FALSE, FALSE, 0);
    
    GtkWidget* sourceBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(sourceFrame), sourceBox);
    gtk_container_set_border_width(GTK_CONTAINER(sourceBox), 10);
    
    GtkWidget* sourceCombo = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(sourceBox), sourceCombo, TRUE, TRUE, 5);
    g_signal_connect(sourceCombo, "changed", G_CALLBACK(onSourceComboChangedCallback), this);
    
    // 保存辐射源下拉框引用
    m_sourceCombo = sourceCombo;
    
    // 新增：仿真执行时间输入框
    GtkWidget* timeFrame = gtk_frame_new("仿真参数");
    gtk_box_pack_start(GTK_BOX(rightBox), timeFrame, FALSE, FALSE, 0);
    
    GtkWidget* timeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(timeFrame), timeBox);
    gtk_container_set_border_width(GTK_CONTAINER(timeBox), 6);
    
    GtkWidget* timeLabel = gtk_label_new("仿真执行时间(s):");
    gtk_box_pack_start(GTK_BOX(timeBox), timeLabel, FALSE, FALSE, 5);
    
    GtkWidget* timeEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(timeEntry), "10"); // 默认值10秒
    gtk_box_pack_start(GTK_BOX(timeBox), timeEntry, TRUE, TRUE, 5);
    
    // 保存时间输入框引用
    m_timeEntry = timeEntry;
    
    // 开始按钮
    GtkWidget* startButton = gtk_button_new_with_label("开始");
    gtk_widget_set_size_request(startButton, -1, 40);
    gtk_box_pack_start(GTK_BOX(rightBox), startButton, FALSE, FALSE, 10);
    g_signal_connect(startButton, "clicked", G_CALLBACK(onSinglePlatformSimulationCallback), this);
    
    // CSS样式，使按钮更美观
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "button {\n"
        "  background-image: linear-gradient(to bottom, #3498db, #2980b9);\n"
        "  color: white;\n"
        "  border-radius: 5px;\n"
        "  font-weight: bold;\n"
        "}\n"
        "button:hover {\n"
        "  background-image: linear-gradient(to bottom, #3cb0fd, #3498db);\n"
        "}\n", -1, NULL);
    
    GtkStyleContext* context = gtk_widget_get_style_context(startButton);
    gtk_style_context_add_provider(context,
                                 GTK_STYLE_PROVIDER(provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
    
    // 结果区域
    GtkWidget* resultFrame = gtk_frame_new("仿真结果");
    gtk_box_pack_start(GTK_BOX(rightBox), resultFrame, TRUE, TRUE, 0);
    
    GtkWidget* resultBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(resultFrame), resultBox);
    gtk_container_set_border_width(GTK_CONTAINER(resultBox), 10);
    
    // 创建表格
    GtkWidget* table = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(table), 5);
    gtk_grid_set_column_spacing(GTK_GRID(table), 10);
    gtk_box_pack_start(GTK_BOX(resultBox), table, TRUE, TRUE, 0);
    
    // 添加表头
    GtkWidget* dirDataLabel = gtk_label_new("测向数据");
    gtk_widget_set_halign(dirDataLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), dirDataLabel, 0, 1, 1, 1);
    
    GtkWidget* locDataLabel = gtk_label_new("定位数据");
    gtk_widget_set_halign(locDataLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), locDataLabel, 0, 2, 1, 1);
    
    // 添加结果值（初始为空）    
    GtkWidget* dirDataValue = gtk_label_new("--");
    gtk_widget_set_halign(dirDataValue, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), dirDataValue, 1, 1, 1, 1);
    
    // 保存测向数据标签引用
    m_dirDataValue = dirDataValue;
    
    GtkWidget* locDataValue = gtk_label_new("--");
    gtk_widget_set_halign(locDataValue, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), locDataValue, 1, 2, 1, 1);
    
    // 保存定位数据标签引用
    m_locDataValue = locDataValue;
    
    // 误差结果区域
    GtkWidget* errorFrame = gtk_frame_new("误差分析");
    gtk_box_pack_start(GTK_BOX(rightBox), errorFrame, TRUE, TRUE, 0);
    
    GtkWidget* errorBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(errorFrame), errorBox);
    gtk_container_set_border_width(GTK_CONTAINER(errorBox), 10);
    
    // 创建误差表格
    GtkWidget* errorTable = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(errorTable), 5);
    gtk_grid_set_column_spacing(GTK_GRID(errorTable), 10);
    gtk_box_pack_start(GTK_BOX(errorBox), errorTable, TRUE, TRUE, 0);
    
    // 保存误差表格引用
    m_errorTable = errorTable;
    
    // 初始化误差表格 - 默认显示干涉仪体制的误差项
    updateErrorTable("干涉仪体制");
    
    // 加载数据并填充下拉框
    m_devices = ReconnaissanceDeviceDAO::getInstance().getAllReconnaissanceDevices();
    m_sources = RadiationSourceDAO::getInstance().getAllRadiationSources();
    updateDeviceCombo();
    updateSourceCombo();
    
    return container;
}

void SinglePlatformView::updateDirectionData(const std::string& data) {
    gtk_label_set_text(GTK_LABEL(m_dirDataValue), data.c_str());
}

void SinglePlatformView::updateLocationData(const std::string& data) {
    gtk_label_set_text(GTK_LABEL(m_locDataValue), data.c_str());
}

void SinglePlatformView::updateErrorTable(const std::string& techSystem) {
    if (!m_errorTable) {
        return;
    }
    
    // 清空表格中的所有子控件
    GList* children = gtk_container_get_children(GTK_CONTAINER(m_errorTable));
    for (GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    // 根据技术体制添加不同的误差项
    if (techSystem == "时差体制" || techSystem.find("时差") != std::string::npos) {
        // 时差体制误差项：时延误差、通道热噪声误差、时间测量误差、时差测量误差、测向误差
        const char* errorItems[] = {
            "时延误差", "通道热噪声误差", "时间测量误差", "时差测量误差", "测向误差"
        };
        
        for (int i = 0; i < 5; i++) {
            GtkWidget* errorLabel = gtk_label_new(errorItems[i]);
            gtk_widget_set_halign(errorLabel, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(m_errorTable), errorLabel, 0, i, 1, 1);
        }
    } else if (techSystem == "干涉仪体制" || techSystem.find("干涉") != std::string::npos) {
        // 干涉仪体制误差项：对中误差、姿态测量误差、圆锥效应误差、天线阵测向误差、测向误差
        const char* errorItems[] = {
            "对中误差", "姿态测量误差", "圆锥效应误差", "天线阵测向误差", "测向误差"
        };
        
        for (int i = 0; i < 5; i++) {
            GtkWidget* errorLabel = gtk_label_new(errorItems[i]);
            gtk_widget_set_halign(errorLabel, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(m_errorTable), errorLabel, 0, i, 1, 1);
        }
    } 
    
    // 显示所有控件
    gtk_widget_show_all(m_errorTable);
}

std::string SinglePlatformView::getSelectedTechSystem() const {
    gchar* text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_algoCombo));
    std::string result(text ? text : "");
    g_free(text);
    return result;
}

std::string SinglePlatformView::getSelectedDevice() const {
    gchar* text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_radarCombo));
    std::string result(text ? text : "");
    g_free(text);
    
    // 如果设备列表为空，返回空字符串
    if (m_devices.empty()) {
        return "";
    }
    
    // 如果结果为空或"无侦察设备"，返回空字符串
    if (result.empty() || result == "无侦察设备") {
        return "";
    }
    
    // 确保选中的设备名称存在于设备列表中
    for (const auto& device : m_devices) {
        if (device.getDeviceName() == result) {
            return result;
        }
    }
    
    // 如果没有匹配的设备，返回第一个设备的名称
    if (!m_devices.empty()) {
        return m_devices[0].getDeviceName();
    }
    
    return "";
}

std::string SinglePlatformView::getSelectedSource() const {
    gchar* text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_sourceCombo));
    std::string result(text ? text : "");
    g_free(text);
    return result;
}

int SinglePlatformView::getSimulationTime() const {
    const gchar* text = gtk_entry_get_text(GTK_ENTRY(m_timeEntry));
    return text ? atoi(text) : 0;
}

GtkWidget* SinglePlatformView::getView() const {
    return m_view;
}

GtkWidget* SinglePlatformView::getErrorTable() const {
    return m_errorTable;
}

// 更新侦察设备下拉列表
void SinglePlatformView::updateDeviceList(const std::vector<ReconnaissanceDevice>& devices) {
    // 保存设备数据
    m_devices = devices;
    
    // 更新下拉框
    updateDeviceCombo();
}

// 更新侦察设备下拉框内容
void SinglePlatformView::updateDeviceCombo() {
    if (!m_radarCombo) {
        return;
    }
    
    // 清空下拉框
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(m_radarCombo));
    
    // 如果没有设备，添加提示信息
    if (m_devices.empty()) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_radarCombo), "无侦察设备");
        gtk_combo_box_set_active(GTK_COMBO_BOX(m_radarCombo), 0);
        return;
    }
    
    // 添加设备列表 - 只能选择移动设备
    for (const auto& dev : m_devices) {
        // 只显示移动设备
        if (dev.getIsStationary()) continue;
        
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_radarCombo), dev.getDeviceName().c_str());
    }
    
    // 默认选择第一项
    gtk_combo_box_set_active(GTK_COMBO_BOX(m_radarCombo), 0);
    
    // 更新地图标记
    updateRadarMarker();
}

// 更新辐射源下拉框内容
void SinglePlatformView::updateSourceCombo() {
    if (!m_sourceCombo) {
        return;
    }
    
    // 清空下拉框
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(m_sourceCombo));
    
    // 如果没有辐射源，添加提示信息
    if (m_sources.empty()) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_sourceCombo), "无辐射源");
        gtk_combo_box_set_active(GTK_COMBO_BOX(m_sourceCombo), 0);
        return;
    }
    
    // 添加辐射源列表 - 只能选择固定辐射源
    for (const auto& src : m_sources) {
        // 只显示固定辐射源
        if (!src.getIsStationary()) continue;
        
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_sourceCombo), src.getRadiationName().c_str());
    }
    
    // 默认选择第一项
    gtk_combo_box_set_active(GTK_COMBO_BOX(m_sourceCombo), 0);
    
    // 更新地图标记
    updateSourceMarker();
}

// 更新侦察设备地图标记
void SinglePlatformView::updateRadarMarker() {
    if (!m_mapView) return;
    
    // 先移除旧标记
    if (m_radarMarker != -1) {
        m_mapView->removeMarker(m_radarMarker);
        m_radarMarker = -1;
    }
    
    // 获取当前选中的设备名称
    gchar* name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_radarCombo));
    if (!name) return;
    
    std::string selName = name;
    g_free(name);
    
    // 查找对应的设备并添加标记
    for (const auto& dev : m_devices) {
        if (dev.getDeviceName() == selName) {
            m_radarMarker = m_mapView->addMarker(dev.getLongitude(), dev.getLatitude(), selName, "", "red");
            break;
        }
    }
}

// 更新辐射源地图标记
void SinglePlatformView::updateSourceMarker() {
    if (!m_mapView) return;
    
    // 先移除旧标记
    if (m_sourceMarker != -1) {
        m_mapView->removeMarker(m_sourceMarker);
        m_sourceMarker = -1;
    }
    
    // 获取当前选中的辐射源名称
    gchar* name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_sourceCombo));
    if (!name) return;
    
    std::string selName = name;
    g_free(name);
    
    // 查找对应的辐射源并添加标记
    for (const auto& src : m_sources) {
        if (src.getRadiationName() == selName) {
            m_sourceMarker = m_mapView->addMarker(src.getLongitude(), src.getLatitude(), selName, "", "blue");
            break;
        }
    }
}

// 侦察设备下拉框回调
extern "C" void onDeviceComboChangedCallback(GtkWidget* widget, gpointer data) {
    // 获取SinglePlatformView实例
    SinglePlatformView* view = static_cast<SinglePlatformView*>(data);
    if (view) {
        view->updateRadarMarker();
    }
}

// 辐射源下拉框回调
extern "C" void onSourceComboChangedCallback(GtkWidget* widget, gpointer data) {
    // 获取SinglePlatformView实例
    SinglePlatformView* view = static_cast<SinglePlatformView*>(data);
    if (view) {
        view->updateSourceMarker();
    }
}

// 全局回调函数实现
extern "C" {
    void onTechSystemChangedCallback(GtkWidget* widget, gpointer data) {
        // 获取SinglePlatformView实例
        SinglePlatformView* view = static_cast<SinglePlatformView*>(data);
        if (view) {
            // 调用静态成员函数
            SinglePlatformView::onTechSystemChanged(widget, view);
        }
    }
    
    void onSinglePlatformSimulationCallback(GtkWidget* widget, gpointer data) {
        g_print("全局回调函数：开始仿真按钮被点击\n");
        
        // 获取SinglePlatformView实例
        SinglePlatformView* view = static_cast<SinglePlatformView*>(data);
        if (view) {
            // 调用静态成员函数
            SinglePlatformView::onSinglePlatformSimulation(widget, view);
        } else {
            g_print("错误：回调中获取SinglePlatformView实例失败\n");
        }
    }
}

// 静态成员函数实现
void SinglePlatformView::onTechSystemChanged(GtkWidget* widget, gpointer data) {
    // 获取SinglePlatformView实例
    SinglePlatformView* view = static_cast<SinglePlatformView*>(data);
    if (!view) return;
    
    // 获取当前选择的技术体制
    gchar* text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    std::string techSystem(text ? text : "");
    g_free(text);
    
    // 更新误差表格
    view->updateErrorTable(techSystem);
    
    // 更新设备和辐射源下拉框
    view->updateDeviceCombo();
    view->updateSourceCombo();
    
    g_print("技术体制已更改为: %s\n", techSystem.c_str());
}

void SinglePlatformView::onSinglePlatformSimulation(GtkWidget* widget, gpointer data) {
    g_print("开始按钮被点击...\n");
    
    // 获取SinglePlatformView实例
    SinglePlatformView* view = static_cast<SinglePlatformView*>(data);
    if (!view) {
        g_print("错误：无法获取SinglePlatformView实例\n");
        return;
    }
    
    g_print("调用SinglePlatformController::startSimulation()...\n");
    
    // 调用控制器启动仿真
    SinglePlatformController::getInstance().startSimulation();
    
    g_print("仿真函数调用完成\n");
}

// 显示错误信息
void SinglePlatformView::showErrorMessage(const std::string& message) {
    if (!m_view) return;
    
    GtkWidget* dialog = gtk_message_dialog_new(
        GTK_WINDOW(gtk_widget_get_toplevel(m_view)),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "%s", message.c_str());
    
    gtk_window_set_title(GTK_WINDOW(dialog), "仿真错误");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// 获取地图视图
MapView* SinglePlatformView::getMapView() const {
    return m_mapView;
}

// 设备移动动画
void SinglePlatformView::animateDeviceMovement(const ReconnaissanceDevice& device, 
                                            const std::vector<std::pair<double, double>>& trajectoryPoints, 
                                            int simulationTime) {
    if (!m_mapView || trajectoryPoints.empty()) return;
    
    // 保存当前相机视角
    std::string saveCameraScript = 
        "window.savedCameraPosition = viewer.camera.position.clone();"
        "window.savedCameraHeading = viewer.camera.heading;"
        "window.savedCameraPitch = viewer.camera.pitch;"
        "window.savedCameraRoll = viewer.camera.roll;";
    m_mapView->executeScript(saveCameraScript);
    
    // 只清除设备实体，保留辐射源标记
    std::string cleanupScript = 
        "// 移除设备实体，但保留其他标记点 "
        "if (window.deviceEntity) { "
        "  viewer.entities.remove(window.deviceEntity); "
        "  window.deviceEntity = null; "
        "} "
        "// 移除设备轨迹线 "
        "if (window.deviceTrailEntities) { "
        "  for (var i = 0; i < window.deviceTrailEntities.length; i++) { "
        "    viewer.entities.remove(window.deviceTrailEntities[i]); "
        "  } "
        "  window.deviceTrailEntities = []; "
        "} else { "
        "  window.deviceTrailEntities = []; "
        "} "
        "// 移除事件监听器 "
        "if (viewer.scene.preRender.numberOfListeners > 0) { "
        "  // 移除之前的事件监听器 "
        "  var listeners = viewer.scene.preRender._listeners; "
        "  if (listeners && listeners.length > 0) { "
        "    for (var i = listeners.length - 1; i >= 0; i--) { "
        "      viewer.scene.preRender.removeEventListener(listeners[i]); "
        "    } "
        "  } "
        "} "
        "if (viewer.clock.onTick.numberOfListeners > 0) { "
        "  var listeners = viewer.clock.onTick._listeners; "
        "  if (listeners && listeners.length > 0) { "
        "    for (var i = listeners.length - 1; i >= 0; i--) { "
        "      viewer.clock.onTick.removeEventListener(listeners[i]); "
        "    } "
        "  } "
        "}";
    m_mapView->executeScript(cleanupScript);
    
    // 获取设备高度
    double deviceAltitude = device.getAltitude();
    
    // 获取设备初始位置
    double initialLongitude = trajectoryPoints[0].first;
    double initialLatitude = trajectoryPoints[0].second;
    
    // 不使用animateDeviceMovement显示航迹线，而是使用JavaScript定时器每秒更新设备位置
    std::stringstream script;
    
    // 恢复之前保存的视角，确保不会变化
    script << "// 恢复保存的相机视角，确保不会变化"
           << "if (window.savedCameraPosition) {"
           << "  viewer.camera.setView({"
           << "    destination: window.savedCameraPosition,"
           << "    orientation: {"
           << "      heading: window.savedCameraHeading,"
           << "      pitch: window.savedCameraPitch,"
           << "      roll: window.savedCameraRoll"
           << "    }"
           << "  });"
           << "  // 禁用相机自动跟踪设备"
           << "  viewer.scene.screenSpaceCameraController.enableZoom = true;"
           << "  viewer.scene.screenSpaceCameraController.enableTilt = true;"
           << "  viewer.scene.screenSpaceCameraController.enableRotate = true;"
           << "  viewer.scene.screenSpaceCameraController.enableTranslate = true;"
           << "}"
           
           // 创建实时更新设备位置的功能
           << "window.deviceTrailEntities = window.deviceTrailEntities || [];"
           << "var currentIndex = 0;"
           << "var trajectoryPoints = [";
    
    // 添加所有轨迹点
    for (size_t i = 0; i < trajectoryPoints.size(); i++) {
        if (i > 0) script << ", ";
        script << "[" << trajectoryPoints[i].first << ", " << trajectoryPoints[i].second << "]";
    }
    
    script << "];"
           << "var updateInterval = " << (simulationTime * 1000) / (trajectoryPoints.size() - 1) << ";" // 计算更新间隔（毫秒）
           
           // 创建轨迹线
           << "var positions = [];"
           << "for (var i = 0; i < trajectoryPoints.length; i++) {"
           << "  positions.push(Cesium.Cartesian3.fromDegrees(trajectoryPoints[i][0], trajectoryPoints[i][1], " << deviceAltitude << "));"
           << "}"
           
           << "var pathEntity = viewer.entities.add({"
           << "  polyline: {"
           << "    positions: positions,"
           << "    width: 2,"
           << "    material: new Cesium.PolylineGlowMaterialProperty({"
           << "      glowPower: 0.2,"
           << "      color: Cesium.Color.YELLOW"
           << "    })"
           << "  }"
           << "});"
           << "window.deviceTrailEntities.push(pathEntity);"
           
           // 更新设备位置的函数
           << "function updateDevicePosition() {"
           << "  if (currentIndex >= trajectoryPoints.length) {"
           << "    console.log('Simulation completed');"
           << "    return;"
           << "  }"
           
           << "  var longitude = trajectoryPoints[currentIndex][0];"
           << "  var latitude = trajectoryPoints[currentIndex][1];"
           
           // 确保相机位置保持不变
           << "  if (window.savedCameraPosition) {"
           << "    viewer.camera.setView({"
           << "      destination: window.savedCameraPosition,"
           << "      orientation: {"
           << "        heading: window.savedCameraHeading,"
           << "        pitch: window.savedCameraPitch,"
           << "        roll: window.savedCameraRoll"
           << "      }"
           << "    });"
           << "  }"
           
           // 删除旧的设备实体
           << "  if (window.deviceEntity) {"
           << "    viewer.entities.remove(window.deviceEntity);"
           << "  }"
           
           // 创建新的设备实体
           << "  window.deviceEntity = viewer.entities.add({"
           << "    position: Cesium.Cartesian3.fromDegrees(longitude, latitude, " << deviceAltitude << "),"
           << "    point: {"
           << "      pixelSize: 15,"
           << "      color: Cesium.Color.RED,"
           << "      outlineColor: Cesium.Color.WHITE,"
           << "      outlineWidth: 2"
           << "    },"
           << "    label: {"
           << "      text: '" << device.getDeviceName() << "\\n高度: " << std::fixed << std::setprecision(2) << deviceAltitude 
           << "米\\n位置: ' + longitude.toFixed(6) + '°, ' + latitude.toFixed(6) + '°',"
           << "      font: '14pt sans-serif',"
           << "      style: Cesium.LabelStyle.FILL_AND_OUTLINE,"
           << "      outlineWidth: 2,"
           << "      verticalOrigin: Cesium.VerticalOrigin.BOTTOM,"
           << "      pixelOffset: new Cesium.Cartesian2(0, -20),"
           << "      showBackground: true,"
           << "      backgroundColor: new Cesium.Color(0.165, 0.165, 0.165, 0.7)"
           << "    },"
           << "    billboard: {"
           << "      image: 'data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAyNCAyNCIgd2lkdGg9IjI0IiBoZWlnaHQ9IjI0Ij48Y2lyY2xlIGN4PSIxMiIgY3k9IjEyIiByPSIxMCIgZmlsbD0iIzAwMCIvPjxjaXJjbGUgY3g9IjEyIiBjeT0iMTIiIHI9IjgiIGZpbGw9IiIgLz48L3N2Zz4=',"
           << "      width: 32,"
           << "      height: 32,"
           << "      verticalOrigin: Cesium.VerticalOrigin.BOTTOM,"
           << "      color: Cesium.Color.RED"
           << "    }"
           << "  });"
           
           // 添加垂直线连接标记点和地面
           << "  var verticalLineEntity = viewer.entities.add({"
           << "    polyline: {"
           << "      positions: [Cesium.Cartesian3.fromDegrees(longitude, latitude, 0), "
           << "                  Cesium.Cartesian3.fromDegrees(longitude, latitude, " << deviceAltitude << ")],"
           << "      width: 1,"
           << "      material: new Cesium.PolylineDashMaterialProperty({"
           << "        color: Cesium.Color.RED"
           << "      })"
           << "    }"
           << "  });"
           << "  window.deviceTrailEntities.push(verticalLineEntity);"
           
           // 更新索引并安排下一次更新
           << "  currentIndex++;"
           << "  if (currentIndex < trajectoryPoints.length) {"
           << "    setTimeout(updateDevicePosition, updateInterval);" // 使用计算的更新间隔
           << "  } else {"
           << "    console.log('Simulation completed');"
           << "    // 添加最终位置标记"
           << "    var finalLongitude = trajectoryPoints[trajectoryPoints.length - 1][0];"
           << "    var finalLatitude = trajectoryPoints[trajectoryPoints.length - 1][1];"
           << "    var finalEntity = viewer.entities.add({"
           << "      position: Cesium.Cartesian3.fromDegrees(finalLongitude, finalLatitude, " << deviceAltitude << "),"
           << "      point: {"
           << "        pixelSize: 10,"
           << "        color: Cesium.Color.GREEN,"
           << "        outlineColor: Cesium.Color.BLACK,"
           << "        outlineWidth: 2"
           << "      },"
           << "      label: {"
           << "        text: '最终位置\\n高度: " << std::fixed << std::setprecision(2) << deviceAltitude << "米',"
           << "        font: '14pt sans-serif',"
           << "        style: Cesium.LabelStyle.FILL_AND_OUTLINE,"
           << "        outlineWidth: 2,"
           << "        verticalOrigin: Cesium.VerticalOrigin.BOTTOM,"
           << "        pixelOffset: new Cesium.Cartesian2(0, -9),"
           << "        showBackground: true,"
           << "        backgroundColor: new Cesium.Color(0.165, 0.165, 0.165, 0.7)"
           << "      }"
           << "    });"
           << "    window.deviceTrailEntities.push(finalEntity);"
           << "  }"
           << "}"
           
           // 启动更新设备位置的循环
           << "updateDevicePosition();";
    
    // 执行脚本
    m_mapView->executeScript(script.str());
    
    g_print("设备移动仿真已启动，仿真时间: %d秒\n", simulationTime);
}