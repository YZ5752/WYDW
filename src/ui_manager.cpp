#include "../include/ui_manager.h"
#include <iostream>
#include <random>

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

// 树视图按钮点击事件处理
static gboolean on_tree_view_button_press(GtkWidget *treeview, GdkEventButton *event, gpointer userdata) {
    if (event->type == GDK_BUTTON_PRESS && event->button == 1) {
        GtkTreePath *path;
        GtkTreeViewColumn *column;
        gint cell_x, cell_y;
        
        // 获取点击的单元格
        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), 
                                        (gint)event->x, (gint)event->y,
                                        &path, &column, &cell_x, &cell_y)) {
            
            // 获取列的类型
            GtkCellRenderer *renderer = GTK_CELL_RENDERER(g_list_first(gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(column)))->data);
            const gchar *columnType = (const gchar *)g_object_get_data(G_OBJECT(renderer), "column-type");
            
            if (columnType) {
                GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
                GtkTreeIter iter;
                
                if (gtk_tree_model_get_iter(model, &iter, path)) {
                    // 获取行的ID或其他标识
                    gchar *name;
                    gtk_tree_model_get(model, &iter, 0, &name, -1);
                    
                    if (g_strcmp0(columnType, "edit") == 0) {
                        g_print("编辑按钮被点击: %s\n", name);
                        
                        // 判断是雷达设备还是辐射源
                        GtkWidget *parent = gtk_widget_get_parent(gtk_widget_get_parent(treeview));
                        UIManager &ui = UIManager::getInstance();
                        if (parent == ui.m_radarDeviceModelPage) {
                            UIManager::onEditRadarDevice(treeview, NULL);
                        } else if (parent == ui.m_radiationSourceModelPage) {
                            UIManager::onEditRadiationSource(treeview, NULL);
                        }
                    } else if (g_strcmp0(columnType, "delete") == 0) {
                        g_print("删除按钮被点击: %s\n", name);
                        
                        // 判断是雷达设备还是辐射源
                        GtkWidget *parent = gtk_widget_get_parent(gtk_widget_get_parent(treeview));
                        UIManager &ui = UIManager::getInstance();
                        if (parent == ui.m_radarDeviceModelPage) {
                            UIManager::onDeleteRadarDevice(treeview, NULL);
                        } else if (parent == ui.m_radiationSourceModelPage) {
                            UIManager::onDeleteRadiationSource(treeview, NULL);
                        }
                    }
                    
                    g_free(name);
                }
            }
            
            gtk_tree_path_free(path);
            return TRUE;
        }
    }
    
    return FALSE;
}
// ... existing code ...

// 创建雷达设备模型UI
GtkWidget* UIManager::createRadarDeviceModelUI() {
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(container), 10);
    
    // 添加标题
    GtkWidget* titleLabel = gtk_label_new("雷达设备模型管理");
    gtk_widget_set_halign(titleLabel, GTK_ALIGN_START);
    PangoAttrList* attrList = pango_attr_list_new();
    PangoAttribute* attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    pango_attr_list_insert(attrList, attr);
    gtk_label_set_attributes(GTK_LABEL(titleLabel), attrList);
    pango_attr_list_unref(attrList);
    gtk_box_pack_start(GTK_BOX(container), titleLabel, FALSE, FALSE, 5);
    
    // 创建添加按钮
    GtkWidget* addButton = gtk_button_new_with_label("添加雷达设备");
    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(buttonBox), addButton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(container), buttonBox, FALSE, FALSE, 5);
    
    // 创建雷达设备列表
    std::vector<std::string> headers = {"ID", "名称", "型号", "工作频率(MHz)", "编辑", "删除"};
    GtkWidget* deviceList = createModelList(headers);
    GtkWidget* scrollWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWindow), 
                                  GTK_POLICY_AUTOMATIC, 
                                  GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrollWindow), deviceList);
    gtk_box_pack_start(GTK_BOX(container), scrollWindow, TRUE, TRUE, 0);
    
    // 设置滚动窗口的最小尺寸
    gtk_widget_set_size_request(scrollWindow, -1, 300);
    
    // 连接信号
    g_signal_connect(deviceList, "button-press-event",
                    G_CALLBACK(on_tree_view_button_press), NULL);
    
    // 更新列表
    updateReconnaissanceDeviceList(deviceList);
    
    return container;
}

