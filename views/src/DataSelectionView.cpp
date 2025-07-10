#include "../DataSelectionView.h"
#include "../../controllers/ApplicationController.h"
#include <gtk/gtk.h>
#include "../../models/RadiationSourceDAO.h"
#include "../../models/RadiationSourceModel.h"
#include <mysql/mysql.h>
#include "../../models/DBConnector.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/ReconnaissanceDeviceModel.h"
#include "../../controllers/DataSelectionController.h"

// 添加refreshTechCombo声明，确保后续能正常调用
void refreshTechCombo(GtkComboBoxText* techCombo, bool isSingle);
// 添加refreshDeviceCombo声明
void refreshDeviceCombo(GtkComboBoxText* deviceCombo, const std::vector<ReconnaissanceDevice>& allDevices, const std::vector<ReconnaissanceDevice>& fixedDevices, const std::vector<ReconnaissanceDevice>& mobileDevices, bool isSingle, const std::string& tech);
struct ImportDialogContext {
    GtkWidget* radioSingle;
    GtkWidget* deviceLabel;
    GtkWidget* deviceCombo;
    GtkWidget* multiDeviceLabel;
    GtkWidget* multiDeviceCombo;
    std::vector<GtkWidget*>* singleRowWidgets;
    std::vector<GtkWidget*>* multiRowWidgets;
    GtkWidget* techCombo;
    GtkWidget** multiDeviceLabels;
    GtkWidget** multiDeviceCombos;
    GtkWidget* azElBox;
};

struct MultiDeviceComboContext {
    GtkWidget* multiDeviceCombo;
    GtkWidget* techCombo;
    std::vector<ReconnaissanceDevice>* allDevices;
};
// 单平台任务类型选择事件回调函数
static void on_radio_single_toggled(GtkToggleButton* btn, gpointer user_data) {
    ImportDialogContext* ctx = static_cast<ImportDialogContext*>(user_data);
    if (gtk_toggle_button_get_active(btn)) {
        gtk_widget_set_sensitive(ctx->deviceCombo, TRUE);
        gtk_widget_show(ctx->deviceLabel);
        gtk_widget_show(ctx->deviceCombo);
        gtk_widget_hide(ctx->multiDeviceLabel);
        gtk_widget_hide(ctx->multiDeviceCombo);
        for (auto w : *(ctx->singleRowWidgets)) gtk_widget_show(w);
        for (auto w : *(ctx->multiRowWidgets)) gtk_widget_hide(w);
        refreshTechCombo(GTK_COMBO_BOX_TEXT(ctx->techCombo), true);
    }
}
// 多平台任务类型选择事件回调函数
static void on_radio_multi_toggled(GtkToggleButton* btn, gpointer user_data) {
    ImportDialogContext* ctx = static_cast<ImportDialogContext*>(user_data);
    if (gtk_toggle_button_get_active(btn)) {
        gtk_widget_set_sensitive(ctx->deviceCombo, FALSE);
        gtk_widget_hide(ctx->deviceLabel);
        gtk_widget_hide(ctx->deviceCombo);
        gtk_widget_show(ctx->multiDeviceLabel);
        gtk_widget_show(ctx->multiDeviceCombo);
        for (auto w : *(ctx->singleRowWidgets)) gtk_widget_hide(w);
        for (auto w : *(ctx->multiRowWidgets)) gtk_widget_show(w);
        refreshTechCombo(GTK_COMBO_BOX_TEXT(ctx->techCombo), false);
    }
}
// 刷新技术体制下拉框
void refreshTechCombo(GtkComboBoxText* techCombo, bool isSingle) {
    gtk_combo_box_text_remove_all(techCombo);
    if (isSingle) {
        gtk_combo_box_text_append_text(techCombo, "时差定位");
        gtk_combo_box_text_append_text(techCombo, "干涉仪定位");
    } else {
        gtk_combo_box_text_append_text(techCombo, "时差定位");
        gtk_combo_box_text_append_text(techCombo, "频差定位");
        gtk_combo_box_text_append_text(techCombo, "测向定位");
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(techCombo), 0);
}

//多平台技术体制选择事件回调函数
static void on_multi_tech_changed(GtkComboBox* combo, gpointer user_data) {
    MultiDeviceComboContext* ctx = static_cast<MultiDeviceComboContext*>(user_data);
    gchar* tech = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ctx->techCombo));
    if (tech) {
        if (std::string(tech) == "频差定位") {
            refreshDeviceCombo(GTK_COMBO_BOX_TEXT(ctx->multiDeviceCombo), *(ctx->allDevices), *(ctx->allDevices), *(ctx->allDevices), false, tech);
        } else {
            // 只显示固定设备
            std::vector<ReconnaissanceDevice> fixedDevices,mobileDevices;
            for (const auto& dev : *(ctx->allDevices)) {
                if (dev.getIsStationary()) fixedDevices.push_back(dev);
                else mobileDevices.push_back(dev);
            }
            refreshDeviceCombo(GTK_COMBO_BOX_TEXT(ctx->multiDeviceCombo), *(ctx->allDevices), fixedDevices, mobileDevices, false, tech);
        }
    } else {
        refreshDeviceCombo(GTK_COMBO_BOX_TEXT(ctx->multiDeviceCombo), *(ctx->allDevices), *(ctx->allDevices), *(ctx->allDevices), false, "");
    }
    // 保持原有多平台下拉框显示/隐藏逻辑
    std::vector<ReconnaissanceDevice> devicesToShow;
    if (tech && std::string(tech) == "频差定位") {
        devicesToShow = *(ctx->allDevices);
    } else {
        for (const auto& dev : *(ctx->allDevices)) {
            if (dev.getIsStationary()) devicesToShow.push_back(dev);
        }
    }
    for (const auto& dev : devicesToShow) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ctx->multiDeviceCombo), dev.getDeviceName().c_str());
    }
    if (!devicesToShow.empty()) gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->multiDeviceCombo), 0);
    if (tech) g_free(tech);
}
// 刷新侦察设备下拉框
void refreshDeviceCombo(GtkComboBoxText* deviceCombo, const std::vector<ReconnaissanceDevice>& allDevices, const std::vector<ReconnaissanceDevice>& fixedDevices, const std::vector<ReconnaissanceDevice>& mobileDevices, bool isSingle, const std::string& tech) {
    gtk_combo_box_text_remove_all(deviceCombo);
    const std::vector<ReconnaissanceDevice>* showList = nullptr;
    if (isSingle) {
        showList = &mobileDevices;
    } else {
        if (tech == "频差定位") showList = &allDevices;
        else showList = &fixedDevices;
    }
    for (const auto& dev : *showList) {
        gtk_combo_box_text_append_text(deviceCombo, dev.getDeviceName().c_str());
    }
    if (!showList->empty()) gtk_combo_box_set_active(GTK_COMBO_BOX(deviceCombo), 0);
    else gtk_combo_box_set_active(GTK_COMBO_BOX(deviceCombo), -1);
}
// 实现DataSelectionView类
DataSelectionView::DataSelectionView() : m_view(nullptr), m_dataList(nullptr), m_filterEntry(nullptr), m_startTimeEntry(nullptr), m_endTimeEntry(nullptr), m_targetCombo(nullptr) {
}

