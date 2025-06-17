#include "../MultiPlatformView.h"
#include "../../controllers/ApplicationController.h"
#include "../../controllers/MultiPlatformController.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/RadiationSourceDAO.h"
#include <gtk/gtk.h>
#include <algorithm>

// 实现MultiPlatformView类
MultiPlatformView::MultiPlatformView() : m_view(nullptr), m_algoCombo(nullptr),  m_resultLabel(nullptr), m_errorLabel(nullptr), m_mapView(nullptr), m_sourceMarker(-1) {
    // 初始化数组
    for (int i = 0; i < 4; ++i) {
        m_radarMarkers[i] = -1;
    }
}

MultiPlatformView::~MultiPlatformView() {
}

// 创建多平台仿真UI
GtkWidget* MultiPlatformView::createView() {
    // 创建页面的主容器
    m_view = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(m_view), 15);
    
    // 左侧：地图显示区域
    GtkWidget* mapFrame = gtk_frame_new("地图");
    gtk_widget_set_size_request(mapFrame, 800, 700);
    gtk_box_pack_start(GTK_BOX(m_view), mapFrame, TRUE, TRUE, 0);
    
    // 显示地图
    m_mapView = new MapView();
    GtkWidget* mapWidget = m_mapView->create();
    gtk_container_add(GTK_CONTAINER(mapFrame), mapWidget);
    
    // 右侧：参数设置和结果区域
    GtkWidget* rightBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(m_view), rightBox, FALSE, FALSE, 0);
    
    // 技术体制选择
    GtkWidget* algoFrame = gtk_frame_new("技术体制");
    gtk_box_pack_start(GTK_BOX(rightBox), algoFrame, FALSE, FALSE, 0);
    
    GtkWidget* algoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(algoFrame), algoBox);
    gtk_container_set_border_width(GTK_CONTAINER(algoBox), 10);
    
    m_algoCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_algoCombo), "时差体制");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_algoCombo), "频差体制");
    gtk_combo_box_set_active(GTK_COMBO_BOX(m_algoCombo), 0);
    gtk_box_pack_start(GTK_BOX(algoBox), m_algoCombo, TRUE, TRUE, 5);
    g_signal_connect(m_algoCombo, "changed", G_CALLBACK(onTechSystemChangedCallback), this);
    
    // 侦察设备模型区美化：用GtkGrid 2x2布局4个Frame
    GtkWidget* radarGridFrame = gtk_frame_new("侦察设备模型");
    gtk_box_pack_start(GTK_BOX(rightBox), radarGridFrame, FALSE, FALSE, 0);
    GtkWidget* radarGrid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(radarGrid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(radarGrid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(radarGrid), 10);
    gtk_container_add(GTK_CONTAINER(radarGridFrame), radarGrid);
    for (int i = 0; i < 4; ++i) {
        char label[32];
        snprintf(label, sizeof(label), "侦察设备模型%d", i+1);
        m_radarFrame[i] = gtk_frame_new(label);
        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_container_add(GTK_CONTAINER(m_radarFrame[i]), box);
        gtk_container_set_border_width(GTK_CONTAINER(box), 5);
        m_radarCombo[i] = gtk_combo_box_text_new();
        gtk_box_pack_start(GTK_BOX(box), m_radarCombo[i], TRUE, TRUE, 5);
        // 2x2布局
        gtk_grid_attach(GTK_GRID(radarGrid), m_radarFrame[i], i%2, i/2, 1, 1);
        g_signal_connect(m_radarCombo[i], "changed", G_CALLBACK(onDeviceComboChangedCallback), this);
    }
    
    // 辐射源模型选择
    GtkWidget* sourceFrame = gtk_frame_new("辐射源模型");
    gtk_box_pack_start(GTK_BOX(rightBox), sourceFrame, FALSE, FALSE, 0);
    
    GtkWidget* sourceBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(sourceFrame), sourceBox);
    gtk_container_set_border_width(GTK_CONTAINER(sourceBox), 10);
    
    m_sourceCombo = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(sourceBox), m_sourceCombo, TRUE, TRUE, 5);
    g_signal_connect(m_sourceCombo, "changed", G_CALLBACK(onSourceComboChangedCallback), this);
    
    // 仿真执行时间输入框
    GtkWidget* timeFrame = gtk_frame_new("仿真参数");
    gtk_box_pack_start(GTK_BOX(rightBox), timeFrame, FALSE, FALSE, 0);
    
    GtkWidget* timeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(timeFrame), timeBox);
    gtk_container_set_border_width(GTK_CONTAINER(timeBox), 10);
    
    GtkWidget* timeLabel = gtk_label_new("仿真执行时间(s):");
    gtk_box_pack_start(GTK_BOX(timeBox), timeLabel, FALSE, FALSE, 5);
    
    GtkWidget* timeEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(timeEntry), "20");
    gtk_box_pack_start(GTK_BOX(timeBox), timeEntry, TRUE, TRUE, 5);
    
    // 开始按钮
    GtkWidget* startButton = gtk_button_new_with_label("开始");
    gtk_widget_set_size_request(startButton, -1, 40);
    gtk_box_pack_start(GTK_BOX(rightBox), startButton, FALSE, FALSE, 10);
    g_signal_connect(startButton, "clicked", G_CALLBACK(onStartSimulationCallback), this);
    
    // 设置按钮样式
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
    
    // 创建结果标签
    m_resultLabel = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(m_resultLabel), TRUE);
    gtk_label_set_line_wrap_mode(GTK_LABEL(m_resultLabel), PANGO_WRAP_WORD);
    gtk_label_set_justify(GTK_LABEL(m_resultLabel), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign(GTK_LABEL(m_resultLabel), 0.0);
    gtk_label_set_yalign(GTK_LABEL(m_resultLabel), 0.0);
    
    // 设置结果标签的最小高度，确保有足够空间显示所有内容
    gtk_widget_set_size_request(m_resultLabel, -1, 300);
    
    // 设置结果标签的边距
    gtk_widget_set_margin_start(m_resultLabel, 5);
    gtk_widget_set_margin_end(m_resultLabel, 5);
    gtk_widget_set_margin_top(m_resultLabel, 5);
    gtk_widget_set_margin_bottom(m_resultLabel, 5);
    
    gtk_box_pack_start(GTK_BOX(resultBox), m_resultLabel, TRUE, TRUE, 0);
    
    // 默认只显示3个侦察设备下拉框
    for (int i = 0; i < 4; ++i) {
        gtk_widget_set_visible(m_radarFrame[i], i < 3);
    }
    
    // 加载数据并填充下拉框
    m_devices = ReconnaissanceDeviceDAO::getInstance().getAllReconnaissanceDevices();
    m_sources = RadiationSourceDAO::getInstance().getAllRadiationSources();
    updateDeviceCombos();
    updateSourceCombo();
    
    // 初始化地图标记
    for (int i = 0; i < 4; ++i) updateRadarMarker(i);
    updateSourceMarker();
    
    return m_view;
}

