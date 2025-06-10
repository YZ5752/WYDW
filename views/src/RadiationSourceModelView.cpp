#include "../RadiationSourceModelView.h"
#include "../../controllers/ApplicationController.h"
#include "../../controllers/RadiationSourceModelController.h"
#include <iostream>

// 回调函数
static void on_add_button_clicked(GtkWidget* widget, gpointer data) {
    RadiationSourceModelView* view = static_cast<RadiationSourceModelView*>(data);
    if (view) {
        // 显示编辑对话框
        RadiationSourceModelController::getInstance().showEditDialog();
    }
}

static void on_edit_button_clicked(GtkWidget* widget, gpointer data) {
    RadiationSourceModelView* view = static_cast<RadiationSourceModelView*>(data);
    if (view) {
        int sourceId = view->getSelectedSourceId();
        if (sourceId > 0) {
            RadiationSourceModelController::getInstance().showEditDialog(sourceId);
        } else {
            GtkWidget* dialog = gtk_message_dialog_new(NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,
                "请先选择一个辐射源");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
    }
}

static void on_delete_button_clicked(GtkWidget* widget, gpointer data) {
    RadiationSourceModelView* view = static_cast<RadiationSourceModelView*>(data);
    if (view) {
        int sourceId = view->getSelectedSourceId();
        if (sourceId > 0) {
            GtkWidget* dialog = gtk_message_dialog_new(NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_QUESTION,
                GTK_BUTTONS_YES_NO,
                "确定要删除选中的辐射源吗？");
                
            gint result = gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            
            if (result == GTK_RESPONSE_YES) {
                RadiationSourceModelController::getInstance().deleteSource(sourceId);
            }
        } else {
            GtkWidget* dialog = gtk_message_dialog_new(NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,
                "请先选择一个辐射源");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
    }
}

static void on_refresh_button_clicked(GtkWidget* widget, gpointer data) {
    RadiationSourceModelView* view = static_cast<RadiationSourceModelView*>(data);
    if (view) {
        RadiationSourceModelController::getInstance().loadSourceData();
    }
}

// 处理固定/移动类型切换的回调函数
static void on_fixed_radio_toggled(GtkToggleButton* button, gpointer data) {
    GtkWidget* frame = GTK_WIDGET(data);
    gboolean active = gtk_toggle_button_get_active(button);
    gtk_widget_set_sensitive(frame, !active);
    
    if (active) {
        // 如果是固定辐射源，将运动参数设为0
        GtkWidget* dialog = gtk_widget_get_toplevel(frame);
        GtkWidget* speedSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "speedSpin"));
        GtkWidget* movAzimuthSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "movAzimuthSpin"));
        GtkWidget* movElevationSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "movElevationSpin"));
        
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(speedSpin), 0.0);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(movAzimuthSpin), 0.0);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(movElevationSpin), 0.0);
    }
}

// 实现RadiationSourceModelView类
RadiationSourceModelView::RadiationSourceModelView() : m_view(nullptr), m_sourceList(nullptr) {
}

RadiationSourceModelView::~RadiationSourceModelView() {
}

