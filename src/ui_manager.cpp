#include "../include/ui_manager.h"
#include <iostream>

// 辅助函数，获取容器中指定索引的子控件
GtkWidget* get_child_at_index(GtkContainer* container, gint index) {
    if (!container) {
        g_print("Error: Null container passed to get_child_at_index\n");
        return NULL;
    }

    GList* children = gtk_container_get_children(container);
    GtkWidget* widget = NULL;
    
    if (children && g_list_length(children) > index) {
        widget = GTK_WIDGET(g_list_nth_data(children, index));
    } else {
        g_print("Warning: Container has %d children, but index %d was requested\n", 
               children ? g_list_length(children) : 0, index);
    }
    
    g_list_free(children);
    return widget;
}

// 绘制地图的回调函数
static gboolean drawMapCallback(GtkWidget* widget, cairo_t* cr, gpointer data) {
    // 绘制淡蓝色背景
    cairo_set_source_rgb(cr, 0.8, 0.9, 1.0);
    cairo_rectangle(cr, 0, 0, 600, 600);
    cairo_fill(cr);
    
    // 绘制经纬度网格
    cairo_set_source_rgb(cr, 0.6, 0.7, 0.8);
    cairo_set_line_width(cr, 0.5);
    
    // 绘制经度线
    for (int i = 0; i <= 12; i++) {
        cairo_move_to(cr, i * 50, 0);
        cairo_line_to(cr, i * 50, 600);
    }
    
    // 绘制纬度线
    for (int i = 0; i <= 12; i++) {
        cairo_move_to(cr, 0, i * 50);
        cairo_line_to(cr, 600, i * 50);
    }
    
    cairo_stroke(cr);
    
    // 绘制地图标题
    cairo_set_source_rgb(cr, 0.2, 0.3, 0.6);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);
    
    cairo_text_extents_t extents;
    const char* text = "地理坐标显示区域";
    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr, 300 - extents.width/2, 20);
    cairo_show_text(cr, text);
    
    return FALSE;
}

// 图表绘制回调函数
static gboolean drawChartCallback(GtkWidget* widget, cairo_t* cr, gpointer data) {
    // 绘制坐标轴
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_set_line_width(cr, 2.0);
    
    // X轴（时间）
    cairo_move_to(cr, 50, 400);
    cairo_line_to(cr, 550, 400);
    cairo_stroke(cr);
    
    // Y轴（精度）
    cairo_move_to(cr, 50, 400);
    cairo_line_to(cr, 50, 50);
    cairo_stroke(cr);
    
    // 坐标轴标签
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 14);
    
    // X轴标签
    cairo_move_to(cr, 300, 430);
    cairo_show_text(cr, "时间(s)");
    
    // Y轴标签
    cairo_move_to(cr, 10, 220);
    cairo_show_text(cr, "精度(m)");
    
    // 模拟数据点
    double points[][2] = {
        {0, 10}, {1, 8}, {2, 7}, {3, 5}, {4, 4}, 
        {5, 3.5}, {6, 3}, {7, 2.5}, {8, 2.3}, {9, 2}
    };
    
    // 绘制折线
    cairo_set_source_rgb(cr, 0.2, 0.4, 0.8);
    cairo_set_line_width(cr, 2.0);
    
    cairo_move_to(cr, 50 + points[0][0] * 50, 400 - points[0][1] * 30);
    for (int i = 1; i < 10; i++) {
        cairo_line_to(cr, 50 + points[i][0] * 50, 400 - points[i][1] * 30);
    }
    cairo_stroke(cr);
    
    // 绘制数据点
    for (int i = 0; i < 10; i++) {
        cairo_arc(cr, 50 + points[i][0] * 50, 400 - points[i][1] * 30, 4, 0, 2 * G_PI);
        cairo_fill(cr);
    }
    
    return FALSE;
}

// 单例实现
UIManager& UIManager::getInstance() {
    static UIManager instance;
    return instance;
}

UIManager::UIManager()
    : m_mainWindow(nullptr),
      m_radarDeviceModelPage(nullptr),
      m_radiationSourceModelPage(nullptr),
      m_singlePlatformPage(nullptr),
      m_multiPlatformPage(nullptr),
      m_dataSelectionPage(nullptr),
      m_evaluationPage(nullptr) {
}

UIManager::~UIManager() {
    if (m_mainWindow) {
        gtk_widget_destroy(m_mainWindow);
    }
}

bool UIManager::initUI(int argc, char** argv) {
    g_print("Starting UI initialization...\n");
    gtk_init(&argc, &argv);
    
    // 创建主窗口
    m_mainWindow = createMainWindow();
    if (!m_mainWindow) {
        g_print("Failed to create main window\n");
        return false;
    }
    
    // 创建各页面
    g_print("Creating UI pages...\n");
    
    // 单独处理每个页面创建，并添加错误检查
    g_print("  Creating radar device model page...\n");
    m_radarDeviceModelPage = createRadarDeviceModelUI();
    if (!m_radarDeviceModelPage) {
        g_print("Failed to create radar device model page\n");
        return false;
    }
    
    g_print("  Creating radiation source model page...\n");
    m_radiationSourceModelPage = createRadiationSourceModelUI();
    if (!m_radiationSourceModelPage) {
        g_print("Failed to create radiation source model page\n");
        return false;
    }
    
    g_print("  Creating single platform page...\n");
    m_singlePlatformPage = createSinglePlatformUI();
    if (!m_singlePlatformPage) {
        g_print("Failed to create single platform page\n");
        return false;
    }
    
    g_print("  Creating multi platform page...\n");
    m_multiPlatformPage = createMultiPlatformUI();
    if (!m_multiPlatformPage) {
        g_print("Failed to create multi platform page\n");
        return false;
    }
    
    g_print("  Creating data selection page...\n");
    m_dataSelectionPage = createDataSelectionUI();
    if (!m_dataSelectionPage) {
        g_print("Failed to create data selection page\n");
        return false;
    }
    
    g_print("  Creating evaluation page...\n");
    m_evaluationPage = createEvaluationUI();
    if (!m_evaluationPage) {
        g_print("Failed to create evaluation page\n");
        return false;
    }
    
    // 创建一个笔记本控件用于页面切换
    g_print("Creating notebook and adding pages...\n");
    GtkWidget* notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(m_mainWindow), notebook);
    
    // 添加页面到笔记本
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), m_radarDeviceModelPage, 
                           gtk_label_new("雷达设备模型"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), m_radiationSourceModelPage, 
                           gtk_label_new("辐射源模型"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), m_singlePlatformPage, 
                           gtk_label_new("单平台仿真"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), m_multiPlatformPage, 
                           gtk_label_new("多平台仿真"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), m_dataSelectionPage, 
                           gtk_label_new("数据分选"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), m_evaluationPage, 
                           gtk_label_new("仿真评估"));
    
    // 显示所有控件
    g_print("Showing all widgets...\n");
    gtk_widget_show_all(m_mainWindow);
    
    g_print("UI initialization completed\n");
    return true;
}