// 创建辐射源模型UI
GtkWidget* UIManager::createRadiationSourceModelUI() {
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(container), 10);
    
    // 添加标题
    GtkWidget* titleLabel = gtk_label_new("辐射源模型管理");
    gtk_widget_set_halign(titleLabel, GTK_ALIGN_START);
    PangoAttrList* attrList = pango_attr_list_new();
    PangoAttribute* attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    pango_attr_list_insert(attrList, attr);
    gtk_label_set_attributes(GTK_LABEL(titleLabel), attrList);
    pango_attr_list_unref(attrList);
    gtk_box_pack_start(GTK_BOX(container), titleLabel, FALSE, FALSE, 5);
    
    // 创建添加按钮
    GtkWidget* addButton = gtk_button_new_with_label("添加辐射源");
    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(buttonBox), addButton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(container), buttonBox, FALSE, FALSE, 5);
    
    // 创建辐射源列表
    std::vector<std::string> headers = {"ID", "名称", "型号", "工作频率(MHz)", "编辑", "删除"};
    GtkWidget* sourceList = createModelList(headers);
    GtkWidget* scrollWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWindow), 
                                  GTK_POLICY_AUTOMATIC, 
                                  GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrollWindow), sourceList);
    gtk_box_pack_start(GTK_BOX(container), scrollWindow, TRUE, TRUE, 0);
    
    // 设置滚动窗口的最小尺寸
    gtk_widget_set_size_request(scrollWindow, -1, 300);
    
    // 连接信号
    g_signal_connect(sourceList, "button-press-event",
                    G_CALLBACK(on_tree_view_button_press), NULL);
    
    // 更新列表
    updateRadiationSourceList(sourceList);
    
    return container;
}

// 雷达设备编辑回调
void UIManager::onEditRadarDevice(GtkWidget* widget, gpointer data) {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
    GtkTreeModel* model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar* id_str;
        gtk_tree_model_get(model, &iter, 0, &id_str, -1);
        
        g_print("编辑雷达设备: %s\n", id_str);
        
        // 转换ID为整数
        int id = atoi(id_str);
        
        // 获取所有设备，并查找对应ID的设备
        std::vector<ReconnaissanceDevice> devices = getAllReconnaissanceDevices();
        for (const auto& device : devices) {
            if (device.getDeviceId() == id) {
                // 这里应该弹出编辑对话框
                // 实际项目中完成该功能
                g_print("找到设备: %s\n", device.getDeviceName().c_str());
                break;
            }
        }
        
        g_free(id_str);
    }
}

// 辐射源编辑回调
void UIManager::onEditRadiationSource(GtkWidget* widget, gpointer data) {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
    GtkTreeModel* model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar* id_str;
        gtk_tree_model_get(model, &iter, 0, &id_str, -1);
        
        g_print("编辑辐射源: %s\n", id_str);
        
        // 转换ID为整数
        int id = atoi(id_str);
        
        // 获取所有辐射源，并查找对应ID的辐射源
        std::vector<RadiationSource> sources = getAllRadiationSources();
        for (const auto& source : sources) {
            if (source.getRadiationId() == id) {
                // 这里应该弹出编辑对话框
                // 实际项目中完成该功能
                g_print("找到辐射源: %s\n", source.getRadiationName().c_str());
                break;
            }
        }
        
        g_free(id_str);
    }
}

// 雷达设备删除回调
void UIManager::onDeleteRadarDevice(GtkWidget* widget, gpointer data) {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
    GtkTreeModel* model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar* id_str;
        gtk_tree_model_get(model, &iter, 0, &id_str, -1);
        
        g_print("删除雷达设备: %s\n", id_str);
        
        // 转换ID为整数
        int id = atoi(id_str);
        
        // 弹出确认对话框
        GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_QUESTION,
                                                GTK_BUTTONS_YES_NO,
                                                "确定要删除雷达设备 %s 吗?", id_str);
        
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        if (response == GTK_RESPONSE_YES) {
            // 删除设备
            deleteReconnaissanceDevice(id);
            
            // 更新列表
            updateReconnaissanceDeviceList(widget);
        }
        
        g_free(id_str);
    }
}

// 辐射源删除回调
void UIManager::onDeleteRadiationSource(GtkWidget* widget, gpointer data) {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
    GtkTreeModel* model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar* id_str;
        gtk_tree_model_get(model, &iter, 0, &id_str, -1);
        
        g_print("删除辐射源: %s\n", id_str);
        
        // 转换ID为整数
        int id = atoi(id_str);
        
        // 弹出确认对话框
        GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_QUESTION,
                                                GTK_BUTTONS_YES_NO,
                                                "确定要删除辐射源 %s 吗?", id_str);
        
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        if (response == GTK_RESPONSE_YES) {
            // 删除辐射源
            deleteRadiationSource(id);
            
            // 更新列表
            updateRadiationSourceList(widget);
        }
        
        g_free(id_str);
    }
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
                           gtk_label_new("侦察设备模型"));
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
    
    gtk_window_set_title(GTK_WINDOW(window), "无源协同定位模块");
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
        g_print("Creating list store with %d columns plus action buttons\n", numCols + 2);
        
        // 根据实际头的数量创建列存储，加上编辑和删除按钮列
        GtkListStore* store = NULL;
        
        // 为了安全，明确指定所有列的类型
        GType* types = new GType[numCols + 2];
        for (int i = 0; i < numCols; i++) {
            types[i] = G_TYPE_STRING;
        }
        types[numCols] = G_TYPE_STRING;     // 编辑按钮
        types[numCols + 1] = G_TYPE_STRING; // 删除按钮
        
        store = gtk_list_store_newv(numCols + 2, types);
        delete[] types;
        
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
        
        // 添加编辑按钮列
        GtkCellRenderer* editRenderer = gtk_cell_renderer_text_new();
        g_object_set(editRenderer, "foreground", "blue", "underline", TRUE, NULL);
        g_object_set_data(G_OBJECT(editRenderer), "column-type", (gpointer)"edit");
        
        GtkTreeViewColumn* editColumn = gtk_tree_view_column_new_with_attributes(
            "编辑", editRenderer, "text", numCols, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), editColumn);
        
        // 添加删除按钮列
        GtkCellRenderer* deleteRenderer = gtk_cell_renderer_text_new();
        g_object_set(deleteRenderer, "foreground", "red", "underline", TRUE, NULL);
        g_object_set_data(G_OBJECT(deleteRenderer), "column-type", (gpointer)"delete");
        
        GtkTreeViewColumn* deleteColumn = gtk_tree_view_column_new_with_attributes(
            "删除", deleteRenderer, "text", numCols + 1, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(list), deleteColumn);
        
        // 添加点击事件处理
        g_signal_connect(list, "button-press-event", G_CALLBACK(on_tree_view_button_press), NULL);
        
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

