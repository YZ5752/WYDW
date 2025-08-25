#include "../MultiPlatformView.h"
#include "../../controllers/ApplicationController.h"
#include "../../controllers/MultiPlatformController.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/RadiationSourceDAO.h"
#include <gtk/gtk.h>
#include <algorithm>
#include <sstream>
#include <iomanip>

// 实现MultiPlatformView类
MultiPlatformView::MultiPlatformView() : m_view(nullptr), m_algoCombo(nullptr), 
    m_resultLabel(nullptr), m_timeEntry(nullptr), m_dfParamsFrame(nullptr), m_tdoaParamsFrame(nullptr), 
    m_tdoaRmsError(nullptr), m_esmToaError(nullptr), m_mapView(nullptr), m_sourceMarker(-1) {
    // 初始化数组
    for (int i = 0; i < 4; ++i) {
        m_radarMarkers[i] = -1;
    }
    for (int i = 0; i < 2; ++i) {
        m_dfMeanError[i] = nullptr;
        m_dfStdDev[i] = nullptr;
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
    
    // 右侧：创建一个固定大小的滚动窗口，容纳所有参数和结果
    GtkWidget* rightScrollWin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(rightScrollWin), 
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(rightScrollWin, 350, 700);
    gtk_box_pack_start(GTK_BOX(m_view), rightScrollWin, FALSE, FALSE, 0);
    
    // 创建右侧内容容器
    GtkWidget* rightBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(rightScrollWin), rightBox);
    
    // 定位算法选择
    GtkWidget* algoFrame = gtk_frame_new("定位算法");
    gtk_box_pack_start(GTK_BOX(rightBox), algoFrame, FALSE, FALSE, 0);
    
    GtkWidget* algoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(algoFrame), algoBox);
    gtk_container_set_border_width(GTK_CONTAINER(algoBox), 10);
    
    m_algoCombo = gtk_combo_box_text_new();
    // 修改为：测向定位、时差定位、频差定位（索引：0=测向，1=时差，2=频差）
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_algoCombo), "测向定位");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_algoCombo), "时差定位");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_algoCombo), "频差定位");
    gtk_combo_box_set_active(GTK_COMBO_BOX(m_algoCombo), 2);  // 默认选择频差定位
    gtk_box_pack_start(GTK_BOX(algoBox), m_algoCombo, TRUE, TRUE, 5);
    g_signal_connect(m_algoCombo, "changed", G_CALLBACK(onAlgorithmChangedCallback), this);
    
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
    gtk_entry_set_text(GTK_ENTRY(timeEntry), "10");
    gtk_box_pack_start(GTK_BOX(timeBox), timeEntry, TRUE, TRUE, 5);
    
    // 保存时间输入框引用
    m_timeEntry = timeEntry;
    
    // 创建测向误差参数UI - 将这部分移动到这里，在开始按钮之前
    createDFParamsUI(rightBox);
    
    // 创建TDOA误差参数UI
    createTDOAParamsUI(rightBox);
    
    // 立即隐藏测向参数UI和TDOA参数UI
    if (m_dfParamsFrame) {
        gtk_widget_set_no_show_all(m_dfParamsFrame, TRUE);
        gtk_widget_hide(m_dfParamsFrame);
    }
    
    if (m_tdoaParamsFrame) {
        gtk_widget_set_no_show_all(m_tdoaParamsFrame, TRUE);
        gtk_widget_hide(m_tdoaParamsFrame);
    }
    
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
    
    // 设置固定尺寸，防止窗口大小变化
    gtk_widget_set_size_request(resultFrame, 300, 200);
    
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
    
    // 设置结果标签的边距
    gtk_widget_set_margin_start(m_resultLabel, 5);
    gtk_widget_set_margin_end(m_resultLabel, 5);
    gtk_widget_set_margin_top(m_resultLabel, 5);
    gtk_widget_set_margin_bottom(m_resultLabel, 5);
    
    // 创建滚动窗口，用于容纳结果标签
    GtkWidget* resultScrollWin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(resultScrollWin), 
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(resultScrollWin, -1, 180);
    gtk_container_add(GTK_CONTAINER(resultScrollWin), m_resultLabel);
    gtk_box_pack_start(GTK_BOX(resultBox), resultScrollWin, TRUE, TRUE, 0);
    
    // 默认只显示3个侦察设备下拉框，并隐藏测向参数
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
void MultiPlatformView::onAlgorithmChangedCallback(GtkWidget* widget, gpointer data) {
    MultiPlatformView* self = static_cast<MultiPlatformView*>(data);
    if (self) self->onAlgorithmChanged();
}

