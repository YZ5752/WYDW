#include "../ReconnaissanceDeviceModelView.h"
#include "../../controllers/ApplicationController.h"
#include "../../controllers/ReconnaissanceDeviceModelController.h"
#include <iostream>
#include <sstream>

// 辅助函数：获取GtkBox中指定索引的子控件
GtkWidget* get_box_child_at_index(GtkBox* box, gint index) {
    GList* children = gtk_container_get_children(GTK_CONTAINER(box));
    GtkWidget* child = nullptr;
    
    if (children && g_list_length(children) > index) {
        child = GTK_WIDGET(g_list_nth_data(children, index));
    }
    
    g_list_free(children);
    return child;
}

// 回调函数
static void on_add_button_clicked(GtkWidget* widget, gpointer data) {
    ReconnaissanceDeviceModelView* view = static_cast<ReconnaissanceDeviceModelView*>(data);
    if (view) {
        // 显示编辑对话框
        ReconnaissanceDeviceModelController::getInstance().showEditDialog();
    }
}

// 设备类型切换时的回调函数
static void on_device_type_toggled(GtkToggleButton* button, gpointer data) {
    GtkWidget* frame = GTK_WIDGET(data);
    gboolean active = gtk_toggle_button_get_active(button);
    gtk_widget_set_sensitive(frame, !active);
    
    if (active) {
        // 如果是固定设备，将运动参数设为0
        GtkWidget* dialog = gtk_widget_get_toplevel(frame);
        GtkWidget* speedSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "speedSpin"));
        GtkWidget* movAzimuthSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "movAzimuthSpin"));
        GtkWidget* movElevationSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "movElevationSpin"));
        
        if (speedSpin) gtk_spin_button_set_value(GTK_SPIN_BUTTON(speedSpin), 0.0);
        if (movAzimuthSpin) gtk_spin_button_set_value(GTK_SPIN_BUTTON(movAzimuthSpin), 0.0);
        if (movElevationSpin) gtk_spin_button_set_value(GTK_SPIN_BUTTON(movElevationSpin), 0.0);
    }
}

// 频率范围最小值变更时的回调函数
static void on_min_freq_changed(GtkSpinButton* spin_button, gpointer data) {
    GtkSpinButton* maxSpin = GTK_SPIN_BUTTON(data);
    double minValue = gtk_spin_button_get_value(spin_button);
    double maxValue = gtk_spin_button_get_value(maxSpin);
    
    // 如果最大值小于或等于最小值，则调整最大值
    if (maxValue <= minValue) {
        gtk_spin_button_set_value(maxSpin, minValue + 0.1);
    }
}

// 方位角范围最小值变更时的回调函数
static void on_min_azimuth_changed(GtkSpinButton* spin_button, gpointer data) {
    GtkSpinButton* maxSpin = GTK_SPIN_BUTTON(data);
    double minValue = gtk_spin_button_get_value(spin_button);
    double maxValue = gtk_spin_button_get_value(maxSpin);
    
    // 如果最大值小于或等于最小值，则调整最大值
    if (maxValue <= minValue) {
        gtk_spin_button_set_value(maxSpin, minValue < 359.0 ? minValue + 1.0 : 360.0);
    }
}

// 俯仰角范围最小值变更时的回调函数
static void on_min_elevation_changed(GtkSpinButton* spin_button, gpointer data) {
    GtkSpinButton* maxSpin = GTK_SPIN_BUTTON(data);
    double minValue = gtk_spin_button_get_value(spin_button);
    double maxValue = gtk_spin_button_get_value(maxSpin);
    
    // 如果最大值小于或等于最小值，则调整最大值
    if (maxValue <= minValue) {
        gtk_spin_button_set_value(maxSpin, minValue < 89.0 ? minValue + 1.0 : 90.0);
    }
}

static void on_refresh_button_clicked(GtkWidget* widget, gpointer data) {
    ReconnaissanceDeviceModelView* view = static_cast<ReconnaissanceDeviceModelView*>(data);
    if (view) {
        ReconnaissanceDeviceModelController::getInstance().loadDeviceData();
    }
}

// 设置按钮点击回调
static void on_view_button_clicked(GtkWidget* widget, gpointer data) {
    // 获取设备ID并显示详情
    gint deviceId = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "device-id"));
    ReconnaissanceDeviceModelView* view = static_cast<ReconnaissanceDeviceModelView*>(data);
    if (view && deviceId > 0) {
        view->showDeviceDetailsDialog(deviceId);
    }
}

static void on_edit_button_clicked(GtkWidget* widget, gpointer data) {
    // 获取设备ID并调用编辑方法
    gint deviceId = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "device-id"));
    if (deviceId > 0) {
        ReconnaissanceDeviceModelController::getInstance().showEditDialog(deviceId);
    }
}

static void on_delete_button_clicked(GtkWidget* widget, gpointer data) {
    // 获取设备ID并调用删除方法
    gint deviceId = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "device-id"));
    GtkWidget* parent = gtk_widget_get_toplevel(widget);
    
    if (deviceId > 0) {
        // 显示确认对话框
        GtkWidget* dialog = gtk_message_dialog_new(
            GTK_WINDOW(parent),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_QUESTION,
            GTK_BUTTONS_YES_NO,
            "确定要删除选中的侦察设备吗？"
        );
        
        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        if (result == GTK_RESPONSE_YES) {
            ReconnaissanceDeviceModelController::getInstance().deleteDevice(deviceId);
        }
    }
}

// 设置自定义单元格渲染回调函数
static void render_button_cell(GtkTreeViewColumn *col,
                              GtkCellRenderer *renderer,
                              GtkTreeModel *model,
                              GtkTreeIter *iter,
                              gpointer user_data) {
    gchar *text;
    gtk_tree_model_get(model, iter, GPOINTER_TO_INT(user_data), &text, -1);
    
    // 设置文本
    g_object_set(renderer, "text", text, NULL);
    
    // 释放分配的字符串
    g_free(text);
}