void UIManager::run() {
    gtk_main();
}

GtkWidget* UIManager::createMainWindow() {
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    gtk_window_set_title(GTK_WINDOW(window), "协同雷达侦察仿真评估系统");
    gtk_window_set_default_size(GTK_WINDOW(window), 1280, 800);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    return window;
}

// 创建通用的表格
GtkWidget* UIManager::createModelList(const std::vector<std::string>& headers) {
    g_print("Creating model list with %zu headers\n", headers.size());
    
    try {
        // 创建树视图
        GtkWidget* list = gtk_tree_view_new();
        if (!list) {
            g_print("Failed to create tree view\n");
            return NULL;
        }
        
        // 创建列存储
        // 注意：我们应该限制列的数量，避免可能的索引越界
        int numCols = headers.size() > 10 ? 10 : headers.size();
        g_print("Creating list store with %d columns\n", numCols + 1);
        
        // 根据实际头的数量创建列存储
        GtkListStore* store;
        if (numCols == 1) {
            store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
        } else if (numCols == 2) {
            store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        } else if (numCols == 3) {
            store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        } else {
            // 默认情况，应该足够用于当前应用
            store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        }
        
        if (!store) {
            g_print("Failed to create list store\n");
            gtk_widget_destroy(list);
            return NULL;
        }
        
        gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));
        g_object_unref(store);
        
        // 添加列
        for (int i = 0; i < numCols; i++) {
            GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
            if (!renderer) {
                g_print("Failed to create cell renderer for column %d\n", i);
                continue;
            }
            
            GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(
                headers[i].c_str(), renderer, "text", i, NULL);
            if (!column) {
                g_print("Failed to create column for header %s\n", headers[i].c_str());
                continue;
            }
            
            gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
        }
        
        g_print("Model list created successfully\n");
        return list;
    } catch (const std::exception& e) {
        g_print("Exception in createModelList: %s\n", e.what());
        return NULL;
    } catch (...) {
        g_print("Unknown exception in createModelList\n");
        return NULL;
    }
}

GtkWidget* UIManager::createRadarDeviceModelUI() {
    g_print("Creating radar device model UI components...\n");
    
    try {
        // 创建页面的主容器
        GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        if (!container) {
            g_print("Failed to create container\n");
            return NULL;
        }
        
        gtk_container_set_border_width(GTK_CONTAINER(container), 15);
        
        // 标题
        g_print("  Creating title label...\n");
        GtkWidget* titleLabel = gtk_label_new(NULL);
        if (!titleLabel) {
            g_print("Failed to create title label\n");
            gtk_widget_destroy(container);
            return NULL;
        }
        
        gtk_label_set_markup(GTK_LABEL(titleLabel), "<span font='16' weight='bold'>雷达设备模型管理</span>");
        gtk_box_pack_start(GTK_BOX(container), titleLabel, FALSE, FALSE, 5);
        
        // 创建表格列表
        g_print("  Creating model list...\n");
        std::vector<std::string> headers = {"名称", "位置（经度）", "位置（纬度）"};
        GtkWidget* treeView = createModelList(headers);
        if (!treeView) {
            g_print("Failed to create tree view\n");
            gtk_widget_destroy(container);
            return NULL;
        }
        
        // 滚动窗口
        g_print("  Creating scroll window...\n");
        GtkWidget* scrollWin = gtk_scrolled_window_new(NULL, NULL);
        if (!scrollWin) {
            g_print("Failed to create scroll window\n");
            gtk_widget_destroy(container);
            return NULL;
        }
        
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWin), 
                                     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_container_add(GTK_CONTAINER(scrollWin), treeView);
        
        // 设置最小高度
        gtk_widget_set_size_request(scrollWin, -1, 500);
        gtk_box_pack_start(GTK_BOX(container), scrollWin, TRUE, TRUE, 0);
        
        // 按钮区域
        g_print("  Creating button box...\n");
        GtkWidget* buttonBox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
        if (!buttonBox) {
            g_print("Failed to create button box\n");
            gtk_widget_destroy(container);
            return NULL;
        }
        
        gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonBox), GTK_BUTTONBOX_END);
        gtk_box_set_spacing(GTK_BOX(buttonBox), 10);
        gtk_box_pack_start(GTK_BOX(container), buttonBox, FALSE, FALSE, 0);
        
        // 新增按钮
        GtkWidget* addButton = gtk_button_new_with_label("新增");
        gtk_container_add(GTK_CONTAINER(buttonBox), addButton);
        g_signal_connect(addButton, "clicked", G_CALLBACK(onAddRadarDevice), treeView);
        
        // 编辑按钮
        GtkWidget* editButton = gtk_button_new_with_label("编辑");
        gtk_container_add(GTK_CONTAINER(buttonBox), editButton);
        g_signal_connect(editButton, "clicked", G_CALLBACK(onEditRadarDevice), treeView);
        
        // 删除按钮
        GtkWidget* deleteButton = gtk_button_new_with_label("删除");
        gtk_container_add(GTK_CONTAINER(buttonBox), deleteButton);
        g_signal_connect(deleteButton, "clicked", G_CALLBACK(onDeleteRadarDevice), treeView);
        
        // 更新列表数据
        g_print("  Updating radar device list...\n");
        updateRadarDeviceList(treeView);
        
        g_print("Radar device model UI created successfully\n");
        return container;
    } catch (const std::exception& e) {
        g_print("Exception in createRadarDeviceModelUI: %s\n", e.what());
        return NULL;
    } catch (...) {
        g_print("Unknown exception in createRadarDeviceModelUI\n");
        return NULL;
    }
}

void UIManager::updateRadarDeviceList(GtkWidget* list) {
    // 这里应该从数据库获取雷达设备模型列表
    // 现在先添加示例数据
    GtkListStore* store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
    gtk_list_store_clear(store);
    
    GtkTreeIter iter;
    
    // 示例数据1
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 
                      0, "雷达设备1", 
                      1, "116.404", 
                      2, "39.915", 
                      -1);
    
    // 示例数据2
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 
                      0, "雷达设备2", 
                      1, "121.473", 
                      2, "31.233", 
                      -1);
}