// 创建辐射源模型UI
GtkWidget* RadiationSourceModelView::createView() {
    g_print("Creating radiation source model UI components...\n");
    
    try {
        // 创建页面的主容器
        m_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        if (!m_view) {
            g_print("Failed to create container\n");
            return NULL;
        }
        
        gtk_container_set_border_width(GTK_CONTAINER(m_view), 15);
        
        // 标题
        g_print("  Creating title label...\n");
        GtkWidget* titleLabel = gtk_label_new(NULL);
        if (!titleLabel) {
            g_print("Failed to create title label\n");
            gtk_widget_destroy(m_view);
            return NULL;
        }
        
        gtk_label_set_markup(GTK_LABEL(titleLabel), "<span font='16' weight='bold'>辐射源模型管理</span>");
        gtk_box_pack_start(GTK_BOX(m_view), titleLabel, FALSE, FALSE, 5);
        
        // 创建表格列表
        g_print("  Creating model list...\n");
        std::vector<std::string> headers = {
            "ID",
            "名称", 
            "类型", 
            "发射功率(kW)", 
            "载波频率(GHz)", 
            "方位角范围(°)"
        };
        
        // 创建列表存储模型
        GtkListStore* store = gtk_list_store_new(6, 
            G_TYPE_STRING,  // ID
            G_TYPE_STRING,  // 名称
            G_TYPE_STRING,  // 类型
            G_TYPE_STRING,  // 发射功率
            G_TYPE_STRING,  // 频率
            G_TYPE_STRING   // 方位角范围
        );
        
        // 创建树形视图
        m_sourceList = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
        g_object_unref(store);
        
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m_sourceList), TRUE);
        
        // 创建列
        for (size_t i = 0; i < headers.size(); i++) {
            GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
            GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(
                headers[i].c_str(), renderer, "text", i, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(m_sourceList), column);
        }
        
        // 滚动窗口
        g_print("  Creating scroll window...\n");
        GtkWidget* scrollWin = gtk_scrolled_window_new(NULL, NULL);
        if (!scrollWin) {
            g_print("Failed to create scroll window\n");
            gtk_widget_destroy(m_view);
            return NULL;
        }
        
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWin), 
                                     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_container_add(GTK_CONTAINER(scrollWin), m_sourceList);
        
        // 设置最小高度
        gtk_widget_set_size_request(scrollWin, -1, 500);
        gtk_box_pack_start(GTK_BOX(m_view), scrollWin, TRUE, TRUE, 0);
        
        // 按钮区域
        g_print("  Creating button box...\n");
        GtkWidget* buttonBox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
        if (!buttonBox) {
            g_print("Failed to create button box\n");
            gtk_widget_destroy(m_view);
            return NULL;
        }
        
        gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonBox), GTK_BUTTONBOX_END);
        gtk_box_set_spacing(GTK_BOX(buttonBox), 10);
        gtk_box_pack_start(GTK_BOX(m_view), buttonBox, FALSE, FALSE, 0);
        
        // 新增按钮
        GtkWidget* addButton = gtk_button_new_with_label("新增");
        gtk_container_add(GTK_CONTAINER(buttonBox), addButton);
        g_signal_connect(addButton, "clicked", G_CALLBACK(on_add_button_clicked), this);
        
        // 编辑按钮
        GtkWidget* editButton = gtk_button_new_with_label("编辑");
        gtk_container_add(GTK_CONTAINER(buttonBox), editButton);
        g_signal_connect(editButton, "clicked", G_CALLBACK(on_edit_button_clicked), this);
        
        // 删除按钮
        GtkWidget* deleteButton = gtk_button_new_with_label("删除");
        gtk_container_add(GTK_CONTAINER(buttonBox), deleteButton);
        g_signal_connect(deleteButton, "clicked", G_CALLBACK(on_delete_button_clicked), this);
        
        // 刷新按钮
        GtkWidget* refreshButton = gtk_button_new_with_label("刷新");
        gtk_container_add(GTK_CONTAINER(buttonBox), refreshButton);
        g_signal_connect(refreshButton, "clicked", G_CALLBACK(on_refresh_button_clicked), this);
        
        g_print("Radiation source model UI created successfully\n");
        return m_view;
    } catch (const std::exception& e) {
        g_print("Exception in createRadiationSourceModelUI: %s\n", e.what());
        return NULL;
    } catch (...) {
        g_print("Unknown exception in createRadiationSourceModelUI\n");
        return NULL;
    }
}

