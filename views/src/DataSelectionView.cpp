#include "../DataSelectionView.h"
#include "../../controllers/ApplicationController.h"
#include <gtk/gtk.h>
#include "../../models/RadiationSourceDAO.h"
#include "../../models/RadiationSourceModel.h"
#include <mysql/mysql.h>
#include "../../models/DBConnector.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/ReconnaissanceDeviceModel.h"

struct ImportDialogContext {
    GtkWidget* radioSingle;
    GtkWidget* deviceLabel;
    GtkWidget* deviceCombo;
    GtkWidget* multiDeviceLabel;
    GtkWidget* multiDeviceCombo;
    std::vector<GtkWidget*>* singleRowWidgets;
    std::vector<GtkWidget*>* multiRowWidgets;
    GtkWidget* techCombo;
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
        gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(ctx->techCombo));
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ctx->techCombo), "时差定位");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ctx->techCombo), "干涉仪定位");
        gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->techCombo), 0);
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
        gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(ctx->techCombo));
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ctx->techCombo), "时差定位");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ctx->techCombo), "频差定位");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ctx->techCombo), "测向定位");
        gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->techCombo), 0);
    }
}
//多平台技术体制选择事件回调函数
static void on_multi_tech_changed(GtkComboBox* combo, gpointer user_data) {
    MultiDeviceComboContext* ctx = static_cast<MultiDeviceComboContext*>(user_data);
    gchar* tech = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ctx->techCombo));
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(ctx->multiDeviceCombo));
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

// 实现DataSelectionView类
DataSelectionView::DataSelectionView() : m_view(nullptr), m_dataList(nullptr), m_filterEntry(nullptr), m_startTimeEntry(nullptr), m_endTimeEntry(nullptr), m_targetCombo(nullptr) {
}

DataSelectionView::~DataSelectionView() {
}

// 从数据库获取关联任务数据
std::vector<std::vector<std::string>> DataSelectionView::getRelatedTasks(int radiationId) {
    std::vector<std::vector<std::string>> tasks;
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn) {
        std::cerr << "No valid database connection" << std::endl;
        return tasks;
    }

// 查询单平台任务
std::string singleSql = "SELECT '单平台', rd.device_name, sp.task_id, sp.target_longitude, sp.target_latitude, sp.target_altitude, sp.azimuth, sp.elevation "
                            "FROM single_platform_task sp "
                            "JOIN reconnaissance_device_models rd ON sp.device_id = rd.device_id "
                            "WHERE sp.radiation_id = " + std::to_string(radiationId);
    if (mysql_query(conn, singleSql.c_str()) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                std::vector<std::string> task;
                for (int i = 0; i < mysql_num_fields(result); ++i) {
                    task.push_back(row[i] ? row[i] : "");
                }
                tasks.push_back(task);
            }
            mysql_free_result(result);
        }
    } else {
        // 调用新增的公共成员函数
        db.showError();
    }

// 查询多平台任务
std::string multiSql = "SELECT '多平台', GROUP_CONCAT(rd.device_name SEPARATOR ', '), mp.task_id, mp.target_longitude, mp.target_latitude, mp.target_altitude, mp.azimuth, mp.elevation "
                           "FROM multi_platform_task mp "
                           "JOIN platform_task_relation ptr ON mp.task_id = ptr.simulation_id "
                           "JOIN reconnaissance_device_models rd ON ptr.device_id = rd.device_id "
                           "WHERE mp.radiation_id = " + std::to_string(radiationId) +
                           " GROUP BY mp.task_id";
    if (mysql_query(conn, multiSql.c_str()) == 0) {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                std::vector<std::string> task;
                for (int i = 0; i < mysql_num_fields(result); ++i) {
                    task.push_back(row[i] ? row[i] : "");
                }
                tasks.push_back(task);
            }
            mysql_free_result(result);
        }
    } else {
        // 调用新增的公共成员函数
        db.showError();
    }

    return tasks;
}