void MultiPlatformView::onAlgorithmChanged() {
    int idx = gtk_combo_box_get_active(GTK_COMBO_BOX(m_algoCombo));
    // 0:测向定位，1:时差定位，2:频差定位
    if (idx == 1) {
        // 时差定位：显示4个侦察设备
        for (int i = 0; i < 4; ++i) {
            gtk_widget_set_visible(m_radarFrame[i], true);
        }
        // 显示TDOA误差参数UI，隐藏测向误差参数UI
        toggleDFParamsUI(false);
        toggleTDOAParamsUI(true);
    } else if (idx == 2) {
        // 频差定位：显示3个侦察设备
        for (int i = 0; i < 4; ++i) {
            gtk_widget_set_visible(m_radarFrame[i], i < 3);
        }
        // 隐藏TDOA误差参数UI和测向误差参数UI
        toggleDFParamsUI(false);
        toggleTDOAParamsUI(false);
    } else if (idx == 0) {
        // 测向定位：显示2个侦察设备
        for (int i = 0; i < 4; ++i) {
            gtk_widget_set_visible(m_radarFrame[i], i < 2);
        }
        
        // 切换到测向定位时，清除当前的选择，以确保仅显示固定设备
        for (int i = 0; i < 2; ++i) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(m_radarCombo[i]), -1);
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(m_sourceCombo), -1);
        
        // 显示测向参数UI，隐藏TDOA误差参数UI
        toggleDFParamsUI(true);
        toggleTDOAParamsUI(false);
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
            // 时差定位：全部只能选固定设备（索引1）
            if (idx == 1 && !dev.getIsStationary()) continue;
            // 频差定位：1-3号可选全部，4号不显示（索引2）
            if (idx == 2 && i >= 3) continue;
            // 测向定位：1-2号只能选固定设备，3-4号不显示（索引0）
            if (idx == 0) {
                if (i >= 2) continue;
                if (!dev.getIsStationary()) continue;
            }
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
        // 时差定位：只能选固定源（索引1）
        if (idx == 1 && !src.getIsStationary()) continue;
        // 频差定位：全部可选（索引2）
        // 测向定位：只能选固定源（索引0）
        if (idx == 0 && !src.getIsStationary()) continue;
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
    // 清除之前的测向误差线
    clearDirectionErrorLines();
    
    // 先检查雷达侦察模型是否符合要求
    if (!checkRadarModels()) {
        return; // 如果检查不通过，直接返回
    }
    
    // 获取当前定位算法：0-测向定位，1-时差定位，2-频差定位
    int techSystem = gtk_combo_box_get_active(GTK_COMBO_BOX(m_algoCombo));
    
    // 获取选中的定位算法名称
    gchar* systemType = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_algoCombo));
    if (!systemType) {
        GtkWidget* dialog = gtk_message_dialog_new(
            GTK_WINDOW(gtk_widget_get_toplevel(m_view)),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "请选择定位算法"
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
    
    // 需要检查的雷达数量
    int requiredRadars = 4; // 默认给出一个值
    
    if (techSystem == 1) {
        requiredRadars = 4; // 时差定位需要4个雷达
    } else if (techSystem == 2) {
        requiredRadars = 3; // 频差定位需要3个雷达
    } else if (techSystem == 0) {
        requiredRadars = 2; // 测向定位需要2个雷达
    }
    
    // 收集已选择的雷达模型名称
    std::vector<std::string> deviceNames;
    for (int i = 0; i < requiredRadars; ++i) {
        gchar* deviceName = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_radarCombo[i]));
        if (deviceName) {
            deviceNames.push_back(deviceName);
            g_free(deviceName);
        }
    }
    
    // 获取仿真时间
    double simulationTime = 10.0; // 默认值
    if (m_timeEntry) {
        const char* timeStr = gtk_entry_get_text(GTK_ENTRY(m_timeEntry));
        if (timeStr && *timeStr) {
            try {
                simulationTime = std::stod(timeStr);
                // 检查仿真时间是否合理
                if (simulationTime < 1.0 || simulationTime > 3600.0) {
                    GtkWidget* dialog = gtk_message_dialog_new(
                        GTK_WINDOW(gtk_widget_get_toplevel(m_view)),
                        GTK_DIALOG_MODAL,
                        GTK_MESSAGE_WARNING,
                        GTK_BUTTONS_OK,
                        "仿真时间必须在1到3600秒之间，使用默认值10秒"
                    );
                    gtk_dialog_run(GTK_DIALOG(dialog));
                    gtk_widget_destroy(dialog);
                    simulationTime = 20.0;
                }
            } catch (const std::exception& e) {
                GtkWidget* dialog = gtk_message_dialog_new(
                    GTK_WINDOW(gtk_widget_get_toplevel(m_view)),
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_WARNING,
                    GTK_BUTTONS_OK,
                    "仿真时间格式无效，使用默认值10秒"
                );
                gtk_dialog_run(GTK_DIALOG(dialog));
                gtk_widget_destroy(dialog);
            }
        }
    }
    
    if (techSystem == 1) {
        // 获取TDOA误差参数
        double tdoaRmsError = getTDOARmsError();
        double esmToaError = getESMToaError();
        
        // 检查TDOA均方根误差是否为负数
        if (tdoaRmsError < 0) {
            GtkWidget* dialog = gtk_message_dialog_new(
                GTK_WINDOW(gtk_widget_get_toplevel(m_view)),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK,
                "均方根误差不能为负值，请重新输入"
            );
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            return; // 阻止仿真继续进行
        }
        
        // 将误差参数传递给控制器
        MultiPlatformController::getInstance().startSimulation(
            deviceNames,
            sourceName,
            systemType,
            simulationTime
        );
        
        // 在调用完成后设置全局误差参数供控制器使用
        MultiPlatformController::getInstance().setTDOAErrorParams(tdoaRmsError, esmToaError);
    } else {
        // 调用控制器开始仿真
        MultiPlatformController::getInstance().startSimulation(
            deviceNames,
            sourceName,
            systemType,
            simulationTime
        );
    }
    
    // 释放内存
    g_free(systemType);
    g_free(sourceName);
}

