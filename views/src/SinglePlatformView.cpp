#include "../SinglePlatformView.h"
#include "../components/MapView.h"
#include "../../controllers/SinglePlatformController.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/RadiationSourceDAO.h"
#include <gtk/gtk.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>  // 添加数学函数支持，用于fabs函数

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
    m_trajectoryLineId(-1),
    m_hasResult(false),
    m_lastLon(0),
    m_lastLat(0),
    m_lastAlt(0),
    m_lastAz(0),
    m_lastEl(0) {
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

    // 创建方向和位置数据显示标签（用于定位和测向数据的传递）
    GtkWidget* dirDataBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(resultBox), dirDataBox, FALSE, FALSE, 0);
    GtkWidget* dirDataLabel = gtk_label_new("方向数据:");
    gtk_box_pack_start(GTK_BOX(dirDataBox), dirDataLabel, FALSE, FALSE, 0);
    m_dirDataValue = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(dirDataBox), m_dirDataValue, TRUE, TRUE, 0);
    gtk_widget_set_no_show_all(dirDataBox, TRUE);  // 默认不显示
    
    GtkWidget* locDataBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(resultBox), locDataBox, FALSE, FALSE, 0);
    GtkWidget* locDataLabel = gtk_label_new("位置数据:");
    gtk_box_pack_start(GTK_BOX(locDataBox), locDataLabel, FALSE, FALSE, 0);
    m_locDataValue = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(locDataBox), m_locDataValue, TRUE, TRUE, 0);
    gtk_widget_set_no_show_all(locDataBox, TRUE);  // 默认不显示

    // 创建纵向参数列表
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_pack_start(GTK_BOX(resultBox), grid, TRUE, TRUE, 0);

    GtkWidget* labelLon = gtk_label_new("经度:");
    gtk_widget_set_halign(labelLon, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), labelLon, 0, 0, 1, 1);
    m_resultLon = gtk_label_new("--");
    gtk_widget_set_halign(m_resultLon, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(grid), m_resultLon, 1, 0, 1, 1);

    GtkWidget* labelLat = gtk_label_new("纬度:");
    gtk_widget_set_halign(labelLat, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), labelLat, 0, 1, 1, 1);
    m_resultLat = gtk_label_new("--");
    gtk_widget_set_halign(m_resultLat, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(grid), m_resultLat, 1, 1, 1, 1);

    GtkWidget* labelAlt = gtk_label_new("高度:");
    gtk_widget_set_halign(labelAlt, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), labelAlt, 0, 2, 1, 1);
    m_resultAlt = gtk_label_new("--");
    gtk_widget_set_halign(m_resultAlt, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(grid), m_resultAlt, 1, 2, 1, 1);

    GtkWidget* labelAz = gtk_label_new("方位角:");
    gtk_widget_set_halign(labelAz, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), labelAz, 0, 3, 1, 1);
    m_resultAz = gtk_label_new("--");
    gtk_widget_set_halign(m_resultAz, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(grid), m_resultAz, 1, 3, 1, 1);

    GtkWidget* labelEl = gtk_label_new("俯仰角:");
    gtk_widget_set_halign(labelEl, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), labelEl, 0, 4, 1, 1);
    m_resultEl = gtk_label_new("--");
    gtk_widget_set_halign(m_resultEl, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(grid), m_resultEl, 1, 4, 1, 1);
    
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
            
            GtkWidget* errorValue = gtk_label_new("--");
            gtk_widget_set_halign(errorValue, GTK_ALIGN_END);
            gtk_grid_attach(GTK_GRID(m_errorTable), errorValue, 1, i, 1, 1);
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
            
            GtkWidget* errorValue = gtk_label_new("--");
            gtk_widget_set_halign(errorValue, GTK_ALIGN_END);
            gtk_grid_attach(GTK_GRID(m_errorTable), errorValue, 1, i, 1, 1);
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
    
    // 注释掉清除测向误差线的代码
    // view->clearDirectionErrorLines();
    
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
    
    g_print("开始设备移动动画，轨迹点数量: %zu\n", trajectoryPoints.size());
    
    // 获取定位数据和测向数据的文本
    const gchar* locDataStr = gtk_label_get_text(GTK_LABEL(m_locDataValue));
    const gchar* dirDataStr = gtk_label_get_text(GTK_LABEL(m_dirDataValue));
    
    // 解析定位数据（经度、纬度、高度）
    double calculatedLongitude = 0;
    double calculatedLatitude = 0;
    double calculatedAltitude = 0;
    
    // 优先使用缓存中的结果，避免解析失败
    if (m_hasResult) {
        calculatedLongitude = m_lastLon;
        calculatedLatitude = m_lastLat;
        calculatedAltitude = m_lastAlt;
        g_print("使用缓存的定位结果: 经度=%.6f°, 纬度=%.6f°, 高度=%.2fm\n", 
                calculatedLongitude, calculatedLatitude, calculatedAltitude);
    }
    // 尝试从标签解析
    else if (locDataStr && strstr(locDataStr, "经度:") != NULL) {
        if (sscanf(locDataStr, "经度: %lf°, 纬度: %lf°, 高度: %lfm", 
               &calculatedLongitude, &calculatedLatitude, &calculatedAltitude) == 3) {
            g_print("解析定位结果: 经度=%.6f°, 纬度=%.6f°, 高度=%.2fm\n", 
                    calculatedLongitude, calculatedLatitude, calculatedAltitude);
        } else {
            g_print("解析定位结果失败，格式不匹配: %s\n", locDataStr);
            // 使用辐射源位置作为默认值
            for (const auto& src : m_sources) {
                if (src.getRadiationName() == getSelectedSource()) {
                    calculatedLongitude = src.getLongitude();
                    calculatedLatitude = src.getLatitude();
                    calculatedAltitude = src.getAltitude();
                    break;
                }
            }
        }
    } else {
        // 使用辐射源位置作为默认值（这里仅用于可视化）
        for (const auto& src : m_sources) {
            if (src.getRadiationName() == getSelectedSource()) {
                calculatedLongitude = src.getLongitude();
                calculatedLatitude = src.getLatitude();
                calculatedAltitude = src.getAltitude();
                break;
            }
        }
    }
    
    // 获取辐射源的位置和名称
    double radiationSourceLongitude = 0;
    double radiationSourceLatitude = 0;
    double radiationSourceAltitude = 0;
    std::string sourceName = "辐射源";
    bool sourceFound = false;
    
    for (const auto& src : m_sources) {
        if (src.getRadiationName() == getSelectedSource()) {
            radiationSourceLongitude = src.getLongitude();
            radiationSourceLatitude = src.getLatitude();
            radiationSourceAltitude = src.getAltitude();
            sourceName = src.getRadiationName();
            sourceFound = true;
            break;
        }
    }
    
    if (!sourceFound) {
        g_print("警告：未找到辐射源位置，使用默认值\n");
    }
    
    // 使用TrajectorySimulator执行动画，并在动画结束时回调显示参数
    auto self = this;
    TrajectorySimulator::getInstance().animateDeviceMovement(
        m_mapView,
        device,
        trajectoryPoints,
        simulationTime,
        calculatedLongitude,
        calculatedLatitude,
        calculatedAltitude,
        sourceName,
        radiationSourceLongitude,
        radiationSourceLatitude,
        radiationSourceAltitude
    );
    
    // 动画结束后显示参数（直接从缓存读取）
    g_timeout_add(simulationTime * 1000 + 1200, [](gpointer data) -> gboolean {
        auto* view = static_cast<SinglePlatformView*>(data);
        double lon=0, lat=0, alt=0, az=0, el=0;
        if (view->getSimulationResult(lon, lat, alt, az, el)) {
            view->showSimulationResult(lon, lat, alt, az, el);
        }
        return G_SOURCE_REMOVE;
    }, self);
    
    // 注释掉动画结束后显示测向误差线的代码
    /*
    // 动画结束后显示测向误差线
    g_timeout_add(simulationTime * 1000 + 1500, [](gpointer data) -> gboolean {
        auto* view = static_cast<SinglePlatformView*>(data);
        
        // 获取测向误差角度 - 从控制器获取
        double errorAngle = 5.0;  // 默认值
        
        // 从控制器获取误差因素
        const std::vector<double>& errorFactors = 
            SinglePlatformController::getInstance().getLastErrorFactors();
        
        // 检查是否有误差因素数据
        if (!errorFactors.empty()) {
            // 干涉仪体制和时差体制的误差因素数组中，最后一个通常是测向误差
            errorAngle = errorFactors.back();
            g_print("从控制器获取测向误差角度：%.2f度\n", errorAngle);
        } else {
            g_print("控制器未提供误差因素数据，使用默认误差角度：%.2f度\n", errorAngle);
        }
        
        // 获取当前选中的侦察设备（这是误差线的起始点）
        gchar* deviceName = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(view->m_radarCombo));
        if (!deviceName) {
            g_print("错误：获取选中的侦察设备失败\n");
            return G_SOURCE_REMOVE;
        }
        
        std::string selDeviceName = deviceName;
        g_free(deviceName);
        
        // 查找对应的设备
        ReconnaissanceDevice device;
        bool deviceFound = false;
        
        for (const auto& dev : view->m_devices) {
            if (dev.getDeviceName() == selDeviceName) {
                device = dev;
                deviceFound = true;
                break;
            }
        }
        
        if (!deviceFound) {
            g_print("错误：未找到选中的设备\n");
            return G_SOURCE_REMOVE;
        }
        
        // 使用侦察设备当前位置作为测向误差线的起点
        double deviceLongitude = device.getLongitude();
        double deviceLatitude = device.getLatitude();
        double deviceAltitude = device.getAltitude();
        
        // 获取定位计算结果作为目标点（即辐射源的计算位置）
        double calculatedLon = 0, calculatedLat = 0, calculatedAlt = 0, calculatedAz = 0, calculatedEl = 0;
        bool hasResult = view->getSimulationResult(calculatedLon, calculatedLat, calculatedAlt, calculatedAz, calculatedEl);
        
        if (hasResult) {
            // 使用计算结果作为目标点，设备位置作为起点
            g_print("使用仿真结果作为辐射源位置：(%.6f, %.6f, %.2f)\n", 
                    calculatedLon, calculatedLat, calculatedAlt);
                    
            // 创建临时设备对象，使用当前侦察设备的位置
            ReconnaissanceDevice tempDevice;
            tempDevice.setDeviceName(device.getDeviceName());
            tempDevice.setLongitude(deviceLongitude);
            tempDevice.setLatitude(deviceLatitude);
            tempDevice.setAltitude(deviceAltitude);
            
            // 显示测向误差线 - 从侦察设备位置到计算的辐射源位置
            view->showDirectionErrorLines(errorAngle, 
                                         deviceLongitude, deviceLatitude, deviceAltitude,
                                         calculatedLon, calculatedLat, calculatedAlt);
        } else {
            // 如果没有计算结果，使用实际辐射源位置作为目标点
            // 获取当前选中的辐射源
            gchar* sourceName = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(view->m_sourceCombo));
            if (!sourceName) {
                g_print("错误：获取选中的辐射源失败\n");
                return G_SOURCE_REMOVE;
            }
            
            std::string selSourceName = sourceName;
            g_free(sourceName);
            
            // 查找对应的辐射源
            RadiationSource source;
            bool sourceFound = false;
            
            for (const auto& src : view->m_sources) {
                if (src.getRadiationName() == selSourceName) {
                    source = src;
                    sourceFound = true;
                    break;
                }
            }
            
            if (!sourceFound) {
                g_print("错误：未找到选中的辐射源\n");
                return G_SOURCE_REMOVE;
            }
            
            // 使用实际辐射源位置作为目标点
            double sourceLongitude = source.getLongitude();
            double sourceLatitude = source.getLatitude();
            double sourceAltitude = source.getAltitude();
            
            g_print("使用实际辐射源位置：(%.6f, %.6f, %.2f)\n", 
                    sourceLongitude, sourceLatitude, sourceAltitude);
                    
            // 显示测向误差线 - 从侦察设备位置到实际辐射源位置
            view->showDirectionErrorLines(errorAngle,
                                         deviceLongitude, deviceLatitude, deviceAltitude,
                                         sourceLongitude, sourceLatitude, sourceAltitude);
        }
        
        return G_SOURCE_REMOVE;
    }, self);
    */
}