// 雷达设备模型对话框回调函数
void UIManager::onAddRadarDevice(GtkWidget* widget, gpointer data) {
    GtkWidget* dialog = gtk_dialog_new_with_buttons("新增雷达设备模型",
                                                   GTK_WINDOW(UIManager::getInstance().m_mainWindow),
                                                   GTK_DIALOG_MODAL,
                                                   "取消", GTK_RESPONSE_CANCEL,
                                                   "确定", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    GtkWidget* contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(contentArea), 15);
    
    // 名称输入
    GtkWidget* nameBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(contentArea), nameBox, FALSE, FALSE, 5);
    
    GtkWidget* nameLabel = gtk_label_new("名称:");
    gtk_box_pack_start(GTK_BOX(nameBox), nameLabel, FALSE, FALSE, 5);
    
    GtkWidget* nameEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(nameBox), nameEntry, TRUE, TRUE, 5);
    
    // 经度输入
    GtkWidget* lonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(contentArea), lonBox, FALSE, FALSE, 5);
    
    GtkWidget* lonLabel = gtk_label_new("经度:");
    gtk_box_pack_start(GTK_BOX(lonBox), lonLabel, FALSE, FALSE, 5);
    
    GtkWidget* lonEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(lonBox), lonEntry, TRUE, TRUE, 5);
    
    // 纬度输入
    GtkWidget* latBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(contentArea), latBox, FALSE, FALSE, 5);
    
    GtkWidget* latLabel = gtk_label_new("纬度:");
    gtk_box_pack_start(GTK_BOX(latBox), latLabel, FALSE, FALSE, 5);
    
    GtkWidget* latEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(latBox), latEntry, TRUE, TRUE, 5);
    
    // 技术体制
    GtkWidget* techBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(contentArea), techBox, FALSE, FALSE, 5);
    
    GtkWidget* techLabel = gtk_label_new("技术体制:");
    gtk_box_pack_start(GTK_BOX(techBox), techLabel, FALSE, FALSE, 5);
    
    GtkWidget* techCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(techCombo), "干涉仪体制");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(techCombo), "时差体制");
    gtk_combo_box_set_active(GTK_COMBO_BOX(techCombo), 0);
    gtk_box_pack_start(GTK_BOX(techBox), techCombo, TRUE, TRUE, 5);
    
    // 基本参数输入
    GtkWidget* basicBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(contentArea), basicBox, FALSE, FALSE, 5);
    
    GtkWidget* basicLabel = gtk_label_new("基本参数:");
    gtk_box_pack_start(GTK_BOX(basicBox), basicLabel, FALSE, FALSE, 5);
    
    GtkWidget* basicEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(basicBox), basicEntry, TRUE, TRUE, 5);
    
    // 工作参数输入
    GtkWidget* workBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(contentArea), workBox, FALSE, FALSE, 5);
    
    GtkWidget* workLabel = gtk_label_new("工作参数:");
    gtk_box_pack_start(GTK_BOX(workBox), workLabel, FALSE, FALSE, 5);
    
    GtkWidget* workEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(workBox), workEntry, TRUE, TRUE, 5);
    
    gtk_widget_show_all(dialog);
    
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        // 获取输入值并添加到数据库
        // 然后更新列表
        UIManager::getInstance().updateRadarDeviceList(GTK_WIDGET(data));
    }
    
    gtk_widget_destroy(dialog);
}

void UIManager::onEditRadarDevice(GtkWidget* widget, gpointer data) {
    // 获取选中的行
    GtkTreeView* treeView = GTK_TREE_VIEW(data);
    GtkTreeSelection* selection = gtk_tree_view_get_selection(treeView);
    GtkTreeModel* model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        // 获取选中的数据
        gchar* name;
        gchar* longitude;
        gchar* latitude;
        
        gtk_tree_model_get(model, &iter, 
                          0, &name, 
                          1, &longitude, 
                          2, &latitude, 
                          -1);
        
        // 创建编辑对话框，这里简化为与新增相同
        onAddRadarDevice(widget, data);
        
        g_free(name);
        g_free(longitude);
        g_free(latitude);
    }
}

void UIManager::onDeleteRadarDevice(GtkWidget* widget, gpointer data) {
    // 获取选中的行
    GtkTreeView* treeView = GTK_TREE_VIEW(data);
    GtkTreeSelection* selection = gtk_tree_view_get_selection(treeView);
    GtkTreeModel* model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        // 从数据库删除对应的记录
        // 然后从列表中删除
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    }
}

GtkWidget* UIManager::createRadiationSourceModelUI() {
    g_print("Creating radiation source model UI components...\n");
    
    try {
        // 创建页面的主容器
        GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        if (!container) {
            g_print("Failed to create container for radiation source model UI\n");
            return NULL;
        }
        
        gtk_container_set_border_width(GTK_CONTAINER(container), 15);
        
        // 标题
        g_print("  Creating title label...\n");
        GtkWidget* titleLabel = gtk_label_new(NULL);
        if (!titleLabel) {
            g_print("Failed to create title label\n");
            gtk_widget_destroy(container);
            return NULL;
        }
        
        gtk_label_set_markup(GTK_LABEL(titleLabel), "<span font='16' weight='bold'>辐射源模型管理</span>");
        gtk_box_pack_start(GTK_BOX(container), titleLabel, FALSE, FALSE, 5);
        
        // 创建表格列表
        g_print("  Creating radiation source model list...\n");
        std::vector<std::string> headers = {"名称", "位置（经度）", "位置（纬度）"};
        GtkWidget* treeView = createModelList(headers);
        if (!treeView) {
            g_print("Failed to create tree view for radiation source\n");
            gtk_widget_destroy(container);
            return NULL;
        }
        
        // 滚动窗口
        g_print("  Creating scroll window...\n");
        GtkWidget* scrollWin = gtk_scrolled_window_new(NULL, NULL);
        if (!scrollWin) {
            g_print("Failed to create scroll window\n");
            gtk_widget_destroy(container);
            return NULL;
        }
        
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWin), 
                                     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_container_add(GTK_CONTAINER(scrollWin), treeView);
        
        // 设置最小高度
        gtk_widget_set_size_request(scrollWin, -1, 500);
        gtk_box_pack_start(GTK_BOX(container), scrollWin, TRUE, TRUE, 0);
        
        // 按钮区域
        g_print("  Creating button box...\n");
        GtkWidget* buttonBox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
        if (!buttonBox) {
            g_print("Failed to create button box\n");
            gtk_widget_destroy(container);
            return NULL;
        }
        
        gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonBox), GTK_BUTTONBOX_END);
        gtk_box_set_spacing(GTK_BOX(buttonBox), 10);
        gtk_box_pack_start(GTK_BOX(container), buttonBox, FALSE, FALSE, 0);
        
        // 新增按钮
        GtkWidget* addButton = gtk_button_new_with_label("新增");
        if (!addButton) {
            g_print("Failed to create add button\n");
            gtk_widget_destroy(container);
            return NULL;
        }
        
        gtk_container_add(GTK_CONTAINER(buttonBox), addButton);
        g_signal_connect(addButton, "clicked", G_CALLBACK(onAddRadiationSource), treeView);
        
        // 编辑按钮
        GtkWidget* editButton = gtk_button_new_with_label("编辑");
        if (!editButton) {
            g_print("Failed to create edit button\n");
            gtk_widget_destroy(container);
            return NULL;
        }
        
        gtk_container_add(GTK_CONTAINER(buttonBox), editButton);
        g_signal_connect(editButton, "clicked", G_CALLBACK(onEditRadiationSource), treeView);
        
        // 删除按钮
        GtkWidget* deleteButton = gtk_button_new_with_label("删除");
        if (!deleteButton) {
            g_print("Failed to create delete button\n");
            gtk_widget_destroy(container);
            return NULL;
        }
        
        gtk_container_add(GTK_CONTAINER(buttonBox), deleteButton);
        g_signal_connect(deleteButton, "clicked", G_CALLBACK(onDeleteRadiationSource), treeView);
        
        // 更新列表数据
        g_print("  Updating radiation source list...\n");
        updateRadiationSourceList(treeView);
        
        g_print("Radiation source model UI created successfully\n");
        return container;
    } catch (const std::exception& e) {
        g_print("Exception in createRadiationSourceModelUI: %s\n", e.what());
        return NULL;
    } catch (...) {
        g_print("Unknown exception in createRadiationSourceModelUI\n");
        return NULL;
    }
}