// 检查雷达侦察模型是否有效
bool MultiPlatformView::checkRadarModels() {
    // 获取当前定位算法：0-测向定位，1-时差定位，2-频差定位
    int techSystem = gtk_combo_box_get_active(GTK_COMBO_BOX(m_algoCombo));
    
    // 需要检查的雷达数量
    int requiredRadars = 4; // 默认值
    
    if (techSystem == 1) {
        requiredRadars = 4; // 时差定位需要4个雷达
    } else if (techSystem == 2) {
        requiredRadars = 3; // 频差定位需要3个雷达
    } else if (techSystem == 0) {
        requiredRadars = 2; // 测向定位需要2个雷达
    }
    
    // 收集已选择的雷达模型名称
    std::vector<std::string> deviceNames;
    for (int i = 0; i < requiredRadars; ++i) {
        gchar* deviceName = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_radarCombo[i]));
        if (!deviceName) {
            GtkWidget* dialog = gtk_message_dialog_new(
                GTK_WINDOW(gtk_widget_get_toplevel(m_view)),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK,
                "请选择所有必要的侦察设备 (%d/%d)", i+1, requiredRadars
            );
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            return false;
        }
        
        std::string name = deviceName;
        g_free(deviceName);
        deviceNames.push_back(name);
    }
    
    // 检查是否有重复的雷达模型
    std::sort(deviceNames.begin(), deviceNames.end());
    auto it = std::unique(deviceNames.begin(), deviceNames.end());
    int uniqueCount = std::distance(deviceNames.begin(), it);
    
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
        // 更新结果标签
        gtk_label_set_markup(GTK_LABEL(m_resultLabel), result.c_str());
        
        // 确保滚动到顶部显示最新结果
        GtkWidget* parent = gtk_widget_get_parent(m_resultLabel);
        if (GTK_IS_SCROLLED_WINDOW(parent)) {
            GtkAdjustment* adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(parent));
            gtk_adjustment_set_value(adj, 0);
        }
    }
}

