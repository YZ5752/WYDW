#include "../include/ui_manager.h"
#include <iostream>

// 静态方法实现

// 更新雷达设备列表
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
                      1, "雷达（固定）", 
                      2, "时差", 
                      3, "3.5", 
                      4, "1000-2000", 
                      5, "编辑", 
                      6, "删除", 
                      -1);
    
    // 示例数据2
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 
                      0, "雷达设备2", 
                      1, "侦察机（移动）", 
                      2, "频差", 
                      3, "2.8", 
                      4, "800-1500", 
                      5, "编辑", 
                      6, "删除", 
                      -1);
}

// 更新辐射源列表
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
                      1, "雷达站（固定）", 
                      2, "100", 
                      3, "5", 
                      4, "1000-2000", 
                      5, "90", 
                      6, "编辑", 
                      7, "删除", 
                      -1);
    
    // 示例数据2
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 
                      0, "辐射源2", 
                      1, "飞机（移动）", 
                      2, "200", 
                      3, "10", 
                      4, "800-1500", 
                      5, "180", 
                      6, "编辑", 
                      7, "删除", 
                      -1);
} 