GtkWidget* UIManager::createSinglePlatformUI() {
    // 创建页面的主容器
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(container), 15);
    
    // 左侧：地图显示区域
    GtkWidget* mapFrame = gtk_frame_new("地图");
    gtk_widget_set_size_request(mapFrame, 800, 700);
    gtk_box_pack_start(GTK_BOX(container), mapFrame, TRUE, TRUE, 0);
    
    // 显示WebKit地图
    showWebMap(mapFrame);
    
    // 右侧：参数设置和结果区域
    GtkWidget* rightBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(container), rightBox, FALSE, FALSE, 0);

    // 技术体制选择
    GtkWidget* algoFrame = gtk_frame_new("技术体制");
    gtk_box_pack_start(GTK_BOX(rightBox), algoFrame, FALSE, FALSE, 0);
    
    GtkWidget* algoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(algoFrame), algoBox);
    gtk_container_set_border_width(GTK_CONTAINER(algoBox), 10);
    
    GtkWidget* algoCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algoCombo), "干涉仪体制");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algoCombo), "时差体制");
    gtk_combo_box_set_active(GTK_COMBO_BOX(algoCombo), 0);
    gtk_box_pack_start(GTK_BOX(algoBox), algoCombo, TRUE, TRUE, 5);
    g_signal_connect(algoCombo, "changed", G_CALLBACK(onTechSystemChanged), NULL);
    
    // 雷达设备模型选择
    GtkWidget* radarFrame = gtk_frame_new("侦察设备模型");
    gtk_box_pack_start(GTK_BOX(rightBox), radarFrame, FALSE, FALSE, 0);
    
    GtkWidget* radarBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(radarFrame), radarBox);
    gtk_container_set_border_width(GTK_CONTAINER(radarBox), 10);
    
    GtkWidget* radarCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(radarCombo), "侦察设备1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(radarCombo), "侦察设备2");
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
    
    // 新增：仿真执行时间输入框
    GtkWidget* timeFrame = gtk_frame_new("仿真参数");
    gtk_box_pack_start(GTK_BOX(rightBox), timeFrame, FALSE, FALSE, 0);
    
    GtkWidget* timeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(timeFrame), timeBox);
    gtk_container_set_border_width(GTK_CONTAINER(timeBox), 6);
    
    GtkWidget* timeLabel = gtk_label_new("仿真执行时间(s):");
    gtk_box_pack_start(GTK_BOX(timeBox), timeLabel, FALSE, FALSE, 5);
    
    GtkWidget* timeEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(timeEntry), "10"); // 默认值10秒
    gtk_box_pack_start(GTK_BOX(timeBox), timeEntry, TRUE, TRUE, 5);
    g_object_set_data(G_OBJECT(rightBox), "time-entry", timeEntry); // 存储输入框指针
    
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
    GtkWidget* dirDataLabel = gtk_label_new("测向数据");
    gtk_widget_set_halign(dirDataLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), dirDataLabel, 0, 1, 1, 1);
    
    GtkWidget* locDataLabel = gtk_label_new("定位数据");
    gtk_widget_set_halign(locDataLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), locDataLabel, 0, 2, 1, 1);
    
    // 添加结果值（初始为空）    
    GtkWidget* dirDataValue = gtk_label_new("--");
    gtk_widget_set_halign(dirDataValue, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), dirDataValue, 1, 1, 1, 1);
    
    GtkWidget* locDataValue = gtk_label_new("--");
    gtk_widget_set_halign(locDataValue, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), locDataValue, 1, 2, 1, 1);

    // 误差结果区域
    GtkWidget* errorFrame = gtk_frame_new("误差分析");
    gtk_box_pack_start(GTK_BOX(rightBox), errorFrame, TRUE, TRUE, 0);
    
    GtkWidget* errorBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(errorFrame), errorBox);
    gtk_container_set_border_width(GTK_CONTAINER(errorBox), 10);
    
    // 创建误差表格
    GtkWidget* errorTable = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(errorTable), 5);
    gtk_grid_set_column_spacing(GTK_GRID(errorTable), 10);
    gtk_box_pack_start(GTK_BOX(errorBox), errorTable, TRUE, TRUE, 0);
    
    // 保存技术体制下拉框的引用，以便在回调函数中获取选择的值
    g_object_set_data(G_OBJECT(container), "algo-combo", algoCombo);
    
    // 保存误差表格的引用，以便在回调函数中更新
    g_object_set_data(G_OBJECT(container), "error-table", errorTable);
    
    // 初始化误差表格 - 默认显示干涉仪体制的误差项
    updateErrorTable(errorTable, "干涉仪体制");
    
    return container;
}