// 更新目标数据列表
void DataSelectionView::updateTaskList(int radiationId) {
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(m_dataList)));
    gtk_list_store_clear(store);

    std::vector<std::vector<std::string>> tasks = getRelatedTasks(radiationId);
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
    
    // 启用指针悬停效果
    gtk_widget_set_has_tooltip(m_dataList, TRUE);
    
    // 添加列
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
    gtk_tree_view_column_set_min_width(column4, 150);
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
        int radiationId = sources[0].getRadiationId(); // 假设 RadiationSource 类有 getRadiationId 方法
        updateTaskList(radiationId);
    }
    
    return m_view;
}

// 更新数据列表
void DataSelectionView::updateDataList(const std::vector<std::string>& dataItems) {
    // 实现更新数据列表的逻辑
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

// 删除选中的数据项
void DataSelectionView::deleteSelectedItems() {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0) {
        std::cerr << "数据库连接异常" << std::endl;
        std::cerr.flush();
        return;
    }

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_dataList));
    GtkTreeModel *model;
    GList *selectedRows = gtk_tree_selection_get_selected_rows(selection, &model);
    GList *iter = selectedRows;
    while (iter != nullptr) {
        GtkTreePath *path = static_cast<GtkTreePath*>(iter->data);
        GtkTreeIter treeIter;
        if (gtk_tree_model_get_iter(model, &treeIter, path)) {
            // 获取任务类型
            gchar *taskTypeValue;
            gtk_tree_model_get(model, &treeIter, 1, &taskTypeValue, -1);
            std::string taskType = taskTypeValue ? taskTypeValue : "";
            g_free(taskTypeValue);
            // 获取任务ID
            int taskId = 0;
            gtk_tree_model_get(model, &treeIter, 5, &taskId, -1);
            // 根据任务类型和任务ID删除
            std::string tableName;
            if (taskType == "单平台") {
                tableName = "single_platform_task";
            } else if (taskType == "多平台") {
                tableName = "multi_platform_task";
            } else {
                std::cerr << "未知任务类型: " << taskType << std::endl;
                continue;
            }
            std::string sql = "DELETE FROM " + tableName + " WHERE task_id = " + std::to_string(taskId);
            if (mysql_query(conn, sql.c_str()) != 0) {
                std::cerr << "删除失败: " << mysql_error(conn) << std::endl;
            } else {
                std::cout << "成功删除任务ID: " << taskId << std::endl;
            }
        }
        iter = g_list_next(iter);
    }
    g_list_free_full(selectedRows, reinterpret_cast<GDestroyNotify>(gtk_tree_path_free));
    // 刷新列表
    if (GTK_IS_COMBO_BOX(m_targetCombo)) {
        int active = gtk_combo_box_get_active(GTK_COMBO_BOX(m_targetCombo));
        if (active >= 0) {
            auto sources = RadiationSourceDAO::getInstance().getAllRadiationSources();
            if (active < static_cast<int>(sources.size())) {
                updateTaskList(sources[active].getRadiationId());
            }
        }
    }
}

// 1. 移除 multiDeviceCombo 相关结构体、信号、回调
// 2. 新增刷新 techCombo 和 deviceCombo 的辅助函数
// 3. onImportButtonClicked 内部重构
// 4. on_radio_single_toggled、on_radio_multi_toggled、on_tech_changed 统一逻辑

