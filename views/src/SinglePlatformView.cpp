#include "../SinglePlatformView.h"
#include "../components/MapView.h"
#include "../../controllers/SinglePlatformController.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/RadiationSourceDAO.h"
#include <gtk/gtk.h>
#include <string>

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
    m_sourceMarker(-1) {
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
    } else {
        // 默认情况，显示通用误差项
        const char* errorItems[] = {
            "测量误差", "系统误差", "随机误差", "定位误差", "测向误差"
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
        // 获取SinglePlatformView实例
        SinglePlatformView* view = static_cast<SinglePlatformView*>(data);
        if (view) {
            // 调用静态成员函数
            SinglePlatformView::onSinglePlatformSimulation(widget, view);
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
    // 获取SinglePlatformView实例
    SinglePlatformView* view = static_cast<SinglePlatformView*>(data);
    if (!view) return;
    
    // 调用控制器启动仿真
    SinglePlatformController::getInstance().startSimulation();
}