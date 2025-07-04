#include "../DataSelectionView.h"
#include "../../controllers/ApplicationController.h"
#include <gtk/gtk.h>
#include "../../models/RadiationSourceDAO.h"
#include "../../models/RadiationSourceModel.h"
#include <mysql/mysql.h>
#include "../../models/DBConnector.h"


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
    std::string singleSql = "SELECT '单平台', rd.device_name, sp.task_id, sp.target_longitude, sp.target_latitude "
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
    std::string multiSql = "SELECT '多平台', GROUP_CONCAT(rd.device_name SEPARATOR ', '), mp.task_id, mp.target_longitude, mp.target_latitude "
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
                           2, task[1].c_str(), // 侦察设备
                           3, ("坐标: (" + task[3] + ", " + task[4] + ")").c_str(), // 定位数据
                           4, ("任务ID: " + task[2]).c_str(), // 任务时间（这里用任务ID代替）
                           5, "查看", // 添加"查看"链接
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
            int radiationId = sources[activeIndex].getRadiationId(); // 假设 RadiationSource 类有 getRadiationId 方法
            view->updateTaskList(radiationId);
        }
    }
}

// 行激活事件回调函数，用于实现点击选择/取消选择
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
    
    // 如果点击的是"查看"列
    if (columnIndex == 5) {
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
            gchar *taskIdValue;
            gtk_tree_model_get(model, &iter, 4, &taskIdValue, -1);
            std::string taskIdStr = taskIdValue ? taskIdValue : "";
            g_free(taskIdValue);
            
            // 解析任务ID
            const std::string prefix = "任务ID: ";
            size_t prefixPos = taskIdStr.find(prefix);
            if (prefixPos != std::string::npos) {
                std::string idStr = taskIdStr.substr(prefixPos + prefix.length());
                try {
                    int taskId = std::stoi(idStr);
                    view->showTaskDetailsDialog(taskId, taskType);
                } catch (const std::exception& e) {
                    std::cerr << "无效任务ID: " << taskIdStr << " 错误: " << e.what() << std::endl;
                }
            }
        }
    } else {
        // 原有的选择/取消选择逻辑
        GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
        if (gtk_tree_selection_path_is_selected(selection, path)) {
            gtk_tree_selection_unselect_path(selection, path);
        } else {
            gtk_tree_selection_select_path(selection, path);
        }
    }
}