// 获取视图控件
GtkWidget* MultiPlatformView::getView() const {
    return m_view;
}

// 技术体制切换回调
void MultiPlatformView::onTechSystemChangedCallback(GtkWidget* widget, gpointer data) {
    MultiPlatformView* self = static_cast<MultiPlatformView*>(data);
    if (self) self->onTechSystemChanged();
}

void MultiPlatformView::onTechSystemChanged() {
    int idx = gtk_combo_box_get_active(GTK_COMBO_BOX(m_algoCombo));
    // 0:时差体制，1:频差体制
    for (int i = 0; i < 4; ++i) {
        gtk_widget_set_visible(m_radarFrame[i], (idx == 0) ? (i < 4) : (i < 3));
    }
    updateDeviceCombos();
    updateSourceCombo();
}

// 刷新侦察设备下拉框内容
void MultiPlatformView::updateDeviceCombos() {
    int idx = gtk_combo_box_get_active(GTK_COMBO_BOX(m_algoCombo));
    for (int i = 0; i < 4; ++i) {
        gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(m_radarCombo[i]));
        for (const auto& dev : m_devices) {
            // 时差体制：全部只能选固定设备
            if (idx == 0 && !dev.getIsStationary()) continue;
            // 频差体制：1-3号可选全部，4号不显示
            if (idx == 1 && i >= 3) continue;
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_radarCombo[i]), dev.getDeviceName().c_str());
        }
        // 默认选中第i个（如有）
        if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_radarCombo[i])) < 0 && gtk_combo_box_get_active(GTK_COMBO_BOX(m_radarCombo[i])) != 0) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(m_radarCombo[i]), 0);
        }
        updateRadarMarker(i);
    }
}