// 更新误差显示 - 由于已经删除误差标签，这个函数现在可以留空或者合并到updateResult中
void MultiPlatformView::updateError(const std::string& error) {
    // 不再需要单独的误差显示
}

// 显示测向误差线 - 从设备到目标位置，带误差角度
void MultiPlatformView::showDirectionErrorLines(int deviceIndex, 
                                             double targetLongitude, double targetLatitude, double targetAltitude,
                                             double errorAngle,
                                             const std::string& lineColor) {
    if (!m_mapView || deviceIndex < 0 || deviceIndex >= 4) {
        g_print("错误：无效的设备索引 %d 或地图视图为空\n", deviceIndex);
        return;
    }

    // 获取选中的设备
    gchar* deviceName = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_radarCombo[deviceIndex]));
    if (!deviceName) {
        g_print("错误：设备 %d 未选择\n", deviceIndex);
        return;
    }
    
    std::string selDeviceName = deviceName;
    g_free(deviceName);
    
    // 查找设备
    ReconnaissanceDevice device;
    bool deviceFound = false;
    
    for (const auto& dev : m_devices) {
        if (dev.getDeviceName() == selDeviceName) {
            device = dev;
            deviceFound = true;
            break;
        }
    }
    
    if (!deviceFound) {
        g_print("错误：未找到设备 %s\n", selDeviceName.c_str());
        return;
    }
    
    g_print("显示设备 %s 的测向误差线，目标位置: (%.6f, %.6f, %.2f)，误差角度: %.2f°\n",
            selDeviceName.c_str(), targetLongitude, targetLatitude, targetAltitude, errorAngle);
    
    // 显示测向误差线
    bool success = m_directionErrorLines.showDirectionErrorLines(
        m_mapView,
        device,
        targetLongitude,
        targetLatitude,
        targetAltitude,
        errorAngle,
        lineColor,
        20000.0 // 20km长度
    );
    
    if (!success) {
        g_print("错误：显示测向误差线失败\n");
    } else {
        g_print("成功显示设备 %s 的测向误差线\n", selDeviceName.c_str());
    }
}

// 显示多设备测向误差线 - 从所有设备到目标位置
void MultiPlatformView::showMultipleDeviceErrorLines(const std::vector<int>& deviceIndices,
                                                  double targetLongitude, double targetLatitude, double targetAltitude,
                                                  double errorAngle) {
    if (!m_mapView || deviceIndices.empty()) {
        g_print("错误：设备索引为空或地图视图为空\n");
        return;
    }
    
    // 清除之前的误差线
    clearDirectionErrorLines();
    
    // 为不同设备使用不同颜色
    const std::string colors[] = {"#FF0000", "#00FF00", "#0000FF", "#FF00FF"};
    
    // 为每个设备显示测向误差线
    for (size_t i = 0; i < deviceIndices.size(); ++i) {
        int deviceIndex = deviceIndices[i];
        if (deviceIndex >= 0 && deviceIndex < 4) {
            showDirectionErrorLines(
                deviceIndex,
                targetLongitude, targetLatitude, targetAltitude,
                errorAngle,
                colors[i % 4] // 循环使用颜色
            );
        }
    }
}