void UIManager::updateRadiationSourceList(GtkWidget* list) {
    // 这里应该从数据库获取辐射源模型列表
    // 现在先添加示例数据
    GtkListStore* store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
    gtk_list_store_clear(store);
    
    GtkTreeIter iter;
    
    // 示例数据1
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 
                      0, "辐射源1", 
                      1, "116.404", 
                      2, "39.915", 
                      -1);
    
    // 示例数据2
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 
                      0, "辐射源2", 
                      1, "121.473", 
                      2, "31.233", 
                      -1);
}

// 辐射源模型对话框回调函数
void UIManager::onAddRadiationSource(GtkWidget* widget, gpointer data) {
    GtkWidget* dialog = gtk_dialog_new_with_buttons("新增辐射源模型",
                                                   GTK_WINDOW(UIManager::getInstance().m_mainWindow),
                                                   GTK_DIALOG_MODAL,
                                                   "取消", GTK_RESPONSE_CANCEL,
                                                   "确定", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    GtkWidget* contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(contentArea), 15);
    
    // 名称输入
    GtkWidget* nameBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(contentArea), nameBox, FALSE, FALSE, 5);
    
    GtkWidget* nameLabel = gtk_label_new("名称:");
    gtk_box_pack_start(GTK_BOX(nameBox), nameLabel, FALSE, FALSE, 5);
    
    GtkWidget* nameEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(nameBox), nameEntry, TRUE, TRUE, 5);
    
    // 经度输入
    GtkWidget* lonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(contentArea), lonBox, FALSE, FALSE, 5);
    
    GtkWidget* lonLabel = gtk_label_new("经度:");
    gtk_box_pack_start(GTK_BOX(lonBox), lonLabel, FALSE, FALSE, 5);
    
    GtkWidget* lonEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(lonBox), lonEntry, TRUE, TRUE, 5);
    
    // 纬度输入
    GtkWidget* latBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(contentArea), latBox, FALSE, FALSE, 5);
    
    GtkWidget* latLabel = gtk_label_new("纬度:");
    gtk_box_pack_start(GTK_BOX(latBox), latLabel, FALSE, FALSE, 5);
    
    GtkWidget* latEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(latBox), latEntry, TRUE, TRUE, 5);
    
    // 发射功率
    GtkWidget* powerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(contentArea), powerBox, FALSE, FALSE, 5);
    
    GtkWidget* powerLabel = gtk_label_new("发射功率:");
    gtk_box_pack_start(GTK_BOX(powerBox), powerLabel, FALSE, FALSE, 5);
    
    GtkWidget* powerEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(powerBox), powerEntry, TRUE, TRUE, 5);
    
    // 扫描周期
    GtkWidget* periodBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(contentArea), periodBox, FALSE, FALSE, 5);
    
    GtkWidget* periodLabel = gtk_label_new("扫描周期:");
    gtk_box_pack_start(GTK_BOX(periodBox), periodLabel, FALSE, FALSE, 5);
    
    GtkWidget* periodEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(periodBox), periodEntry, TRUE, TRUE, 5);
    
    // 频率范围
    GtkWidget* freqBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(contentArea), freqBox, FALSE, FALSE, 5);
    
    GtkWidget* freqLabel = gtk_label_new("频率范围:");
    gtk_box_pack_start(GTK_BOX(freqBox), freqLabel, FALSE, FALSE, 5);
    
    GtkWidget* freqMinEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(freqMinEntry), "最小值");
    gtk_box_pack_start(GTK_BOX(freqBox), freqMinEntry, TRUE, TRUE, 5);
    
    GtkWidget* freqMaxEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(freqMaxEntry), "最大值");
    gtk_box_pack_start(GTK_BOX(freqBox), freqMaxEntry, TRUE, TRUE, 5);
    
    // 工作扇区
    GtkWidget* sectorBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(contentArea), sectorBox, FALSE, FALSE, 5);
    
    GtkWidget* sectorLabel = gtk_label_new("工作扇区:");
    gtk_box_pack_start(GTK_BOX(sectorBox), sectorLabel, FALSE, FALSE, 5);
    
    GtkWidget* sectorEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(sectorBox), sectorEntry, TRUE, TRUE, 5);
    
    gtk_widget_show_all(dialog);
    
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        // 获取输入值并添加到数据库
        // 然后更新列表
        UIManager::getInstance().updateRadiationSourceList(GTK_WIDGET(data));
    }
    
    gtk_widget_destroy(dialog);
}

void UIManager::onEditRadiationSource(GtkWidget* widget, gpointer data) {
    // 获取选中的行
    GtkTreeView* treeView = GTK_TREE_VIEW(data);
    GtkTreeSelection* selection = gtk_tree_view_get_selection(treeView);
    GtkTreeModel* model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        // 获取选中的数据
        gchar* name;
        gchar* longitude;
        gchar* latitude;
        
        gtk_tree_model_get(model, &iter, 
                          0, &name, 
                          1, &longitude, 
                          2, &latitude, 
                          -1);
        
        // 创建编辑对话框，这里简化为与新增相同
        onAddRadiationSource(widget, data);
        
        g_free(name);
        g_free(longitude);
        g_free(latitude);
    }
}

void UIManager::onDeleteRadiationSource(GtkWidget* widget, gpointer data) {
    // 获取选中的行
    GtkTreeView* treeView = GTK_TREE_VIEW(data);
    GtkTreeSelection* selection = gtk_tree_view_get_selection(treeView);
    GtkTreeModel* model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        // 从数据库删除对应的记录
        // 然后从列表中删除
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    }
}

