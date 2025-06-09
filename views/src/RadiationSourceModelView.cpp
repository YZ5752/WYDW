#include "../RadiationSourceModelView.h"
#include "../../controllers/ApplicationController.h"
#include <iostream>

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
            "名称", 
            "类型", 
            "发射功率(dBm)", 
            "频率范围(MHz)", 
            "方位角范围(°)"
        };
        
        // 创建列表存储模型
        GtkListStore* store = gtk_list_store_new(5, 
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
        
        // 更新列表数据
        g_print("  Updating radiation source list...\n");
        std::vector<RadiationSource> initialSources;
        
        // 添加临时测试数据
        RadiationSource tempSource;
        tempSource.setRadiationId(0);
        tempSource.setRadiationName("加载中...");
        tempSource.setIsStationary(true);
        tempSource.setTransmitPower(0);
        tempSource.setCarrierFrequency(0);
        tempSource.setAzimuthStart(0);
        tempSource.setAzimuthEnd(0);
        initialSources.push_back(tempSource);
        
        // 初始化显示
        updateSourceList(initialSources);
        
        // 调用控制器更新列表
        ApplicationController::updateRadiationSourceList(m_sourceList);
        
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
    // 实现创建编辑对话框的逻辑
    return nullptr; // 暂未实现
}

// 更新辐射源列表
void RadiationSourceModelView::updateSourceList(const std::vector<RadiationSource>& sources) {
    m_sources = sources;
    
    // 创建列表存储
    GtkListStore* store = gtk_list_store_new(5, 
        G_TYPE_STRING,  // 名称
        G_TYPE_STRING,  // 类型
        G_TYPE_STRING,  // 发射功率
        G_TYPE_STRING,  // 频率范围
        G_TYPE_STRING   // 方位角范围
    );
    
    // 添加数据
    for (const auto& source : sources) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        
        char powerStr[32], freqStr[64], azimuthStr[64];
        snprintf(powerStr, sizeof(powerStr), "%.1f", source.getTransmitPower());
        snprintf(freqStr, sizeof(freqStr), "%.1f", source.getCarrierFrequency());
        snprintf(azimuthStr, sizeof(azimuthStr), "%.1f-%.1f", source.getAzimuthStart(), source.getAzimuthEnd());
        
        std::string typeStr = source.getIsStationary() ? "固定" : "移动";
        
        gtk_list_store_set(store, &iter,
            0, source.getRadiationName().c_str(),
            1, typeStr.c_str(),
            2, powerStr,
            3, freqStr,
            4, azimuthStr,
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

// 从编辑对话框获取辐射源数据
RadiationSource RadiationSourceModelView::getSourceFromDialog(GtkWidget* dialog) const {
    // 实现从对话框获取数据的逻辑
    return RadiationSource(); // 暂未实现
}

// 获取视图控件
GtkWidget* RadiationSourceModelView::getView() const {
    return m_view;
}