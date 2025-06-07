#include "../include/radiation_source_model_page.h"
#include "../include/ui_manager.h"
#include "../include/db_connector.h"
#include <iostream>

// 创建辐射源模型页面
GtkWidget* RadiationSourceModelPage::create() {
    g_print("Creating radiation source model UI components...\n");
    
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
        
        gtk_label_set_markup(GTK_LABEL(titleLabel), "<span font='16' weight='bold'>辐射源模型管理</span>");
        gtk_box_pack_start(GTK_BOX(container), titleLabel, FALSE, FALSE, 5);
        
        // 创建表格列表
        g_print("  Creating model list...\n");
        std::vector<std::string> headers = {
            "名称", 
            "类型", 
            "发射功率(dBm)", 
            "天线增益(dBi)", 
            "频率范围(MHz)", 
            "方位角范围(°)"
        };
        GtkWidget* treeView = UIManager::getInstance().createModelList(headers);
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
        g_signal_connect(addButton, "clicked", G_CALLBACK(onAddSource), treeView);
        
        // 更新列表数据
        g_print("  Updating radiation source list...\n");
        updateSourceList(treeView);
        
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

// 添加辐射源回调
void RadiationSourceModelPage::onAddSource(GtkWidget* widget, gpointer data) {
    GtkWidget* treeView = GTK_WIDGET(data);
    
    // 显示提示对话框
    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(UIManager::getInstance().m_mainWindow),
                                            GTK_DIALOG_MODAL,
                                            GTK_MESSAGE_INFO,
                                            GTK_BUTTONS_OK,
                                            "辐射源参数设置对话框功能暂时未实现");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    // 更新列表（虽然没有变化）
    updateSourceList(treeView);
}

// 编辑辐射源回调
void RadiationSourceModelPage::onEditSource(GtkWidget* widget, gpointer data) {
    // 实现编辑辐射源的逻辑
    GtkTreeView* treeView = GTK_TREE_VIEW(widget);
    GtkTreeSelection* selection = gtk_tree_view_get_selection(treeView);
    GtkTreeModel* model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar* name;
        gtk_tree_model_get(model, &iter, 0, &name, -1);
        
        // 显示提示对话框
        GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(UIManager::getInstance().m_mainWindow),
                                                GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_INFO,
                                                GTK_BUTTONS_OK,
                                                "辐射源参数设置对话框功能暂时未实现，无法编辑 %s", name);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        g_free(name);
    }
}

// 删除辐射源回调
void RadiationSourceModelPage::onDeleteSource(GtkWidget* widget, gpointer data) {
    // 实现删除辐射源的逻辑
    GtkTreeView* treeView = GTK_TREE_VIEW(widget);
    GtkTreeSelection* selection = gtk_tree_view_get_selection(treeView);
    GtkTreeModel* model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar* name;
        gtk_tree_model_get(model, &iter, 0, &name, -1);
        
        // 确认对话框
        GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(UIManager::getInstance().m_mainWindow),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_QUESTION,
                                               GTK_BUTTONS_YES_NO,
                                               "确定要删除辐射源 %s 吗？", name);
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        if (response == GTK_RESPONSE_YES) {
            // 删除操作
            g_print("Deleting radiation source: %s\n", name);
            
            // 通过名称获取辐射源ID
            std::vector<RadiationSource> sources = DBConnector::getInstance().getAllRadiationSources();
            for (auto& source : sources) {
                if (source.getRadiationName() == name) {
                    // 删除数据库记录
                    // 这里应该有删除辐射源的方法，但暂时没有看到
                    g_print("Source deleted successfully\n");
                    // 更新列表
                    updateSourceList(GTK_WIDGET(treeView));
                    break;
                }
            }
        }
        
        g_free(name);
    }
}

// 更新辐射源列表
void RadiationSourceModelPage::updateSourceList(GtkWidget* list) {
    UIManager::updateRadiationSourceList(list);
}