void SinglePlatformView::showSimulationResult(double lon, double lat, double alt, double az, double el) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%.6f°", lon);
    gtk_label_set_text(GTK_LABEL(m_resultLon), buf);
    snprintf(buf, sizeof(buf), "%.6f°", lat);
    gtk_label_set_text(GTK_LABEL(m_resultLat), buf);
    snprintf(buf, sizeof(buf), "%.2fm", alt);
    gtk_label_set_text(GTK_LABEL(m_resultAlt), buf);
    snprintf(buf, sizeof(buf), "%.2f°", az);
    gtk_label_set_text(GTK_LABEL(m_resultAz), buf);
    snprintf(buf, sizeof(buf), "%.2f°", el);
    gtk_label_set_text(GTK_LABEL(m_resultEl), buf);
}

void SinglePlatformView::clearSimulationResult() {
    gtk_label_set_text(GTK_LABEL(m_resultLon), "--");
    gtk_label_set_text(GTK_LABEL(m_resultLat), "--");
    gtk_label_set_text(GTK_LABEL(m_resultAlt), "--");
    gtk_label_set_text(GTK_LABEL(m_resultAz), "--");
    gtk_label_set_text(GTK_LABEL(m_resultEl), "--");
    
    // 清空误差分析表格中的结果值
    if (m_errorTable) {
        // 获取当前技术体制
        std::string techSystem = getSelectedTechSystem();
        
        // 重新初始化误差表格 - 仅显示标签，不显示数值
        updateErrorTable(techSystem);
    }
}