// 刷新辐射源下拉框内容
void MultiPlatformView::updateSourceCombo() {
    int idx = gtk_combo_box_get_active(GTK_COMBO_BOX(m_algoCombo));
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(m_sourceCombo));
    for (const auto& src : m_sources) {
        // 时差体制：只能选固定
        if (idx == 0 && !src.getIsStationary()) continue;
        // 频差体制：全部可选
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_sourceCombo), src.getRadiationName().c_str());
    }
    if (gtk_combo_box_get_active(GTK_COMBO_BOX(m_sourceCombo)) < 0 && gtk_combo_box_get_active(GTK_COMBO_BOX(m_sourceCombo)) != 0) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(m_sourceCombo), 0);
    }
    updateSourceMarker();
}

// 侦察设备下拉框回调
void MultiPlatformView::onDeviceComboChangedCallback(GtkComboBox* combo, gpointer user_data) {
    MultiPlatformView* self = static_cast<MultiPlatformView*>(user_data);
    if (!self) return;
    // 查找是哪个下拉框
    for (int i = 0; i < 4; ++i) {
        if (GTK_WIDGET(combo) == self->m_radarCombo[i]) {
            self->onDeviceComboChanged(i);
            break;
        }
    }
}

void MultiPlatformView::onDeviceComboChanged(int idx) {
    updateRadarMarker(idx);
}

// 辐射源下拉框回调
void MultiPlatformView::onSourceComboChangedCallback(GtkComboBox* combo, gpointer user_data) {
    MultiPlatformView* self = static_cast<MultiPlatformView*>(user_data);
    if (self) self->onSourceComboChanged();
}

void MultiPlatformView::onSourceComboChanged() {
    updateSourceMarker();
}

// 更新单个侦察设备标记
void MultiPlatformView::updateRadarMarker(int idx) {
    if (!m_mapView) return;
    // 先移除旧标记
    if (m_radarMarkers[idx] != -1) {
        m_mapView->removeMarker(m_radarMarkers[idx]);
        m_radarMarkers[idx] = -1;
    }
    gchar* name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_radarCombo[idx]));
    if (!name) return;
    std::string selName = name;
    g_free(name);
    for (const auto& dev : m_devices) {
        if (dev.getDeviceName() == selName) {
            m_radarMarkers[idx] = m_mapView->addMarker(dev.getLongitude(), dev.getLatitude(), selName, "", "red");
            break;
        }
    }
}

// 更新辐射源标记
void MultiPlatformView::updateSourceMarker() {
    if (!m_mapView) return;
    if (m_sourceMarker != -1) {
        m_mapView->removeMarker(m_sourceMarker);
        m_sourceMarker = -1;
    }
    gchar* name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_sourceCombo));
    if (!name) return;
    std::string selName = name;
    g_free(name);
    for (const auto& src : m_sources) {
        if (src.getRadiationName() == selName) {
            m_sourceMarker = m_mapView->addMarker(src.getLongitude(), src.getLatitude(), selName, "", "blue");
            break;
        }
    }
}

// 开始仿真按钮回调
void MultiPlatformView::onStartSimulationCallback(GtkWidget* widget, gpointer data) {
    MultiPlatformView* self = static_cast<MultiPlatformView*>(data);
    if (self) self->onStartSimulation();
}