static void on_row_activated(GtkTreeView* treeView, GtkTreePath* path, GtkTreeViewColumn* column, gpointer data) {
    ReconnaissanceDeviceModelView* view = static_cast<ReconnaissanceDeviceModelView*>(data);
    
    // 获取设备ID
    int deviceId = view->getSelectedDeviceId();
    if (deviceId < 0) return;
    
    // 获取点击的列索引
    int columnIndex = -1;
    for (int i = 0; i < gtk_tree_view_get_n_columns(treeView); i++) {
        if (gtk_tree_view_get_column(treeView, i) == column) {
            columnIndex = i;
            break;
        }
    }
    
    // 根据列索引执行相应操作
    if (columnIndex == 4) { // 查看
        view->showDeviceDetailsDialog(deviceId);
    } else if (columnIndex == 5) { // 编辑
        ReconnaissanceDeviceModelController::getInstance().showEditDialog(deviceId);
    } else if (columnIndex == 6) { // 删除
        // 显示确认对话框
        GtkWidget* dialog = gtk_message_dialog_new(
            GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(treeView))),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_QUESTION,
            GTK_BUTTONS_YES_NO,
            "确定要删除选中的侦察设备吗？"
        );
        
        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        if (result == GTK_RESPONSE_YES) {
            ReconnaissanceDeviceModelController::getInstance().deleteDevice(deviceId);
        }
    } else if (columnIndex >= 0 && columnIndex <= 3) { // 双击其他列显示详情
        view->showDeviceDetailsDialog(deviceId);
    }
}

// 单击处理函数
static gboolean on_button_press(GtkWidget* widget, GdkEventButton* event, gpointer data) {
    // 只处理左键单击
    if (event->type != GDK_BUTTON_PRESS || event->button != 1)
        return FALSE;
        
    ReconnaissanceDeviceModelView* view = static_cast<ReconnaissanceDeviceModelView*>(data);
    GtkTreeView* treeView = GTK_TREE_VIEW(widget);
    
    // 获取点击位置的路径和列
    GtkTreePath* path = NULL;
    GtkTreeViewColumn* column = NULL;
    gint cell_x, cell_y;
    
    if (!gtk_tree_view_get_path_at_pos(treeView, event->x, event->y, &path, &column, &cell_x, &cell_y)) {
        return FALSE;  // 没有点击到任何项
    }
    
    // 选中点击的行
    GtkTreeSelection* selection = gtk_tree_view_get_selection(treeView);
    gtk_tree_selection_select_path(selection, path);
    
    // 获取列索引
    int columnIndex = -1;
    for (int i = 0; i < gtk_tree_view_get_n_columns(treeView); i++) {
        if (gtk_tree_view_get_column(treeView, i) == column) {
            columnIndex = i;
            break;
        }
    }
    
    // 处理操作列的单击
    if (columnIndex >= 4 && columnIndex <= 6) {
        int deviceId = view->getSelectedDeviceId();
        if (deviceId < 0) {
            gtk_tree_path_free(path);
            return FALSE;
        }
        
        if (columnIndex == 4) { // 查看
            view->showDeviceDetailsDialog(deviceId);
        } else if (columnIndex == 5) { // 编辑
            ReconnaissanceDeviceModelController::getInstance().showEditDialog(deviceId);
        } else if (columnIndex == 6) { // 删除
            // 显示确认对话框
            GtkWidget* dialog = gtk_message_dialog_new(
                GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(treeView))),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_QUESTION,
                GTK_BUTTONS_YES_NO,
                "确定要删除选中的侦察设备吗？"
            );
            
            gint result = gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            
            if (result == GTK_RESPONSE_YES) {
                ReconnaissanceDeviceModelController::getInstance().deleteDevice(deviceId);
            }
        }
        
        gtk_tree_path_free(path);
        return TRUE; // 事件已处理
    }
    
    gtk_tree_path_free(path);
    return FALSE; // 继续传递事件
}

// 提示信息回调函数
static gboolean on_tree_view_query_tooltip(GtkWidget *widget, 
                                         gint x, 
                                         gint y, 
                                         gboolean keyboard_mode, 
                                         GtkTooltip *tooltip, 
                                         gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(widget);
    GtkTreeModel *model;
    GtkTreeIter iter;
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
    if (columnIndex == 4) {
        gtk_tooltip_set_text(tooltip, "点击查看设备详情");
        gtk_tree_path_free(path);
        return TRUE;
    } else if (columnIndex == 5) {
        gtk_tooltip_set_text(tooltip, "点击编辑设备信息");
        gtk_tree_path_free(path);
        return TRUE;
    } else if (columnIndex == 6) {
        gtk_tooltip_set_text(tooltip, "点击删除此设备");
        gtk_tree_path_free(path);
        return TRUE;
    }
    
    gtk_tree_path_free(path);
    return FALSE;
}

ReconnaissanceDeviceModelView::ReconnaissanceDeviceModelView() : m_view(nullptr), m_deviceList(nullptr) {
    g_print("创建侦察设备模型视图\n");
}

ReconnaissanceDeviceModelView::~ReconnaissanceDeviceModelView() {
    g_print("销毁侦察设备模型视图\n");
}

// 设置容器控件
void ReconnaissanceDeviceModelView::setContainer(GtkWidget* container) {
    m_view = container;
}

// 设置树视图控件
void ReconnaissanceDeviceModelView::setTreeView(GtkWidget* treeView) {
    m_deviceList = treeView;
}