DataSelectionView::~DataSelectionView() {
}

// 更新目标数据列表
void DataSelectionView::updateTaskList(int radiationId) {
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(m_dataList)));
    gtk_list_store_clear(store);

    // 通过Controller获取数据
    std::vector<std::vector<std::string>> tasks = DataSelectionController::getInstance().getRelatedTasks(radiationId);
    GtkTreeIter iter;
    for (const auto &task : tasks) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, false,
                           1, task[0].c_str(), // 任务类型
                           2, ("("+task[3] + "°, " + task[4] + "°, " + task[5]+")").c_str(), // 定位数据
                           3, ("("+task[6] + "°, " + task[7]+"°)").c_str(), // 测向数据
                           4, "查看", // 添加"查看"链接
                           5, std::stoi(task[2]), // 隐藏列：任务ID
                           -1);
    }
}

// 辐射源模型下拉列表选择事件回调函数
static void on_target_combo_changed(GtkComboBox *combo, gpointer user_data) {
    DataSelectionView *view = static_cast<DataSelectionView*>(user_data);
    int activeIndex = gtk_combo_box_get_active(combo);
    if (activeIndex >= 0) {
        auto &dao = RadiationSourceDAO::getInstance();
        std::vector<RadiationSource> sources = dao.getAllRadiationSources();
        if (activeIndex < static_cast<int>(sources.size())) {
            int radiationId = sources[activeIndex].getRadiationId(); 
            view->updateTaskList(radiationId);
        }
    }
}

// 行激活事件回调函数，用于实现点击"操作"列
static void on_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {
    DataSelectionView *view = static_cast<DataSelectionView*>(user_data);
    // 获取点击的列索引
    int columnIndex = -1;
    for (int i = 0; i < gtk_tree_view_get_n_columns(tree_view); i++) {
        if (gtk_tree_view_get_column(tree_view, i) == column) {
            columnIndex = i;
            break;
        }
    }
    // 如果点击的是"操作"列
    if (columnIndex == 4) {
        // 获取选中行的数据
        GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
        GtkTreeIter iter;
        if (gtk_tree_model_get_iter(model, &iter, path)) {
            // 获取任务类型
            gchar *taskTypeValue;
            gtk_tree_model_get(model, &iter, 1, &taskTypeValue, -1);
            std::string taskType = taskTypeValue ? taskTypeValue : "";
            g_free(taskTypeValue);
            // 获取任务ID
            int taskId = 0;
            gtk_tree_model_get(model, &iter, 5, &taskId, -1);
            view->showTaskDetailsDialog(taskId, taskType);
        }
    }
}