// --- 辅助函数 ---
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
// 刷新侦察设备下拉框
void refreshDeviceCombo(GtkComboBoxText* deviceCombo, const std::vector<ReconnaissanceDevice>& allDevices, const std::vector<ReconnaissanceDevice>& fixedDevices, bool isSingle, const std::string& tech) {
    gtk_combo_box_text_remove_all(deviceCombo);
    const std::vector<ReconnaissanceDevice>* showList = nullptr;
    if (isSingle) {
        showList = &fixedDevices;
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
// 技术体制切换回调
static void on_tech_changed(GtkComboBox* combo, gpointer user_data) {
    auto ctx = static_cast<ImportDialogContext*>(user_data);
    bool isSingle = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx->radioSingle));
    // 设备列表准备
    auto* allDevices = reinterpret_cast<std::vector<ReconnaissanceDevice>*>(g_object_get_data(G_OBJECT(combo), "allDevices"));
    auto* fixedDevices = reinterpret_cast<std::vector<ReconnaissanceDevice>*>(g_object_get_data(G_OBJECT(combo), "fixedDevices"));
    gchar* techText = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
    std::string tech = techText ? techText : "";
    if (techText) g_free(techText);
    refreshDeviceCombo(GTK_COMBO_BOX_TEXT(ctx->deviceCombo), *allDevices, *fixedDevices, isSingle, tech);
}
// 单/多平台切换回调
static void on_radio_type_toggled(GtkToggleButton* btn, gpointer user_data) {
    ImportDialogContext* ctx = static_cast<ImportDialogContext*>(user_data);
    bool isSingle = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx->radioSingle));
    refreshTechCombo(GTK_COMBO_BOX_TEXT(ctx->techCombo), isSingle);
    // 设备列表准备
    auto* allDevices = reinterpret_cast<std::vector<ReconnaissanceDevice>*>(g_object_get_data(G_OBJECT(ctx->techCombo), "allDevices"));
    auto* fixedDevices = reinterpret_cast<std::vector<ReconnaissanceDevice>*>(g_object_get_data(G_OBJECT(ctx->techCombo), "fixedDevices"));
    gchar* techText = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ctx->techCombo));
    std::string tech = techText ? techText : "";
    if (techText) g_free(techText);
    refreshDeviceCombo(GTK_COMBO_BOX_TEXT(ctx->deviceCombo), *allDevices, *fixedDevices, isSingle, tech);
    // 字段显示
    if (isSingle) {
        for (auto w : *(ctx->singleRowWidgets)) gtk_widget_show(w);
        for (auto w : *(ctx->multiRowWidgets)) gtk_widget_hide(w);
    } else {
        for (auto w : *(ctx->singleRowWidgets)) gtk_widget_hide(w);
        for (auto w : *(ctx->multiRowWidgets)) gtk_widget_show(w);
    }
}
// --- onImportButtonClicked 重构 ---
void DataSelectionView::onImportButtonClicked(GtkWidget* widget, gpointer user_data) {
    DataSelectionView* view = static_cast<DataSelectionView*>(user_data);
    // 设备列表准备
    auto& deviceDao = ReconnaissanceDeviceDAO::getInstance();
    std::vector<ReconnaissanceDevice> allDevices = deviceDao.getAllReconnaissanceDevices();
    std::vector<ReconnaissanceDevice> fixedDevices;
    for (const auto& dev : allDevices) {
        if (dev.getIsStationary()) fixedDevices.push_back(dev);
    }
    // 当前辐射源ID
    int radiationId = -1;
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
        "新建任务",
        GTK_WINDOW(GTK_IS_WINDOW(parentWindow) ? parentWindow : nullptr),
        GTK_DIALOG_MODAL,
        "取消", GTK_RESPONSE_CANCEL,
        "确定", GTK_RESPONSE_OK,
        nullptr
    );
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 600);
    GtkWidget* contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(contentArea), 10);
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_container_add(GTK_CONTAINER(contentArea), grid);
    int row = 0;
    // 任务类型
    GtkWidget* typeLabel = gtk_label_new("任务类型");
    gtk_widget_set_halign(typeLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), typeLabel, 0, row, 1, 1);
    GtkWidget* typeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* radioSingle = gtk_radio_button_new_with_label(NULL, "单平台");
    GtkWidget* radioMulti = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radioSingle), "多平台");
    gtk_box_pack_start(GTK_BOX(typeBox), radioSingle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(typeBox), radioMulti, FALSE, FALSE, 0);
    gtk_grid_attach(GTK_GRID(grid), typeBox, 1, row, 1, 1);
    row++;
    // 技术体制
    GtkWidget* techLabel = gtk_label_new("技术体制");
    gtk_widget_set_halign(techLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), techLabel, 0, row, 1, 1);
    GtkWidget* techCombo = gtk_combo_box_text_new();
    gtk_grid_attach(GTK_GRID(grid), techCombo, 1, row, 1, 1);
    row++;
    // 侦察设备
    GtkWidget* deviceLabel = gtk_label_new("侦察设备");
    gtk_widget_set_halign(deviceLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), deviceLabel, 0, row, 1, 1);
    GtkWidget* deviceCombo = gtk_combo_box_text_new();
    gtk_grid_attach(GTK_GRID(grid), deviceCombo, 1, row, 1, 1);
    row++;
    // 字段名、Entry控件映射
    struct FieldEntry { const char* label; GtkWidget* entry; };
    std::vector<FieldEntry> singleFields = {
        {"执行时间(s)", gtk_entry_new()},           // 0
        {"目标经度(°)", gtk_entry_new()},           // 1
        {"目标纬度(°)", gtk_entry_new()},           // 2
        {"目标高度(m)", gtk_entry_new()},           // 3
        {"方位角(°)", gtk_entry_new()},             // 4
        {"俯仰角(°)", gtk_entry_new()},             // 5
        {"测向误差(°)", gtk_entry_new()},           // 6
        {"定位距离(m)", gtk_entry_new()},           // 7
        {"定位时间(s)", gtk_entry_new()},           // 8
        {"定位精度(m)", gtk_entry_new()},           // 9
        {"测向精度(°)", gtk_entry_new()}            // 10
    };
    std::vector<FieldEntry> multiFields = {
        {"执行时间(s)", gtk_entry_new()},
        {"目标经度(°)", gtk_entry_new()},
        {"目标纬度(°)", gtk_entry_new()},
        {"目标高度(m)", gtk_entry_new()},
        {"定位距离(m)", gtk_entry_new()},
        {"定位时间(s)", gtk_entry_new()},
        {"定位精度(m)", gtk_entry_new()},
        {"运动速度(m/s)", gtk_entry_new()},
        {"运动方位角(°)", gtk_entry_new()},
        {"运动俯仰角(°)", gtk_entry_new()},
        {"方位角(°)", gtk_entry_new()},
        {"俯仰角(°)", gtk_entry_new()}
    };
    std::vector<GtkWidget*> singleRowWidgets, multiRowWidgets;
    for (auto& f : singleFields) {
        GtkWidget* lbl = gtk_label_new(f.label);
        gtk_widget_set_halign(lbl, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), lbl, 0, row, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), f.entry, 1, row, 1, 1);
        singleRowWidgets.push_back(lbl);
        singleRowWidgets.push_back(f.entry);
        row++;
    }
    int multiRow = 3;
    for (auto& f : multiFields) {
        GtkWidget* lbl = gtk_label_new(f.label);
        gtk_widget_set_halign(lbl, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), lbl, 0, multiRow, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), f.entry, 1, multiRow, 1, 1);
        multiRowWidgets.push_back(lbl);
        multiRowWidgets.push_back(f.entry);
        multiRow++;
    }
    // 默认只显示单平台字段
    for (auto w : multiRowWidgets) gtk_widget_hide(w);
    // 绑定设备列表到 techCombo，便于回调使用
    g_object_set_data(G_OBJECT(techCombo), "allDevices", &allDevices);
    g_object_set_data(G_OBJECT(techCombo), "fixedDevices", &fixedDevices);
    // 绑定设备列表到 ctx
    ImportDialogContext* ctx = new ImportDialogContext{radioSingle, deviceLabel, deviceCombo, nullptr, nullptr, &singleRowWidgets, &multiRowWidgets, techCombo};
    g_object_set_data(G_OBJECT(techCombo), "ctx", ctx);
    // 信号连接
    g_signal_connect(radioSingle, "toggled", G_CALLBACK(on_radio_type_toggled), ctx);
    g_signal_connect(radioMulti, "toggled", G_CALLBACK(on_radio_type_toggled), ctx);
    g_signal_connect(techCombo, "changed", G_CALLBACK(on_tech_changed), ctx);
    // 初始化下拉框
    refreshTechCombo(GTK_COMBO_BOX_TEXT(techCombo), true);
    refreshDeviceCombo(GTK_COMBO_BOX_TEXT(deviceCombo), allDevices, fixedDevices, true, "时差定位");
    // 显示对话框
    gtk_widget_show_all(dialog);
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
        int deviceId = -1;
        gchar* techText = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(techCombo));
        std::string tech = techText ? techText : "";
        if (techText) g_free(techText);
        int devIdx = gtk_combo_box_get_active(GTK_COMBO_BOX(deviceCombo));
        // 设备列表
        const std::vector<ReconnaissanceDevice>* showList = nullptr;
        if (isSingle) {
            showList = &fixedDevices;
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
        // showList 非空且 devIdx 有效才允许取值
        if (showList && devIdx >= 0 && devIdx < (int)showList->size()) {
            deviceId = (*showList)[devIdx].getDeviceId();
        } else {
            valid = false;
            errMsg = "请选择有效的侦察设备！";
        }
        if (isSingle) {
            // 字段校验
            for (auto& f : singleFields) {
                const char* txt = gtk_entry_get_text(GTK_ENTRY(f.entry));
                if (!txt || strlen(txt) == 0) { valid = false; errMsg = std::string(f.label) + "不能为空！"; break; }
                values.push_back(txt ? txt : "");
            }
            // 非负数校验
            if (valid) {
                double execTime = atof(values[0].c_str());
                double positioningTime = atof(values[8].c_str());
                double angleError = atof(values[6].c_str());
                double directionFindingAccuracy = atof(values[10].c_str());
                double positioningDistance = atof(values[7].c_str());
                if (execTime < 0) { valid = false; errMsg = "执行时间不能为负数！"; }
                else if (positioningTime < 0) { valid = false; errMsg = "定位时间不能为负数！"; }
                else if (angleError < 0) { valid = false; errMsg = "测向误差不能为负数！"; }
                else if (directionFindingAccuracy < 0) { valid = false; errMsg = "测向精度不能为负数！"; }
                else if (positioningDistance < 0) { valid = false; errMsg = "定位距离不能为负数！"; }
            }
            if (!valid) {
                GtkWidget* err = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", errMsg.c_str());
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                continue;
            }
            // 插入数据库
            DBConnector& db = DBConnector::getInstance();
            MYSQL* conn = db.getConnection();
            char sql[1024];
            snprintf(sql, sizeof(sql),
                "INSERT INTO single_platform_task (device_id, radiation_id, tech_system, execution_time, target_longitude, target_latitude, target_altitude, azimuth, elevation, angle_error, positioning_distance, positioning_time, positioning_accuracy, direction_finding_accuracy) "
                "VALUES (%d, %d, '%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
                deviceId, radiationId, techSystem.c_str(),
                values[0].c_str(), values[1].c_str(), values[2].c_str(), values[3].c_str(), values[4].c_str(), values[5].c_str(), values[6].c_str(), values[7].c_str(), values[8].c_str(), values[9].c_str(), values[10].c_str()
            );
            if (mysql_query(conn, sql) != 0) {
                GtkWidget* err = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "数据库插入失败: %s", mysql_error(conn));
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                continue;
            } else {
                // 刷新列表
                if (GTK_IS_COMBO_BOX(view->m_targetCombo)) {
                    int active = gtk_combo_box_get_active(GTK_COMBO_BOX(view->m_targetCombo));
                    auto sources = RadiationSourceDAO::getInstance().getAllRadiationSources();
                    if (active >= 0 && active < (int)sources.size()) {
                        view->updateTaskList(sources[active].getRadiationId());
                    }
                }
                gtk_widget_destroy(dialog);
                break;
            }
        } else { // 多平台
            for (auto& f : multiFields) {
                const char* txt = gtk_entry_get_text(GTK_ENTRY(f.entry));
                if (!txt || strlen(txt) == 0) { valid = false; errMsg = std::string(f.label) + "不能为空！"; break; }
                values.push_back(txt ? txt : "");
            }
            if (!valid) {
                GtkWidget* err = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", errMsg.c_str());
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                continue;
            }
            // 插入数据库
            DBConnector& db = DBConnector::getInstance();
            MYSQL* conn = db.getConnection();
            char sql[1024];
            snprintf(sql, sizeof(sql),
                "INSERT INTO multi_platform_task (radiation_id, tech_system, execution_time, target_longitude, target_latitude, target_altitude, positioning_distance, positioning_time, positioning_accuracy, movement_speed, movement_azimuth, movement_elevation, azimuth, elevation) "
                "VALUES (%d, '%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
                radiationId, techSystem.c_str(),
                values[0].c_str(), values[1].c_str(), values[2].c_str(), values[3].c_str(), values[4].c_str(), values[5].c_str(), values[6].c_str(), values[7].c_str(), values[8].c_str(), values[9].c_str(), values[10].c_str(), values[11].c_str()
            );
            if (mysql_query(conn, sql) != 0) {
                GtkWidget* err = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "数据库插入失败: %s", mysql_error(conn));
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                continue;
            } else {
                // TODO: 插入deviceId到平台任务关联表（如有）
                if (GTK_IS_COMBO_BOX(view->m_targetCombo)) {
                    int active = gtk_combo_box_get_active(GTK_COMBO_BOX(view->m_targetCombo));
                    auto sources = RadiationSourceDAO::getInstance().getAllRadiationSources();
                    if (active >= 0 && active < (int)sources.size()) {
                        view->updateTaskList(sources[active].getRadiationId());
                    }
                }
                gtk_widget_destroy(dialog);
                break;
            }
        }
    }
}

