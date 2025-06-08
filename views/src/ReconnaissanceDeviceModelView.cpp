#include "../ReconnaissanceDeviceModelView.h"
#include "../../controllers/ApplicationController.h"
#include <iostream>
#include <sstream>

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
                                              G_TYPE_STRING,  // 名称
                                              G_TYPE_STRING,  // 类型
                                              G_TYPE_STRING,  // 基线长度
                                              G_TYPE_STRING,  // 频率范围
                                              G_TYPE_STRING,  // 角度范围
                                              G_TYPE_STRING,  // 编辑按钮
                                              G_TYPE_STRING); // 删除按钮
        
        // 创建树视图
        m_deviceList = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
        g_object_unref(store);
        
        // 添加列
        GtkCellRenderer* renderer;
        GtkTreeViewColumn* column;
        
        // 名称列
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("名称", renderer, "text", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(m_deviceList), column);
        
        // 类型列
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("类型", renderer, "text", 1, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(m_deviceList), column);
        
        // 基线长度列
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("基线长度(m)", renderer, "text", 2, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(m_deviceList), column);
        
        // 频率范围列
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("频率范围(MHz)", renderer, "text", 3, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(m_deviceList), column);
        
        // 角度范围列
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("角度范围(°)", renderer, "text", 4, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(m_deviceList), column);
        
        // 编辑按钮列
        renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "foreground", "blue", "underline", TRUE, NULL);
        column = gtk_tree_view_column_new_with_attributes("编辑", renderer, "text", 5, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(m_deviceList), column);
        
        // 删除按钮列
        renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "foreground", "red", "underline", TRUE, NULL);
        column = gtk_tree_view_column_new_with_attributes("删除", renderer, "text", 6, NULL);
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
        
        // 连接信号
        g_signal_connect(addButton, "clicked", G_CALLBACK(+[](GtkWidget* widget, gpointer data) {
            // 这里应该调用控制器的方法来处理新增设备
            g_print("新增侦察设备按钮被点击\n");
            ReconnaissanceDeviceModelView* view = static_cast<ReconnaissanceDeviceModelView*>(data);
            GtkWidget* dialog = view->createEditDialog();
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }), this);
        
        // 更新列表数据
        g_print("  更新侦察设备列表...\n");
        ApplicationController::updateReconnaissanceDeviceList(m_deviceList);
        
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
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        "侦察设备参数设置",
        nullptr,
        GTK_DIALOG_MODAL,
        "取消", GTK_RESPONSE_CANCEL,
        "确定", GTK_RESPONSE_OK,
        nullptr
    );
    
    // 设置对话框大小
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 300);
    
    // 获取内容区域
    GtkWidget* contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(contentArea), 10);
    
    // 创建表格布局
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(contentArea), grid);
    
    // 设备名称
    GtkWidget* nameLabel = gtk_label_new("设备名称:");
    gtk_grid_attach(GTK_GRID(grid), nameLabel, 0, 0, 1, 1);
    
    GtkWidget* nameEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(nameEntry), device.getDeviceName().c_str());
    gtk_grid_attach(GTK_GRID(grid), nameEntry, 1, 0, 1, 1);
    
    // 设备类型
    GtkWidget* typeLabel = gtk_label_new("设备类型:");
    gtk_grid_attach(GTK_GRID(grid), typeLabel, 0, 1, 1, 1);
    
    GtkWidget* typeCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(typeCombo), nullptr, "固定式");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(typeCombo), nullptr, "移动式");
    gtk_combo_box_set_active(GTK_COMBO_BOX(typeCombo), device.getIsStationary() ? 0 : 1);
    gtk_grid_attach(GTK_GRID(grid), typeCombo, 1, 1, 1, 1);
    
    // 基线长度
    GtkWidget* baselineLabel = gtk_label_new("基线长度(m):");
    gtk_grid_attach(GTK_GRID(grid), baselineLabel, 0, 2, 1, 1);
    
    GtkWidget* baselineEntry = gtk_entry_new();
    std::ostringstream baselineSS;
    baselineSS << device.getBaselineLength();
    gtk_entry_set_text(GTK_ENTRY(baselineEntry), baselineSS.str().c_str());
    gtk_grid_attach(GTK_GRID(grid), baselineEntry, 1, 2, 1, 1);
    
    // 最小频率
    GtkWidget* minFreqLabel = gtk_label_new("最小频率(MHz):");
    gtk_grid_attach(GTK_GRID(grid), minFreqLabel, 0, 3, 1, 1);
    
    GtkWidget* minFreqEntry = gtk_entry_new();
    std::ostringstream minFreqSS;
    minFreqSS << device.getFreqRangeMin();
    gtk_entry_set_text(GTK_ENTRY(minFreqEntry), minFreqSS.str().c_str());
    gtk_grid_attach(GTK_GRID(grid), minFreqEntry, 1, 3, 1, 1);
    
    // 最大频率
    GtkWidget* maxFreqLabel = gtk_label_new("最大频率(MHz):");
    gtk_grid_attach(GTK_GRID(grid), maxFreqLabel, 0, 4, 1, 1);
    
    GtkWidget* maxFreqEntry = gtk_entry_new();
    std::ostringstream maxFreqSS;
    maxFreqSS << device.getFreqRangeMax();
    gtk_entry_set_text(GTK_ENTRY(maxFreqEntry), maxFreqSS.str().c_str());
    gtk_grid_attach(GTK_GRID(grid), maxFreqEntry, 1, 4, 1, 1);
    
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
                          0, "无数据", 
                          1, "请添加侦察设备", 
                          2, "", 
                          3, "", 
                          4, "", 
                          5, "编辑",
                          6, "删除",
                          -1);
        return;
    }
    
    // 添加设备数据
    GtkTreeIter iter;
    for (const auto& device : devices) {
        gtk_list_store_append(store, &iter);
        
        // 将数值转换为字符串
        std::ostringstream baselineSS;
        baselineSS << device.getBaselineLength();
        
        // 设置列表数据
        gtk_list_store_set(store, &iter, 
                          0, device.getDeviceName().c_str(), 
                          1, device.getDeviceTypeString().c_str(), 
                          2, baselineSS.str().c_str(),
                          3, device.getFreqRangeString().c_str(), 
                          4, device.getAngleRangeString().c_str(), 
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
        gchar* name;
        gtk_tree_model_get(model, &iter, 0, &name, -1);
        
        // 通过名称查找设备ID
        for (const auto& device : m_devices) {
            if (device.getDeviceName() == name) {
                g_free(name);
                return device.getDeviceId();
            }
        }
        
        g_free(name);
    }
    
    return -1; // 未选中或未找到
}

