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
            // 调用控制器的方法来处理新增设备
            g_print("新增侦察设备按钮被点击\n");
            ReconnaissanceDeviceModelView* view = static_cast<ReconnaissanceDeviceModelView*>(data);
            
            // 获取控制器实例并显示编辑对话框
            ReconnaissanceDeviceModelController::getInstance().showEditDialog();
        }), this);
        
        // 设置树视图的行点击事件
        g_signal_connect(m_deviceList, "row-activated", G_CALLBACK(+[](GtkTreeView* treeView, GtkTreePath* path, GtkTreeViewColumn* column, gpointer data) {
            ReconnaissanceDeviceModelView* view = static_cast<ReconnaissanceDeviceModelView*>(data);
            
            // 获取选中行的索引
            gint* indices = gtk_tree_path_get_indices(path);
            if (!indices) return;
            
            // 获取点击的列
            gint columnNum = -1;
            gint numColumns = gtk_tree_view_get_n_columns(treeView);
            for (gint i = 0; i < numColumns; i++) {
                if (gtk_tree_view_get_column(treeView, i) == column) {
                    columnNum = i;
                    break;
                }
            }
            
            // 如果点击的是编辑列
            if (columnNum == 5) {
                // 获取设备ID并调用控制器的编辑方法
                int deviceId = view->getSelectedDeviceId();
                if (deviceId >= 0) {
                    ReconnaissanceDeviceModelController::getInstance().showEditDialog(deviceId);
                }
            }
            // 如果点击的是删除列
            else if (columnNum == 6) {
                // 获取设备ID并调用控制器的删除方法
                int deviceId = view->getSelectedDeviceId();
                if (deviceId >= 0) {
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
            }
        }), this);
        
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
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        "侦察设备参数设置",
        nullptr,
        GTK_DIALOG_MODAL,
        "取消", GTK_RESPONSE_CANCEL,
        "确定", GTK_RESPONSE_OK,
        nullptr
    );
    
    // 设置对话框大小
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 600);
    
    // 获取内容区域
    GtkWidget* contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(contentArea), 10);
    
    // 创建滚动窗口以容纳所有字段
    GtkWidget* scrollWin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWin), 
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(contentArea), scrollWin);
    
    // 创建表格布局
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(scrollWin), grid);
    
    int row = 0;
    
    // 设备名称
    GtkWidget* nameLabel = gtk_label_new("设备名称:");
    gtk_widget_set_halign(nameLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), nameLabel, 0, row, 1, 1);
    
    GtkWidget* nameEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(nameEntry), device.getDeviceName().c_str());
    gtk_grid_attach(GTK_GRID(grid), nameEntry, 1, row, 1, 1);
    row++;
    
    // 设备类型
    GtkWidget* typeLabel = gtk_label_new("设备类型:");
    gtk_widget_set_halign(typeLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), typeLabel, 0, row, 1, 1);
    
    GtkWidget* typeCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(typeCombo), nullptr, "固定设备");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(typeCombo), nullptr, "移动设备");
    gtk_combo_box_set_active(GTK_COMBO_BOX(typeCombo), device.getIsStationary() ? 0 : 1);
    gtk_grid_attach(GTK_GRID(grid), typeCombo, 1, row, 1, 1);
    row++;
    
    // 基线长度
    GtkWidget* baselineLabel = gtk_label_new("基线长度(m):");
    gtk_widget_set_halign(baselineLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), baselineLabel, 0, row, 1, 1);
    
    GtkWidget* baselineEntry = gtk_entry_new();
    std::ostringstream baselineSS;
    baselineSS << device.getBaselineLength();
    gtk_entry_set_text(GTK_ENTRY(baselineEntry), baselineSS.str().c_str());
    gtk_grid_attach(GTK_GRID(grid), baselineEntry, 1, row, 1, 1);
    row++;
    
    // 噪声功率谱密度
    GtkWidget* noisePsdLabel = gtk_label_new("噪声功率谱密度(dBm/Hz):");
    gtk_widget_set_halign(noisePsdLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), noisePsdLabel, 0, row, 1, 1);
    
    GtkWidget* noisePsdEntry = gtk_entry_new();
    std::ostringstream noisePsdSS;
    noisePsdSS << device.getNoisePsd();
    gtk_entry_set_text(GTK_ENTRY(noisePsdEntry), noisePsdSS.str().c_str());
    gtk_grid_attach(GTK_GRID(grid), noisePsdEntry, 1, row, 1, 1);
    row++;
    
    // 采样速率
    GtkWidget* sampleRateLabel = gtk_label_new("采样速率(GHz):");
    gtk_widget_set_halign(sampleRateLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), sampleRateLabel, 0, row, 1, 1);
    
    GtkWidget* sampleRateEntry = gtk_entry_new();
    std::ostringstream sampleRateSS;
    sampleRateSS << device.getSampleRate();
    gtk_entry_set_text(GTK_ENTRY(sampleRateEntry), sampleRateSS.str().c_str());
    gtk_grid_attach(GTK_GRID(grid), sampleRateEntry, 1, row, 1, 1);
    row++;
    
    // 侦收频率范围
    GtkWidget* freqRangeLabel = gtk_label_new("侦收频率范围(GHz):");
    gtk_widget_set_halign(freqRangeLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), freqRangeLabel, 0, row, 1, 1);
    
    // 创建水平布局容器
    GtkWidget* freqRangeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    // 最小频率
    GtkWidget* minFreqEntry = gtk_entry_new();
    gtk_widget_set_size_request(minFreqEntry, 80, -1);
    std::ostringstream minFreqSS;
    minFreqSS << device.getFreqRangeMin();
    gtk_entry_set_text(GTK_ENTRY(minFreqEntry), minFreqSS.str().c_str());
    gtk_box_pack_start(GTK_BOX(freqRangeBox), minFreqEntry, FALSE, FALSE, 0);
    
    // 分隔符
    GtkWidget* freqRangeSeparator = gtk_label_new(" - ");
    gtk_box_pack_start(GTK_BOX(freqRangeBox), freqRangeSeparator, FALSE, FALSE, 0);
    
    // 最大频率
    GtkWidget* maxFreqEntry = gtk_entry_new();
    gtk_widget_set_size_request(maxFreqEntry, 80, -1);
    std::ostringstream maxFreqSS;
    maxFreqSS << device.getFreqRangeMax();
    gtk_entry_set_text(GTK_ENTRY(maxFreqEntry), maxFreqSS.str().c_str());
    gtk_box_pack_start(GTK_BOX(freqRangeBox), maxFreqEntry, FALSE, FALSE, 0);
    
    gtk_grid_attach(GTK_GRID(grid), freqRangeBox, 1, row, 1, 1);
    row++;
    
    // 方位角范围
    GtkWidget* azimuthRangeLabel = gtk_label_new("方位角范围(度):");
    gtk_widget_set_halign(azimuthRangeLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), azimuthRangeLabel, 0, row, 1, 1);
    
    // 创建水平布局容器
    GtkWidget* azimuthRangeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    // 方位角下限
    GtkWidget* minAzimuthEntry = gtk_entry_new();
    gtk_widget_set_size_request(minAzimuthEntry, 80, -1);
    std::ostringstream minAzimuthSS;
    minAzimuthSS << device.getAngleAzimuthMin();
    gtk_entry_set_text(GTK_ENTRY(minAzimuthEntry), minAzimuthSS.str().c_str());
    gtk_box_pack_start(GTK_BOX(azimuthRangeBox), minAzimuthEntry, FALSE, FALSE, 0);
    
    // 分隔符
    GtkWidget* azimuthRangeSeparator = gtk_label_new(" - ");
    gtk_box_pack_start(GTK_BOX(azimuthRangeBox), azimuthRangeSeparator, FALSE, FALSE, 0);
    
    // 方位角上限
    GtkWidget* maxAzimuthEntry = gtk_entry_new();
    gtk_widget_set_size_request(maxAzimuthEntry, 80, -1);
    std::ostringstream maxAzimuthSS;
    maxAzimuthSS << device.getAngleAzimuthMax();
    gtk_entry_set_text(GTK_ENTRY(maxAzimuthEntry), maxAzimuthSS.str().c_str());
    gtk_box_pack_start(GTK_BOX(azimuthRangeBox), maxAzimuthEntry, FALSE, FALSE, 0);
    
    gtk_grid_attach(GTK_GRID(grid), azimuthRangeBox, 1, row, 1, 1);
    row++;
    
    // 俯仰角范围
    GtkWidget* elevationRangeLabel = gtk_label_new("俯仰角范围(度):");
    gtk_widget_set_halign(elevationRangeLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), elevationRangeLabel, 0, row, 1, 1);
    
    // 创建水平布局容器
    GtkWidget* elevationRangeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    // 俯仰角下限
    GtkWidget* minElevationEntry = gtk_entry_new();
    gtk_widget_set_size_request(minElevationEntry, 80, -1);
    std::ostringstream minElevationSS;
    minElevationSS << device.getAngleElevationMin();
    gtk_entry_set_text(GTK_ENTRY(minElevationEntry), minElevationSS.str().c_str());
    gtk_box_pack_start(GTK_BOX(elevationRangeBox), minElevationEntry, FALSE, FALSE, 0);
    
    // 分隔符
    GtkWidget* elevationRangeSeparator = gtk_label_new(" - ");
    gtk_box_pack_start(GTK_BOX(elevationRangeBox), elevationRangeSeparator, FALSE, FALSE, 0);
    
    // 俯仰角上限
    GtkWidget* maxElevationEntry = gtk_entry_new();
    gtk_widget_set_size_request(maxElevationEntry, 80, -1);
    std::ostringstream maxElevationSS;
    maxElevationSS << device.getAngleElevationMax();
    gtk_entry_set_text(GTK_ENTRY(maxElevationEntry), maxElevationSS.str().c_str());
    gtk_box_pack_start(GTK_BOX(elevationRangeBox), maxElevationEntry, FALSE, FALSE, 0);
    
    gtk_grid_attach(GTK_GRID(grid), elevationRangeBox, 1, row, 1, 1);
    row++;
    
    // 运动参数标题
    GtkWidget* movementLabel = gtk_label_new("运动参数");
    gtk_widget_set_halign(movementLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), movementLabel, 0, row, 2, 1);
    row++;
    
    // 运动速度
    GtkWidget* speedLabel = gtk_label_new("运动速度(m/s):");
    gtk_widget_set_halign(speedLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), speedLabel, 0, row, 1, 1);
    
    GtkWidget* speedEntry = gtk_entry_new();
    std::ostringstream speedSS;
    speedSS << device.getMovementSpeed();
    gtk_entry_set_text(GTK_ENTRY(speedEntry), speedSS.str().c_str());
    gtk_grid_attach(GTK_GRID(grid), speedEntry, 1, row, 1, 1);
    row++;
    
    // 运动方位角
    GtkWidget* movAzimuthLabel = gtk_label_new("运动方位角(度):");
    gtk_widget_set_halign(movAzimuthLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), movAzimuthLabel, 0, row, 1, 1);
    
    GtkWidget* movAzimuthEntry = gtk_entry_new();
    std::ostringstream movAzimuthSS;
    movAzimuthSS << device.getMovementAzimuth();
    gtk_entry_set_text(GTK_ENTRY(movAzimuthEntry), movAzimuthSS.str().c_str());
    gtk_grid_attach(GTK_GRID(grid), movAzimuthEntry, 1, row, 1, 1);
    row++;
    
    // 运动俯仰角
    GtkWidget* movElevationLabel = gtk_label_new("运动俯仰角(度):");
    gtk_widget_set_halign(movElevationLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), movElevationLabel, 0, row, 1, 1);
    
    GtkWidget* movElevationEntry = gtk_entry_new();
    std::ostringstream movElevationSS;
    movElevationSS << device.getMovementElevation();
    gtk_entry_set_text(GTK_ENTRY(movElevationEntry), movElevationSS.str().c_str());
    gtk_grid_attach(GTK_GRID(grid), movElevationEntry, 1, row, 1, 1);
    row++;
    
    // 位置坐标标题
    GtkWidget* positionLabel = gtk_label_new("位置坐标");
    gtk_widget_set_halign(positionLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), positionLabel, 0, row, 2, 1);
    row++;
    
    // 经度
    GtkWidget* longitudeLabel = gtk_label_new("经度(度):");
    gtk_widget_set_halign(longitudeLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), longitudeLabel, 0, row, 1, 1);
    
    GtkWidget* longitudeEntry = gtk_entry_new();
    std::ostringstream longitudeSS;
    longitudeSS << device.getLongitude();
    gtk_entry_set_text(GTK_ENTRY(longitudeEntry), longitudeSS.str().c_str());
    gtk_grid_attach(GTK_GRID(grid), longitudeEntry, 1, row, 1, 1);
    row++;
    
    // 纬度
    GtkWidget* latitudeLabel = gtk_label_new("纬度(度):");
    gtk_widget_set_halign(latitudeLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), latitudeLabel, 0, row, 1, 1);
    
    GtkWidget* latitudeEntry = gtk_entry_new();
    std::ostringstream latitudeSS;
    latitudeSS << device.getLatitude();
    gtk_entry_set_text(GTK_ENTRY(latitudeEntry), latitudeSS.str().c_str());
    gtk_grid_attach(GTK_GRID(grid), latitudeEntry, 1, row, 1, 1);
    row++;
    
    // 高度
    GtkWidget* altitudeLabel = gtk_label_new("高度(m):");
    gtk_widget_set_halign(altitudeLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), altitudeLabel, 0, row, 1, 1);
    
    GtkWidget* altitudeEntry = gtk_entry_new();
    std::ostringstream altitudeSS;
    altitudeSS << device.getAltitude();
    gtk_entry_set_text(GTK_ENTRY(altitudeEntry), altitudeSS.str().c_str());
    gtk_grid_attach(GTK_GRID(grid), altitudeEntry, 1, row, 1, 1);
    row++;
    
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
    GtkWidget* scrollWin = gtk_bin_get_child(GTK_BIN(contentArea));
    GtkWidget* grid = gtk_bin_get_child(GTK_BIN(scrollWin));
    
    // 获取基本信息控件
    GtkWidget* nameEntry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 0);
    GtkWidget* typeCombo = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
    GtkWidget* baselineEntry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 2);
    GtkWidget* noisePsdEntry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 3);
    GtkWidget* sampleRateEntry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 4);
    
    // 获取频率范围控件
    GtkWidget* freqRangeBox = gtk_grid_get_child_at(GTK_GRID(grid), 1, 5);
    GtkWidget* minFreqEntry = get_box_child_at_index(GTK_BOX(freqRangeBox), 0);
    GtkWidget* maxFreqEntry = get_box_child_at_index(GTK_BOX(freqRangeBox), 2);
    
    // 获取方位角范围控件
    GtkWidget* azimuthRangeBox = gtk_grid_get_child_at(GTK_GRID(grid), 1, 6);
    GtkWidget* minAzimuthEntry = get_box_child_at_index(GTK_BOX(azimuthRangeBox), 0);
    GtkWidget* maxAzimuthEntry = get_box_child_at_index(GTK_BOX(azimuthRangeBox), 2);
    
    // 获取俯仰角范围控件
    GtkWidget* elevationRangeBox = gtk_grid_get_child_at(GTK_GRID(grid), 1, 7);
    GtkWidget* minElevationEntry = get_box_child_at_index(GTK_BOX(elevationRangeBox), 0);
    GtkWidget* maxElevationEntry = get_box_child_at_index(GTK_BOX(elevationRangeBox), 2);
    
    // 获取运动参数控件
    GtkWidget* speedEntry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 9);
    GtkWidget* movAzimuthEntry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 10);
    GtkWidget* movElevationEntry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 11);
    
    // 获取位置坐标控件
    GtkWidget* longitudeEntry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 13);
    GtkWidget* latitudeEntry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 14);
    GtkWidget* altitudeEntry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 15);
    
    // 设置设备基本属性
    device.setDeviceName(gtk_entry_get_text(GTK_ENTRY(nameEntry)));
    device.setIsStationary(gtk_combo_box_get_active(GTK_COMBO_BOX(typeCombo)) == 0);
    
    try {
        double baselineLength = std::stod(gtk_entry_get_text(GTK_ENTRY(baselineEntry)));
        device.setBaselineLength(baselineLength);
    } catch (...) {
        g_print("基线长度格式错误\n");
    }
    
    try {
        double noisePsd = std::stod(gtk_entry_get_text(GTK_ENTRY(noisePsdEntry)));
        device.setNoisePsd(noisePsd);
    } catch (...) {
        g_print("噪声功率谱密度格式错误\n");
    }
    
    try {
        double sampleRate = std::stod(gtk_entry_get_text(GTK_ENTRY(sampleRateEntry)));
        device.setSampleRate(sampleRate);
    } catch (...) {
        g_print("采样速率格式错误\n");
    }
    
    // 设置频率范围
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
    
    // 设置方位角范围
    try {
        double minAzimuth = std::stod(gtk_entry_get_text(GTK_ENTRY(minAzimuthEntry)));
        device.setAngleAzimuthMin(minAzimuth);
    } catch (...) {
        g_print("方位角下限格式错误\n");
    }
    
    try {
        double maxAzimuth = std::stod(gtk_entry_get_text(GTK_ENTRY(maxAzimuthEntry)));
        device.setAngleAzimuthMax(maxAzimuth);
    } catch (...) {
        g_print("方位角上限格式错误\n");
    }
    
    // 设置俯仰角范围
    try {
        double minElevation = std::stod(gtk_entry_get_text(GTK_ENTRY(minElevationEntry)));
        device.setAngleElevationMin(minElevation);
    } catch (...) {
        g_print("俯仰角下限格式错误\n");
    }
    
    try {
        double maxElevation = std::stod(gtk_entry_get_text(GTK_ENTRY(maxElevationEntry)));
        device.setAngleElevationMax(maxElevation);
    } catch (...) {
        g_print("俯仰角上限格式错误\n");
    }
    
    // 设置运动参数
    try {
        double speed = std::stod(gtk_entry_get_text(GTK_ENTRY(speedEntry)));
        device.setMovementSpeed(speed);
    } catch (...) {
        g_print("运动速度格式错误\n");
    }
    
    try {
        double movAzimuth = std::stod(gtk_entry_get_text(GTK_ENTRY(movAzimuthEntry)));
        device.setMovementAzimuth(movAzimuth);
    } catch (...) {
        g_print("运动方位角格式错误\n");
    }
    
    try {
        double movElevation = std::stod(gtk_entry_get_text(GTK_ENTRY(movElevationEntry)));
        device.setMovementElevation(movElevation);
    } catch (...) {
        g_print("运动俯仰角格式错误\n");
    }
    
    // 设置位置坐标
    try {
        double longitude = std::stod(gtk_entry_get_text(GTK_ENTRY(longitudeEntry)));
        device.setLongitude(longitude);
    } catch (...) {
        g_print("经度格式错误\n");
    }
    
    try {
        double latitude = std::stod(gtk_entry_get_text(GTK_ENTRY(latitudeEntry)));
        device.setLatitude(latitude);
    } catch (...) {
        g_print("纬度格式错误\n");
    }
    
    try {
        double altitude = std::stod(gtk_entry_get_text(GTK_ENTRY(altitudeEntry)));
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