#include "../EvaluationView.h"
#include "../../controllers/ApplicationController.h"
#include "../../controllers/EvaluationController.h"
#include <gtk/gtk.h>
#include <iostream>
#include <cmath>

// 添加绘图回调函数
static gboolean on_chart_draw(GtkWidget* widget, cairo_t* cr, gpointer data) {
    auto* timeData = static_cast<const std::map<double, double>*>(data);
    if (!timeData || timeData->empty()) return FALSE;
    
    // 获取绘图区域尺寸
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    
    // 设置边距
    int marginLeft = 50;
    int marginRight = 20;
    int marginTop = 20;
    int marginBottom = 50;
    
    int graphWidth = width - marginLeft - marginRight;
    int graphHeight = height - marginTop - marginBottom;
    
    // 找到数据范围
    double minTime = timeData->begin()->first;
    double maxTime = timeData->rbegin()->first;
    
    double minAccuracy = std::numeric_limits<double>::max();
    double maxAccuracy = std::numeric_limits<double>::lowest();
    
    for (const auto& pair : *timeData) {
        minAccuracy = std::min(minAccuracy, pair.second);
        maxAccuracy = std::max(maxAccuracy, pair.second);
    }
    
    // 确保Y轴范围非零，并且设置合理的范围
    if (maxAccuracy - minAccuracy < 1e-6) {
        // 如果范围太小，扩大范围以确保可视化
        double avgAccuracy = (minAccuracy + maxAccuracy) / 2.0;
        minAccuracy = avgAccuracy - 0.5; // 向下扩展0.5单位
        maxAccuracy = avgAccuracy + 0.5; // 向上扩展0.5单位
    } else {
        // 计算平均值和标准差，用于确定合理的Y轴范围
        double sum = 0.0;
        for (const auto& pair : *timeData) {
            sum += pair.second;
        }
        double mean = sum / timeData->size();
        
        double sumSquaredDiff = 0.0;
        for (const auto& pair : *timeData) {
            double diff = pair.second - mean;
            sumSquaredDiff += diff * diff;
        }
        double stdDev = sqrt(sumSquaredDiff / timeData->size());
        
        // 设置Y轴范围为平均值周围的合理范围（平均值±3个标准差）
        // 但不小于最小值和不大于最大值
        double lowerBound = std::max(minAccuracy, mean - 3 * stdDev);
        double upperBound = std::min(maxAccuracy, mean + 3 * stdDev);
        
        // 确保范围至少包含所有数据点的±10%
        double range = maxAccuracy - minAccuracy;
        if (range > 0) {
            minAccuracy = std::max(lowerBound, minAccuracy - range * 0.1);
            maxAccuracy = std::min(upperBound, maxAccuracy + range * 0.1);
        }
        
        // 确保范围足够大以便于可视化
        if (maxAccuracy - minAccuracy < 0.1) {
            double avgAccuracy = (minAccuracy + maxAccuracy) / 2.0;
            minAccuracy = avgAccuracy - 0.05;
            maxAccuracy = avgAccuracy + 0.05;
        }
    }
    
    // 确保X轴范围非零
    if (maxTime - minTime < 1e-6) {
        double avgTime = (minTime + maxTime) / 2.0;
        minTime = avgTime - 50.0; // 向左扩展50秒
        maxTime = avgTime + 50.0; // 向右扩展50秒
    } else {
        // 扩展X轴范围，增加10%的边距
        double timeRange = maxTime - minTime;
        minTime -= timeRange * 0.1;
        maxTime += timeRange * 0.1;
    }
    
    // 绘制背景
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    
    // 绘制网格线
    cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 0.5); // 浅灰色半透明
    cairo_set_line_width(cr, 0.5);
    
    // 水平网格线
    int numYGrids = 10;
    for (int i = 0; i <= numYGrids; i++) {
        double y = marginTop + graphHeight * i / numYGrids;
        cairo_move_to(cr, marginLeft, y);
        cairo_line_to(cr, width - marginRight, y);
    }
    cairo_stroke(cr);
    
    // 垂直网格线
    int numXGrids = 10;
    for (int i = 0; i <= numXGrids; i++) {
        double x = marginLeft + graphWidth * i / numXGrids;
        cairo_move_to(cr, x, marginTop);
        cairo_line_to(cr, x, height - marginBottom);
    }
    cairo_stroke(cr);
    
    // 绘制坐标轴
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1);
    
    // X轴
    cairo_move_to(cr, marginLeft, height - marginBottom);
    cairo_line_to(cr, width - marginRight, height - marginBottom);
    cairo_stroke(cr);
    
    // Y轴
    cairo_move_to(cr, marginLeft, height - marginBottom);
    cairo_line_to(cr, marginLeft, marginTop);
    cairo_stroke(cr);
    
    // 绘制标签
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);
    
    // X轴标签
    cairo_text_extents_t extents;
    const char* xLabel = "时间 (s)";
    cairo_text_extents(cr, xLabel, &extents);
    cairo_move_to(cr, width / 2 - extents.width / 2, height - marginBottom / 2);
    cairo_show_text(cr, xLabel);
    
    // Y轴标签
    const char* yLabel = "定位精度 (m)";
    cairo_text_extents(cr, yLabel, &extents);
    cairo_save(cr);
    cairo_move_to(cr, marginLeft / 3, height / 2 + extents.width / 2);
    cairo_rotate(cr, -G_PI / 2);
    cairo_show_text(cr, yLabel);
    cairo_restore(cr);
    
    // 绘制刻度
    int numXTicks = 5;
    int numYTicks = 5;
    
    // X轴刻度
    for (int i = 0; i <= numXTicks; i++) {
        double t = minTime + (maxTime - minTime) * i / numXTicks;
        double x = marginLeft + graphWidth * i / numXTicks;
        
        // 刻度线
        cairo_move_to(cr, x, height - marginBottom);
        cairo_line_to(cr, x, height - marginBottom + 5);
        cairo_stroke(cr);
        
        // 刻度值
        char tickLabel[32];
        snprintf(tickLabel, sizeof(tickLabel), "%.1f", t);
        cairo_text_extents(cr, tickLabel, &extents);
        cairo_move_to(cr, x - extents.width / 2, height - marginBottom + 20);
        cairo_show_text(cr, tickLabel);
    }
    
    // Y轴刻度
    for (int i = 0; i <= numYTicks; i++) {
        double a = minAccuracy + (maxAccuracy - minAccuracy) * (numYTicks - i) / numYTicks;
        double y = marginTop + graphHeight * i / numYTicks;
        
        // 刻度线
        cairo_move_to(cr, marginLeft, y);
        cairo_line_to(cr, marginLeft - 5, y);
        cairo_stroke(cr);
        
        // 刻度值 - 根据数值大小选择合适的格式
        char tickLabel[32];
        if (fabs(a) < 0.001) {
            snprintf(tickLabel, sizeof(tickLabel), "%.6f", a);
        } else if (fabs(a) < 1.0) {
            snprintf(tickLabel, sizeof(tickLabel), "%.4f", a);
        } else if (fabs(a) < 10.0) {
            snprintf(tickLabel, sizeof(tickLabel), "%.3f", a);
        } else {
            snprintf(tickLabel, sizeof(tickLabel), "%.2f", a);
        }
        cairo_text_extents(cr, tickLabel, &extents);
        cairo_move_to(cr, marginLeft - 10 - extents.width, y + extents.height / 2);
        cairo_show_text(cr, tickLabel);
    }
    
    // 绘制数据点和连线
    cairo_set_source_rgb(cr, 0, 0.5, 1);
    cairo_set_line_width(cr, 2);
    
    // 先绘制连线
    bool first = true;
    for (const auto& pair : *timeData) {
        double t = pair.first;
        double a = pair.second;
        
        double x = marginLeft + graphWidth * (t - minTime) / (maxTime - minTime);
        double y = marginTop + graphHeight * (1 - (a - minAccuracy) / (maxAccuracy - minAccuracy));
        
        if (first) {
            cairo_move_to(cr, x, y);
            first = false;
        } else {
            cairo_line_to(cr, x, y);
        }
    }
    cairo_stroke(cr);
    
    // 再绘制数据点
    cairo_set_source_rgb(cr, 1, 0, 0); // 红色点
    for (const auto& pair : *timeData) {
        double t = pair.first;
        double a = pair.second;
        
        double x = marginLeft + graphWidth * (t - minTime) / (maxTime - minTime);
        double y = marginTop + graphHeight * (1 - (a - minAccuracy) / (maxAccuracy - minAccuracy));
        
        // 绘制点
        cairo_arc(cr, x, y, 4, 0, 2 * G_PI);
        cairo_fill(cr);
    }
    
    return FALSE;
}