// 从编辑对话框获取侦察设备数据
ReconnaissanceDevice ReconnaissanceDeviceModelView::getDeviceFromDialog(GtkWidget* dialog) const {
    ReconnaissanceDevice device;
    
    // 获取对话框内容区域
    GtkWidget* contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* grid = gtk_bin_get_child(GTK_BIN(contentArea));
    
    // 获取各个输入控件
    GtkWidget* nameEntry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 0);
    GtkWidget* typeCombo = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
    GtkWidget* baselineEntry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 2);
    GtkWidget* minFreqEntry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 3);
    GtkWidget* maxFreqEntry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 4);
    
    // 设置设备属性
    device.setDeviceName(gtk_entry_get_text(GTK_ENTRY(nameEntry)));
    device.setIsStationary(gtk_combo_box_get_active(GTK_COMBO_BOX(typeCombo)) == 0);
    
    try {
        double baselineLength = std::stod(gtk_entry_get_text(GTK_ENTRY(baselineEntry)));
        device.setBaselineLength(baselineLength);
    } catch (...) {
        g_print("基线长度格式错误\n");
    }
    
    try {
        double minFreq = std::stod(gtk_entry_get_text(GTK_ENTRY(minFreqEntry)));
        device.setFreqRangeMin(minFreq);
    } catch (...) {
        g_print("最小频率格式错误\n");
    }
    
    try {
        double maxFreq = std::stod(gtk_entry_get_text(GTK_ENTRY(maxFreqEntry)));
        device.setFreqRangeMax(maxFreq);
    } catch (...) {
        g_print("最大频率格式错误\n");
    }
    
    return device;
}

// 获取视图控件
GtkWidget* ReconnaissanceDeviceModelView::getView() const {
    return m_view;
} 