// 创建侦察设备模型UI
GtkWidget* ReconnaissanceDeviceModelView::createView() {
    g_print("创建侦察设备模型UI组件...\n");
    
    try {
        // 创建页面的主容器
        m_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        if (!m_view) {
            g_print("创建容器失败\n");
            return nullptr;
        }
        
        gtk_container_set_border_width(GTK_CONTAINER(m_view), 15);
        
        // 标题
        g_print("  创建标题标签...\n");
        GtkWidget* titleLabel = gtk_label_new(nullptr);
        if (!titleLabel) {
            g_print("创建标题标签失败\n");
            gtk_widget_destroy(m_view);
            m_view = nullptr;
            return nullptr;
        }
        
        gtk_label_set_markup(GTK_LABEL(titleLabel), "<span font='16' weight='bold'>侦察设备模型管理</span>");
        gtk_box_pack_start(GTK_BOX(m_view), titleLabel, FALSE, FALSE, 5);
        
        // 创建表格列表
        g_print("  创建模型列表...\n");
        
        // 创建列表存储
        GtkListStore* store = gtk_list_store_new(7, 
                                              G_TYPE_STRING,  // ID
                                              G_TYPE_STRING,  // 名称
                                              G_TYPE_STRING,  // 类型
                                              G_TYPE_STRING,  // 位置
                                              G_TYPE_STRING,  // 查看按钮
                                              G_TYPE_STRING,  // 编辑按钮
                                              G_TYPE_STRING); // 删除按钮
        
        // 创建树视图
        m_deviceList = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
        g_object_unref(store);
        
        // 添加列
        GtkCellRenderer* renderer;
        GtkTreeViewColumn* column;
        
        // ID列
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(m_deviceList), column);
        
        // 名称列
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("名称", renderer, "text", 1, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(m_deviceList), column);
        
        // 类型列
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("类型", renderer, "text", 2, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(m_deviceList), column);
        
        // 位置列
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("位置信息", renderer, "text", 3, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(m_deviceList), column);
        
        // 查看按钮列
        renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, 
                    "foreground", "green", 
                    "underline", PANGO_UNDERLINE_SINGLE,
                    "xalign", 0.0,  // 左对齐
                    "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE,
                    NULL);
        column = gtk_tree_view_column_new_with_attributes("查看", renderer, "text", 4, NULL);
        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
        gtk_tree_view_column_set_fixed_width(column, 60);
        gtk_tree_view_append_column(GTK_TREE_VIEW(m_deviceList), column);
        
        // 编辑按钮列
        renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, 
                    "foreground", "blue", 
                    "underline", PANGO_UNDERLINE_SINGLE,
                    "xalign", 0.0,  // 左对齐
                    "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE,
                    NULL);
        column = gtk_tree_view_column_new_with_attributes("编辑", renderer, "text", 5, NULL);
        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
        gtk_tree_view_column_set_fixed_width(column, 60);
        gtk_tree_view_append_column(GTK_TREE_VIEW(m_deviceList), column);
        
        // 删除按钮列
        renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, 
                    "foreground", "red", 
                    "underline", PANGO_UNDERLINE_SINGLE,
                    "xalign", 0.0,  // 左对齐
                    "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE,
                    NULL);
        column = gtk_tree_view_column_new_with_attributes("删除", renderer, "text", 6, NULL);
        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
        gtk_tree_view_column_set_fixed_width(column, 60);
        gtk_tree_view_append_column(GTK_TREE_VIEW(m_deviceList), column);
        
        // 滚动窗口
        g_print("  创建滚动窗口...\n");
        GtkWidget* scrollWin = gtk_scrolled_window_new(NULL, NULL);
        if (!scrollWin) {
            g_print("创建滚动窗口失败\n");
            gtk_widget_destroy(m_view);
            m_view = nullptr;
            return nullptr;
        }
        
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWin), 
                                     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_container_add(GTK_CONTAINER(scrollWin), m_deviceList);
        
        // 设置最小高度
        gtk_widget_set_size_request(scrollWin, -1, 500);
        gtk_box_pack_start(GTK_BOX(m_view), scrollWin, TRUE, TRUE, 0);
        
        // 按钮区域
        g_print("  创建按钮区域...\n");
        GtkWidget* buttonBox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
        if (!buttonBox) {
            g_print("创建按钮区域失败\n");
            gtk_widget_destroy(m_view);
            m_view = nullptr;
            return nullptr;
        }
        
        gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonBox), GTK_BUTTONBOX_END);
        gtk_box_set_spacing(GTK_BOX(buttonBox), 10);
        gtk_box_pack_start(GTK_BOX(m_view), buttonBox, FALSE, FALSE, 0);
        
        // 新增按钮
        GtkWidget* addButton = gtk_button_new_with_label("新增");
        gtk_container_add(GTK_CONTAINER(buttonBox), addButton);
        g_signal_connect(addButton, "clicked", G_CALLBACK(on_add_button_clicked), this);
        
        // 刷新按钮
        GtkWidget* refreshButton = gtk_button_new_with_label("刷新");
        gtk_container_add(GTK_CONTAINER(buttonBox), refreshButton);
        g_signal_connect(refreshButton, "clicked", G_CALLBACK(on_refresh_button_clicked), this);
        
        // 设置树视图的行点击事件
        g_signal_connect(m_deviceList, "row-activated", G_CALLBACK(on_row_activated), this);
        
        // 添加单击事件处理
        g_signal_connect(m_deviceList, "button-press-event", G_CALLBACK(on_button_press), this);
        
        // 允许单击选择行
        GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_deviceList));
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
        
        // 启用指针悬停效果
        gtk_widget_set_has_tooltip(m_deviceList, TRUE);
        g_signal_connect(m_deviceList, "query-tooltip", G_CALLBACK(on_tree_view_query_tooltip), NULL);
        
        // 加载设备数据
        g_print("  加载侦察设备数据...\n");
        ReconnaissanceDeviceModelController::getInstance().loadDeviceData();
        
        g_print("侦察设备模型UI创建成功\n");
        return m_view;
    } catch (const std::exception& e) {
        g_print("创建侦察设备模型UI异常: %s\n", e.what());
        return nullptr;
    } catch (...) {
        g_print("创建侦察设备模型UI未知异常\n");
        return nullptr;
    }
}