// 实现EvaluationView类
EvaluationView::EvaluationView() : m_view(nullptr), m_modeCombo(nullptr), m_metricsCheckList(nullptr), m_resultsTable(nullptr), m_chartArea(nullptr), m_targetCombo(nullptr), m_radiationSources(std::vector<RadiationSource>()) {
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
    
    // 从控制器获取辐射源数据
    m_targetCombo = gtk_combo_box_text_new();
    loadRadiationSources();
    
    gtk_box_pack_start(GTK_BOX(targetBox), m_targetCombo, TRUE, TRUE, 5);
    
    // 添加选择变化回调
    g_signal_connect(m_targetCombo, "changed", G_CALLBACK(+[](GtkComboBox* combo, gpointer data) {
        EvaluationView* view = static_cast<EvaluationView*>(data);
        view->onRadiationSourceSelected();
    }), this);
    
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
    
    // 开始评估按钮回调
    g_signal_connect(startButton, "clicked", G_CALLBACK(+[](GtkButton* button, gpointer data) {
        EvaluationView* view = static_cast<EvaluationView*>(data);
        view->startEvaluation();
    }), this);
    
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
    if (!m_resultsTable) return;
    
    // 确保表格行数与结果数量匹配
    int numRows = 0;
    GList* children = gtk_container_get_children(GTK_CONTAINER(m_resultsTable));
    if (children) {
        numRows = g_list_length(children) / 2; // 每行有两个单元格（指标名和值）
        g_list_free(children);
    }
    
    // 结果从1行开始（跳过表头）
    for (size_t i = 0; i < results.size() && i < 4; i++) {
        // 获取值单元格（位于第二列）
        GtkWidget* valueLabel = gtk_grid_get_child_at(GTK_GRID(m_resultsTable), 1, i+1);
        if (valueLabel && GTK_IS_LABEL(valueLabel)) {
            // 根据指标类型格式化显示
            const std::string& metricName = results[i].first;
            double value = results[i].second;
            
            char valueStr[64];
            if (metricName == "最远定位距离") {
                snprintf(valueStr, sizeof(valueStr), "%.2f m", value);
            } else if (metricName == "定位时间") {
                snprintf(valueStr, sizeof(valueStr), "%.2f s", value);
            } else if (metricName == "定位精度") {
                snprintf(valueStr, sizeof(valueStr), "%.6f m", value);
            } else if (metricName == "测向精度") {
                snprintf(valueStr, sizeof(valueStr), "%.6f°", value);
            } else {
                snprintf(valueStr, sizeof(valueStr), "%.2f", value);
            }
            
            gtk_label_set_text(GTK_LABEL(valueLabel), valueStr);
        }
    }
}