// 创建辐射源编辑对话框
GtkWidget* RadiationSourceModelView::createEditDialog(const RadiationSource& source) {
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        source.getRadiationId() > 0 ? "编辑辐射源" : "新增辐射源",
        NULL,
        GTK_DIALOG_MODAL,
        "保存", GTK_RESPONSE_ACCEPT,
        "取消", GTK_RESPONSE_CANCEL,
        NULL);
    
    GtkWidget* contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(contentArea), 10);
    
    // 创建表单网格
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(contentArea), grid);
    
    // 辐射源名称
    GtkWidget* nameLabel = gtk_label_new("名称:");
    gtk_grid_attach(GTK_GRID(grid), nameLabel, 0, 0, 1, 1);
    
    GtkWidget* nameEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(nameEntry), source.getRadiationName().c_str());
    gtk_grid_attach(GTK_GRID(grid), nameEntry, 1, 0, 1, 1);
    
    // 辐射源类型
    GtkWidget* typeLabel = gtk_label_new("类型:");
    gtk_grid_attach(GTK_GRID(grid), typeLabel, 0, 1, 1, 1);
    
    GtkWidget* fixedRadio = gtk_radio_button_new_with_label(NULL, "固定");
    GtkWidget* mobileRadio = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(fixedRadio), "移动");
    
    GtkWidget* typeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(typeBox), fixedRadio, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(typeBox), mobileRadio, FALSE, FALSE, 0);
    gtk_grid_attach(GTK_GRID(grid), typeBox, 1, 1, 1, 1);
    
    // 设置默认选中状态
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(source.getIsStationary() ? fixedRadio : mobileRadio), TRUE);
    
    // 发射功率
    GtkWidget* powerLabel = gtk_label_new("发射功率(kW):");
    gtk_grid_attach(GTK_GRID(grid), powerLabel, 0, 2, 1, 1);
    
    GtkWidget* powerSpin = gtk_spin_button_new_with_range(0.0, 1000.0, 0.1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(powerSpin), source.getTransmitPower());
    gtk_grid_attach(GTK_GRID(grid), powerSpin, 1, 2, 1, 1);
    
    // 扫描周期
    GtkWidget* periodLabel = gtk_label_new("扫描周期(秒):");
    gtk_grid_attach(GTK_GRID(grid), periodLabel, 0, 3, 1, 1);
    
    GtkWidget* periodSpin = gtk_spin_button_new_with_range(0.1, 100.0, 0.1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(periodSpin), source.getScanPeriod() > 0 ? source.getScanPeriod() : 1.0);
    gtk_grid_attach(GTK_GRID(grid), periodSpin, 1, 3, 1, 1);
    
    // 载波频率
    GtkWidget* freqLabel = gtk_label_new("载波频率(GHz):");
    gtk_grid_attach(GTK_GRID(grid), freqLabel, 0, 4, 1, 1);
    
    GtkWidget* freqSpin = gtk_spin_button_new_with_range(0.1, 100.0, 0.1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(freqSpin), source.getCarrierFrequency());
    gtk_grid_attach(GTK_GRID(grid), freqSpin, 1, 4, 1, 1);
    
    // 方位角范围
    GtkWidget* azimuthLabel = gtk_label_new("方位角范围(度):");
    gtk_grid_attach(GTK_GRID(grid), azimuthLabel, 0, 5, 1, 1);
    
    GtkWidget* azimuthBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget* azimuthStartSpin = gtk_spin_button_new_with_range(0.0, 359.0, 0.1);
    GtkWidget* azimuthEndSpin = gtk_spin_button_new_with_range(0.1, 360.0, 0.1);
    GtkWidget* azimuthSeparator = gtk_label_new("至");
    
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(azimuthStartSpin), source.getAzimuthStart());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(azimuthEndSpin), source.getAzimuthEnd() > 0 ? source.getAzimuthEnd() : 360.0);
    
    gtk_box_pack_start(GTK_BOX(azimuthBox), azimuthStartSpin, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(azimuthBox), azimuthSeparator, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(azimuthBox), azimuthEndSpin, TRUE, TRUE, 0);
    
    gtk_grid_attach(GTK_GRID(grid), azimuthBox, 1, 5, 1, 1);
    
    // 俯仰角范围
    GtkWidget* elevationLabel = gtk_label_new("俯仰角范围(度):");
    gtk_grid_attach(GTK_GRID(grid), elevationLabel, 0, 6, 1, 1);
    
    GtkWidget* elevationBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget* elevationStartSpin = gtk_spin_button_new_with_range(-90.0, 89.0, 0.1);
    GtkWidget* elevationEndSpin = gtk_spin_button_new_with_range(-89.0, 90.0, 0.1);
    GtkWidget* elevationSeparator = gtk_label_new("至");
    
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(elevationStartSpin), source.getElevationStart());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(elevationEndSpin), source.getElevationEnd() > -90 ? source.getElevationEnd() : 90.0);
    
    gtk_box_pack_start(GTK_BOX(elevationBox), elevationStartSpin, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(elevationBox), elevationSeparator, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(elevationBox), elevationEndSpin, TRUE, TRUE, 0);
    
    gtk_grid_attach(GTK_GRID(grid), elevationBox, 1, 6, 1, 1);
    
    // 位置信息
    GtkWidget* positionLabel = gtk_label_new("位置(经度,纬度,高度):");
    gtk_grid_attach(GTK_GRID(grid), positionLabel, 0, 7, 1, 1);
    
    GtkWidget* positionBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget* longitudeSpin = gtk_spin_button_new_with_range(-180.0, 180.0, 0.000001);
    GtkWidget* latitudeSpin = gtk_spin_button_new_with_range(-90.0, 90.0, 0.000001);
    GtkWidget* altitudeSpin = gtk_spin_button_new_with_range(0.0, 10000.0, 0.1);
    
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(longitudeSpin), 6);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(latitudeSpin), 6);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(altitudeSpin), 1);
    
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(longitudeSpin), source.getLongitude());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(latitudeSpin), source.getLatitude());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(altitudeSpin), source.getAltitude());
    
    gtk_box_pack_start(GTK_BOX(positionBox), longitudeSpin, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(positionBox), latitudeSpin, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(positionBox), altitudeSpin, TRUE, TRUE, 0);
    
    gtk_grid_attach(GTK_GRID(grid), positionBox, 1, 7, 1, 1);
    
    // 运动参数（只在移动辐射源时可用）
    GtkWidget* movementFrame = gtk_frame_new("运动参数");
    gtk_grid_attach(GTK_GRID(grid), movementFrame, 0, 8, 2, 1);
    
    GtkWidget* movementGrid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(movementGrid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(movementGrid), 5);
    gtk_container_add(GTK_CONTAINER(movementFrame), movementGrid);
    
    // 运动速度
    GtkWidget* speedLabel = gtk_label_new("速度(米/秒):");
    gtk_grid_attach(GTK_GRID(movementGrid), speedLabel, 0, 0, 1, 1);
    
    GtkWidget* speedSpin = gtk_spin_button_new_with_range(0.0, 1000.0, 0.1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(speedSpin), source.getMovementSpeed());
    gtk_grid_attach(GTK_GRID(movementGrid), speedSpin, 1, 0, 1, 1);
    
    // 运动方位角
    GtkWidget* movAzimuthLabel = gtk_label_new("方位角(度):");
    gtk_grid_attach(GTK_GRID(movementGrid), movAzimuthLabel, 0, 1, 1, 1);
    
    GtkWidget* movAzimuthSpin = gtk_spin_button_new_with_range(0.0, 359.99, 0.01);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(movAzimuthSpin), source.getMovementAzimuth());
    gtk_grid_attach(GTK_GRID(movementGrid), movAzimuthSpin, 1, 1, 1, 1);
    
    // 运动俯仰角
    GtkWidget* movElevationLabel = gtk_label_new("俯仰角(度):");
    gtk_grid_attach(GTK_GRID(movementGrid), movElevationLabel, 0, 2, 1, 1);
    
    GtkWidget* movElevationSpin = gtk_spin_button_new_with_range(-90.0, 90.0, 0.01);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(movElevationSpin), source.getMovementElevation());
    gtk_grid_attach(GTK_GRID(movementGrid), movElevationSpin, 1, 2, 1, 1);
    
    // 保存对话框中的控件引用，以便在getSourceFromDialog中使用
    g_object_set_data(G_OBJECT(dialog), "nameEntry", nameEntry);
    g_object_set_data(G_OBJECT(dialog), "fixedRadio", fixedRadio);
    g_object_set_data(G_OBJECT(dialog), "powerSpin", powerSpin);
    g_object_set_data(G_OBJECT(dialog), "periodSpin", periodSpin);
    g_object_set_data(G_OBJECT(dialog), "freqSpin", freqSpin);
    g_object_set_data(G_OBJECT(dialog), "azimuthStartSpin", azimuthStartSpin);
    g_object_set_data(G_OBJECT(dialog), "azimuthEndSpin", azimuthEndSpin);
    g_object_set_data(G_OBJECT(dialog), "elevationStartSpin", elevationStartSpin);
    g_object_set_data(G_OBJECT(dialog), "elevationEndSpin", elevationEndSpin);
    g_object_set_data(G_OBJECT(dialog), "longitudeSpin", longitudeSpin);
    g_object_set_data(G_OBJECT(dialog), "latitudeSpin", latitudeSpin);
    g_object_set_data(G_OBJECT(dialog), "altitudeSpin", altitudeSpin);
    g_object_set_data(G_OBJECT(dialog), "speedSpin", speedSpin);
    g_object_set_data(G_OBJECT(dialog), "movAzimuthSpin", movAzimuthSpin);
    g_object_set_data(G_OBJECT(dialog), "movElevationSpin", movElevationSpin);
    
    // 保存原始辐射源ID
    g_object_set_data(G_OBJECT(dialog), "sourceId", GINT_TO_POINTER(source.getRadiationId()));
    
    // 根据固定/移动类型切换运动参数的可用性
    g_signal_connect(fixedRadio, "toggled", G_CALLBACK(on_fixed_radio_toggled), movementFrame);
    
    // 初始设置运动参数框架的可用性
    gtk_widget_set_sensitive(movementFrame, !source.getIsStationary());
    
    // 显示对话框
    gtk_widget_show_all(dialog);
    
    // 运行对话框并处理结果
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        RadiationSource newSource = getSourceFromDialog(dialog);
        
        if (source.getRadiationId() > 0) {
            // 编辑现有辐射源
            newSource.setRadiationId(source.getRadiationId());
            RadiationSourceModelController::getInstance().editSource(newSource);
        } else {
            // 添加新辐射源
            RadiationSourceModelController::getInstance().addSource(newSource);
        }
    }
    
    gtk_widget_destroy(dialog);
    return NULL;
}