GtkWidget* UIManager::createSinglePlatformUI() {
    // 创建页面的主容器
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(container), 15);
    
    // 左侧：地图显示区域
    GtkWidget* mapFrame = gtk_frame_new("地图");
    gtk_widget_set_size_request(mapFrame, 800, 700);
    gtk_box_pack_start(GTK_BOX(container), mapFrame, TRUE, TRUE, 0);
    
    // 显示地图
    showMap(mapFrame);
    
    // 右侧：参数设置和结果区域
    GtkWidget* rightBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(container), rightBox, FALSE, FALSE, 0);
    
    // 雷达设备模型选择
    GtkWidget* radarFrame = gtk_frame_new("雷达设备模型");
    gtk_box_pack_start(GTK_BOX(rightBox), radarFrame, FALSE, FALSE, 0);
    
    GtkWidget* radarBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(radarFrame), radarBox);
    gtk_container_set_border_width(GTK_CONTAINER(radarBox), 10);
    
    GtkWidget* radarCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(radarCombo), "雷达设备1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(radarCombo), "雷达设备2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(radarCombo), 0);
    gtk_box_pack_start(GTK_BOX(radarBox), radarCombo, TRUE, TRUE, 5);
    
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
    
    // 定位算法选择
    GtkWidget* algoFrame = gtk_frame_new("定位算法");
    gtk_box_pack_start(GTK_BOX(rightBox), algoFrame, FALSE, FALSE, 0);
    
    GtkWidget* algoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(algoFrame), algoBox);
    gtk_container_set_border_width(GTK_CONTAINER(algoBox), 10);
    
    GtkWidget* algoCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algoCombo), "快速定位");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algoCombo), "基线定位");
    gtk_combo_box_set_active(GTK_COMBO_BOX(algoCombo), 0);
    gtk_box_pack_start(GTK_BOX(algoBox), algoCombo, TRUE, TRUE, 5);
    
    // 开始按钮
    GtkWidget* startButton = gtk_button_new_with_label("开始");
    gtk_widget_set_size_request(startButton, -1, 40);
    gtk_box_pack_start(GTK_BOX(rightBox), startButton, FALSE, FALSE, 10);
    g_signal_connect(startButton, "clicked", G_CALLBACK(onSinglePlatformSimulation), NULL);
    
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
    
    // 创建表格
    GtkWidget* table = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(table), 5);
    gtk_grid_set_column_spacing(GTK_GRID(table), 10);
    gtk_box_pack_start(GTK_BOX(resultBox), table, TRUE, TRUE, 0);
    
    // 添加表头
    GtkWidget* powerLabel = gtk_label_new("威力");
    gtk_widget_set_halign(powerLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), powerLabel, 0, 0, 1, 1);
    
    GtkWidget* dirErrorLabel = gtk_label_new("测向误差");
    gtk_widget_set_halign(dirErrorLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), dirErrorLabel, 0, 1, 1, 1);
    
    GtkWidget* paramErrorLabel = gtk_label_new("参数测量误差");
    gtk_widget_set_halign(paramErrorLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), paramErrorLabel, 0, 2, 1, 1);
    
    // 添加结果值（初始为空）
    GtkWidget* powerValue = gtk_label_new("--");
    gtk_widget_set_halign(powerValue, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), powerValue, 1, 0, 1, 1);
    
    GtkWidget* dirErrorValue = gtk_label_new("--");
    gtk_widget_set_halign(dirErrorValue, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), dirErrorValue, 1, 1, 1, 1);
    
    GtkWidget* paramErrorValue = gtk_label_new("--");
    gtk_widget_set_halign(paramErrorValue, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), paramErrorValue, 1, 2, 1, 1);
    
    return container;
}

// 单平台仿真回调
void UIManager::onSinglePlatformSimulation(GtkWidget* widget, gpointer data) {
    g_print("Starting single platform simulation...\n");
    // 这里应该调用仿真引擎进行仿真计算
    // 现在简单模拟
    
    // 获取界面上的控件并更新结果
    GtkWidget* window = UIManager::getInstance().m_mainWindow;
    if (!window) {
        g_print("Error: Main window is NULL\n");
        return;
    }
    
    GtkWidget* notebook = gtk_bin_get_child(GTK_BIN(window));
    if (!notebook) {
        g_print("Error: Notebook is NULL\n");
        return;
    }
    
    GtkWidget* singlePlatformPage = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 2);
    if (!singlePlatformPage) {
        g_print("Error: Single platform page is NULL\n");
        return;
    }
    
    // 找到结果区域
    GtkWidget* container = GTK_WIDGET(singlePlatformPage);
    GtkWidget* hbox = gtk_bin_get_child(GTK_BIN(container));
    if (!hbox) {
        g_print("Error: Horizontal box is NULL\n");
        return;
    }
    
    GtkWidget* rightBox = get_child_at_index(GTK_CONTAINER(hbox), 1);
    if (!rightBox) {
        g_print("Error: Right box is NULL\n");
        return;
    }
    
    GtkWidget* resultFrame = get_child_at_index(GTK_CONTAINER(rightBox), 4);
    if (!resultFrame) {
        g_print("Error: Result frame is NULL\n");
        return;
    }
    
    GtkWidget* resultBox = gtk_bin_get_child(GTK_BIN(resultFrame));
    if (!resultBox) {
        g_print("Error: Result box is NULL\n");
        return;
    }
    
    GtkWidget* table = get_child_at_index(GTK_CONTAINER(resultBox), 0);
    if (!table) {
        g_print("Error: Table is NULL\n");
        return;
    }
    
    // 更新表格的结果值
    GtkWidget* powerValue = gtk_grid_get_child_at(GTK_GRID(table), 1, 0);
    GtkWidget* dirErrorValue = gtk_grid_get_child_at(GTK_GRID(table), 1, 1);
    GtkWidget* paramErrorValue = gtk_grid_get_child_at(GTK_GRID(table), 1, 2);
    
    // 设置模拟结果值
    if (powerValue) gtk_label_set_text(GTK_LABEL(powerValue), "85.6");
    if (dirErrorValue) gtk_label_set_text(GTK_LABEL(dirErrorValue), "0.23°");
    if (paramErrorValue) gtk_label_set_text(GTK_LABEL(paramErrorValue), "0.15");
    
    // 在地图上添加模拟的目标位置
    GtkWidget* mapFrame = get_child_at_index(GTK_CONTAINER(hbox), 0);
    if (!mapFrame) {
        g_print("Error: Map frame is NULL\n");
        return;
    }
    
    GtkWidget* drawingArea = gtk_bin_get_child(GTK_BIN(mapFrame));
    if (!drawingArea) {
        g_print("Error: Drawing area is NULL\n");
        return;
    }
    
    // 触发地图重绘
    gtk_widget_queue_draw(drawingArea);
    g_print("Single platform simulation completed\n");
}