// 创建侦察设备编辑对话框
GtkWidget* ReconnaissanceDeviceModelView::createEditDialog(const ReconnaissanceDevice& device) {
    // 判断是新增还是编辑操作
    bool isNewDevice = (device.getDeviceId() == 0);
    
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        isNewDevice ? "新增侦察设备" : "编辑侦察设备",
        nullptr,
        GTK_DIALOG_MODAL,
        "保存", GTK_RESPONSE_OK,
        "取消", GTK_RESPONSE_CANCEL,
        nullptr
    );
    
    // 设置对话框大小
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 600);
    
    // 获取内容区域
    GtkWidget* contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(contentArea), 10);
    
    // 创建表格布局
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(contentArea), grid);
    
    int row = 0;
    
    // 设备名称
    GtkWidget* nameLabel = gtk_label_new("设备名称:");
    gtk_widget_set_halign(nameLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), nameLabel, 0, row, 1, 1);
    
    GtkWidget* nameEntry = gtk_entry_new();
    if (!isNewDevice) {
        gtk_entry_set_text(GTK_ENTRY(nameEntry), device.getDeviceName().c_str());
    }
    gtk_grid_attach(GTK_GRID(grid), nameEntry, 1, row, 1, 1);
    row++;
    
    // 设备类型
    GtkWidget* typeLabel = gtk_label_new("设备类型:");
    gtk_widget_set_halign(typeLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), typeLabel, 0, row, 1, 1);
    
    // 创建单选按钮
    GtkWidget* fixedRadio = gtk_radio_button_new_with_label(NULL, "固定");
    GtkWidget* mobileRadio = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(fixedRadio), "移动");
    
    GtkWidget* typeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(typeBox), fixedRadio, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(typeBox), mobileRadio, FALSE, FALSE, 0);
    gtk_grid_attach(GTK_GRID(grid), typeBox, 1, row, 1, 1);
    
    // 设置默认选中状态
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(isNewDevice ? fixedRadio : (device.getIsStationary() ? fixedRadio : mobileRadio)), TRUE);
    row++;
    
    // 基线长度
    GtkWidget* baselineLabel = gtk_label_new("基线长度(m):");
    gtk_widget_set_halign(baselineLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), baselineLabel, 0, row, 1, 1);
    
    GtkWidget* baselineSpin = gtk_spin_button_new_with_range(0, 1000.0, 0.1);
    if (!isNewDevice) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(baselineSpin), device.getBaselineLength());
    } else {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(baselineSpin), 0);
    }
    gtk_grid_attach(GTK_GRID(grid), baselineSpin, 1, row, 1, 1);
    row++;
    
    // 噪声功率谱密度
    GtkWidget* noisePsdLabel = gtk_label_new("噪声功率谱密度(dBm/Hz):");
    gtk_widget_set_halign(noisePsdLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), noisePsdLabel, 0, row, 1, 1);
    
    GtkWidget* noisePsdSpin = gtk_spin_button_new_with_range(-200.0, 0.0, 0.1);
    if (!isNewDevice) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(noisePsdSpin), device.getNoisePsd());
    } else {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(noisePsdSpin), 0);
    }
    gtk_grid_attach(GTK_GRID(grid), noisePsdSpin, 1, row, 1, 1);
    row++;
    
    // 采样速率
    GtkWidget* sampleRateLabel = gtk_label_new("采样速率(GHz):");
    gtk_widget_set_halign(sampleRateLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), sampleRateLabel, 0, row, 1, 1);
    
    GtkWidget* sampleRateSpin = gtk_spin_button_new_with_range(0, 100.0, 0.1);
    if (!isNewDevice) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(sampleRateSpin), device.getSampleRate());
    } else {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(sampleRateSpin), 0);
    }
    gtk_grid_attach(GTK_GRID(grid), sampleRateSpin, 1, row, 1, 1);
    row++;
    
    // 侦收频率范围
    GtkWidget* freqRangeLabel = gtk_label_new("侦收频率范围(GHz):");
    gtk_widget_set_halign(freqRangeLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), freqRangeLabel, 0, row, 1, 1);
    
    // 创建水平布局容器
    GtkWidget* freqRangeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    // 最小频率
    GtkWidget* minFreqSpin = gtk_spin_button_new_with_range(0, 100.0, 0.1);
    // 确保最大频率大于最小频率
    double minFreq, maxFreq;
    if (!isNewDevice) {
        minFreq = device.getFreqRangeMin();
        maxFreq = device.getFreqRangeMax();
        if (maxFreq <= minFreq) {
            maxFreq = minFreq + 0.1;
        }
    } else {
        minFreq = 0.0;
        maxFreq = 0.0;
    }
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(minFreqSpin), minFreq);
    gtk_box_pack_start(GTK_BOX(freqRangeBox), minFreqSpin, TRUE, TRUE, 0);
    
    // 分隔符
    GtkWidget* freqRangeSeparator = gtk_label_new(" 至 ");
    gtk_box_pack_start(GTK_BOX(freqRangeBox), freqRangeSeparator, FALSE, FALSE, 0);
    
    // 最大频率
    GtkWidget* maxFreqSpin = gtk_spin_button_new_with_range(0, 100.0, 0.1);
    // 确保最大频率大于最小频率
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(maxFreqSpin), maxFreq);
    gtk_box_pack_start(GTK_BOX(freqRangeBox), maxFreqSpin, TRUE, TRUE, 0);
    
    gtk_grid_attach(GTK_GRID(grid), freqRangeBox, 1, row, 1, 1);
    row++;
    
    // 方位角范围
    GtkWidget* azimuthRangeLabel = gtk_label_new("方位角范围(度):");
    gtk_widget_set_halign(azimuthRangeLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), azimuthRangeLabel, 0, row, 1, 1);
    
    // 创建水平布局容器
    GtkWidget* azimuthRangeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    // 方位角下限
    GtkWidget* minAzimuthSpin = gtk_spin_button_new_with_range(0.0, 359.0, 0.1);
    // 确保方位角上限大于下限
    double minAzimuth, maxAzimuth;
    if (!isNewDevice) {
        minAzimuth = device.getAngleAzimuthMin();
        maxAzimuth = device.getAngleAzimuthMax();
        if (maxAzimuth <= minAzimuth) {
            maxAzimuth = minAzimuth + 1.0;
            if (maxAzimuth > 360.0) {
                maxAzimuth = 360.0;
                minAzimuth = 359.0;
            }
        }
    } else {
        minAzimuth = 0.0;
        maxAzimuth = 0.0;
    }
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(minAzimuthSpin), minAzimuth);
    gtk_box_pack_start(GTK_BOX(azimuthRangeBox), minAzimuthSpin, TRUE, TRUE, 0);
    
    // 分隔符
    GtkWidget* azimuthRangeSeparator = gtk_label_new(" 至 ");
    gtk_box_pack_start(GTK_BOX(azimuthRangeBox), azimuthRangeSeparator, FALSE, FALSE, 0);
    
    // 方位角上限
    GtkWidget* maxAzimuthSpin = gtk_spin_button_new_with_range(0, 360.0, 0.1);
    // 确保方位角上限大于下限
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(maxAzimuthSpin), maxAzimuth);
    gtk_box_pack_start(GTK_BOX(azimuthRangeBox), maxAzimuthSpin, TRUE, TRUE, 0);
    
    gtk_grid_attach(GTK_GRID(grid), azimuthRangeBox, 1, row, 1, 1);
    row++;
    
    // 俯仰角范围
    GtkWidget* elevationRangeLabel = gtk_label_new("俯仰角范围(度):");
    gtk_widget_set_halign(elevationRangeLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), elevationRangeLabel, 0, row, 1, 1);
    
    // 创建水平布局容器
    GtkWidget* elevationRangeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    // 俯仰角下限
    GtkWidget* minElevationSpin = gtk_spin_button_new_with_range(-90.0, 89.0, 0.1);
    // 确保俯仰角上限大于下限
    double minElevation, maxElevation;
    if (!isNewDevice) {
        minElevation = device.getAngleElevationMin();
        maxElevation = device.getAngleElevationMax();
        if (maxElevation <= minElevation) {
            maxElevation = minElevation + 1.0;
            if (maxElevation > 90.0) {
                maxElevation = 90.0;
                minElevation = 89.0;
            }
        }
    } else {
        minElevation = 0.0;
        maxElevation = 0.0;
    }
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(minElevationSpin), minElevation);
    gtk_box_pack_start(GTK_BOX(elevationRangeBox), minElevationSpin, TRUE, TRUE, 0);
    
    // 分隔符
    GtkWidget* elevationRangeSeparator = gtk_label_new(" 至 ");
    gtk_box_pack_start(GTK_BOX(elevationRangeBox), elevationRangeSeparator, FALSE, FALSE, 0);
    
    // 俯仰角上限
    GtkWidget* maxElevationSpin = gtk_spin_button_new_with_range(-89.0, 90.0, 0.1);
    // 确保俯仰角上限大于下限
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(maxElevationSpin), maxElevation);
    gtk_box_pack_start(GTK_BOX(elevationRangeBox), maxElevationSpin, TRUE, TRUE, 0);
    
    gtk_grid_attach(GTK_GRID(grid), elevationRangeBox, 1, row, 1, 1);
    row++;
    
    // 位置坐标
    GtkWidget* positionLabel = gtk_label_new("位置坐标:");
    gtk_widget_set_halign(positionLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), positionLabel, 0, row, 1, 1);
    
    // 创建水平布局容器
    GtkWidget* positionBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    // 经度
    GtkWidget* longitudeSpin = gtk_spin_button_new_with_range(-180.0, 180.0, 0.000001);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(longitudeSpin), 6);
    if (!isNewDevice) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(longitudeSpin), device.getLongitude());
    } else {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(longitudeSpin), 0.0);
    }
    gtk_box_pack_start(GTK_BOX(positionBox), longitudeSpin, TRUE, TRUE, 0);
    
    // 纬度
    GtkWidget* latitudeSpin = gtk_spin_button_new_with_range(-90.0, 90.0, 0.000001);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(latitudeSpin), 6);
    if (!isNewDevice) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(latitudeSpin), device.getLatitude());
    } else {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(latitudeSpin), 0.0);
    }
    gtk_box_pack_start(GTK_BOX(positionBox), latitudeSpin, TRUE, TRUE, 0);
    
    // 高度
    GtkWidget* altitudeSpin = gtk_spin_button_new_with_range(0.0, 10000.0, 0.1);
    if (!isNewDevice) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(altitudeSpin), device.getAltitude());
    } else {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(altitudeSpin), 0.0);
    }
    gtk_box_pack_start(GTK_BOX(positionBox), altitudeSpin, TRUE, TRUE, 0);
    
    gtk_grid_attach(GTK_GRID(grid), positionBox, 1, row, 1, 1);
    row++;
    
    // 运动参数区域
    GtkWidget* movementFrame = gtk_frame_new("运动参数");
    gtk_grid_attach(GTK_GRID(grid), movementFrame, 0, row, 2, 1);
    row++;
    
    GtkWidget* movementGrid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(movementGrid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(movementGrid), 5);
    gtk_container_set_border_width(GTK_CONTAINER(movementGrid), 10);
    gtk_container_add(GTK_CONTAINER(movementFrame), movementGrid);
    
    // 运动速度
    GtkWidget* speedLabel = gtk_label_new("运动速度(m/s):");
    gtk_widget_set_halign(speedLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(movementGrid), speedLabel, 0, 0, 1, 1);
    
    GtkWidget* speedSpin = gtk_spin_button_new_with_range(0.0, 1000.0, 0.1);
    if (!isNewDevice) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(speedSpin), device.getMovementSpeed());
    } else {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(speedSpin), 0.0);
    }
    gtk_grid_attach(GTK_GRID(movementGrid), speedSpin, 1, 0, 1, 1);
    
    // 运动方位角
    GtkWidget* movAzimuthLabel = gtk_label_new("运动方位角(度):");
    gtk_widget_set_halign(movAzimuthLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(movementGrid), movAzimuthLabel, 0, 1, 1, 1);
    
    GtkWidget* movAzimuthSpin = gtk_spin_button_new_with_range(0.0, 359.99, 0.01);
    if (!isNewDevice) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(movAzimuthSpin), device.getMovementAzimuth());
    } else {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(movAzimuthSpin), 0.0);
    }
    gtk_grid_attach(GTK_GRID(movementGrid), movAzimuthSpin, 1, 1, 1, 1);
    
    // 运动俯仰角
    GtkWidget* movElevationLabel = gtk_label_new("运动俯仰角(度):");
    gtk_widget_set_halign(movElevationLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(movementGrid), movElevationLabel, 0, 2, 1, 1);
    
    GtkWidget* movElevationSpin = gtk_spin_button_new_with_range(-90.0, 90.0, 0.01);
    if (!isNewDevice) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(movElevationSpin), device.getMovementElevation());
    } else {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(movElevationSpin), 0.0);
    }
    gtk_grid_attach(GTK_GRID(movementGrid), movElevationSpin, 1, 2, 1, 1);
    
    // 保存对话框中的控件引用，以便在getDeviceFromDialog中使用
    g_object_set_data(G_OBJECT(dialog), "nameEntry", nameEntry);
    g_object_set_data(G_OBJECT(dialog), "fixedRadio", fixedRadio);
    g_object_set_data(G_OBJECT(dialog), "baselineSpin", baselineSpin);
    g_object_set_data(G_OBJECT(dialog), "noisePsdSpin", noisePsdSpin);
    g_object_set_data(G_OBJECT(dialog), "sampleRateSpin", sampleRateSpin);
    g_object_set_data(G_OBJECT(dialog), "minFreqSpin", minFreqSpin);
    g_object_set_data(G_OBJECT(dialog), "maxFreqSpin", maxFreqSpin);
    g_object_set_data(G_OBJECT(dialog), "minAzimuthSpin", minAzimuthSpin);
    g_object_set_data(G_OBJECT(dialog), "maxAzimuthSpin", maxAzimuthSpin);
    g_object_set_data(G_OBJECT(dialog), "minElevationSpin", minElevationSpin);
    g_object_set_data(G_OBJECT(dialog), "maxElevationSpin", maxElevationSpin);
    g_object_set_data(G_OBJECT(dialog), "longitudeSpin", longitudeSpin);
    g_object_set_data(G_OBJECT(dialog), "latitudeSpin", latitudeSpin);
    g_object_set_data(G_OBJECT(dialog), "altitudeSpin", altitudeSpin);
    g_object_set_data(G_OBJECT(dialog), "speedSpin", speedSpin);
    g_object_set_data(G_OBJECT(dialog), "movAzimuthSpin", movAzimuthSpin);
    g_object_set_data(G_OBJECT(dialog), "movElevationSpin", movElevationSpin);
    
    // 根据固定/移动类型切换运动参数的可用性
    g_signal_connect(fixedRadio, "toggled", G_CALLBACK(on_device_type_toggled), movementFrame);
    
    // 连接范围值变更信号
    g_signal_connect(minFreqSpin, "value-changed", G_CALLBACK(on_min_freq_changed), maxFreqSpin);
    g_signal_connect(minAzimuthSpin, "value-changed", G_CALLBACK(on_min_azimuth_changed), maxAzimuthSpin);
    g_signal_connect(minElevationSpin, "value-changed", G_CALLBACK(on_min_elevation_changed), maxElevationSpin);
    
    // 初始设置运动参数框架的可用性
    gtk_widget_set_sensitive(movementFrame, !device.getIsStationary());
    
    // 显示对话框
    gtk_widget_show_all(dialog);
    
    return dialog;
}