// 获取视图控件
GtkWidget* DataSelectionView::getView() const {
    return m_view;
}

// 删除按钮点击事件回调函数，触发删除选中数据项操作。
void DataSelectionView::onDeleteButtonClicked(GtkWidget* widget, gpointer user_data) {
    DataSelectionView* view = static_cast<DataSelectionView*>(user_data);
    view->deleteSelectedItems();
}

// 显示任务详情对话框
void DataSelectionView::showTaskDetailsDialog(int taskId, const std::string& taskType) {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0) {
        std::cerr << "数据库连接异常" << std::endl;
        std::cerr.flush();
        return;
    }

    // 根据任务类型和ID查询详细信息
    std::string tableName;
    std::string sql;
    if (taskType == "单平台") {
        tableName = "single_platform_task";
        sql = "SELECT spt.*, rdm.device_name, rsm.radiation_name "
              "FROM single_platform_task spt "
              "JOIN reconnaissance_device_models rdm ON spt.device_id = rdm.device_id "
              "JOIN radiation_source_models rsm ON spt.radiation_id = rsm.radiation_id "
              "WHERE spt.task_id = " + std::to_string(taskId);
    } else if (taskType == "多平台") {
        tableName = "multi_platform_task";
        sql = "SELECT mpt.*, rsm.radiation_name "
              "FROM multi_platform_task mpt "
              "JOIN radiation_source_models rsm ON mpt.radiation_id = rsm.radiation_id "
              "WHERE mpt.task_id = " + std::to_string(taskId);
    } else {
        std::cerr << "未知任务类型: " << taskType << std::endl;
        return;
    }

    if (mysql_query(conn, sql.c_str()) != 0) {
        std::cerr << "查询失败: " << mysql_error(conn) << std::endl;
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        std::cerr << "获取结果集失败: " << mysql_error(conn) << std::endl;
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        std::cerr << "未找到任务ID: " << taskId << std::endl;
        mysql_free_result(result);
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

    // 多平台任务：最上面显示辐射源名称和关联侦察设备
    if (taskType == "多平台") {
        // 先查辐射源名称
        int mpRadiationNameIdx = mysql_num_fields(result) - 1;
        std::string mpRadiationName = row[mpRadiationNameIdx] ? row[mpRadiationNameIdx] : "";
        GtkWidget* nameLabel1 = gtk_label_new("辐射源名称");
        gtk_widget_set_halign(nameLabel1, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), nameLabel1, 0, row_idx, 1, 1);
        GtkWidget* valueLabel1 = gtk_label_new(mpRadiationName.c_str());
        gtk_widget_set_halign(valueLabel1, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), valueLabel1, 1, row_idx, 1, 1);
        row_idx++;
        // 关联侦察设备
        std::string deviceSql = "SELECT rdm.device_name "
                               "FROM platform_task_relation ptr "
                               "JOIN reconnaissance_device_models rdm ON ptr.device_id = rdm.device_id "
                               "WHERE ptr.simulation_id = " + std::to_string(taskId);
        if (mysql_query(conn, deviceSql.c_str()) == 0) {
            MYSQL_RES* deviceResult = mysql_store_result(conn);
            if (deviceResult) {
                std::vector<std::string> deviceNames;
                MYSQL_ROW deviceRow;
                while ((deviceRow = mysql_fetch_row(deviceResult))) {
                    if (deviceRow[0]) deviceNames.push_back(deviceRow[0]);
                }
                if (!deviceNames.empty()) {
                    GtkWidget* deviceLabel = gtk_label_new("关联侦察设备");
                    gtk_widget_set_halign(deviceLabel, GTK_ALIGN_START);
                    gtk_grid_attach(GTK_GRID(grid), deviceLabel, 0, row_idx, 1, 1);
                    std::string allDevices;
                    for (size_t i = 0; i < deviceNames.size(); ++i) {
                        if (i > 0) allDevices += "，";
                        allDevices += deviceNames[i];
                    }
                    GtkWidget* deviceValueLabel = gtk_label_new(allDevices.c_str());
                    gtk_widget_set_halign(deviceValueLabel, GTK_ALIGN_START);
                    gtk_grid_attach(GTK_GRID(grid), deviceValueLabel, 1, row_idx, 1, 1);
                    row_idx++;
                }
                mysql_free_result(deviceResult);
            }
        }
    }

    // 获取字段名和值
    unsigned int num_fields = mysql_num_fields(result);
    MYSQL_FIELD* fields = mysql_fetch_fields(result);
    // 在for循环外部声明
    int deviceNameIdx = -1, radiationNameIdx = -1;
    if (taskType == "单平台") {
        deviceNameIdx = num_fields - 2;
        radiationNameIdx = num_fields - 1;
        std::string deviceName = row[deviceNameIdx] ? row[deviceNameIdx] : "";
        std::string radiationName = row[radiationNameIdx] ? row[radiationNameIdx] : "";
        // 先显示设备名称
        GtkWidget* nameLabel1 = gtk_label_new("设备名称");
        gtk_widget_set_halign(nameLabel1, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), nameLabel1, 0, row_idx, 1, 1);
        GtkWidget* valueLabel1 = gtk_label_new(deviceName.c_str());
        gtk_widget_set_halign(valueLabel1, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), valueLabel1, 1, row_idx, 1, 1);
        row_idx++;
        // 再显示辐射源名称
        GtkWidget* nameLabel2 = gtk_label_new("辐射源名称");
        gtk_widget_set_halign(nameLabel2, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), nameLabel2, 0, row_idx, 1, 1);
        GtkWidget* valueLabel2 = gtk_label_new(radiationName.c_str());
        gtk_widget_set_halign(valueLabel2, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), valueLabel2, 1, row_idx, 1, 1);
        row_idx++;
    }
    for (unsigned int i = 0; i < num_fields; i++) {
        const char* fieldName = fields[i].name;
        const char* value = row[i] ? row[i] : "NULL";
        std::string displayName; // 先声明
        // 多平台任务字段名映射
        if (taskType == "多平台") {
            if (strcmp(fieldName, "positioning_distance") == 0) {
                displayName = "定位距离";
                std::string str = value;
                str += "m";
                value = str.c_str();
            } else if (strcmp(fieldName, "movement_speed") == 0) {
                displayName = "运动速度";
                std::string str = value;
                str += "m/s";
                value = str.c_str();
            } else if (strcmp(fieldName, "movement_azimuth") == 0) {
                displayName = "运动方位角";
                std::string str = value;
                str += "°";
                value = str.c_str();
            } else if (strcmp(fieldName, "movement_elevation") == 0) {
                displayName = "运动俯仰角";
                std::string str = value;
                str += "°";
                value = str.c_str();
            } else if (strcmp(fieldName, "azimuth") == 0) {
                displayName = "方位角";
                std::string str = value;
                str += "°";
                value = str.c_str();
            } else if (strcmp(fieldName, "elevation") == 0) {
                displayName = "俯仰角";
                std::string str = value;
                str += "°";
                value = str.c_str();
            }
        }
        // 单平台任务跳过设备ID、辐射源ID、任务ID、设备名称、辐射源名称
        if (taskType == "单平台") {
            if (strcmp(fieldName, "device_id") == 0 || strcmp(fieldName, "radiation_id") == 0 || strcmp(fieldName, "task_id") == 0) continue;
            if ((int)i == deviceNameIdx || (int)i == radiationNameIdx) continue;
        }
        // 多平台任务跳过radiation_id、radiation_name、task_id
        else if (taskType == "多平台") {
            if (strcmp(fieldName, "radiation_id") == 0 || strcmp(fieldName, "radiation_name") == 0 || strcmp(fieldName, "task_id") == 0) continue;
        }
        // 格式化字段名称
        if (displayName.empty()) {
            if (strcmp(fieldName, "tech_system") == 0) {
                // 技术体制映射
                if (strcmp(value, "FDOA") == 0) {
                    displayName = "技术体制";
                    value = "频差定位";
                } else if (strcmp(value, "TDOA") == 0) {
                    displayName = "技术体制";
                    value = "时差定位";
                } else if (strcmp(value, "INTERFEROMETER") == 0) {
                    displayName = "技术体制";
                    value = "干涉仪定位";
                } else if (strcmp(value, "FD") == 0) {
                    displayName = "技术体制";
                    value = "测向定位";
                } else {
                    displayName = "技术体制";
                }
            } else if (strcmp(fieldName, "execution_time") == 0) {
                displayName = "执行时间";
                std::string timeStr = value;
                timeStr += "s";
                value = timeStr.c_str();
            } else if (strcmp(fieldName, "target_longitude") == 0) {
                displayName = "目标经度";
                std::string str = value;
                str += "°";
                value = str.c_str();
            } else if (strcmp(fieldName, "target_latitude") == 0) {
                displayName = "目标纬度";
                std::string str = value;
                str += "°";
                value = str.c_str();
            } else if (strcmp(fieldName, "target_altitude") == 0) {
                displayName = "目标高度";
                std::string str = value;
                str += "m";
                value = str.c_str();
            } else if (strcmp(fieldName, "target_angle") == 0) {
                displayName = "测向数据(度)";
            } else if (strcmp(fieldName, "angle_error") == 0) {
                displayName = "测向误差";
                std::string str = value;
                str += "°";
                value = str.c_str();
            } else if (strcmp(fieldName, "positioning_distance") == 0) {
                displayName = "定位距离";
                std::string str = value;
                str += "m";
                value = str.c_str();
            } else if (strcmp(fieldName, "positioning_time") == 0) {
                displayName = "定位时间";
                std::string str = value;
                str += "s";
                value = str.c_str();
            } else if (strcmp(fieldName, "positioning_accuracy") == 0) {
                displayName = "定位精度";
                std::string str = value;
                str += "m";
                value = str.c_str();
            } else if (strcmp(fieldName, "direction_finding_accuracy") == 0) {
                displayName = "测向精度";
                std::string str = value;
                str += "°";
                value = str.c_str();
            } else if (strcmp(fieldName, "created_at") == 0) {
                displayName = "创建时间";
            } else if (taskType == "单平台" && strcmp(fieldName, "azimuth") == 0) {
                displayName = "方位角";
                std::string str = value;
                str += "°";
                value = str.c_str();
            } else if (taskType == "单平台" && strcmp(fieldName, "elevation") == 0) {
                displayName = "俯仰角";
                std::string str = value;
                str += "°";
                value = str.c_str();
            } else {
                // 将下划线转换为空格，首字母大写
                displayName = fieldName;
                for (char& c : displayName) {
                    if (c == '_') c = ' ';
                }
                if (!displayName.empty()) {
                    displayName[0] = toupper(displayName[0]);
                }
            }
        }
        GtkWidget* nameLabel = gtk_label_new(displayName.c_str());
        gtk_widget_set_halign(nameLabel, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), nameLabel, 0, row_idx, 1, 1);
        GtkWidget* valueLabel = gtk_label_new(value);
        gtk_widget_set_halign(valueLabel, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), valueLabel, 1, row_idx, 1, 1);
        row_idx++;
    }

    mysql_free_result(result);

    // 显示对话框
    gtk_widget_show_all(dialog);
    
    // 运行对话框并在关闭时销毁
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}