// 更新误差表格函数
void UIManager::updateErrorTable(GtkWidget* table, const std::string& techSystem) {
    if (!table) {
        return;
    }
    
    // 清空表格中的所有子控件
    GList* children = gtk_container_get_children(GTK_CONTAINER(table));
    for (GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    // 根据技术体制添加不同的误差项
    if (techSystem == "时差体制" || techSystem.find("时差") != std::string::npos) {
        // 时差体制误差项：时延误差、通道热噪声误差、时间测量误差、时差测量误差、测向误差
        const char* errorItems[] = {
            "时延误差", "通道热噪声误差", "时间测量误差", "时差测量误差", "测向误差"
        };
        
        for (int i = 0; i < 5; i++) {
            GtkWidget* errorLabel = gtk_label_new(errorItems[i]);
            gtk_widget_set_halign(errorLabel, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(table), errorLabel, 0, i, 1, 1);
        }
    } else if (techSystem == "干涉仪体制" || techSystem.find("干涉") != std::string::npos) {
        // 干涉仪体制误差项：对中误差、姿态测量误差、圆锥效应误差、天线阵测向误差、测向误差
        const char* errorItems[] = {
            "对中误差", "姿态测量误差", "圆锥效应误差", "天线阵测向误差", "测向误差"
        };
        
        for (int i = 0; i < 5; i++) {
            GtkWidget* errorLabel = gtk_label_new(errorItems[i]);
            gtk_widget_set_halign(errorLabel, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(table), errorLabel, 0, i, 1, 1);
        }
    } else {
        // 默认情况，显示通用误差项
        const char* errorItems[] = {
            "测量误差", "系统误差", "随机误差", "定位误差", "测向误差"
        };
        
        for (int i = 0; i < 5; i++) {
            GtkWidget* errorLabel = gtk_label_new(errorItems[i]);
            gtk_widget_set_halign(errorLabel, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(table), errorLabel, 0, i, 1, 1);
        }
    }
    
    // 显示所有控件
    gtk_widget_show_all(table);
}

// 单平台仿真回调
void UIManager::onSinglePlatformSimulation(GtkWidget* widget, gpointer data) {
    // 这里应该调用仿真引擎进行计算
    // 现在简单模拟
    
    // 获取界面上的控件
    GtkWidget* window = UIManager::getInstance().m_mainWindow;
    GtkWidget* notebook = gtk_bin_get_child(GTK_BIN(window));
    GtkWidget* singlePlatformPage = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 2);
    
    // 获取控件
    GtkWidget* container = GTK_WIDGET(singlePlatformPage);
    GtkWidget* hbox = gtk_bin_get_child(GTK_BIN(container));
    GtkWidget* mapFrame = get_child_at_index(GTK_CONTAINER(hbox), 0);
    
    // 在地图上添加模拟目标
    UIManager& ui = UIManager::getInstance();
    
    // 清除之前的点
    ui.clearPoints();
    
    // 添加模拟目标位置
    ui.addPoint(119.97, 31.71, "目标");
    
    // 添加雷达设备位置
    ui.addPoint(118.78, 32.07, "雷达1");
    ui.addPoint(120.58, 31.30, "雷达2");
    ui.addPoint(120.30, 31.57, "雷达3");
    
    // 设置中心点
    ui.m_mapView.setCenter(119.4, 31.9, 100000);
    
    // 获取技术体制选择
    GtkWidget* rightBox = get_child_at_index(GTK_CONTAINER(hbox), 1);
    GtkWidget* settingsFrame = get_child_at_index(GTK_CONTAINER(rightBox), 0);
    GtkWidget* settingsBox = gtk_bin_get_child(GTK_BIN(settingsFrame));
    GtkWidget* techSystemBox = get_child_at_index(GTK_CONTAINER(settingsBox), 0);
    GtkWidget* techSystemCombo = get_child_at_index(GTK_CONTAINER(techSystemBox), 1);
    const gchar* techSystem = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(techSystemCombo));
    
    // 获取误差表格
    GtkWidget* errorTable = GTK_WIDGET(g_object_get_data(G_OBJECT(container), "error-table"));
    
    // 更新误差表格内容
    if (errorTable) {
        ui.updateErrorTable(errorTable, techSystem);
    }
}