// 更新侦察设备列表
void ReconnaissanceDeviceModelView::updateDeviceList(const std::vector<ReconnaissanceDevice>& devices) {
    m_devices = devices;
    
    if (!m_deviceList) {
        g_print("设备列表未初始化，无法更新\n");
        return;
    }
    
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_deviceList));
    GtkListStore* store = GTK_LIST_STORE(model);
    
    // 清空列表
    gtk_list_store_clear(store);
    
    // 如果没有设备，添加提示信息
    if (devices.empty()) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 
                          0, "", 
                          1, "无数据", 
                          2, "请添加侦察设备", 
                          3, "", 
                          4, "查看",
                          5, "编辑",
                          6, "删除",
                          -1);
        return;
    }
    
    // 添加设备数据
    GtkTreeIter iter;
    for (const auto& device : devices) {
        gtk_list_store_append(store, &iter);
        
        // 构建位置信息字符串
        std::ostringstream positionSS;
        positionSS << "经度:" << device.getLongitude() << "°, 纬度:" << device.getLatitude() << "°, 高度:" << device.getAltitude() << "m";
        
        // 设置列表数据
        gtk_list_store_set(store, &iter, 
                          0, std::to_string(device.getDeviceId()).c_str(), 
                          1, device.getDeviceName().c_str(), 
                          2, device.getDeviceTypeString().c_str(), 
                          3, positionSS.str().c_str(),
                          4, "查看",
                          5, "编辑",
                          6, "删除",
                          -1);
    }
}