// 创建数据选择UI
GtkWidget* DataSelectionView::createView() {
    // 创建页面的主容器
    m_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(m_view), 15);
    
    // 控制区域
    GtkWidget* controlBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(m_view), controlBox, FALSE, FALSE, 0);
    
    // 左侧：辐射源模型选择
    GtkWidget* targetFrame = gtk_frame_new("辐射源模型");
    gtk_box_pack_start(GTK_BOX(controlBox), targetFrame, FALSE, FALSE, 0);
    
    GtkWidget* targetBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(targetFrame), targetBox);
    gtk_container_set_border_width(GTK_CONTAINER(targetBox), 10);
    
    GtkWidget* targetLabel = gtk_label_new("选择辐射源模型:");
    gtk_box_pack_start(GTK_BOX(targetBox), targetLabel, FALSE, FALSE, 0);
    
    m_targetCombo = gtk_combo_box_text_new();

    // 从数据库获取所有辐射源信息
    auto& dao = RadiationSourceDAO::getInstance();
    std::vector<RadiationSource> sources = dao.getAllRadiationSources();

    // 将辐射源名称添加到下拉列表
    for (const auto& source : sources) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_targetCombo), source.getRadiationName().c_str());
    }

    if (!sources.empty()) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(m_targetCombo), 0);
    }
    gtk_widget_set_size_request(m_targetCombo, 200, -1);
    gtk_box_pack_start(GTK_BOX(targetBox), m_targetCombo, FALSE, FALSE, 0);

    // 连接下拉列表选择事件
    g_signal_connect(G_OBJECT(m_targetCombo), "changed", G_CALLBACK(on_target_combo_changed), this);

    // 右侧：按钮区域
    GtkWidget* buttonBox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonBox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing(GTK_BOX(buttonBox), 10);
    gtk_box_pack_start(GTK_BOX(controlBox), buttonBox, TRUE, TRUE, 0);
    
    GtkWidget* deleteButton = gtk_button_new_with_label("删除");
    g_signal_connect(G_OBJECT(deleteButton), "clicked", G_CALLBACK(DataSelectionView::onDeleteButtonClicked), this);
    gtk_container_add(GTK_CONTAINER(buttonBox), deleteButton);
    
    GtkWidget* importButton = gtk_button_new_with_label("录入");
    g_signal_connect(G_OBJECT(importButton), "clicked", G_CALLBACK(DataSelectionView::onImportButtonClicked), this);
    gtk_container_add(GTK_CONTAINER(buttonBox), importButton);
    
    // 数据列表区域
    GtkWidget* dataFrame = gtk_frame_new("目标数据列表");
    gtk_box_pack_start(GTK_BOX(m_view), dataFrame, TRUE, TRUE, 0);
    
    GtkWidget* dataBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(dataFrame), dataBox);
    gtk_container_set_border_width(GTK_CONTAINER(dataBox), 10);
    
    // 创建列表存储
    GtkListStore* store = gtk_list_store_new(6, 
                                         G_TYPE_BOOLEAN,  // 选择
                                         G_TYPE_STRING,   // 任务类型
                                         G_TYPE_STRING,  // 定位数据
                                         G_TYPE_STRING,  // 测向数据
                                         G_TYPE_STRING,  // 查看按钮    
                                         G_TYPE_INT);    // 隐藏列：任务ID

    // 创建树视图
    m_dataList = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);

    // 设置树视图支持多行选择
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_dataList));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

    // 连接行激活事件
    g_signal_connect(G_OBJECT(m_dataList), "row-activated", G_CALLBACK(on_row_activated), this);
    
    
    // 选择列
    GtkCellRenderer* toggleRenderer = gtk_cell_renderer_toggle_new();
    GtkTreeViewColumn* toggleColumn = gtk_tree_view_column_new_with_attributes(
        "选择", toggleRenderer, "active", 0, NULL);
    gtk_tree_view_column_set_min_width(toggleColumn, 60);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_dataList), toggleColumn);
    
    // 任务类型列
    GtkCellRenderer* textRenderer1 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column1 = gtk_tree_view_column_new_with_attributes(
        "任务类型", textRenderer1, "text", 1, NULL);
    gtk_tree_view_column_set_min_width(column1, 120);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_dataList), column1);
    
    // 定位数据列
    GtkCellRenderer* textRenderer3 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column3 = gtk_tree_view_column_new_with_attributes(
        "定位数据(经度，纬度，高度)", textRenderer3, "text", 2, NULL);
    gtk_tree_view_column_set_min_width(column3, 200);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_dataList), column3);
    
    // 测向数据列
    GtkCellRenderer* textRenderer4 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column4 = gtk_tree_view_column_new_with_attributes(
        "测向数据(方位角，俯仰角)", textRenderer4, "text", 3, NULL);
    gtk_tree_view_column_set_min_width(column4, 200);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_dataList), column4);
    
    // 查看按钮列
    GtkCellRenderer* textRenderer5 = gtk_cell_renderer_text_new();
    g_object_set(textRenderer5, 
                "foreground", "green", 
                "underline", PANGO_UNDERLINE_SINGLE,
                "xalign", 0.0,  // 左对齐
                "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE,
                NULL);
    GtkTreeViewColumn* column5 = gtk_tree_view_column_new_with_attributes(
        "操作", textRenderer5, "text", 4, NULL);
    gtk_tree_view_column_set_sizing(column5, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(column5, 60);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_dataList), column5);
    
    // 滚动窗口
    GtkWidget* scrollWin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWin), 
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrollWin), m_dataList);
    gtk_box_pack_start(GTK_BOX(dataBox), scrollWin, TRUE, TRUE, 0);

    // 加载 CSS 样式
    GtkCssProvider *cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cssProvider,
        ".highlighted-row {"
        "    background-color: yellow;"
        "}", -1, NULL);
    GtkStyleContext *styleContext = gtk_widget_get_style_context(GTK_WIDGET(m_dataList));
    gtk_style_context_add_provider(styleContext, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(cssProvider);

    // 初始化数据列表
    if (!sources.empty()) {
        int radiationId = sources[0].getRadiationId(); 
        updateTaskList(radiationId);
    }
    
    return m_view;
}

// 获取选中的数据项
std::vector<std::vector<std::string>> DataSelectionView::getSelectedData() const {
    std::vector<std::vector<std::string>> selectedData;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_dataList));
    GtkTreeModel *model;
    GList *selectedRows = gtk_tree_selection_get_selected_rows(selection, &model);
    GList *iter = selectedRows;
    while (iter != nullptr) {
        GtkTreePath *path = static_cast<GtkTreePath*>(iter->data);
        GtkTreeIter treeIter;
        if (gtk_tree_model_get_iter(model, &treeIter, path)) {
            std::vector<std::string> rowData;
            
            // 获取布尔值（选择状态）
            gboolean selected;
            gtk_tree_model_get(model, &treeIter, 0, &selected, -1);
            rowData.push_back(selected ? "true" : "false");
            
            // 获取字符串列
            for (int i = 1; i <= 4; ++i) {
                gchar *value;
                gtk_tree_model_get(model, &treeIter, i, &value, -1);
                if (value) {
                    rowData.push_back(value);
                    g_free(value);
                } else {
                    rowData.push_back("");
                }
            }
            selectedData.push_back(rowData);
        }
        iter = g_list_next(iter);
    }
    g_list_free_full(selectedRows, reinterpret_cast<GDestroyNotify>(gtk_tree_path_free));
    return selectedData;
}

// 删除选中数据项操作
void DataSelectionView::onDeleteButtonClicked(GtkWidget* widget, gpointer user_data) {
    DataSelectionView* view = static_cast<DataSelectionView*>(user_data);
    DataSelectionController::getInstance().deleteSelectedItems(view);
}