// 提示信息回调函数
static gboolean on_tree_view_query_tooltip(GtkWidget *widget, 
                                         gint x, 
                                         gint y, 
                                         gboolean keyboard_mode, 
                                         GtkTooltip *tooltip, 
                                         gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(widget);
    GtkTreePath *path = NULL;
    GtkTreeViewColumn *column = NULL;
    
    if (!gtk_tree_view_get_path_at_pos(tree_view, x, y, &path, &column, NULL, NULL))
        return FALSE;
        
    // 找到点击的列索引
    int columnIndex = -1;
    for (int i = 0; i < gtk_tree_view_get_n_columns(tree_view); i++) {
        if (gtk_tree_view_get_column(tree_view, i) == column) {
            columnIndex = i;
            break;
        }
    }
    
    // 对操作列显示提示
    if (columnIndex == 5) {
        gtk_tooltip_set_text(tooltip, "点击查看任务详情");
        gtk_tree_path_free(path);
        return TRUE;
    }
    
    gtk_tree_path_free(path);
    return FALSE;
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
                                         G_TYPE_STRING,   // 侦察设备
                                         G_TYPE_STRING,  // 定位数据
                                         G_TYPE_STRING,  // 任务时间
                                         G_TYPE_STRING);  // 查看按钮    

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
    g_signal_connect(m_dataList, "query-tooltip", G_CALLBACK(on_tree_view_query_tooltip), NULL);
    
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
    
    // 侦察设备列
    GtkCellRenderer* textRenderer2 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column2 = gtk_tree_view_column_new_with_attributes(
        "侦察设备", textRenderer2, "text", 2, NULL);
    gtk_tree_view_column_set_min_width(column2, 150);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_dataList), column2);
    
    // 定位数据列
    GtkCellRenderer* textRenderer3 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column3 = gtk_tree_view_column_new_with_attributes(
        "定位数据 (经度、纬度)", textRenderer3, "text", 3, NULL);
    gtk_tree_view_column_set_min_width(column3, 200);
    gtk_tree_view_append_column(GTK_TREE_VIEW(m_dataList), column3);

    // 任务时间列
    GtkCellRenderer* textRenderer4 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column4 = gtk_tree_view_column_new_with_attributes(
        "任务信息", textRenderer4, "text", 4, NULL);
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
        "操作", textRenderer5, "text", 5, NULL);
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

    std::vector<std::vector<std::string>> selectedData = getSelectedData();
    for (const auto& row : selectedData) {
        if (row.size() < 5) {
            std::cerr << "无效数据行，缺少必要字段" << std::endl;
            continue;
        }

        // 解析任务ID
        const std::string& taskInfo = row[4];
        const std::string prefix = "任务ID: ";
        size_t prefixPos = taskInfo.find(prefix);
        
        if (prefixPos == std::string::npos) {
            std::cerr << "任务ID格式错误: " << taskInfo << std::endl;
            continue;
        }
        
        unsigned long taskId = 0;
        try {
            // 跳过前缀（中文字符）
            std::string idStr = taskInfo.substr(prefixPos + prefix.length());
            taskId = std::stoul(idStr);
        } catch (const std::exception& e) {
            std::cerr << "无效任务ID: " << taskInfo << " 错误: " << e.what() << std::endl;
            continue;
        }

        // 确定表名和字段
        std::string tableName;
        std::string whereClause;
        if (row[1] == "单平台") {
            tableName = "single_platform_task";
            whereClause = "task_id = ?";
        } else if (row[1] == "多平台") {
            tableName = "multi_platform_task";
            whereClause = "task_id = ?";
        } else {
            std::cerr << "未知任务类型: " << row[1] << std::endl;
            continue;
        }

        // 构建预处理语句
        std::string sql = "DELETE FROM " + tableName + " WHERE " + whereClause;
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            std::cerr << "初始化声明失败" << std::endl;
            continue;
        }

        if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
            std::cerr << "准备声明失败: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            continue;
        }

        // 绑定参数
        MYSQL_BIND bind;
        memset(&bind, 0, sizeof(bind));
        bind.buffer_type = MYSQL_TYPE_LONG;
        bind.buffer = &taskId;
        bind.is_unsigned = true;

        if (mysql_stmt_bind_param(stmt, &bind) != 0) {
            std::cerr << "参数绑定失败: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            continue;
        }

        // 执行删除
        if (mysql_stmt_execute(stmt) != 0) {
            std::cerr << "删除失败: " << mysql_stmt_error(stmt) << std::endl;
        } else {
            std::cout << "成功删除任务ID: " << taskId << std::endl;
        }

        mysql_stmt_close(stmt);
    }

    // 强制刷新错误输出
    std::cerr.flush();
    fflush(stderr);

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