// 获取当前选中的侦察设备ID
int ReconnaissanceDeviceModelView::getSelectedDeviceId() const {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_deviceList));
    GtkTreeModel* model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar* idStr;
        gtk_tree_model_get(model, &iter, 0, &idStr, -1);
        
        if (idStr && strlen(idStr) > 0) {
            int deviceId = atoi(idStr);
            g_free(idStr);
            return deviceId;
        }
        
        if (idStr) g_free(idStr);
    }
    
    return -1; // 未选中或未找到
}

// 从编辑对话框获取侦察设备数据
ReconnaissanceDevice ReconnaissanceDeviceModelView::getDeviceFromDialog(GtkWidget* dialog) const {
    ReconnaissanceDevice device;
    
    // 获取控件引用
    GtkWidget* nameEntry = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "nameEntry"));
    GtkWidget* fixedRadio = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "fixedRadio"));
    GtkWidget* baselineSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "baselineSpin"));
    GtkWidget* noisePsdSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "noisePsdSpin"));
    GtkWidget* sampleRateSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "sampleRateSpin"));
    GtkWidget* minFreqSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "minFreqSpin"));
    GtkWidget* maxFreqSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "maxFreqSpin"));
    GtkWidget* minAzimuthSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "minAzimuthSpin"));
    GtkWidget* maxAzimuthSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "maxAzimuthSpin"));
    GtkWidget* minElevationSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "minElevationSpin"));
    GtkWidget* maxElevationSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "maxElevationSpin"));
    GtkWidget* longitudeSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "longitudeSpin"));
    GtkWidget* latitudeSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "latitudeSpin"));
    GtkWidget* altitudeSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "altitudeSpin"));
    GtkWidget* speedSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "speedSpin"));
    GtkWidget* movAzimuthSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "movAzimuthSpin"));
    GtkWidget* movElevationSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "movElevationSpin"));
    
    // 设置设备基本属性
    device.setDeviceName(gtk_entry_get_text(GTK_ENTRY(nameEntry)));
    device.setIsStationary(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fixedRadio)));
    
    try {
        double baselineLength = gtk_spin_button_get_value(GTK_SPIN_BUTTON(baselineSpin));
        device.setBaselineLength(baselineLength);
    } catch (...) {
        g_print("基线长度格式错误\n");
    }
    
    try {
        double noisePsd = gtk_spin_button_get_value(GTK_SPIN_BUTTON(noisePsdSpin));
        device.setNoisePsd(noisePsd);
    } catch (...) {
        g_print("噪声功率谱密度格式错误\n");
    }
    
    try {
        double sampleRate = gtk_spin_button_get_value(GTK_SPIN_BUTTON(sampleRateSpin));
        device.setSampleRate(sampleRate);
    } catch (...) {
        g_print("采样速率格式错误\n");
    }
    
    // 设置频率范围
    try {
        double minFreq = gtk_spin_button_get_value(GTK_SPIN_BUTTON(minFreqSpin));
        device.setFreqRangeMin(minFreq);
    } catch (...) {
        g_print("最小频率格式错误\n");
    }
    
    try {
        double maxFreq = gtk_spin_button_get_value(GTK_SPIN_BUTTON(maxFreqSpin));
        device.setFreqRangeMax(maxFreq);
    } catch (...) {
        g_print("最大频率格式错误\n");
    }
    
    // 设置方位角范围
    try {
        double minAzimuth = gtk_spin_button_get_value(GTK_SPIN_BUTTON(minAzimuthSpin));
        device.setAngleAzimuthMin(minAzimuth);
    } catch (...) {
        g_print("方位角下限格式错误\n");
    }
    
    try {
        double maxAzimuth = gtk_spin_button_get_value(GTK_SPIN_BUTTON(maxAzimuthSpin));
        device.setAngleAzimuthMax(maxAzimuth);
    } catch (...) {
        g_print("方位角上限格式错误\n");
    }
    
    // 设置俯仰角范围
    try {
        double minElevation = gtk_spin_button_get_value(GTK_SPIN_BUTTON(minElevationSpin));
        device.setAngleElevationMin(minElevation);
    } catch (...) {
        g_print("俯仰角下限格式错误\n");
    }
    
    try {
        double maxElevation = gtk_spin_button_get_value(GTK_SPIN_BUTTON(maxElevationSpin));
        device.setAngleElevationMax(maxElevation);
    } catch (...) {
        g_print("俯仰角上限格式错误\n");
    }
    
    // 设置运动参数
    try {
        double speed = gtk_spin_button_get_value(GTK_SPIN_BUTTON(speedSpin));
        device.setMovementSpeed(speed);
    } catch (...) {
        g_print("运动速度格式错误\n");
    }
    
    try {
        double movAzimuth = gtk_spin_button_get_value(GTK_SPIN_BUTTON(movAzimuthSpin));
        device.setMovementAzimuth(movAzimuth);
    } catch (...) {
        g_print("运动方位角格式错误\n");
    }
    
    try {
        double movElevation = gtk_spin_button_get_value(GTK_SPIN_BUTTON(movElevationSpin));
        device.setMovementElevation(movElevation);
    } catch (...) {
        g_print("运动俯仰角格式错误\n");
    }
    
    // 设置位置坐标
    try {
        double longitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(longitudeSpin));
        device.setLongitude(longitude);
    } catch (...) {
        g_print("经度格式错误\n");
    }
    
    try {
        double latitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(latitudeSpin));
        device.setLatitude(latitude);
    } catch (...) {
        g_print("纬度格式错误\n");
    }
    
    try {
        double altitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(altitudeSpin));
        device.setAltitude(altitude);
    } catch (...) {
        g_print("高度格式错误\n");
    }
    
    return device;
}