// 清除测向误差线
void MultiPlatformView::clearDirectionErrorLines() {
    if (!m_mapView) return;
    m_directionErrorLines.clearDirectionErrorLines(m_mapView);
    // 兼容性清理：确保多设备(df-*)与旧容器残留被移除
    std::stringstream deepClear;
    deepClear
        << "try {\n"
        << "  var list = viewer.entities.values;\n"
        << "  for (var i = list.length - 1; i >= 0; i--) {\n"
        << "    var e = list[i];\n"
        << "    var id = e.id ? ('' + e.id) : '';\n"
        << "    if (id.indexOf('df-') === 0) { viewer.entities.remove(e); }\n"
        << "  }\n"
        << "  list = viewer.entities.values;\n"
        << "  for (var j = list.length - 1; j >= 0; j--) {\n"
        << "    var e2 = list[j];\n"
        << "    var pid = (e2.parent && e2.parent.id) ? ('' + e2.parent.id) : '';\n"
        << "    var nm  = e2.name ? ('' + e2.name) : '';\n"
        << "    var pnm = (e2.parent && e2.parent.name) ? ('' + e2.parent.name) : '';\n"
        << "    if (pid.indexOf('df-') === 0 || nm.indexOf('测向误差线') !== -1 || pnm.indexOf('测向误差线') !== -1) {\n"
        << "      viewer.entities.remove(e2);\n"
        << "    }\n"
        << "  }\n"
        << "  var legacy = viewer.entities.getById('direction-error-lines');\n"
        << "  if (legacy) viewer.entities.removeById('direction-error-lines');\n"
        << "} catch (e) { console.warn('mp clear df lines failed', e); }";
    m_mapView->executeScript(deepClear.str());
   
   // 清除误差点和误差圆：这些实体没有ID，需要特殊处理
   std::stringstream clearErrorEntities;
   clearErrorEntities
       << "try {\n"
       << "  console.log('开始强力清理误差实体...');\n"
       << "  var list = viewer.entities.values;\n"
       << "  var removedCount = 0;\n"
       << "  \n"
       << "  // 第一轮：清除所有红色小点（误差点）\n"
       << "  for (var i = list.length - 1; i >= 0; i--) {\n"
       << "    var e = list[i];\n"
       << "    if (e.point && e.point.color && e.point.color.equals(Cesium.Color.RED)) {\n"
       << "      viewer.entities.remove(e);\n"
       << "      removedCount++;\n"
       << "    }\n"
       << "  }\n"
       << "  console.log('第一轮清除红色点完成，移除数量: ' + removedCount);\n"
       << "  \n"
       << "  // 第二轮：清除所有椭圆实体（误差圆）\n"
       << "  list = viewer.entities.values;\n"
       << "  removedCount = 0;\n"
       << "  for (var j = list.length - 1; j >= 0; j--) {\n"
       << "    var e2 = list[j];\n"
       << "    if (e2.ellipse) {\n"
       << "      viewer.entities.remove(e2);\n"
       << "      removedCount++;\n"
       << "    }\n"
       << "  }\n"
       << "  console.log('第二轮清除椭圆完成，移除数量: ' + removedCount);\n"
       << "  \n"
       << "  // 第三轮：清除所有没有ID的实体（兜底清理）\n"
       << "  list = viewer.entities.values;\n"
       << "  removedCount = 0;\n"
       << "  for (var k = list.length - 1; k >= 0; k--) {\n"
       << "    var e3 = list[k];\n"
       << "    if (!e3.id || e3.id === '') {\n"
       << "      viewer.entities.remove(e3);\n"
       << "      removedCount++;\n"
       << "    }\n"
       << "  }\n"
       << "  console.log('第三轮清除无ID实体完成，移除数量: ' + removedCount);\n"
       << "  console.log('误差实体强力清理完成！');\n"
       << "} catch (e) { \n"
       << "  console.error('误差实体清理失败:', e); \n"
       << "  // 兜底方案：强制清除所有实体\n"
       << "  try {\n"
       << "    viewer.entities.removeAll();\n"
       << "    console.log('兜底方案：已清除所有实体');\n"
       << "  } catch (e2) {\n"
       << "    console.error('兜底方案也失败了:', e2);\n"
       << "  }\n"
       << "}";
   m_mapView->executeScript(clearErrorEntities.str());
}