// 多平台协同侦察UI
GtkWidget* UIManager::createMultiPlatformUI() {
    // 创建页面的主容器
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(container), 15);
    
    // 左侧：地图显示区域
    GtkWidget* mapFrame = gtk_frame_new("地图");
    gtk_widget_set_size_request(mapFrame, 800, 700);
    gtk_box_pack_start(GTK_BOX(container), mapFrame, TRUE, TRUE, 0);
    
    // 只显示Cesium三维地图
    showWebMap(mapFrame);
    
    // 右侧：参数设置和结果区域
    GtkWidget* rightBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(container), rightBox, FALSE, FALSE, 0);

    // 技术体制选择
    GtkWidget* algoFrame = gtk_frame_new("技术体制");
    gtk_box_pack_start(GTK_BOX(rightBox), algoFrame, FALSE, FALSE, 0);
    
    GtkWidget* algoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(algoFrame), algoBox);
    gtk_container_set_border_width(GTK_CONTAINER(algoBox), 10);
    
    GtkWidget* algoCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algoCombo), "时差体制");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algoCombo), "频差体制");
    // gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algoCombo), "测向定位");
    gtk_combo_box_set_active(GTK_COMBO_BOX(algoCombo), 0);
    gtk_box_pack_start(GTK_BOX(algoBox), algoCombo, TRUE, TRUE, 5);
    
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
    
    // 新增：仿真执行时间输入框
    GtkWidget* timeFrame = gtk_frame_new("仿真参数");
    gtk_box_pack_start(GTK_BOX(rightBox), timeFrame, FALSE, FALSE, 0);
    
    GtkWidget* timeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(timeFrame), timeBox);
    gtk_container_set_border_width(GTK_CONTAINER(timeBox), 10);
    
    GtkWidget* timeLabel = gtk_label_new("仿真执行时间(s):");
    gtk_box_pack_start(GTK_BOX(timeBox), timeLabel, FALSE, FALSE, 5);
    
    GtkWidget* timeEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(timeEntry), "20"); // 默认值20秒
    gtk_box_pack_start(GTK_BOX(timeBox), timeEntry, TRUE, TRUE, 5);
    g_object_set_data(G_OBJECT(rightBox), "time-entry", timeEntry); // 存储输入框指针

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
    GtkWidget* dirDataLabel = gtk_label_new("测向数据");
    gtk_widget_set_halign(dirDataLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), dirDataLabel, 0, 1, 1, 1);
    
    GtkWidget* locDataLabel = gtk_label_new("定位数据");
    gtk_widget_set_halign(locDataLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), locDataLabel, 0, 2, 1, 1);
    
    // 添加结果值（初始为空）    
    GtkWidget* dirDataValue = gtk_label_new("--");
    gtk_widget_set_halign(dirDataValue, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), dirDataValue, 1, 1, 1, 1);
    
    GtkWidget* locDataValue = gtk_label_new("--");
    gtk_widget_set_halign(locDataValue, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), locDataValue, 1, 2, 1, 1);

    // 保存技术体制下拉框的引用，以便在回调函数中获取选择的值
    g_object_set_data(G_OBJECT(container), "algo-combo", algoCombo);
    
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
    
    // 获取控件
    GtkWidget* container = GTK_WIDGET(multiPlatformPage);
    GtkWidget* hbox = gtk_bin_get_child(GTK_BIN(container));
    GtkWidget* rightBox = get_child_at_index(GTK_CONTAINER(hbox), 1);
    
    // 获取仿真执行时间
    GtkWidget* timeEntry = GTK_WIDGET(g_object_get_data(G_OBJECT(rightBox), "time-entry"));
    if (timeEntry) {
        const gchar* timeStr = gtk_entry_get_text(GTK_ENTRY(timeEntry));
        double executionTime = atof(timeStr);
        g_print("仿真执行时间: %.1f 秒\n", executionTime);
    }
    
    GtkWidget* mapFrame = get_child_at_index(GTK_CONTAINER(hbox), 0);
    
    // 在地图上添加模拟目标
    UIManager& ui = UIManager::getInstance();
    
    // 清除之前的点
    ui.clearPoints();
    
    // 添加模拟目标位置
    ui.addPoint(119.97, 31.71, "目标");
    
    // 添加雷达设备位置
    ui.addPoint(118.78, 32.07, "雷达1");
    ui.addPoint(120.58, 31.30, "雷达2");
    ui.addPoint(120.30, 31.57, "雷达3");
    
    // 设置中心点
    ui.m_mapView.setCenter(119.4, 31.9, 100000);
    
    // 获取技术体制选择
    GtkWidget* techSystemBox = get_child_at_index(GTK_CONTAINER(rightBox), 1);
    GtkWidget* settingsBox = gtk_bin_get_child(GTK_BIN(techSystemBox));
    GtkWidget* algoCombo = GTK_WIDGET(g_object_get_data(G_OBJECT(container), "algo-combo"));
    const gchar* techSystem = algoCombo ? 
        gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(algoCombo)) :
        "TDOA";
    
    // 获取误差表格
    GtkWidget* errorTable = GTK_WIDGET(g_object_get_data(G_OBJECT(container), "error-table"));
    
    // 更新误差表格内容
    if (errorTable) {
        ui.updateErrorTable(errorTable, techSystem);
    }
}