// 多平台协同侦察UI创建 (简化版)
GtkWidget* UIManager::createMultiPlatformUI() {
    // 创建页面的主容器
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(container), 15);
    
    // 左侧：地图显示区域
    GtkWidget* mapFrame = gtk_frame_new("地图");
    gtk_widget_set_size_request(mapFrame, 800, 700);
    gtk_box_pack_start(GTK_BOX(container), mapFrame, TRUE, TRUE, 0);
    
    // 显示地图
    showMap(mapFrame);
    
    // 右侧：参数设置和结果区域
    GtkWidget* rightBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(container), rightBox, FALSE, FALSE, 0);
    
    // 雷达设备模型选择1
    GtkWidget* radar1Frame = gtk_frame_new("雷达设备模型1");
    gtk_box_pack_start(GTK_BOX(rightBox), radar1Frame, FALSE, FALSE, 0);
    
    GtkWidget* radar1Box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(radar1Frame), radar1Box);
    gtk_container_set_border_width(GTK_CONTAINER(radar1Box), 10);
    
    GtkWidget* radar1Combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(radar1Combo), "雷达设备1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(radar1Combo), "雷达设备2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(radar1Combo), 0);
    gtk_box_pack_start(GTK_BOX(radar1Box), radar1Combo, TRUE, TRUE, 5);
    
    // 雷达设备模型选择2
    GtkWidget* radar2Frame = gtk_frame_new("雷达设备模型2");
    gtk_box_pack_start(GTK_BOX(rightBox), radar2Frame, FALSE, FALSE, 0);
    
    GtkWidget* radar2Box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(radar2Frame), radar2Box);
    gtk_container_set_border_width(GTK_CONTAINER(radar2Box), 10);
    
    GtkWidget* radar2Combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(radar2Combo), "雷达设备1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(radar2Combo), "雷达设备2");
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
    
    // 定位算法选择
    GtkWidget* algoFrame = gtk_frame_new("定位算法");
    gtk_box_pack_start(GTK_BOX(rightBox), algoFrame, FALSE, FALSE, 0);
    
    GtkWidget* algoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(algoFrame), algoBox);
    gtk_container_set_border_width(GTK_CONTAINER(algoBox), 10);
    
    GtkWidget* algoCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algoCombo), "时差定位");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algoCombo), "频差定位");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algoCombo), "测向定位");
    gtk_combo_box_set_active(GTK_COMBO_BOX(algoCombo), 0);
    gtk_box_pack_start(GTK_BOX(algoBox), algoCombo, TRUE, TRUE, 5);
    
    // 开始按钮
    GtkWidget* startButton = gtk_button_new_with_label("开始");
    gtk_widget_set_size_request(startButton, -1, 40);
    gtk_box_pack_start(GTK_BOX(rightBox), startButton, FALSE, FALSE, 10);
    g_signal_connect(startButton, "clicked", G_CALLBACK(onMultiPlatformSimulation), NULL);
    
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
    
    return container;
}

// 多平台仿真回调
void UIManager::onMultiPlatformSimulation(GtkWidget* widget, gpointer data) {
    // 这里应该调用仿真引擎进行多平台协同仿真计算
    // 现在简单模拟
    
    // 获取界面上的控件
    GtkWidget* window = UIManager::getInstance().m_mainWindow;
    GtkWidget* notebook = gtk_bin_get_child(GTK_BIN(window));
    GtkWidget* multiPlatformPage = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 3);
    
    // 获取地图
    GtkWidget* container = GTK_WIDGET(multiPlatformPage);
    GtkWidget* hbox = gtk_bin_get_child(GTK_BIN(container));
    GtkWidget* mapFrame = get_child_at_index(GTK_CONTAINER(hbox), 0);
    GtkWidget* drawingArea = gtk_bin_get_child(GTK_BIN(mapFrame));
    
    // 触发地图重绘
    gtk_widget_queue_draw(drawingArea);
}

// 创建数据显示UI (简化版，仅用于兼容)
GtkWidget* UIManager::createDataDisplayUI() {
    // 这个函数不再需要，使用createDataSelectionUI代替
    // 为了兼容性，返回一个空容器
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    return container;
}