// 录入选中的数据项到数据库
void DataSelectionView::importSelectedItems() {
    DBConnector& db = DBConnector::getInstance();
    MYSQL* conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0) {
        std::cerr << "数据库连接异常" << std::endl;
        std::cerr.flush();
        return;
    }

    std::vector<std::vector<std::string>> selectedData = getSelectedData();
    for (const auto& row : selectedData) {
        if (row.size() < 5) {
            std::cerr << "无效数据行，缺少必要字段" << std::endl;
            continue;
        }

        // 解析任务ID
        const std::string& taskInfo = row[4];
        const std::string prefix = "任务ID: ";
        size_t prefixPos = taskInfo.find(prefix);
        
        if (prefixPos == std::string::npos) {
            std::cerr << "任务ID格式错误: " << taskInfo << std::endl;
            continue;
        }
        
        unsigned long taskId = 0;
        try {
            // 跳过前缀（中文字符）
            std::string idStr = taskInfo.substr(prefixPos + prefix.length());
            taskId = std::stoul(idStr);
        } catch (const std::exception& e) {
            std::cerr << "无效任务ID: " << taskInfo << " 错误: " << e.what() << std::endl;
            continue;
        }

        // 构建插入语句 - 修复单平台target_angle字段问题
        std::string insertSQL;
        if (row[1] == "单平台") {
            insertSQL = 
                "INSERT INTO single_platform_intelligence "
                "(radiation_id, radiation_name, target_longitude, target_latitude, target_altitude, target_angle) "
                "SELECT spt.radiation_id, rsm.radiation_name, spt.target_longitude, "
                "spt.target_latitude, spt.target_altitude, "
                // 使用COALESCE确保target_angle不为空
                "COALESCE(spt.target_angle, 0.0) "
                "FROM single_platform_task spt "
                "JOIN radiation_source_models rsm ON spt.radiation_id = rsm.radiation_id "
                "WHERE spt.task_id = ?";
        } else if (row[1] == "多平台") {
            insertSQL = 
                "INSERT INTO multi_platform_intelligence "
                "(radiation_id, radiation_name, target_longitude, target_latitude, target_altitude, "
                "movement_speed, movement_azimuth, movement_elevation) "
                "SELECT mpt.radiation_id, rsm.radiation_name, mpt.target_longitude, "
                "mpt.target_latitude, mpt.target_altitude, mpt.movement_speed, "
                "mpt.movement_azimuth, mpt.movement_elevation "
                "FROM multi_platform_task mpt "
                "JOIN radiation_source_models rsm ON mpt.radiation_id = rsm.radiation_id "
                "WHERE mpt.task_id = ?";
        } else {
            std::cerr << "未知任务类型: " << row[1] << std::endl;
            continue;
        }

        // 准备预处理语句
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt || mysql_stmt_prepare(stmt, insertSQL.c_str(), insertSQL.length()) != 0) {
            std::cerr << "准备声明失败: " << (stmt ? mysql_stmt_error(stmt) : "初始化失败") << std::endl;
            if (stmt) mysql_stmt_close(stmt);
            continue;
        }

        // 绑定参数
        MYSQL_BIND bind;
        memset(&bind, 0, sizeof(bind));
        bind.buffer_type = MYSQL_TYPE_LONG;
        bind.buffer = &taskId;
        bind.is_unsigned = true;

        if (mysql_stmt_bind_param(stmt, &bind) != 0) {
            std::cerr << "参数绑定失败: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            continue;
        }

        // 执行插入
        if (mysql_stmt_execute(stmt) != 0) {
            std::cerr << "录入失败: " << mysql_stmt_error(stmt) << std::endl;
            
            // 获取更详细的错误信息
            const char* errorMsg = mysql_stmt_error(stmt);
            if (errorMsg) {
                std::cerr << "详细错误信息: " << errorMsg << std::endl;
            }
        } else {
            std::cout << "成功录入任务ID: " << taskId << " (" << row[1] << ")" << std::endl;
        }

        mysql_stmt_close(stmt);
    }

    // 强制刷新错误输出
    std::cerr.flush();
    fflush(stderr);
}

// 获取数据筛选条件
std::string DataSelectionView::getFilterCondition() const {
    // 实现获取数据筛选条件的逻辑
    return "";
}

// 获取开始时间
std::string DataSelectionView::getStartTime() const {
    // 实现获取开始时间的逻辑
    return "";
}

