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
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    if (gtk_tree_selection_path_is_selected(selection, path)) {
        gtk_tree_selection_unselect_path(selection, path);
    } else {
        gtk_tree_selection_select_path(selection, path);
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
    GtkListStore* store = gtk_list_store_new(5, 
                                         G_TYPE_BOOLEAN,  // 选择
                                         G_TYPE_STRING,   // 任务类型
                                         G_TYPE_STRING,   // 侦察设备
                                         G_TYPE_STRING,  // 定位数据
                                         G_TYPE_STRING);  // 任务时间    

    // 创建树视图
    m_dataList = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);

    // 设置树视图支持多行选择
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_dataList));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

    // 连接行激活事件
    g_signal_connect(G_OBJECT(m_dataList), "row-activated", G_CALLBACK(on_row_activated), this);
    
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