// 创建数据显示UI (简化版，仅用于兼容)
GtkWidget* UIManager::createDataDisplayUI() {
    // 这个函数不再需要，使用createDataSelectionUI代替
    // 为了兼容性，返回一个空容器
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    return container;
}

// 创建数据分选UI (优化布局版)
GtkWidget* UIManager::createDataSelectionUI() {
    // 创建页面的主容器
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(container), 15);
    
    // 顶部控制区域 - 水平布局
    GtkWidget* controlBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(container), controlBox, FALSE, FALSE, 0);
    
    // 左侧：目标选择区域
    GtkWidget* targetFrame = gtk_frame_new("辐射源模型");
    gtk_box_pack_start(GTK_BOX(controlBox), targetFrame, FALSE, FALSE, 0);
    
    GtkWidget* targetBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(targetFrame), targetBox);
    gtk_container_set_border_width(GTK_CONTAINER(targetBox), 10);
    
    // 添加标签说明
    GtkWidget* targetLabel = gtk_label_new("选择辐射源模型:");
    gtk_box_pack_start(GTK_BOX(targetBox), targetLabel, FALSE, FALSE, 0);
    
    // 调整下拉框大小
    GtkWidget* targetCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(targetCombo), "辐射源1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(targetCombo), "辐射源2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(targetCombo), 0);
    gtk_widget_set_size_request(targetCombo, 200, -1); // 设置宽度为200px
    gtk_box_pack_start(GTK_BOX(targetBox), targetCombo, FALSE, FALSE, 0);
    g_signal_connect(targetCombo, "changed", G_CALLBACK(onDataSelectionChanged), NULL);
    
    // 右侧：按钮区域
    GtkWidget* buttonBox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonBox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing(GTK_BOX(buttonBox), 10);
    gtk_box_pack_start(GTK_BOX(controlBox), buttonBox, TRUE, TRUE, 0);
    
    // 删除按钮
    GtkWidget* deleteButton = gtk_button_new_with_label("删除");
    gtk_container_add(GTK_CONTAINER(buttonBox), deleteButton);
    
    // 高亮显示按钮
    GtkWidget* highlightButton = gtk_button_new_with_label("高亮显示");
    gtk_container_add(GTK_CONTAINER(buttonBox), highlightButton);
    
    // 录入按钮
    GtkWidget* importButton = gtk_button_new_with_label("录入");
    gtk_container_add(GTK_CONTAINER(buttonBox), importButton);
    g_signal_connect(importButton, "clicked", G_CALLBACK(onDataImport), NULL);
    
    // 数据列表区域 (占据剩余空间)
    GtkWidget* dataFrame = gtk_frame_new("目标数据列表");
    gtk_box_pack_start(GTK_BOX(container), dataFrame, TRUE, TRUE, 0);
    
    GtkWidget* dataBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(dataFrame), dataBox);
    gtk_container_set_border_width(GTK_CONTAINER(dataBox), 10);
    
    // 创建列表存储 (删除了时间戳列)
    GtkListStore* store = gtk_list_store_new(4, 
                                           G_TYPE_BOOLEAN,  // 选择列
                                           G_TYPE_STRING,   // 侦察设备ID
                                           G_TYPE_STRING,   // 测向数据
                                           G_TYPE_STRING);  // 定位数据
    
    GtkWidget* treeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    // 添加选择列
    GtkCellRenderer* toggleRenderer = gtk_cell_renderer_toggle_new();
    GtkTreeViewColumn* toggleColumn = gtk_tree_view_column_new_with_attributes(
        "选择", toggleRenderer, "active", 0, NULL);
    gtk_tree_view_column_set_min_width(toggleColumn, 60); // 设置最小宽度
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), toggleColumn);
    
    // 添加侦察设备列
    GtkCellRenderer* textRenderer1 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column1 = gtk_tree_view_column_new_with_attributes(
        "侦察设备", textRenderer1, "text", 1, NULL);
    gtk_tree_view_column_set_min_width(column1, 120); // 设置最小宽度
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column1);
    
    // 添加测向数据列
    GtkCellRenderer* textRenderer2 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column2 = gtk_tree_view_column_new_with_attributes(
        "测向数据 (方位角)", textRenderer2, "text", 2, NULL);
    gtk_tree_view_column_set_min_width(column2, 150); // 设置最小宽度
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column2);
    
    // 添加定位数据列
    GtkCellRenderer* textRenderer3 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column3 = gtk_tree_view_column_new_with_attributes(
        "定位数据 (经纬度)", textRenderer3, "text", 3, NULL);
    gtk_tree_view_column_set_min_width(column3, 200); // 设置最小宽度
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column3);
    
    // 添加示例数据
    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 
                      0, FALSE,
                      1, "侦察设备1", 
                      2, "方位角: 45°", 
                      3, "坐标: (116.5, 39.9)", 
                      -1);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 
                      0, FALSE,
                      1, "侦察设备2", 
                      2, "方位角: 130°", 
                      3, "坐标: (116.4, 39.8)", 
                      -1);
    
    // 滚动窗口
    GtkWidget* scrollWin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWin), 
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrollWin), treeView);
    gtk_box_pack_start(GTK_BOX(dataBox), scrollWin, TRUE, TRUE, 0);
    
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
        UIManager& ui = UIManager::getInstance();
        
        // 清除之前的点
        ui.clearPoints(); 
        
        // 根据选择显示不同目标
        if (strcmp(selected_text, "目标1") == 0) {
            ui.addPoint(119.97, 31.71, "目标1");
            ui.m_mapView.setCenter(119.97, 31.71, 8);
        } else if (strcmp(selected_text, "目标2") == 0) {
            ui.addPoint(120.30, 31.57, "目标2");
            ui.m_mapView.setCenter(120.30, 31.57, 8);
        }
        
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
    
    GtkWidget* targetLabel = gtk_label_new("辐射源模型");
    gtk_widget_set_halign(targetLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(targetBox), targetLabel, FALSE, FALSE, 0);
    
    GtkWidget* targetCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(targetCombo), "辐射源1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(targetCombo), "辐射源2");
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