// 获取视图控件
GtkWidget* ReconnaissanceDeviceModelView::getView() const {
    return m_view;
}

// 显示设备详情对话框
void ReconnaissanceDeviceModelView::showDeviceDetailsDialog(int deviceId) {
    // 获取设备数据
    ReconnaissanceDevice device;
    bool found = false;
    
    // 先在内存中查找
    for (const auto& d : m_devices) {
        if (d.getDeviceId() == deviceId) {
            device = d;
            found = true;
            break;
        }
    }
    
    // 如果在内存中找不到，尝试从数据库加载
    if (!found) {
        device = ReconnaissanceDeviceDAO::getInstance().getReconnaissanceDeviceById(deviceId);
        if (device.getDeviceId() <= 0) {
            g_print("无法找到ID为%d的侦察设备\n", deviceId);
            return;
        }
    }
    
    // 创建对话框
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        "侦察设备详情",
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
    
    // 创建网格布局用于显示设备信息
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_container_add(GTK_CONTAINER(contentArea), grid);
    
    // 标题标签
    gchar* titleMarkup = g_markup_printf_escaped("<span font='16' weight='bold'>%s 详细信息</span>", device.getDeviceName().c_str());
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
    int row = 2;
    
    // 添加信息项
    auto addInfoRow = [&](const char* label, const char* value) {
        GtkWidget* nameLabel = gtk_label_new(label);
        gtk_widget_set_halign(nameLabel, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), nameLabel, 0, row, 1, 1);
        
        GtkWidget* valueLabel = gtk_label_new(value);
        gtk_widget_set_halign(valueLabel, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), valueLabel, 1, row, 1, 1);
        
        row++;
    };
    
    // 设备ID
    addInfoRow("设备ID:", std::to_string(device.getDeviceId()).c_str());
    
    // 设备名称
    addInfoRow("设备名称:", device.getDeviceName().c_str());
    
    // 设备类型
    addInfoRow("设备类型:", device.getDeviceTypeString().c_str());
    
    // 基线长度
    std::ostringstream baselineSS;
    baselineSS << device.getBaselineLength() << " m";
    addInfoRow("基线长度:", baselineSS.str().c_str());
    
    // 噪声功率谱密度
    std::ostringstream noiseSS;
    noiseSS << device.getNoisePsd() << " dBm/Hz";
    addInfoRow("噪声功率谱密度:", noiseSS.str().c_str());
    
    // 采样速率
    std::ostringstream sampleRateSS;
    sampleRateSS << device.getSampleRate() << " GHz";
    addInfoRow("采样速率:", sampleRateSS.str().c_str());
    
    // 侦收频率范围
    std::ostringstream freqRangeSS;
    freqRangeSS << device.getFreqRangeMin() << " - " << device.getFreqRangeMax() << " GHz";
    addInfoRow("侦收频率范围:", freqRangeSS.str().c_str());
    
    // 方位角范围
    std::ostringstream azimuthRangeSS;
    azimuthRangeSS << device.getAngleAzimuthMin() << " - " << device.getAngleAzimuthMax() << " 度";
    addInfoRow("方位角范围:", azimuthRangeSS.str().c_str());
    
    // 俯仰角范围
    std::ostringstream elevationRangeSS;
    elevationRangeSS << device.getAngleElevationMin() << " - " << device.getAngleElevationMax() << " 度";
    addInfoRow("俯仰角范围:", elevationRangeSS.str().c_str());
    
    // 位置坐标
    std::ostringstream positionSS;
    positionSS << "经度: " << device.getLongitude() << "°, 纬度: " << device.getLatitude() << "°, 高度: " << device.getAltitude() << " m";
    addInfoRow("位置坐标:", positionSS.str().c_str());
    
    // 如果是移动设备，显示运动参数
    if (!device.getIsStationary()) {
        // 添加运动参数分隔符和标题
        GtkWidget* movementSeparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_grid_attach(GTK_GRID(grid), movementSeparator, 0, row, 2, 1);
        row++;
        
        GtkWidget* movementLabel = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(movementLabel), "<span weight='bold'>运动参数</span>");
        gtk_widget_set_halign(movementLabel, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), movementLabel, 0, row, 2, 1);
        row++;
        
        // 运动速度
        std::ostringstream speedSS;
        speedSS << device.getMovementSpeed() << " m/s";
        addInfoRow("运动速度:", speedSS.str().c_str());
        
        // 运动方位角
        std::ostringstream movAzimuthSS;
        movAzimuthSS << device.getMovementAzimuth() << " 度";
        addInfoRow("运动方位角:", movAzimuthSS.str().c_str());
        
        // 运动俯仰角
        std::ostringstream movElevationSS;
        movElevationSS << device.getMovementElevation() << " 度";
        addInfoRow("运动俯仰角:", movElevationSS.str().c_str());
    }
    
    // 创建/更新时间
    if (!device.getCreatedAt().empty()) {
        addInfoRow("创建时间:", device.getCreatedAt().c_str());
    }
    
    if (!device.getUpdatedAt().empty()) {
        addInfoRow("更新时间:", device.getUpdatedAt().c_str());
    }
    
    // 显示对话框
    gtk_widget_show_all(dialog);
    
    // 运行对话框并在关闭时销毁
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
} 