// 创建测向误差参数UI
void MultiPlatformView::createDFParamsUI(GtkWidget* parent) {
    // 创建框架
    m_dfParamsFrame = gtk_frame_new("测向误差参数");
    gtk_box_pack_start(GTK_BOX(parent), m_dfParamsFrame, FALSE, FALSE, 0);
    
    // 创建网格布局
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(m_dfParamsFrame), grid);
    
    // 添加标题行
    GtkWidget* deviceLabel = gtk_label_new("侦察设备");
    GtkWidget* meanErrorLabel = gtk_label_new("均值误差");
    GtkWidget* stdDevLabel = gtk_label_new("标准差");
    
    // 设置标签对齐方式
    gtk_label_set_xalign(GTK_LABEL(deviceLabel), 0.0);  // 左对齐
    gtk_label_set_xalign(GTK_LABEL(meanErrorLabel), 0.5);  // 居中对齐
    gtk_label_set_xalign(GTK_LABEL(stdDevLabel), 0.5);  // 居中对齐
    
    gtk_grid_attach(GTK_GRID(grid), deviceLabel, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), meanErrorLabel, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), stdDevLabel, 2, 0, 1, 1);
    
    // 为两个设备添加输入行
    for (int i = 0; i < 2; i++) {
        char label[32];
        snprintf(label, sizeof(label), "模型%d", i+1);
        GtkWidget* deviceNameLabel = gtk_label_new(label);
        gtk_label_set_xalign(GTK_LABEL(deviceNameLabel), 0.0);  // 左对齐
        
        // 创建均值误差输入框
        m_dfMeanError[i] = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(m_dfMeanError[i]), 8);
        
        // 创建标准差输入框
        m_dfStdDev[i] = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(m_dfStdDev[i]), 8);
        
        // 将控件添加到网格
        gtk_grid_attach(GTK_GRID(grid), deviceNameLabel, 0, i+1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), m_dfMeanError[i], 1, i+1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), m_dfStdDev[i], 2, i+1, 1, 1);
    }
    
    // 使用gtk_widget_set_no_show_all确保参数框默认不显示
    gtk_widget_set_no_show_all(m_dfParamsFrame, TRUE);
    gtk_widget_hide(m_dfParamsFrame);
}

// 显示/隐藏测向误差参数UI
void MultiPlatformView::toggleDFParamsUI(bool show) {
    if (m_dfParamsFrame) {
        if (show) {
            // 当选择测向定位时，设置默认值并显示
            if (m_dfMeanError[0]) gtk_entry_set_text(GTK_ENTRY(m_dfMeanError[0]), "0");
            if (m_dfMeanError[1]) gtk_entry_set_text(GTK_ENTRY(m_dfMeanError[1]), "0");
            if (m_dfStdDev[0]) gtk_entry_set_text(GTK_ENTRY(m_dfStdDev[0]), "0");
            if (m_dfStdDev[1]) gtk_entry_set_text(GTK_ENTRY(m_dfStdDev[1]), "0");
            
            gtk_widget_set_no_show_all(m_dfParamsFrame, FALSE);
            gtk_widget_show_all(m_dfParamsFrame);
        } else {
            // 隐藏参数框
            gtk_widget_hide(m_dfParamsFrame);
            gtk_widget_set_no_show_all(m_dfParamsFrame, TRUE);
        }
    }
}