// 根据技术体制展示设备列表
static void on_tech_changed(GtkComboBox* combo, gpointer user_data) {
    auto ctx = static_cast<ImportDialogContext*>(user_data);
    bool isSingle = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx->radioSingle));
    // 设备列表准备
    auto* allDevices = reinterpret_cast<std::vector<ReconnaissanceDevice>*>(g_object_get_data(G_OBJECT(combo), "allDevices"));
    auto* fixedDevices = reinterpret_cast<std::vector<ReconnaissanceDevice>*>(g_object_get_data(G_OBJECT(combo), "fixedDevices"));
    auto* mobileDevices = reinterpret_cast<std::vector<ReconnaissanceDevice>*>(g_object_get_data(G_OBJECT(combo), "mobileDevices"));
    gchar* techText = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
    std::string tech = techText ? techText : "";
    if (techText) g_free(techText);
    refreshDeviceCombo(GTK_COMBO_BOX_TEXT(ctx->deviceCombo), *allDevices, *fixedDevices, *mobileDevices, isSingle, tech);
    // 多平台设备下拉框动态控制
    if (!isSingle && ctx->multiDeviceLabels && ctx->multiDeviceCombos) {
        int needCount = 0;
        const std::vector<ReconnaissanceDevice>* showList = nullptr;
        if (tech == "时差定位") { needCount = 4; showList = fixedDevices; }
        else if (tech == "测向定位") { needCount = 2; showList = fixedDevices; }
        else if (tech == "频差定位") { needCount = 3; showList = allDevices; }
        // 先全部隐藏
        for (int i = 0; i < 4; ++i) {
            gtk_widget_hide(ctx->multiDeviceLabels[i]);
            gtk_widget_hide(ctx->multiDeviceCombos[i]);
        }
        // 显示需要的下拉框并填充
        for (int i = 0; i < needCount; ++i) {
            gtk_widget_show(ctx->multiDeviceLabels[i]);
            gtk_widget_show(ctx->multiDeviceCombos[i]);
            gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(ctx->multiDeviceCombos[i]));
            if (showList) {
                for (const auto& dev : *showList) {
                    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ctx->multiDeviceCombos[i]), dev.getDeviceName().c_str());
                }
                if (!showList->empty())
                    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->multiDeviceCombos[i]), 0);
                else
                    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->multiDeviceCombos[i]), -1);
            }
        }
    } else if (ctx->multiDeviceLabels && ctx->multiDeviceCombos) {
        // 单平台时全部隐藏
        for (int i = 0; i < 4; ++i) {
            gtk_widget_hide(ctx->multiDeviceLabels[i]);
            gtk_widget_hide(ctx->multiDeviceCombos[i]);
        }
    }
}
// 单/多平台切换回调
static void on_radio_type_toggled(GtkToggleButton* btn, gpointer user_data) {
    ImportDialogContext* ctx = static_cast<ImportDialogContext*>(user_data);
    bool isSingle = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx->radioSingle));
    refreshTechCombo(GTK_COMBO_BOX_TEXT(ctx->techCombo), isSingle);
    // 设备列表准备
    auto* allDevices = reinterpret_cast<std::vector<ReconnaissanceDevice>*>(g_object_get_data(G_OBJECT(ctx->techCombo), "allDevices"));
    auto* fixedDevices = reinterpret_cast<std::vector<ReconnaissanceDevice>*>(g_object_get_data(G_OBJECT(ctx->techCombo), "fixedDevices"));
    auto* mobileDevices = reinterpret_cast<std::vector<ReconnaissanceDevice>*>(g_object_get_data(G_OBJECT(ctx->techCombo), "mobileDevices"));
    gchar* techText = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ctx->techCombo));
    std::string tech = techText ? techText : "";
    if (techText) g_free(techText);
    refreshDeviceCombo(GTK_COMBO_BOX_TEXT(ctx->deviceCombo), *allDevices, *fixedDevices, *mobileDevices, isSingle, tech);
    // 字段显示
    if (isSingle) {
        gtk_widget_show(ctx->deviceLabel);
        gtk_widget_show(ctx->deviceCombo);
        for (auto w : *(ctx->singleRowWidgets)) gtk_widget_show(w);
        for (auto w : *(ctx->multiRowWidgets)) gtk_widget_hide(w);
    } else {
        gtk_widget_hide(ctx->deviceLabel);
        gtk_widget_hide(ctx->deviceCombo);
        for (auto w : *(ctx->singleRowWidgets)) gtk_widget_hide(w);
        for (auto w : *(ctx->multiRowWidgets)) gtk_widget_show(w);
        // 多平台切换时自动刷新设备下拉框
        on_tech_changed(GTK_COMBO_BOX(ctx->techCombo), ctx);
    }
}