// 创建数据分选UI (简化版)
GtkWidget* UIManager::createDataSelectionUI() {
    // 创建页面的主容器
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(container), 15);
    
    // 左侧：地图显示区域
    GtkWidget* mapFrame = gtk_frame_new("地图");
    gtk_widget_set_size_request(mapFrame, 800, 700);
    gtk_box_pack_start(GTK_BOX(container), mapFrame, TRUE, TRUE, 0);
    
    // 显示地图
    showMap(mapFrame);
    
    // 右侧：目标选择和数据列表区域
    GtkWidget* rightBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(container), rightBox, FALSE, FALSE, 0);
    
    // 目标选择区域
    GtkWidget* targetFrame = gtk_frame_new("目标选择");
    gtk_box_pack_start(GTK_BOX(rightBox), targetFrame, FALSE, FALSE, 0);
    
    GtkWidget* targetBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(targetFrame), targetBox);
    gtk_container_set_border_width(GTK_CONTAINER(targetBox), 10);
    
    GtkWidget* targetCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(targetCombo), "目标1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(targetCombo), "目标2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(targetCombo), 0);
    gtk_box_pack_start(GTK_BOX(targetBox), targetCombo, TRUE, TRUE, 5);
    g_signal_connect(targetCombo, "changed", G_CALLBACK(onDataSelectionChanged), NULL);
    
    // 数据列表区域
    GtkWidget* dataFrame = gtk_frame_new("目标数据列表");
    gtk_box_pack_start(GTK_BOX(rightBox), dataFrame, TRUE, TRUE, 0);
    
    GtkWidget* dataBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(dataFrame), dataBox);
    gtk_container_set_border_width(GTK_CONTAINER(dataBox), 10);
    
    // 创建列表
    GtkListStore* store = gtk_list_store_new(5, 
                                           G_TYPE_BOOLEAN,  // 选择列
                                           G_TYPE_STRING,   // 预警机ID
                                           G_TYPE_STRING,   // 测向数据
                                           G_TYPE_STRING,   // 定位数据
                                           G_TYPE_STRING);  // 时间戳
    
    GtkWidget* treeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    // 添加选择列
    GtkCellRenderer* toggleRenderer = gtk_cell_renderer_toggle_new();
    GtkTreeViewColumn* toggleColumn = gtk_tree_view_column_new_with_attributes(
        "选择", toggleRenderer, "active", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), toggleColumn);
    
    // 添加预警机ID列
    GtkCellRenderer* textRenderer1 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column1 = gtk_tree_view_column_new_with_attributes(
        "预警机ID", textRenderer1, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column1);
    
    // 添加测向数据列
    GtkCellRenderer* textRenderer2 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column2 = gtk_tree_view_column_new_with_attributes(
        "测向数据", textRenderer2, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column2);
    
    // 添加定位数据列
    GtkCellRenderer* textRenderer3 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column3 = gtk_tree_view_column_new_with_attributes(
        "定位数据", textRenderer3, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column3);
    
    // 添加时间戳列
    GtkCellRenderer* textRenderer4 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column4 = gtk_tree_view_column_new_with_attributes(
        "时间戳", textRenderer4, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column4);
    
    // 添加示例数据
    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 
                      0, FALSE,
                      1, "预警机A", 
                      2, "方位角: 45°", 
                      3, "坐标: (116.5, 39.9)", 
                      4, "2023-05-15 14:30:25", 
                      -1);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 
                      0, FALSE,
                      1, "预警机B", 
                      2, "方位角: 130°", 
                      3, "坐标: (116.4, 39.8)", 
                      4, "2023-05-15 14:31:10", 
                      -1);
    
    // 滚动窗口
    GtkWidget* scrollWin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWin), 
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrollWin), treeView);
    gtk_box_pack_start(GTK_BOX(dataBox), scrollWin, TRUE, TRUE, 0);
    
    // 按钮区域
    GtkWidget* buttonBox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonBox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing(GTK_BOX(buttonBox), 10);
    gtk_box_pack_start(GTK_BOX(dataBox), buttonBox, FALSE, FALSE, 10);
    
    // 删除按钮
    GtkWidget* deleteButton = gtk_button_new_with_label("删除");
    gtk_container_add(GTK_CONTAINER(buttonBox), deleteButton);
    
    // 高亮显示按钮
    GtkWidget* highlightButton = gtk_button_new_with_label("高亮显示");
    gtk_container_add(GTK_CONTAINER(buttonBox), highlightButton);
    
    // 录入按钮
    GtkWidget* importButton = gtk_button_new_with_label("录入");
    gtk_container_add(GTK_CONTAINER(buttonBox), importButton);
    g_signal_connect(importButton, "clicked", G_CALLBACK(onDataImport), treeView);
    
    // CSS样式
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "button {\n"
        "  background-image: linear-gradient(to bottom, #27ae60, #2ecc71);\n"
        "  color: white;\n"
        "  border-radius: 3px;\n"
        "  font-weight: bold;\n"
        "}\n"
        "button:hover {\n"
        "  background-image: linear-gradient(to bottom, #2ecc71, #27ae60);\n"
        "}\n", -1, NULL);
    
    GtkStyleContext* context;
    
    context = gtk_widget_get_style_context(deleteButton);
    gtk_style_context_add_provider(context,
                                 GTK_STYLE_PROVIDER(provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    context = gtk_widget_get_style_context(highlightButton);
    gtk_style_context_add_provider(context,
                                 GTK_STYLE_PROVIDER(provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    context = gtk_widget_get_style_context(importButton);
    gtk_style_context_add_provider(context,
                                 GTK_STYLE_PROVIDER(provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    g_object_unref(provider);
    
    return container;
}

void UIManager::onDataSelectionChanged(GtkWidget* widget, gpointer data) {
    // 获取选中的目标
    gchar* selected_text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    if (selected_text) {
        // 根据选中的目标从数据库加载对应的数据
        // 并更新地图上的显示
        
        g_free(selected_text);
    }
}

void UIManager::onDataImport(GtkWidget* widget, gpointer data) {
    // 获取选中的数据行
    GtkTreeView* treeView = GTK_TREE_VIEW(data);
    GtkTreeModel* model = gtk_tree_view_get_model(treeView);
    
    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first(model, &iter);
    
    while (valid) {
        gboolean selected;
        gtk_tree_model_get(model, &iter, 0, &selected, -1);
        
        if (selected) {
            // 处理选中的数据
            gchar* id, *dir_data, *loc_data, *timestamp;
            gtk_tree_model_get(model, &iter, 
                              1, &id, 
                              2, &dir_data, 
                              3, &loc_data, 
                              4, &timestamp, 
                              -1);
            
            // 将数据存入数据库
            // 这里只是简单的打印
            g_print("录入数据: ID=%s, 测向=%s, 定位=%s, 时间=%s\n", 
                   id, dir_data, loc_data, timestamp);
            
            g_free(id);
            g_free(dir_data);
            g_free(loc_data);
            g_free(timestamp);
        }
        
        valid = gtk_tree_model_iter_next(model, &iter);
    }
}

// 创建评估UI (简化版)
GtkWidget* UIManager::createEvaluationUI() {
    // 创建页面的主容器
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(container), 15);
    
    // 标题
    GtkWidget* titleLabel = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(titleLabel), "<span font='16' weight='bold'>协同定位评估</span>");
    gtk_box_pack_start(GTK_BOX(container), titleLabel, FALSE, FALSE, 5);
    
    // 参数选择区域
    GtkWidget* paramFrame = gtk_frame_new("评估参数");
    gtk_box_pack_start(GTK_BOX(container), paramFrame, FALSE, FALSE, 0);
    
    GtkWidget* paramBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_container_add(GTK_CONTAINER(paramFrame), paramBox);
    gtk_container_set_border_width(GTK_CONTAINER(paramBox), 10);
    
    // 目标选择
    GtkWidget* targetBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(paramBox), targetBox, TRUE, TRUE, 0);
    
    GtkWidget* targetLabel = gtk_label_new("目标选择");
    gtk_widget_set_halign(targetLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(targetBox), targetLabel, FALSE, FALSE, 0);
    
    GtkWidget* targetCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(targetCombo), "目标1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(targetCombo), "目标2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(targetCombo), 0);
    gtk_box_pack_start(GTK_BOX(targetBox), targetCombo, TRUE, TRUE, 5);
    
    // 评估类型
    GtkWidget* typeBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(paramBox), typeBox, TRUE, TRUE, 0);
    
    GtkWidget* typeLabel = gtk_label_new("评估类型");
    gtk_widget_set_halign(typeLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(typeBox), typeLabel, FALSE, FALSE, 0);
    
    GtkWidget* singleRadio = gtk_radio_button_new_with_label(NULL, "单平台");
    gtk_box_pack_start(GTK_BOX(typeBox), singleRadio, FALSE, FALSE, 0);
    
    GtkWidget* multiRadio = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(singleRadio), "多平台");
    gtk_box_pack_start(GTK_BOX(typeBox), multiRadio, FALSE, FALSE, 0);
    
    // 开始按钮
    GtkWidget* startButton = gtk_button_new_with_label("开始评估");
    gtk_widget_set_size_request(startButton, 100, 35);
    gtk_box_pack_start(GTK_BOX(paramBox), startButton, FALSE, FALSE, 0);
    g_signal_connect(startButton, "clicked", G_CALLBACK(onEvaluationStart), NULL);
    
    // CSS样式，使按钮更美观
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
    
    // 结果区域 - 包含表格和图表
    GtkWidget* resultsBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(container), resultsBox, TRUE, TRUE, 0);
    
    // 左侧：结果表格
    GtkWidget* tableFrame = gtk_frame_new("评估结果");
    gtk_widget_set_size_request(tableFrame, 400, 500);
    gtk_box_pack_start(GTK_BOX(resultsBox), tableFrame, FALSE, FALSE, 0);
    
    GtkWidget* tableBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(tableFrame), tableBox);
    gtk_container_set_border_width(GTK_CONTAINER(tableBox), 10);
    
    // 创建表格
    GtkWidget* table = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(table), 10);
    gtk_grid_set_column_spacing(GTK_GRID(table), 15);
    gtk_box_pack_start(GTK_BOX(tableBox), table, TRUE, TRUE, 0);
    
    // 添加表头
    GtkWidget* headerLabel1 = gtk_label_new("指标");
    gtk_widget_set_halign(headerLabel1, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), headerLabel1, 0, 0, 1, 1);
    
    GtkWidget* headerLabel2 = gtk_label_new("数值");
    gtk_widget_set_halign(headerLabel2, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), headerLabel2, 1, 0, 1, 1);
    
    // 添加行
    const char* metrics[] = {
        "最远定位距离", "定位时间", "定位精度", "测向精度"
    };
    
    for (int i = 0; i < 4; i++) {
        GtkWidget* metricLabel = gtk_label_new(metrics[i]);
        gtk_widget_set_halign(metricLabel, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(table), metricLabel, 0, i+1, 1, 1);
        
        GtkWidget* valueLabel = gtk_label_new("--");
        gtk_widget_set_halign(valueLabel, GTK_ALIGN_END);
        gtk_grid_attach(GTK_GRID(table), valueLabel, 1, i+1, 1, 1);
    }
    
    // 导出按钮
    GtkWidget* exportButton = gtk_button_new_with_label("导出结果");
    gtk_box_pack_start(GTK_BOX(tableBox), exportButton, FALSE, FALSE, 5);
    g_signal_connect(exportButton, "clicked", G_CALLBACK(onExportResults), NULL);
    
    // 右侧：图表区域
    GtkWidget* chartFrame = gtk_frame_new("定位精度随时间变化");
    gtk_box_pack_start(GTK_BOX(resultsBox), chartFrame, TRUE, TRUE, 0);
    
    // 创建绘图区域
    GtkWidget* drawingArea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(chartFrame), drawingArea);
    
    // 绘制示例图表 - 使用预定义的回调函数
    g_signal_connect(drawingArea, "draw", G_CALLBACK(drawChartCallback), NULL);
    
    return container;
}

