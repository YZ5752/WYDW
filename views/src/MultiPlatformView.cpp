#include "../MultiPlatformView.h"
#include "../../controllers/ApplicationController.h"
#include <gtk/gtk.h>

// 实现MultiPlatformView类
MultiPlatformView::MultiPlatformView() : m_view(nullptr), m_algoCombo(nullptr), m_platformList(nullptr), m_resultLabel(nullptr), m_errorLabel(nullptr), m_chartArea(nullptr) {
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
    
    // TODO: 显示地图
    // 这里需要实现地图显示功能
    
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
    
    // 雷达设备模型选择1
    GtkWidget* radar1Frame = gtk_frame_new("侦察设备模型1");
    gtk_box_pack_start(GTK_BOX(rightBox), radar1Frame, FALSE, FALSE, 0);
    
    GtkWidget* radar1Box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(radar1Frame), radar1Box);
    gtk_container_set_border_width(GTK_CONTAINER(radar1Box), 10);
    
    GtkWidget* radar1Combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(radar1Combo), "侦察设备1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(radar1Combo), "侦察设备2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(radar1Combo), 0);
    gtk_box_pack_start(GTK_BOX(radar1Box), radar1Combo, TRUE, TRUE, 5);
    
    // 雷达设备模型选择2
    GtkWidget* radar2Frame = gtk_frame_new("侦察设备模型2");
    gtk_box_pack_start(GTK_BOX(rightBox), radar2Frame, FALSE, FALSE, 0);
    
    GtkWidget* radar2Box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(radar2Frame), radar2Box);
    gtk_container_set_border_width(GTK_CONTAINER(radar2Box), 10);
    
    GtkWidget* radar2Combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(radar2Combo), "侦察设备1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(radar2Combo), "侦察设备2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(radar2Combo), 1);
    gtk_box_pack_start(GTK_BOX(radar2Box), radar2Combo, TRUE, TRUE, 5);
    
    // 辐射源模型选择
    GtkWidget* sourceFrame = gtk_frame_new("辐射源模型");
    gtk_box_pack_start(GTK_BOX(rightBox), sourceFrame, FALSE, FALSE, 0);
    
    GtkWidget* sourceBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(sourceFrame), sourceBox);
    gtk_container_set_border_width(GTK_CONTAINER(sourceBox), 10);
    
    GtkWidget* sourceCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(sourceCombo), "辐射源1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(sourceCombo), "辐射源2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(sourceCombo), 0);
    gtk_box_pack_start(GTK_BOX(sourceBox), sourceCombo, TRUE, TRUE, 5);
    
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
    
    // 创建结果表格
    GtkWidget* table = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(table), 5);
    gtk_grid_set_column_spacing(GTK_GRID(table), 10);
    gtk_box_pack_start(GTK_BOX(resultBox), table, TRUE, TRUE, 0);
    
    // 添加表格内容
    GtkWidget* dirDataLabel = gtk_label_new("测向数据");
    gtk_widget_set_halign(dirDataLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), dirDataLabel, 0, 1, 1, 1);
    
    GtkWidget* locDataLabel = gtk_label_new("定位数据");
    gtk_widget_set_halign(locDataLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), locDataLabel, 0, 2, 1, 1);
    
    m_resultLabel = gtk_label_new("--");
    gtk_widget_set_halign(m_resultLabel, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), m_resultLabel, 1, 1, 1, 1);
    
    m_errorLabel = gtk_label_new("--");
    gtk_widget_set_halign(m_errorLabel, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), m_errorLabel, 1, 2, 1, 1);
    
    return m_view;
}

// 添加平台到列表
void MultiPlatformView::addPlatformToList(const std::string& deviceName, const Coordinate& position) {
    // 实现添加平台到列表的逻辑
}

// 删除平台从列表
void MultiPlatformView::removePlatformFromList(int index) {
    // 实现删除平台从列表的逻辑
}

// 更新定位结果显示
void MultiPlatformView::updateLocationResult(const Coordinate& position, double error) {
    // 实现更新定位结果显示的逻辑
    if (m_resultLabel && m_errorLabel) {
        char buf[128];
        snprintf(buf, sizeof(buf), "坐标: (%.2f, %.2f, %.2f)", position.x, position.y, position.z);
        gtk_label_set_text(GTK_LABEL(m_resultLabel), buf);
        
        snprintf(buf, sizeof(buf), "误差: %.2f 米", error);
        gtk_label_set_text(GTK_LABEL(m_errorLabel), buf);
    }
}

// 显示协同定位精度图表
void MultiPlatformView::showAccuracyChart(const std::vector<double>& errors) {
    // 实现显示协同定位精度图表的逻辑
}

// 获取选择的算法
std::string MultiPlatformView::getSelectedAlgorithm() const {
    if (m_algoCombo) {
        gchar* text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_algoCombo));
        if (text) {
            std::string result(text);
            g_free(text);
            return result;
        }
    }
    return "";
}

// 获取平台列表
std::vector<std::pair<std::string, Coordinate>> MultiPlatformView::getPlatformList() const {
    // 实现获取平台列表的逻辑
    return std::vector<std::pair<std::string, Coordinate>>();
}

// 获取视图控件
GtkWidget* MultiPlatformView::getView() const {
    return m_view;
} 