void SinglePlatformView::setSimulationResult(double lon, double lat, double alt, double az, double el) {
    m_lastLon = lon;
    m_lastLat = lat;
    m_lastAlt = alt;
    m_lastAz = az;
    m_lastEl = el;
    m_hasResult = true;
}

bool SinglePlatformView::getSimulationResult(double& lon, double& lat, double& alt, double& az, double& el) {
    if (!m_hasResult) return false;
    lon = m_lastLon;
    lat = m_lastLat;
    alt = m_lastAlt;
    az = m_lastAz;
    el = m_lastEl;
    return true;
}

// 从指定位置显示测向误差线
void SinglePlatformView::showDirectionErrorLines(
    double errorAngle, 
    double deviceLongitude, double deviceLatitude, double deviceAltitude,
    double targetLongitude, double targetLatitude, double targetAltitude) {
    
    if (!m_mapView) {
        g_print("错误：地图视图为空，无法显示测向误差线\n");
        return;
    }
    
    g_print("显示测向误差线：设备位置(%.6f, %.6f, %.2f), 目标位置(%.6f, %.6f, %.2f), 误差角度=%.2f度\n",
            deviceLongitude, deviceLatitude, deviceAltitude,
            targetLongitude, targetLatitude, targetAltitude,
            errorAngle);
    
    // 创建临时设备对象，使用指定的侦察设备位置
    ReconnaissanceDevice tempDevice;
    tempDevice.setDeviceName("侦察设备");
    tempDevice.setLongitude(deviceLongitude);
    tempDevice.setLatitude(deviceLatitude);
    tempDevice.setAltitude(deviceAltitude);
    
    // 显示测向误差线，不显示误差角度文本
    bool success = m_directionErrorLines.showDirectionErrorLines(
        m_mapView,
        tempDevice,
        targetLongitude,
        targetLatitude,
        targetAltitude,
        errorAngle,
        "#FF0000",  // 红色
        20000.0     // 20km
    );
    
    if (!success) {
        g_print("错误：显示测向误差线失败\n");
    } else {
        g_print("成功显示测向误差线\n");
    }
}

// 清除测向误差线
void SinglePlatformView::clearDirectionErrorLines() {
    if (!m_mapView) return;
    m_directionErrorLines.clearDirectionErrorLines(m_mapView);
}