void UIManager::onEvaluationStart(GtkWidget* widget, gpointer data) {
    g_print("Starting evaluation...\n");
    // 这里应该调用评估引擎进行评估计算
    // 现在简单模拟
    
    // 获取界面上的控件
    GtkWidget* window = UIManager::getInstance().m_mainWindow;
    if (!window) {
        g_print("Error: Main window is NULL\n");
        return;
    }
    
    GtkWidget* notebook = gtk_bin_get_child(GTK_BIN(window));
    if (!notebook) {
        g_print("Error: Notebook is NULL\n");
        return;
    }
    
    GtkWidget* evaluationPage = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 5);
    if (!evaluationPage) {
        g_print("Error: Evaluation page is NULL\n");
        return;
    }
    
    // 找到结果表格
    GtkWidget* container = GTK_WIDGET(evaluationPage);
    GtkWidget* resultsBox = get_child_at_index(GTK_CONTAINER(container), 2);
    if (!resultsBox) {
        g_print("Error: Results box is NULL\n");
        return;
    }
    
    GtkWidget* tableFrame = get_child_at_index(GTK_CONTAINER(resultsBox), 0);
    if (!tableFrame) {
        g_print("Error: Table frame is NULL\n");
        return;
    }
    
    GtkWidget* tableBox = gtk_bin_get_child(GTK_BIN(tableFrame));
    if (!tableBox) {
        g_print("Error: Table box is NULL\n");
        return;
    }
    
    GtkWidget* table = get_child_at_index(GTK_CONTAINER(tableBox), 0);
    if (!table) {
        g_print("Error: Table is NULL\n");
        return;
    }
    
    // 更新表格的结果值
    const char* values[] = {"120.5 km", "12.3 s", "2.5 m", "0.15°"};
    
    for (int i = 0; i < 4; i++) {
        GtkWidget* valueLabel = gtk_grid_get_child_at(GTK_GRID(table), 1, i+1);
        if (valueLabel) {
            gtk_label_set_text(GTK_LABEL(valueLabel), values[i]);
        } else {
            g_print("Warning: Value label at position (%d, %d) is NULL\n", 1, i+1);
        }
    }
    
    // 触发图表重绘
    GtkWidget* chartFrame = get_child_at_index(GTK_CONTAINER(resultsBox), 1);
    if (!chartFrame) {
        g_print("Error: Chart frame is NULL\n");
        return;
    }
    
    GtkWidget* drawingArea = gtk_bin_get_child(GTK_BIN(chartFrame));
    if (!drawingArea) {
        g_print("Error: Drawing area is NULL\n");
        return;
    }
    
    gtk_widget_queue_draw(drawingArea);
    g_print("Evaluation completed\n");
}

void UIManager::onExportResults(GtkWidget* widget, gpointer data) {
    // 创建文件选择对话框
    GtkWidget* dialog = gtk_file_chooser_dialog_new("导出结果",
                                                   GTK_WINDOW(UIManager::getInstance().m_mainWindow),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   "取消", GTK_RESPONSE_CANCEL,
                                                   "保存", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    // 设置默认文件名
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "评估结果.txt");
    
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        // 这里应该将评估结果导出到所选文件
        g_print("评估结果已导出到: %s\n", filename);
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

// 显示地图
void UIManager::showMap(GtkWidget* container) {
    // 创建绘图区域
    GtkWidget* drawingArea = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawingArea, 600, 600);
    gtk_container_add(GTK_CONTAINER(container), drawingArea);
    
    // 连接绘图事件
    g_signal_connect(drawingArea, "draw", G_CALLBACK(drawMapCallback), NULL);
}

// 定义未实现的函数
void UIManager::showLocationResults(GtkWidget* map, const std::vector<LocationResult>& results) {
    // 在地图上显示定位结果
    // 这个函数在实际应用中会在地图上显示定位结果点
}

void UIManager::showReferencePoints(GtkWidget* map, const std::map<std::string, Coordinate>& points) {
    // 在地图上显示基准点
    // 这个函数在实际应用中会在地图上显示基准点
}

void UIManager::showAccuracyChart(GtkWidget* container, const std::map<double, double>& data) {
    // 显示定位精度图表
    // 这个函数在实际应用中会创建并显示精度随时间变化的图表
}

void UIManager::showResultsTable(GtkWidget* container, const std::vector<std::pair<std::string, double>>& results) {
    // 显示统计结果表格
    // 这个函数在实际应用中会创建并显示评估结果表格
}