// 获取结束时间
std::string DataSelectionView::getEndTime() const {
    // 实现获取结束时间的逻辑
    return "";
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

// 录入按钮点击事件回调函数，触发录入选中数据项到数据库操作。
void DataSelectionView::onImportButtonClicked(GtkWidget* widget, gpointer user_data) {
    DataSelectionView* view = static_cast<DataSelectionView*>(user_data);
    view->importSelectedItems();
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

    // 标题标签
    gchar* titleMarkup = g_markup_printf_escaped("<span font='16' weight='bold'>%s 任务详情 (ID: %d)</span>", 
                                                taskType.c_str(), taskId);
    GtkWidget* titleLabel = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(titleLabel), titleMarkup);
    g_free(titleMarkup);

    // 将标题居中并占据整行
    gtk_widget_set_halign(titleLabel, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), titleLabel, 0, 0, 2, 1);

    // 添加分隔符
    GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(grid), separator, 0, 1, 2, 1);

    // 显示数据
    int row_idx = 2;

    // 添加信息项
    auto addInfoRow = [&](const char* label, const char* value) {
        GtkWidget* nameLabel = gtk_label_new(label);
        gtk_widget_set_halign(nameLabel, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), nameLabel, 0, row_idx, 1, 1);
        
        GtkWidget* valueLabel = gtk_label_new(value);
        gtk_widget_set_halign(valueLabel, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), valueLabel, 1, row_idx, 1, 1);
        
        row_idx++;
    };

    unsigned int num_fields = mysql_num_fields(result);
    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    // 获取字段名和值
    for (unsigned int i = 0; i < num_fields; i++) {
        const char* fieldName = fields[i].name;
        const char* value = row[i] ? row[i] : "NULL";

        // 跳过任务ID字段，因为已经在标题中显示
        if (strcmp(fieldName, "task_id") == 0) {
            continue;
        }

        // 格式化字段名称
        std::string displayName;
        if (strcmp(fieldName, "tech_system") == 0) {
            displayName = "技术体制";
        } else if (strcmp(fieldName, "device_id") == 0) {
            displayName = "设备ID";
        } else if (strcmp(fieldName, "radiation_id") == 0) {
            displayName = "辐射源ID";
        } else if (strcmp(fieldName, "execution_time") == 0) {
            displayName = "执行时长(秒)";
        } else if (strcmp(fieldName, "target_longitude") == 0) {
            displayName = "目标经度(度)";
        } else if (strcmp(fieldName, "target_latitude") == 0) {
            displayName = "目标纬度(度)";
        } else if (strcmp(fieldName, "target_altitude") == 0) {
            displayName = "目标高度(米)";
        } else if (strcmp(fieldName, "target_angle") == 0) {
            displayName = "测向数据(度)";
        } else if (strcmp(fieldName, "angle_error") == 0) {
            displayName = "测向误差(度)";
        } else if (strcmp(fieldName, "max_positioning_distance") == 0) {
            displayName = "最远定位距离(米)";
        } else if (strcmp(fieldName, "positioning_time") == 0) {
            displayName = "定位时间(秒)";
        } else if (strcmp(fieldName, "positioning_accuracy") == 0) {
            displayName = "定位精度(米)";
        } else if (strcmp(fieldName, "direction_finding_accuracy") == 0) {
            displayName = "测向精度(度)";
        } else if (strcmp(fieldName, "created_at") == 0) {
            displayName = "创建时间";
        } else if (strcmp(fieldName, "device_name") == 0) {
            displayName = "设备名称";
        } else if (strcmp(fieldName, "radiation_name") == 0) {
            displayName = "辐射源名称";
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

        addInfoRow(displayName.c_str(), value);
    }

    mysql_free_result(result);

    // 如果是多平台任务，查询关联的设备
    if (taskType == "多平台") {
        std::string deviceSql = "SELECT rdm.device_name "
                               "FROM platform_task_relation ptr "
                               "JOIN reconnaissance_device_models rdm ON ptr.device_id = rdm.device_id "
                               "WHERE ptr.simulation_id = " + std::to_string(taskId);
        
        if (mysql_query(conn, deviceSql.c_str()) == 0) {
            MYSQL_RES* deviceResult = mysql_store_result(conn);
            if (deviceResult) {
                // 添加设备列表分隔符和标题
                GtkWidget* deviceSeparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
                gtk_grid_attach(GTK_GRID(grid), deviceSeparator, 0, row_idx, 2, 1);
                row_idx++;
                
                GtkWidget* deviceLabel = gtk_label_new(NULL);
                gtk_label_set_markup(GTK_LABEL(deviceLabel), "<span weight='bold'>关联侦察设备</span>");
                gtk_widget_set_halign(deviceLabel, GTK_ALIGN_START);
                gtk_grid_attach(GTK_GRID(grid), deviceLabel, 0, row_idx, 2, 1);
                row_idx++;
                
                // 列出所有关联设备
                int deviceCount = 1;
                MYSQL_ROW deviceRow;
                while ((deviceRow = mysql_fetch_row(deviceResult))) {
                    std::string deviceLabel = "设备 " + std::to_string(deviceCount);
                    addInfoRow(deviceLabel.c_str(), deviceRow[0]);
                    deviceCount++;
                }
                
                mysql_free_result(deviceResult);
            }
        }
    }

    // 显示对话框
    gtk_widget_show_all(dialog);
    
    // 运行对话框并在关闭时销毁
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}