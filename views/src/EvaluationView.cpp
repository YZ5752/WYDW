#include "../EvaluationView.h"
#include "../../controllers/ApplicationController.h"
#include <gtk/gtk.h>

// 实现EvaluationView类
EvaluationView::EvaluationView() : m_view(nullptr), m_modeCombo(nullptr), m_metricsCheckList(nullptr), m_resultsTable(nullptr), m_chartArea(nullptr) {
}

EvaluationView::~EvaluationView() {
}

// 创建评估UI
GtkWidget* EvaluationView::createView() {
    // 创建页面的主容器
    m_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(m_view), 15);
    
    // 标题
    GtkWidget* titleLabel = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(titleLabel), "<span font='16' weight='bold'>协同定位评估</span>");
    gtk_box_pack_start(GTK_BOX(m_view), titleLabel, FALSE, FALSE, 5);
    
    // 评估参数区域
    GtkWidget* paramFrame = gtk_frame_new("评估参数");
    gtk_box_pack_start(GTK_BOX(m_view), paramFrame, FALSE, FALSE, 0);
    
    GtkWidget* paramBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_container_add(GTK_CONTAINER(paramFrame), paramBox);
    gtk_container_set_border_width(GTK_CONTAINER(paramBox), 10);
    
    // 辐射源选择
    GtkWidget* targetBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(paramBox), targetBox, TRUE, TRUE, 0);
    
    GtkWidget* targetLabel = gtk_label_new("辐射源模型");
    gtk_widget_set_halign(targetLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(targetBox), targetLabel, FALSE, FALSE, 0);
    
    GtkWidget* targetCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(targetCombo), "辐射源1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(targetCombo), "辐射源2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(targetCombo), 0);
    gtk_box_pack_start(GTK_BOX(targetBox), targetCombo, TRUE, TRUE, 5);
    
    // 评估类型选择
    GtkWidget* typeBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(paramBox), typeBox, TRUE, TRUE, 0);
    
    GtkWidget* typeLabel = gtk_label_new("评估类型");
    gtk_widget_set_halign(typeLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(typeBox), typeLabel, FALSE, FALSE, 0);
    
    // 单平台选项
    GtkWidget* singleRadio = gtk_radio_button_new_with_label(NULL, "单平台");
    gtk_box_pack_start(GTK_BOX(typeBox), singleRadio, FALSE, FALSE, 0);
    
    // 多平台选项
    GtkWidget* multiRadio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(singleRadio), "多平台");
    gtk_box_pack_start(GTK_BOX(typeBox), multiRadio, FALSE, FALSE, 0);
    
    // 开始评估按钮
    GtkWidget* startButton = gtk_button_new_with_label("开始评估");
    gtk_widget_set_size_request(startButton, 100, 35);
    gtk_box_pack_start(GTK_BOX(paramBox), startButton, FALSE, FALSE, 0);
    
    // 设置按钮样式
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "button {\n"
        "  background-image: linear-gradient(to bottom, #9b59b6, #8e44ad);\n"
        "  color: white;\n"
        "  border-radius: 5px;\n"
        "  font-weight: bold;\n"
        "}\n"
        "button:hover {\n"
        "  background-image: linear-gradient(to bottom, #a66bbe, #9b59b6);\n"
        "}\n", -1, NULL);
    
    GtkStyleContext* context = gtk_widget_get_style_context(startButton);
    gtk_style_context_add_provider(context,
                                 GTK_STYLE_PROVIDER(provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
    
    // 结果区域
    GtkWidget* resultsBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(m_view), resultsBox, TRUE, TRUE, 0);
    
    // 评估结果表格
    GtkWidget* tableFrame = gtk_frame_new("评估结果");
    gtk_widget_set_size_request(tableFrame, 400, 500);
    gtk_box_pack_start(GTK_BOX(resultsBox), tableFrame, FALSE, FALSE, 0);
    
    GtkWidget* tableBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(tableFrame), tableBox);
    gtk_container_set_border_width(GTK_CONTAINER(tableBox), 10);
    
    // 创建表格
    m_resultsTable = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(m_resultsTable), 10);
    gtk_grid_set_column_spacing(GTK_GRID(m_resultsTable), 15);
    gtk_box_pack_start(GTK_BOX(tableBox), m_resultsTable, TRUE, TRUE, 0);
    
    // 表头
    GtkWidget* headerLabel1 = gtk_label_new("指标");
    gtk_widget_set_halign(headerLabel1, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(m_resultsTable), headerLabel1, 0, 0, 1, 1);
    
    GtkWidget* headerLabel2 = gtk_label_new("数值");
    gtk_widget_set_halign(headerLabel2, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(m_resultsTable), headerLabel2, 1, 0, 1, 1);
    
    // 添加指标行
    const char* metrics[] = {"最远定位距离", "定位时间", "定位精度", "测向精度"};
    for (int i = 0; i < 4; i++) {
        GtkWidget* metricLabel = gtk_label_new(metrics[i]);
        gtk_widget_set_halign(metricLabel, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(m_resultsTable), metricLabel, 0, i+1, 1, 1);
        
        GtkWidget* valueLabel = gtk_label_new("--");
        gtk_widget_set_halign(valueLabel, GTK_ALIGN_END);
        gtk_grid_attach(GTK_GRID(m_resultsTable), valueLabel, 1, i+1, 1, 1);
    }
    
    // 导出按钮
    GtkWidget* exportButton = gtk_button_new_with_label("导出结果");
    gtk_box_pack_start(GTK_BOX(tableBox), exportButton, FALSE, FALSE, 5);
    
    // 图表区域
    GtkWidget* chartFrame = gtk_frame_new("定位精度随时间变化");
    gtk_box_pack_start(GTK_BOX(resultsBox), chartFrame, TRUE, TRUE, 0);
    
    // 绘图区域
    m_chartArea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(chartFrame), m_chartArea);
    
    return m_view;
}

// 更新评估结果表格
void EvaluationView::updateResultsTable(const std::vector<std::pair<std::string, double>>& results) {
    // 实现更新评估结果表格的逻辑
}

// 显示定位精度图表
void EvaluationView::showAccuracyChart(const std::map<double, double>& data) {
    // 实现显示定位精度图表的逻辑
}

// 获取选择的评估模式
std::string EvaluationView::getSelectedMode() const {
    // 实现获取选择的评估模式的逻辑
    return "";
}

// 获取选择的评估指标
std::vector<std::string> EvaluationView::getSelectedMetrics() const {
    // 实现获取选择的评估指标的逻辑
    return std::vector<std::string>();
}

// 获取视图控件
GtkWidget* EvaluationView::getView() const {
    return m_view;
} 