void DataSelectionView::onImportButtonClicked(GtkWidget* widget, gpointer user_data) {
    // 字段名、Entry控件映射
    struct FieldEntry { const char* label; GtkWidget* entry; };
    // 侦察设备
    GtkWidget* deviceLabel = gtk_label_new("侦察设备");
    gtk_widget_set_halign(deviceLabel, GTK_ALIGN_START);
    // 目标坐标（经度/纬度/高度）
    GtkWidget* posLabel = gtk_label_new("目标坐标");
    gtk_widget_set_halign(posLabel, GTK_ALIGN_START);
    // 执行时间/定位时间（公共字段）
    GtkWidget* execTimeEntry = gtk_entry_new();//执行时间
    GtkWidget* posTimeEntry = gtk_entry_new();//定位时间
    // 方位角/俯仰角（公共字段）
    GtkWidget* azimuthLabel = gtk_label_new("方位角(°)");
    gtk_widget_set_halign(azimuthLabel, GTK_ALIGN_START);
    GtkWidget* azimuthEntry = gtk_entry_new();
    GtkWidget* elevationLabel = gtk_label_new("俯仰角(°)");
    gtk_widget_set_halign(elevationLabel, GTK_ALIGN_START);
    GtkWidget* elevationEntry = gtk_entry_new();
    // 定位距离/定位精度（公共字段）
    GtkWidget* posDistLabel = gtk_label_new("定位距离(m)");
    gtk_widget_set_halign(posDistLabel, GTK_ALIGN_START);
    GtkWidget* posDistEntry = gtk_entry_new();//定位距离
    GtkWidget* posAccLabel = gtk_label_new("定位精度(m)");
    gtk_widget_set_halign(posAccLabel, GTK_ALIGN_START);
    GtkWidget* posAccEntry = gtk_entry_new();//定位精度
    // 测向误差/测向精度（单平台）
    GtkWidget* angleErrorLabel = gtk_label_new("测向误差(°)");
    gtk_widget_set_halign(angleErrorLabel, GTK_ALIGN_START);
    GtkWidget* angleErrorEntry = gtk_entry_new(); // 测向误差
    GtkWidget* directionAccLabel = gtk_label_new("测向精度(°)");
    gtk_widget_set_halign(directionAccLabel, GTK_ALIGN_START);
    GtkWidget* directionAccEntry = gtk_entry_new(); // 测向精度
    // 运动速度/运动方位角/运动俯仰角（多平台）
    GtkWidget* moveParamLabel = gtk_label_new("运动参数");
    gtk_widget_set_halign(moveParamLabel, GTK_ALIGN_START);
    GtkWidget* moveSpeedEntry = gtk_entry_new(); // 运动速度
    GtkWidget* moveAzEntry = gtk_entry_new();    // 运动方位角
    GtkWidget* moveElEntry = gtk_entry_new();    // 运动俯仰角
    // 设备列表准备
    auto& deviceDao = ReconnaissanceDeviceDAO::getInstance();
    std::vector<ReconnaissanceDevice> allDevices = deviceDao.getAllReconnaissanceDevices();
    std::vector<ReconnaissanceDevice> fixedDevices;
    std::vector<ReconnaissanceDevice> mobileDevices;
    for (const auto& dev : allDevices) {
        if (dev.getIsStationary()) fixedDevices.push_back(dev);
        else mobileDevices.push_back(dev);
    }
    // 当前辐射源ID
    int radiationId = -1;
    DataSelectionView* view = static_cast<DataSelectionView*>(user_data);
    if (GTK_IS_COMBO_BOX(view->m_targetCombo)) {
        int active = gtk_combo_box_get_active(GTK_COMBO_BOX(view->m_targetCombo));
        auto sources = RadiationSourceDAO::getInstance().getAllRadiationSources();
        if (active >= 0 && active < (int)sources.size()) {
            radiationId = sources[active].getRadiationId();
        }
    }
    // 创建对话框时设置父窗口
    GtkWidget* parentWindow = gtk_widget_get_toplevel(GTK_WIDGET(view->m_view));
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        "录入任务",
        GTK_WINDOW(GTK_IS_WINDOW(parentWindow) ? parentWindow : nullptr),
        GTK_DIALOG_MODAL,
        "取消", GTK_RESPONSE_CANCEL,
        "确定", GTK_RESPONSE_OK,
        nullptr
    );
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 500);//设置对话框大小
    GtkWidget* contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));//获取对话框内容区域
    gtk_container_set_border_width(GTK_CONTAINER(contentArea), 10);//设置内容区域边框宽度
    GtkWidget* grid = gtk_grid_new();//创建网格布局
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);//设置列间距
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);//设置行间距
    gtk_container_add(GTK_CONTAINER(contentArea), grid);//将网格布局添加到内容区域
    int row = 0;//行索引
    // 单平台和多平台字段容器
    std::vector<GtkWidget*> singleRowWidgets, multiRowWidgets;
    // 任务类型
    GtkWidget* typeLabel = gtk_label_new("任务类型");//任务类型标签
    gtk_widget_set_halign(typeLabel, GTK_ALIGN_START);
    GtkWidget* typeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* radioSingle = gtk_radio_button_new_with_label(NULL, "单平台");
    GtkWidget* radioMulti = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radioSingle), "多平台");
    gtk_box_pack_start(GTK_BOX(typeBox), radioSingle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(typeBox), radioMulti, FALSE, FALSE, 0);
    gtk_grid_attach(GTK_GRID(grid), typeLabel, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), typeBox, 1, row, 3, 1);
    row++;
    // 技术体制
    GtkWidget* techLabel = gtk_label_new("技术体制");
    gtk_widget_set_halign(techLabel, GTK_ALIGN_START);
    GtkWidget* techCombo = gtk_combo_box_text_new();
    gtk_grid_attach(GTK_GRID(grid), techLabel, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), techCombo, 1, row, 3, 1);
    row++;
    // 侦察设备（单平台）
    gtk_grid_attach(GTK_GRID(grid), deviceLabel, 0, row, 1, 1);
    GtkWidget* deviceCombo = gtk_combo_box_text_new();
    gtk_grid_attach(GTK_GRID(grid), deviceCombo, 1, row, 3, 1);
    row++;
    // 目标经度/纬度/高度
    gtk_grid_attach(GTK_GRID(grid), posLabel, 0, row, 1, 1);
    GtkWidget* longitudeEntry = gtk_entry_new();
    GtkWidget* latitudeEntry = gtk_entry_new();
    GtkWidget* altitudeEntry = gtk_entry_new();
    gtk_widget_set_hexpand(longitudeEntry, TRUE);
    gtk_widget_set_hexpand(latitudeEntry, TRUE);
    gtk_widget_set_hexpand(altitudeEntry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), longitudeEntry, 1, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), latitudeEntry, 2, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), altitudeEntry, 3, row, 1, 1);
    row++;
    // 运动速度/运动方位角/运动俯仰角（多平台）
    gtk_grid_attach(GTK_GRID(grid), moveParamLabel, 0, row, 1, 1);
    gtk_widget_set_hexpand(moveSpeedEntry, TRUE);
    gtk_widget_set_hexpand(moveAzEntry, TRUE);
    gtk_widget_set_hexpand(moveElEntry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), moveSpeedEntry, 1, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), moveAzEntry, 2, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), moveElEntry, 3, row, 1, 1);
    multiRowWidgets.push_back(moveParamLabel);
    multiRowWidgets.push_back(moveSpeedEntry);
    multiRowWidgets.push_back(moveAzEntry);
    multiRowWidgets.push_back(moveElEntry);
    row++;
    // 方位角/俯仰角
    gtk_grid_attach(GTK_GRID(grid), azimuthLabel, 0, row, 1, 1);
    gtk_widget_set_hexpand(azimuthEntry, TRUE);
    gtk_widget_set_hexpand(elevationEntry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), azimuthEntry, 1, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), elevationLabel, 2, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), elevationEntry, 3, row, 1, 1);
    row++;
    // 执行时间/定位时间
    GtkWidget* execTimeLabel = gtk_label_new("执行时间(s)");
    gtk_widget_set_halign(execTimeLabel, GTK_ALIGN_START);
    GtkWidget* posTimeLabel = gtk_label_new("定位时间(s)");
    gtk_widget_set_halign(posTimeLabel, GTK_ALIGN_START);
    gtk_widget_set_hexpand(execTimeEntry, TRUE);
    gtk_widget_set_hexpand(posTimeEntry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), execTimeLabel, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), execTimeEntry, 1, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), posTimeLabel, 2, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), posTimeEntry, 3, row, 1, 1);
    row++;
    // 定位距离/定位精度
    gtk_grid_attach(GTK_GRID(grid), posDistLabel, 0, row, 1, 1);
    gtk_widget_set_hexpand(posDistEntry, TRUE);
    gtk_widget_set_hexpand(posAccEntry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), posDistEntry, 1, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), posAccLabel, 2, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), posAccEntry, 3, row, 1, 1);
    row++;
    // 测向误差/测向精度（单平台）
    gtk_grid_attach(GTK_GRID(grid), angleErrorLabel, 0, row, 1, 1);
    gtk_widget_set_hexpand(angleErrorEntry, TRUE);
    gtk_widget_set_hexpand(directionAccEntry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), angleErrorEntry, 1, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), directionAccLabel, 2, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), directionAccEntry, 3, row, 1, 1);
    singleRowWidgets.push_back(angleErrorLabel);
    singleRowWidgets.push_back(angleErrorEntry);
    singleRowWidgets.push_back(directionAccLabel);
    singleRowWidgets.push_back(directionAccEntry);
    row++;
    // 多平台设备下拉框
    int multiRow = row + 1;
    GtkWidget* multiDeviceLabels[4];
    GtkWidget* multiDeviceCombos[4];
    for (int i = 0; i < 4; ++i) {
        char labelTxt[16];
        snprintf(labelTxt, sizeof(labelTxt), "侦察设备%d", i+1);
        multiDeviceLabels[i] = gtk_label_new(labelTxt);
        gtk_widget_set_halign(multiDeviceLabels[i], GTK_ALIGN_START);
        gtk_widget_set_hexpand(multiDeviceLabels[i], TRUE);
        int deviceRow = multiRow + i / 2;
        int deviceCol = (i % 2) * 2; // 0/2
        gtk_grid_attach(GTK_GRID(grid), multiDeviceLabels[i], deviceCol, deviceRow, 1, 1);
        multiDeviceCombos[i] = gtk_combo_box_text_new();
        gtk_widget_set_hexpand(multiDeviceCombos[i], TRUE);
        gtk_grid_attach(GTK_GRID(grid), multiDeviceCombos[i], deviceCol + 1, deviceRow, 1, 1);
        gtk_widget_hide(multiDeviceLabels[i]);
        gtk_widget_hide(multiDeviceCombos[i]);
    }
    // 绑定设备列表到 techCombo，便于回调使用
    g_object_set_data(G_OBJECT(techCombo), "allDevices", &allDevices);
    g_object_set_data(G_OBJECT(techCombo), "fixedDevices", &fixedDevices);
    g_object_set_data(G_OBJECT(techCombo), "mobileDevices", &mobileDevices);
    // 绑定设备列表到 ctx，增加azElBox
    ImportDialogContext* ctx = new ImportDialogContext{radioSingle, deviceLabel, deviceCombo, nullptr, nullptr, &singleRowWidgets, &multiRowWidgets, techCombo, multiDeviceLabels, multiDeviceCombos, nullptr};
    g_object_set_data(G_OBJECT(techCombo), "ctx", ctx);
    // 信号连接
    g_signal_connect(radioSingle, "toggled", G_CALLBACK(on_radio_type_toggled), ctx);
    g_signal_connect(radioMulti, "toggled", G_CALLBACK(on_radio_type_toggled), ctx);
    g_signal_connect(techCombo, "changed", G_CALLBACK(on_tech_changed), ctx);
    // 初始化下拉框
    refreshTechCombo(GTK_COMBO_BOX_TEXT(techCombo), true);
    refreshDeviceCombo(GTK_COMBO_BOX_TEXT(deviceCombo), allDevices, fixedDevices, mobileDevices, true, "时差定位");
    // 显示对话框
    gtk_widget_show_all(dialog);
    // 确保初始时为单平台则隐藏所有多平台设备下拉框
    if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radioMulti))) {
        for (int i = 0; i < 4; ++i) {
            gtk_widget_hide(multiDeviceLabels[i]);
            gtk_widget_hide(multiDeviceCombos[i]);
        }
    }
    for (auto w : multiRowWidgets) gtk_widget_hide(w);

    // 录入逻辑
    while (true) {
        int response = gtk_dialog_run(GTK_DIALOG(dialog));
        if (response != GTK_RESPONSE_OK) {
            gtk_widget_destroy(dialog);
            break;
        }
        bool isSingle = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radioSingle));
        bool valid = true;
        std::string errMsg;
        std::vector<std::string> values;
        std::string techSystem;
        std::vector<int> deviceIds;
        gchar* techText = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(techCombo));
        std::string tech = techText ? techText : "";
        if (techText) g_free(techText);
        int devIdx = gtk_combo_box_get_active(GTK_COMBO_BOX(deviceCombo));
        // 设备列表
        const std::vector<ReconnaissanceDevice>* showList = nullptr;
        //处理技术体制
        if (isSingle) {
            showList = &mobileDevices;
            techSystem = (tech == "时差定位") ? "TDOA" : "INTERFEROMETER";
        } else {
            if (tech == "频差定位") {
                showList = &allDevices;
                techSystem = "FDOA";
            } else if (tech == "时差定位") {
                showList = &fixedDevices;
                techSystem = "TDOA";
            } else if (tech == "测向定位") {
                showList = &fixedDevices;
                techSystem = "FD";
            }
        }
        //处理目标坐标
        const char* longitudeTxt = gtk_entry_get_text(GTK_ENTRY(longitudeEntry));
        const char* latitudeTxt = gtk_entry_get_text(GTK_ENTRY(latitudeEntry));
        const char* altitudeTxt = gtk_entry_get_text(GTK_ENTRY(altitudeEntry));
        if (!longitudeTxt) { valid = false; errMsg = "目标经度不能为空！"; }
        values.push_back(longitudeTxt ? longitudeTxt : "");
        if (!latitudeTxt) { valid = false; errMsg = "目标纬度不能为空！"; }
        values.push_back(latitudeTxt ? latitudeTxt : "");
        if (!altitudeTxt) { valid = false; errMsg = "目标高度不能为空！"; }
        values.push_back(altitudeTxt ? altitudeTxt : "");   
        //处理方位角和俯仰角
        const char* azimuthTxt = gtk_entry_get_text(GTK_ENTRY(azimuthEntry));
        const char* elevationTxt = gtk_entry_get_text(GTK_ENTRY(elevationEntry));
        if (!azimuthTxt) { valid = false; errMsg = "方位角不能为空！"; }
        values.push_back(azimuthTxt ? azimuthTxt : "");
        if (!elevationTxt) { valid = false; errMsg = "俯仰角不能为空！"; }
        values.push_back(elevationTxt ? elevationTxt : "");
        //处理执行时间和定位时间
        const char* execTimeTxt = gtk_entry_get_text(GTK_ENTRY(execTimeEntry));
        const char* posTimeTxt = gtk_entry_get_text(GTK_ENTRY(posTimeEntry));
        //执行时间和定位时间不能为空且不能为负数
        if (!execTimeTxt) { valid = false; errMsg = "执行时间不能为空！"; }
        if (execTimeTxt && atof(execTimeTxt) < 0) { valid = false; errMsg = "执行时间不能为负数！"; }
        values.push_back(execTimeTxt ? execTimeTxt : "");
        if (!posTimeTxt) { valid = false; errMsg = "定位时间不能为空！"; }
        if (posTimeTxt && atof(posTimeTxt) < 0) { valid = false; errMsg = "定位时间不能为负数！"; }
        values.push_back(posTimeTxt ? posTimeTxt : "");
        //处理定位距离和定位精度
        const char* posDistTxt = gtk_entry_get_text(GTK_ENTRY(posDistEntry));
        if (!posDistTxt) { valid = false; errMsg = "定位距离不能为空！"; }
        if (posDistTxt && atof(posDistTxt) < 0) { valid = false; errMsg = "定位距离不能为负数！"; }
        values.push_back(posDistTxt ? posDistTxt : "");
        //处理定位精度
        const char* posAccTxt = gtk_entry_get_text(GTK_ENTRY(posAccEntry));
        if (!posAccTxt) { valid = false; errMsg = "定位精度不能为空！"; }
        if (posAccTxt && atof(posAccTxt) < 0) { valid = false; errMsg = "定位精度不能为负数！"; }
        values.push_back(posAccTxt ? posAccTxt : "");
        // showList 非空且 devIdx 有效才允许取值
        if (showList && devIdx >= 0 && devIdx < (int)showList->size()) {
            deviceIds.push_back((*showList)[devIdx].getDeviceId());
        } 
        if (isSingle) {
            //处理测向误差和测向精度
            const char* angleErrorTxt = gtk_entry_get_text(GTK_ENTRY(angleErrorEntry));
            if (!angleErrorTxt) { valid = false; errMsg = "测向误差不能为空！"; }
            if (angleErrorTxt && atof(angleErrorTxt) < 0) { valid = false; errMsg = "测向误差不能为负数！"; }
            values.push_back(angleErrorTxt ? angleErrorTxt : "");
            const char* directionAccTxt = gtk_entry_get_text(GTK_ENTRY(directionAccEntry));
            if (!directionAccTxt) { valid = false; errMsg = "测向精度不能为空！"; }
            if (directionAccTxt && atof(directionAccTxt) < 0) { valid = false; errMsg = "测向精度不能为负数！"; }
            values.push_back(directionAccTxt ? directionAccTxt : "");
        } else { // 多平台
            for (int i = 0; i < 4; ++i) {
                int idx = gtk_combo_box_get_active(GTK_COMBO_BOX(multiDeviceCombos[i]));
                if (gtk_widget_get_visible(multiDeviceCombos[i]) && showList && idx >= 0 && idx < (int)showList->size()) {
                    deviceIds.push_back((*showList)[idx].getDeviceId());
                }
            }
            //处理运动参数
            const char* movementSpeedTxt = gtk_entry_get_text(GTK_ENTRY(moveSpeedEntry));
            if (!movementSpeedTxt) { valid = false; errMsg = "运动速度不能为空！"; }
            if (movementSpeedTxt && atof(movementSpeedTxt) < 0) { valid = false; errMsg = "运动速度不能为负数！"; }
            values.push_back(movementSpeedTxt ? movementSpeedTxt : "");
            const char* movementAzimuthTxt = gtk_entry_get_text(GTK_ENTRY(moveAzEntry));
            if (!movementAzimuthTxt) { valid = false; errMsg = "运动方位角不能为空！"; }
            values.push_back(movementAzimuthTxt ? movementAzimuthTxt : "");
            const char* movementElevationTxt = gtk_entry_get_text(GTK_ENTRY(moveElEntry));
            if (!movementElevationTxt) { valid = false; errMsg = "运动俯仰角不能为空！"; }
            values.push_back(movementElevationTxt ? movementElevationTxt : "");
        }
        if (!valid) {
            GtkWidget* err = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", errMsg.c_str());
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
            continue;
        }
        // 使用Controller进行数据库操作
        if (DataSelectionController::getInstance().importData(view, isSingle, values, deviceIds, radiationId, techSystem)) {
            gtk_widget_destroy(dialog);
            break;
        } else {
            GtkWidget* err = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "数据库插入失败");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
            continue;
        }
    }
}