// 从编辑对话框获取辐射源数据
RadiationSource RadiationSourceModelView::getSourceFromDialog(GtkWidget* dialog) const {
    RadiationSource source;
    
    // 获取对话框中保存的控件引用
    GtkWidget* nameEntry = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "nameEntry"));
    GtkWidget* fixedRadio = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "fixedRadio"));
    GtkWidget* powerSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "powerSpin"));
    GtkWidget* periodSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "periodSpin"));
    GtkWidget* freqSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "freqSpin"));
    GtkWidget* azimuthStartSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "azimuthStartSpin"));
    GtkWidget* azimuthEndSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "azimuthEndSpin"));
    GtkWidget* elevationStartSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "elevationStartSpin"));
    GtkWidget* elevationEndSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "elevationEndSpin"));
    GtkWidget* longitudeSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "longitudeSpin"));
    GtkWidget* latitudeSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "latitudeSpin"));
    GtkWidget* altitudeSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "altitudeSpin"));
    GtkWidget* speedSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "speedSpin"));
    GtkWidget* movAzimuthSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "movAzimuthSpin"));
    GtkWidget* movElevationSpin = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "movElevationSpin"));
    
    // 设置辐射源ID（如果是编辑现有辐射源）
    int sourceId = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(dialog), "sourceId"));
    if (sourceId > 0) {
        source.setRadiationId(sourceId);
    }
    
    // 设置辐射源属性
    source.setRadiationName(gtk_entry_get_text(GTK_ENTRY(nameEntry)));
    source.setIsStationary(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fixedRadio)));
    source.setTransmitPower(gtk_spin_button_get_value(GTK_SPIN_BUTTON(powerSpin)));
    source.setScanPeriod(gtk_spin_button_get_value(GTK_SPIN_BUTTON(periodSpin)));
    source.setCarrierFrequency(gtk_spin_button_get_value(GTK_SPIN_BUTTON(freqSpin)));
    source.setAzimuthStart(gtk_spin_button_get_value(GTK_SPIN_BUTTON(azimuthStartSpin)));
    source.setAzimuthEnd(gtk_spin_button_get_value(GTK_SPIN_BUTTON(azimuthEndSpin)));
    source.setElevationStart(gtk_spin_button_get_value(GTK_SPIN_BUTTON(elevationStartSpin)));
    source.setElevationEnd(gtk_spin_button_get_value(GTK_SPIN_BUTTON(elevationEndSpin)));
    source.setLongitude(gtk_spin_button_get_value(GTK_SPIN_BUTTON(longitudeSpin)));
    source.setLatitude(gtk_spin_button_get_value(GTK_SPIN_BUTTON(latitudeSpin)));
    source.setAltitude(gtk_spin_button_get_value(GTK_SPIN_BUTTON(altitudeSpin)));
    source.setMovementSpeed(gtk_spin_button_get_value(GTK_SPIN_BUTTON(speedSpin)));
    source.setMovementAzimuth(gtk_spin_button_get_value(GTK_SPIN_BUTTON(movAzimuthSpin)));
    source.setMovementElevation(gtk_spin_button_get_value(GTK_SPIN_BUTTON(movElevationSpin)));
    
    return source;
}