// 开始仿真处理
void MultiPlatformView::onStartSimulation() {
    // 先检查雷达侦察模型是否符合要求
    if (!checkRadarModels()) {
        return; // 如果检查不通过，直接返回
    }
    
    // 获取选中的技术体制
    gchar* systemType = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_algoCombo));
    if (!systemType) {
        GtkWidget* dialog = gtk_message_dialog_new(
            GTK_WINDOW(gtk_widget_get_toplevel(m_view)),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "请选择技术体制"
        );
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // 获取选中的辐射源
    gchar* sourceName = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_sourceCombo));
    if (!sourceName) {
        g_free(systemType);
        GtkWidget* dialog = gtk_message_dialog_new(
            GTK_WINDOW(gtk_widget_get_toplevel(m_view)),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "请选择辐射源模型"
        );
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // 获取选中的侦察设备
    std::vector<std::string> deviceNames;
    int requiredRadars = (strcmp(systemType, "时差体制") == 0) ? 4 : 3;
    for (int i = 0; i < requiredRadars; ++i) {
        gchar* deviceName = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_radarCombo[i]));
        if (deviceName) {
            deviceNames.push_back(deviceName);
            g_free(deviceName);
        }
    }
    
    // 获取仿真时间
    double simulationTime = 20.0; // 默认值
    GtkWidget* timeEntry = nullptr;
    
    // 遍历所有子控件找到时间输入框
    GtkWidget* timeFrame = gtk_widget_get_parent(m_radarFrame[0]);
    GtkWidget* timeBox = gtk_bin_get_child(GTK_BIN(timeFrame));
    if (GTK_IS_BOX(timeBox)) {
        GList* children = gtk_container_get_children(GTK_CONTAINER(timeBox));
        for (GList* l = children; l != NULL; l = l->next) {
            GtkWidget* child = GTK_WIDGET(l->data);
            if (GTK_IS_ENTRY(child)) {
                timeEntry = child;
                break;
            }
        }
        g_list_free(children);
    }
    
    if (timeEntry) {
        const char* timeStr = gtk_entry_get_text(GTK_ENTRY(timeEntry));
        if (timeStr && *timeStr) {
            try {
                simulationTime = std::stod(timeStr);
            } catch (const std::exception& e) {
                GtkWidget* dialog = gtk_message_dialog_new(
                    GTK_WINDOW(gtk_widget_get_toplevel(m_view)),
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_WARNING,
                    GTK_BUTTONS_OK,
                    "仿真时间格式无效，使用默认值20秒"
                );
                gtk_dialog_run(GTK_DIALOG(dialog));
                gtk_widget_destroy(dialog);
            }
        }
    }
    
    // 调用控制器开始仿真
    MultiPlatformController::getInstance().startSimulation(
        deviceNames,
        sourceName,
        systemType,
        simulationTime
    );
    
    // 释放内存
    g_free(systemType);
    g_free(sourceName);
}

// 检查雷达侦察模型是否有效
bool MultiPlatformView::checkRadarModels() {
    // 获取当前技术体制：0-时差体制，1-频差体制
    int techSystem = gtk_combo_box_get_active(GTK_COMBO_BOX(m_algoCombo));
    
    // 需要检查的雷达数量
    int requiredRadars = (techSystem == 0) ? 4 : 3;
    
    // 收集已选择的雷达模型名称
    std::vector<std::string> selectedRadars;
    for (int i = 0; i < requiredRadars; i++) {
        // 确保雷达框是可见的
        if (!gtk_widget_get_visible(m_radarFrame[i])) {
            continue;
        }
        
        gchar* name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_radarCombo[i]));
        if (!name) {
            // 如果未选择雷达，显示警告
            GtkWidget* dialog = gtk_message_dialog_new(
                GTK_WINDOW(gtk_widget_get_toplevel(m_view)),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK,
                "请选择所有侦察设备模型"
            );
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            return false;
        }
        
        selectedRadars.push_back(name);
        g_free(name);
    }
    
    // 检查是否有重复的雷达模型
    std::sort(selectedRadars.begin(), selectedRadars.end());
    auto it = std::unique(selectedRadars.begin(), selectedRadars.end());
    int uniqueCount = std::distance(selectedRadars.begin(), it);
    
    if (uniqueCount < requiredRadars) {
        // 如果有重复或者数量不够，显示警告
        GtkWidget* dialog = gtk_message_dialog_new(
            GTK_WINDOW(gtk_widget_get_toplevel(m_view)),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "请选择%d个不同的侦察设备模型",
            requiredRadars
        );
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return false;
    }
    
    return true;
}

// 更新仿真结果显示
void MultiPlatformView::updateResult(const std::string& result) {
    if (m_resultLabel) {
        gtk_label_set_markup(GTK_LABEL(m_resultLabel), result.c_str());
    }
} 