// 获取测向误差参数
double MultiPlatformView::getDFMeanError(int deviceIndex) const {
    if (deviceIndex >= 0 && deviceIndex < 2 && m_dfMeanError[deviceIndex]) {
        const char* text = gtk_entry_get_text(GTK_ENTRY(m_dfMeanError[deviceIndex]));
        try {
            return std::stod(text);
        } catch (const std::exception& e) {
            // 在异常情况下返回0
            return 0.0;
        }
    }
    return 0.0;
}

double MultiPlatformView::getDFStdDev(int deviceIndex) const {
    if (deviceIndex >= 0 && deviceIndex < 2 && m_dfStdDev[deviceIndex]) {
        const char* text = gtk_entry_get_text(GTK_ENTRY(m_dfStdDev[deviceIndex]));
        try {
            return std::stod(text);
        } catch (const std::exception& e) {
            // 在异常情况下返回0
            return 0.0;
        }
    }
    return 0.0;
}

// 创建TDOA误差参数UI
void MultiPlatformView::createTDOAParamsUI(GtkWidget* parent) {
    // 创建框架
    m_tdoaParamsFrame = gtk_frame_new("时差定位误差参数");
    gtk_box_pack_start(GTK_BOX(parent), m_tdoaParamsFrame, FALSE, FALSE, 0);
    
    // 创建网格布局
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(m_tdoaParamsFrame), grid);
    
    // 添加参数标签和输入框
    GtkWidget* tdoaRmsLabel = gtk_label_new("均方根误差(us):");
    gtk_label_set_xalign(GTK_LABEL(tdoaRmsLabel), 0.0);  // 左对齐
    
    m_tdoaRmsError = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(m_tdoaRmsError), 10);
    gtk_entry_set_text(GTK_ENTRY(m_tdoaRmsError), "0");  
    
    GtkWidget* esmToaLabel = gtk_label_new("TOA误差(us):");
    gtk_label_set_xalign(GTK_LABEL(esmToaLabel), 0.0);  // 左对齐
    
    m_esmToaError = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(m_esmToaError), 10);
    gtk_entry_set_text(GTK_ENTRY(m_esmToaError), "0"); 
    // 将控件添加到网格
    gtk_grid_attach(GTK_GRID(grid), tdoaRmsLabel, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), m_tdoaRmsError, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), esmToaLabel, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), m_esmToaError, 1, 1, 1, 1);
    
    // 使用gtk_widget_set_no_show_all确保参数框默认不显示
    gtk_widget_set_no_show_all(m_tdoaParamsFrame, TRUE);
    gtk_widget_hide(m_tdoaParamsFrame);
}

// 显示/隐藏TDOA误差参数UI
void MultiPlatformView::toggleTDOAParamsUI(bool show) {
    if (m_tdoaParamsFrame) {
        if (show) {
            // 当选择TDOA体制时显示
            gtk_widget_set_no_show_all(m_tdoaParamsFrame, FALSE);
            gtk_widget_show_all(m_tdoaParamsFrame);
            g_print("显示TDOA参数UI\n");
        } else {
            // 隐藏参数框
            gtk_widget_hide(m_tdoaParamsFrame);
            gtk_widget_set_no_show_all(m_tdoaParamsFrame, TRUE);
        }
    }
}

// 获取TDOA rms Error参数
double MultiPlatformView::getTDOARmsError() const {
    if (m_tdoaRmsError) {
        const char* text = gtk_entry_get_text(GTK_ENTRY(m_tdoaRmsError));
        try {
            return std::stod(text) * 1.0e-6;  // 将微秒转换为秒
        } catch (const std::exception& e) {
            // 在异常情况下返回默认值
            return 0;  
        }
    }
    return 0;  // 默认值
}

// 获取ESM toa Error参数
double MultiPlatformView::getESMToaError() const {
    if (m_esmToaError) {
        const char* text = gtk_entry_get_text(GTK_ENTRY(m_esmToaError));
        try {
            return std::stod(text) * 1.0e-6;  // 将微秒转换为秒
        } catch (const std::exception& e) {
            // 在异常情况下返回默认值
            return 0;  
        }
    }
    return 0;  // 默认值
} 