// 更新辐射源列表
void RadiationSourceModelView::updateSourceList(const std::vector<RadiationSource>& sources) {
    m_sources = sources;
    
    std::cout << "RadiationSourceModelView: 更新辐射源列表，共 " << sources.size() << " 条数据" << std::endl;
    
    // 创建列表存储
    GtkListStore* store = gtk_list_store_new(6, 
        G_TYPE_STRING,  // ID
        G_TYPE_STRING,  // 名称
        G_TYPE_STRING,  // 类型
        G_TYPE_STRING,  // 发射功率
        G_TYPE_STRING,  // 频率
        G_TYPE_STRING   // 方位角范围
    );
    
    // 添加数据
    for (const auto& source : sources) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        
        char idStr[16], powerStr[32], freqStr[64], azimuthStr[64];
        snprintf(idStr, sizeof(idStr), "%d", source.getRadiationId());
        snprintf(powerStr, sizeof(powerStr), "%.1f", source.getTransmitPower());
        snprintf(freqStr, sizeof(freqStr), "%.1f", source.getCarrierFrequency());
        snprintf(azimuthStr, sizeof(azimuthStr), "%.1f-%.1f", source.getAzimuthStart(), source.getAzimuthEnd());
        
        std::string typeStr = source.getIsStationary() ? "固定" : "移动";
        
        gtk_list_store_set(store, &iter,
            0, idStr,
            1, source.getRadiationName().c_str(),
            2, typeStr.c_str(),
            3, powerStr,
            4, freqStr,
            5, azimuthStr,
            -1);
    }
    
    // 设置模型
    gtk_tree_view_set_model(GTK_TREE_VIEW(m_sourceList), GTK_TREE_MODEL(store));
    g_object_unref(store);
}

// 获取当前选中的辐射源ID
int RadiationSourceModelView::getSelectedSourceId() const {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_sourceList));
    GtkTreeModel* model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar* name;
        gtk_tree_model_get(model, &iter, 0, &name, -1);
        
        // 通过名称查找ID
        for (size_t i = 0; i < m_sources.size(); i++) {
            if (m_sources[i].getRadiationName() == name) {
                g_free(name);
                return m_sources[i].getRadiationId();
            }
        }
        
        g_free(name);
    }
    
    return -1;
}

// 获取视图控件
GtkWidget* RadiationSourceModelView::getView() const {
    return m_view;
}