// 显示定位精度图表
void EvaluationView::showAccuracyChart(const std::map<double, double>& data) {
    // 实现显示定位精度图表的逻辑
    if (data.empty() || !m_chartArea) return;
    
    // 保存数据到成员变量
    m_chartData = data;
    
    // 清除之前的绘图
    g_signal_handlers_disconnect_matched(m_chartArea, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, NULL, NULL);
    
    // 连接绘图事件，使用独立的回调函数
    g_signal_connect(m_chartArea, "draw", G_CALLBACK(on_chart_draw), &m_chartData);
    
    // 触发重绘
    gtk_widget_queue_draw(m_chartArea);
}

// 获取选择的评估模式
std::string EvaluationView::getSelectedMode() const {
    // 查找单平台单选按钮
    GtkWidget* typeBox = nullptr;
    GtkWidget* paramBox = nullptr;
    GList* children = gtk_container_get_children(GTK_CONTAINER(m_view));
    
    // 找到参数框架
    for (GList* l = children; l != nullptr; l = l->next) {
        GtkWidget* child = GTK_WIDGET(l->data);
        if (GTK_IS_FRAME(child)) {
            const gchar* label = gtk_frame_get_label(GTK_FRAME(child));
            if (label && strcmp(label, "评估参数") == 0) {
                GtkWidget* frameChild = gtk_bin_get_child(GTK_BIN(child));
                if (frameChild && GTK_IS_BOX(frameChild)) {
                    paramBox = frameChild;
                    break;
                }
            }
        }
    }
    g_list_free(children);
    
    if (!paramBox) return "single_platform";  // 默认返回单平台
    
    // 找到类型框
    children = gtk_container_get_children(GTK_CONTAINER(paramBox));
    for (GList* l = children; l != nullptr; l = l->next) {
        GtkWidget* child = GTK_WIDGET(l->data);
        if (GTK_IS_BOX(child)) {
            GList* boxChildren = gtk_container_get_children(GTK_CONTAINER(child));
            GtkWidget* firstChild = boxChildren ? GTK_WIDGET(boxChildren->data) : nullptr;
            g_list_free(boxChildren);
            
            if (firstChild && GTK_IS_LABEL(firstChild)) {
                const gchar* label = gtk_label_get_text(GTK_LABEL(firstChild));
                if (label && strcmp(label, "评估类型") == 0) {
                    typeBox = child;
                    break;
                }
            }
        }
    }
    g_list_free(children);
    
    if (!typeBox) return "single_platform";  // 默认返回单平台
    
    // 找到单选按钮并检查状态
    children = gtk_container_get_children(GTK_CONTAINER(typeBox));
    
    // 跳过标签，获取第一个单选按钮（单平台）
    if (children && children->next && GTK_IS_RADIO_BUTTON(children->next->data)) {
        GtkRadioButton* singleRadio = GTK_RADIO_BUTTON(children->next->data);
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(singleRadio))) {
            g_list_free(children);
            return "single_platform";
        }
    }
    
    g_list_free(children);
    return "multi_platform";
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