void UIManager::showWebMap(GtkWidget* container) {
    GtkWidget* mapWidget = m_mapView.create();
    gtk_widget_set_size_request(mapWidget, 800, 700);
    gtk_container_add(GTK_CONTAINER(container), mapWidget);
    m_mapView.setUse3DMap(true);
    // m_mapView.setMapType("3d");
    m_mapView.setCenter(116.3833, 39.9167, 1000000);
}

// 添加标记点
void UIManager::addPoint(double longitude, double latitude, const std::string& title) {
    m_mapView.addMarker(longitude, latitude, title, "", "#FF0000");
}

// 清除所有标记点
void UIManager::clearPoints() {
    m_mapView.clearMarkers();
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

// 技术体制变化回调
void UIManager::onTechSystemChanged(GtkWidget* widget, gpointer data) {
    // 获取当前选择的技术体制
    const gchar* techSystem = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    if (!techSystem) {
        return;
    }
    
    // 获取当前页面 - 尝试多种方法获取页面和误差表格
    GtkWidget* page = NULL;
    GtkWidget* errorTable = NULL;
    
    // 方法1：通过widget的父级获取页面
    GtkWidget* parent = gtk_widget_get_parent(widget);
    if (parent) {
        parent = gtk_widget_get_parent(parent);
        if (parent) {
            parent = gtk_widget_get_parent(parent);
            if (parent) {
                page = parent;
            }
        }
    }
    
    // 如果找到了页面，尝试获取误差表格
    if (page) {
        errorTable = GTK_WIDGET(g_object_get_data(G_OBJECT(page), "error-table"));
    }
    
    // 方法2：如果通过页面数据未找到误差表格，尝试遍历页面结构
    if (!errorTable && page) {
        // 假设页面是水平布局，右侧是控制面板
        if (GTK_IS_BOX(page)) {
            GList* children = gtk_container_get_children(GTK_CONTAINER(page));
            if (children && g_list_length(children) >= 2) {
                // 获取右侧控制面板
                GtkWidget* rightBox = GTK_WIDGET(g_list_nth_data(children, 1));
                if (rightBox && GTK_IS_BOX(rightBox)) {
                    // 遍历右侧控制面板的子控件，寻找误差分析框
                    GList* rightChildren = gtk_container_get_children(GTK_CONTAINER(rightBox));
                    for (GList* iter = rightChildren; iter != NULL; iter = g_list_next(iter)) {
                        GtkWidget* child = GTK_WIDGET(iter->data);
                        if (GTK_IS_FRAME(child)) {
                            // 检查是否是误差分析框
                            const gchar* frameLabel = gtk_frame_get_label(GTK_FRAME(child));
                            if (frameLabel && strcmp(frameLabel, "误差分析") == 0) {
                                // 获取框内容器
                                GtkWidget* errorBox = gtk_bin_get_child(GTK_BIN(child));
                                if (errorBox && GTK_IS_CONTAINER(errorBox)) {
                                    // 获取误差表格
                                    GList* errorChildren = gtk_container_get_children(GTK_CONTAINER(errorBox));
                                    if (errorChildren && g_list_length(errorChildren) > 0) {
                                        errorTable = GTK_WIDGET(errorChildren->data);
                                    }
                                    g_list_free(errorChildren);
                                }
                                break;
                            }
                        }
                    }
                    g_list_free(rightChildren);
                }
            }
            g_list_free(children);
        }
    }
    
    // 方法3：如果还是没找到，尝试使用全局查找
    if (!errorTable) {
        // 获取主窗口
        GtkWidget* window = UIManager::getInstance().m_mainWindow;
        if (window) {
            // 获取笔记本控件
            GtkWidget* notebook = NULL;
            if (GTK_IS_BIN(window)) {
                notebook = gtk_bin_get_child(GTK_BIN(window));
            } else {
                // 尝试获取第一个子控件
                GList* windowChildren = gtk_container_get_children(GTK_CONTAINER(window));
                if (windowChildren && g_list_length(windowChildren) > 0) {
                    notebook = GTK_WIDGET(windowChildren->data);
                }
                g_list_free(windowChildren);
            }
            
            if (notebook && GTK_IS_NOTEBOOK(notebook)) {
                // 获取当前页面索引
                gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
                
                // 获取单平台仿真页面
                GtkWidget* singlePlatformPage = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 2);
                if (singlePlatformPage) {
                    // 获取水平布局
                    GtkWidget* hbox = NULL;
                    if (GTK_IS_BIN(singlePlatformPage)) {
                        hbox = gtk_bin_get_child(GTK_BIN(singlePlatformPage));
                    } else if (GTK_IS_BOX(singlePlatformPage)) {
                        hbox = singlePlatformPage;  // 页面本身就是一个Box
                    } else {
                        // 尝试获取第一个子控件
                        GList* pageChildren = gtk_container_get_children(GTK_CONTAINER(singlePlatformPage));
                        if (pageChildren && g_list_length(pageChildren) > 0) {
                            hbox = GTK_WIDGET(pageChildren->data);
                        }
                        g_list_free(pageChildren);
                    }
                    
                    if (hbox && GTK_IS_BOX(hbox)) {
                        // 获取右侧控制面板
                        GtkWidget* rightBox = get_child_at_index(GTK_CONTAINER(hbox), 1);
                        if (rightBox && GTK_IS_BOX(rightBox)) {
                            // 查找误差分析框
                            for (int i = 0; i < 10; i++) {  // 假设最多10个子控件
                                GtkWidget* child = get_child_at_index(GTK_CONTAINER(rightBox), i);
                                if (!child) continue;
                                
                                if (GTK_IS_FRAME(child)) {
                                    const gchar* frameLabel = gtk_frame_get_label(GTK_FRAME(child));
                                    if (frameLabel && strcmp(frameLabel, "误差分析") == 0) {
                                        // 获取框内容器
                                        GtkWidget* errorBox = gtk_bin_get_child(GTK_BIN(child));
                                        if (errorBox && GTK_IS_CONTAINER(errorBox)) {
                                            // 获取误差表格
                                            GtkWidget* firstChild = get_child_at_index(GTK_CONTAINER(errorBox), 0);
                                            if (firstChild) {
                                                errorTable = firstChild;
                                            }
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // 方法4：直接在单平台仿真页面中查找误差表格
    if (!errorTable) {
        // 获取主窗口
        GtkWidget* window = UIManager::getInstance().m_mainWindow;
        if (window) {
            // 遍历整个控件树，查找标签为"误差分析"的框
            GList* queue = g_list_append(NULL, window);
            while (queue && !errorTable) {
                GtkWidget* current = GTK_WIDGET(queue->data);
                queue = g_list_delete_link(queue, queue);
                
                // 检查当前控件是否是框架，且标签为"误差分析"
                if (GTK_IS_FRAME(current)) {
                    const gchar* frameLabel = gtk_frame_get_label(GTK_FRAME(current));
                    if (frameLabel && strcmp(frameLabel, "误差分析") == 0) {
                        // 获取框内容器
                        GtkWidget* errorBox = gtk_bin_get_child(GTK_BIN(current));
                        if (errorBox && GTK_IS_CONTAINER(errorBox)) {
                            // 获取误差表格
                            GList* errorChildren = gtk_container_get_children(GTK_CONTAINER(errorBox));
                            if (errorChildren && g_list_length(errorChildren) > 0) {
                                errorTable = GTK_WIDGET(errorChildren->data);
                            }
                            g_list_free(errorChildren);
                        }
                    }
                }
                
                // 如果当前控件是容器，将其子控件加入队列
                if (GTK_IS_CONTAINER(current)) {
                    GList* children = gtk_container_get_children(GTK_CONTAINER(current));
                    for (GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
                        queue = g_list_append(queue, iter->data);
                    }
                    g_list_free(children);
                }
            }
            g_list_free(queue);  // 释放可能剩余的队列
        }
    }
    
    // 如果找到了误差表格，更新它
    if (errorTable) {
        // 更新误差表格
        UIManager::getInstance().updateErrorTable(errorTable, techSystem);
    }
}