// 显示任务详情对话框
void DataSelectionView::showTaskDetailsDialog(int taskId, const std::string& taskType) {
    // 通过控制器获取任务详情数据
    std::map<std::string, std::string> taskDetails = DataSelectionController::getInstance().showTaskDetails(taskId, taskType);
    
    // 检查是否有错误
    if (taskDetails.find("error") != taskDetails.end()) {
        std::cerr << taskDetails["error"] << std::endl;
        std::cerr.flush();
        return;
    }
    
    // 创建对话框
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        (taskType + "任务详情").c_str(),
        nullptr,
        GTK_DIALOG_MODAL,
        "关闭", GTK_RESPONSE_CLOSE,
        nullptr
    );

    // 设置对话框大小
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 500);

    // 获取内容区域
    GtkWidget* contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(contentArea), 10);

    // 创建网格布局用于显示任务信息
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_container_add(GTK_CONTAINER(contentArea), grid);

    // 添加分隔符
    GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(grid), separator, 0, 0, 2, 1);
    int row_idx = 1;

    // 设备和辐射源信息
    if (taskType == "单平台") {
        // 先显示设备名称
        GtkWidget* nameLabel1 = gtk_label_new("设备名称");
        gtk_widget_set_halign(nameLabel1, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), nameLabel1, 0, row_idx, 1, 1);
        GtkWidget* valueLabel1 = gtk_label_new(taskDetails["deviceName"].c_str());
        gtk_widget_set_halign(valueLabel1, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), valueLabel1, 1, row_idx, 1, 1);
        row_idx++;
        
        // 显示辐射源名称
        GtkWidget* nameLabel2 = gtk_label_new("辐射源名称");
        gtk_widget_set_halign(nameLabel2, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), nameLabel2, 0, row_idx, 1, 1);
        GtkWidget* valueLabel2 = gtk_label_new(taskDetails["radiationName"].c_str());
        gtk_widget_set_halign(valueLabel2, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), valueLabel2, 1, row_idx, 1, 1);
        row_idx++;
    } else if (taskType == "多平台") {
        // 显示辐射源名称
        GtkWidget* nameLabel1 = gtk_label_new("辐射源名称");
        gtk_widget_set_halign(nameLabel1, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), nameLabel1, 0, row_idx, 1, 1);
        GtkWidget* valueLabel1 = gtk_label_new(taskDetails["radiationName"].c_str());
        gtk_widget_set_halign(valueLabel1, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), valueLabel1, 1, row_idx, 1, 1);
        row_idx++;
        
        // 显示关联设备
        if (taskDetails.find("deviceNames") != taskDetails.end()) {
            GtkWidget* deviceLabel = gtk_label_new("关联侦察设备");
            gtk_widget_set_halign(deviceLabel, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(grid), deviceLabel, 0, row_idx, 1, 1);
            GtkWidget* deviceValueLabel = gtk_label_new(taskDetails["deviceNames"].c_str());
            gtk_widget_set_halign(deviceValueLabel, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(grid), deviceValueLabel, 1, row_idx, 1, 1);
            row_idx++;
        }
    }

    // 显示其他字段，定义字段名映射和单位
    struct FieldInfo {
        const char* key;
        const char* displayName;
        const char* unit;
    };

    // 定义常用字段的显示名称和单位
    const FieldInfo fieldInfos[] = {
        {"tech_system", "技术体制", ""},
        {"execution_time", "执行时间", "s"},
        {"target_longitude", "目标经度", "°"},
        {"target_latitude", "目标纬度", "°"},
        {"target_altitude", "目标高度", "m"},
        {"azimuth", "方位角", "°"},
        {"elevation", "俯仰角", "°"},
        {"angle_error", "测向误差", "°"},
        {"positioning_distance", "定位距离", "m"},
        {"positioning_time", "定位时间", "s"},
        {"positioning_accuracy", "定位精度", "m"},
        {"direction_finding_accuracy", "测向精度", "°"},
        {"created_at", "创建时间", ""},
        {"movement_speed", "运动速度", "m/s"},
        {"movement_azimuth", "运动方位角", "°"},
        {"movement_elevation", "运动俯仰角", "°"}
    };

    // 遍历字段信息
    for (const auto& field : fieldInfos) {
        if (taskDetails.find(field.key) != taskDetails.end()) {
            GtkWidget* nameLabel = gtk_label_new(field.displayName);
            gtk_widget_set_halign(nameLabel, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(grid), nameLabel, 0, row_idx, 1, 1);
            
            // 处理特殊情况：技术体制
            std::string displayValue = taskDetails[field.key];
            if (strcmp(field.key, "tech_system") == 0) {
                if (displayValue == "FDOA") displayValue = "频差定位";
                else if (displayValue == "TDOA") displayValue = "时差定位";
                else if (displayValue == "INTERFEROMETER") displayValue = "干涉仪定位";
                else if (displayValue == "FD") displayValue = "测向定位";
            }
            
            // 添加单位
            if (strlen(field.unit) > 0) {
                displayValue += field.unit;
            }
            
            GtkWidget* valueLabel = gtk_label_new(displayValue.c_str());
            gtk_widget_set_halign(valueLabel, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(grid), valueLabel, 1, row_idx, 1, 1);
            row_idx++;
        }
    }

    // 显示对话框
    gtk_widget_show_all(dialog);
    
    // 运行对话框并在关闭时销毁
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}