// 在EvaluationView.cpp中添加回调处理方法
void EvaluationView::onRadiationSourceSelected() {
    int activeIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(m_targetCombo));
    if (activeIndex >= 0 && activeIndex < m_radiationSources.size()) {
        RadiationSource selectedSource = m_radiationSources[activeIndex];
        
        // 处理选中的辐射源，可以更新界面其他部分
        g_print("选中辐射源: %s, ID: %d\n", 
                selectedSource.getRadiationName().c_str(),
                selectedSource.getRadiationId());
                
        // 例如，可以调用控制器进行仿真计算
        // EvaluationController::getInstance().runSimulation(selectedSource);
    }
}

// 在EvaluationView类中添加
void EvaluationView::startEvaluation() {
    int activeIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(m_targetCombo));
    if (activeIndex >= 0 && activeIndex < m_radiationSources.size()) {
        RadiationSource selectedSource = m_radiationSources[activeIndex];
        int sourceId = selectedSource.getRadiationId();
        
        g_print("EvaluationView: 开始评估辐射源 ID=%d, 名称=%s\n", 
                sourceId, selectedSource.getRadiationName().c_str());
        
        // 执行评估
        // 获取评估类型（单平台/多平台）
        std::string mode = getSelectedMode();
        bool isSinglePlatform = (mode == "single_platform");
        
        g_print("EvaluationView: 评估模式: %s\n", isSinglePlatform ? "单平台" : "多平台");
        
        // 调用控制器进行评估
        std::vector<std::pair<std::string, double>> results = 
            EvaluationController::getInstance().evaluateRadiationSource(sourceId, isSinglePlatform);
        
        // 更新结果表格
        updateResultsTable(results);
        
        // 更新图表
        std::map<double, double> chartData = 
            EvaluationController::getInstance().getAccuracyTimeData(sourceId, isSinglePlatform);
        showAccuracyChart(chartData);
        
        g_print("评估已完成，共获取 %zu 个结果项和 %zu 个时间数据点\n", results.size(), chartData.size());
    } else {
        g_print("未选择有效的辐射源\n");
    }
}

// 创建辐射源下拉框的辅助方法
void EvaluationView::loadRadiationSources() {
    if (!m_targetCombo) return;
    
    // 清空当前项
    GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(m_targetCombo));
    if (model) {
        gtk_list_store_clear(GTK_LIST_STORE(model));
    }
    
    // 获取辐射源数据
    try {
        m_radiationSources = EvaluationController::getInstance().getAllRadiationSources();
        
        if (!m_radiationSources.empty()) {
            for (const auto& source : m_radiationSources) {
                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_targetCombo), 
                                              source.getRadiationName().c_str());
            }
            gtk_combo_box_set_active(GTK_COMBO_BOX(m_targetCombo), 0);
            gtk_widget_set_sensitive(m_targetCombo, TRUE);
        } else {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_targetCombo), "无辐射源数据");
            gtk_combo_box_set_active(GTK_COMBO_BOX(m_targetCombo), 0);
            gtk_widget_set_sensitive(m_targetCombo, FALSE);
        }
    } catch (const std::exception& e) {
        std::cerr << "加载辐射源时出错: " << e.what() << std::endl;
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_targetCombo), "加载失败");
        gtk_combo_box_set_active(GTK_COMBO_BOX(m_targetCombo), 0);
        gtk_widget_set_sensitive(m_targetCombo, FALSE);
    }
} 