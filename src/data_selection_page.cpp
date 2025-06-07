#include "../include/data_selection_page.h"
#include "../include/ui_manager.h"
#include <gtk/gtk.h>

GtkWidget* createDataSelectionUI() {
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(container), 15);
    GtkWidget* controlBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(container), controlBox, FALSE, FALSE, 0);
    GtkWidget* targetFrame = gtk_frame_new("辐射源模型");
    gtk_box_pack_start(GTK_BOX(controlBox), targetFrame, FALSE, FALSE, 0);
    GtkWidget* targetBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(targetFrame), targetBox);
    gtk_container_set_border_width(GTK_CONTAINER(targetBox), 10);
    GtkWidget* targetLabel = gtk_label_new("选择辐射源模型:");
    gtk_box_pack_start(GTK_BOX(targetBox), targetLabel, FALSE, FALSE, 0);
    GtkWidget* targetCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(targetCombo), "辐射源1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(targetCombo), "辐射源2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(targetCombo), 0);
    gtk_widget_set_size_request(targetCombo, 200, -1);
    gtk_box_pack_start(GTK_BOX(targetBox), targetCombo, FALSE, FALSE, 0);
    // 右侧：按钮区域
    GtkWidget* buttonBox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonBox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing(GTK_BOX(buttonBox), 10);
    gtk_box_pack_start(GTK_BOX(controlBox), buttonBox, TRUE, TRUE, 0);
    GtkWidget* deleteButton = gtk_button_new_with_label("删除");
    gtk_container_add(GTK_CONTAINER(buttonBox), deleteButton);
    GtkWidget* highlightButton = gtk_button_new_with_label("高亮显示");
    gtk_container_add(GTK_CONTAINER(buttonBox), highlightButton);
    GtkWidget* importButton = gtk_button_new_with_label("录入");
    gtk_container_add(GTK_CONTAINER(buttonBox), importButton);
    // 数据列表区域
    GtkWidget* dataFrame = gtk_frame_new("目标数据列表");
    gtk_box_pack_start(GTK_BOX(container), dataFrame, TRUE, TRUE, 0);
    GtkWidget* dataBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(dataFrame), dataBox);
    gtk_container_set_border_width(GTK_CONTAINER(dataBox), 10);
    GtkListStore* store = gtk_list_store_new(4, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget* treeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);
    GtkCellRenderer* toggleRenderer = gtk_cell_renderer_toggle_new();
    GtkTreeViewColumn* toggleColumn = gtk_tree_view_column_new_with_attributes("选择", toggleRenderer, "active", 0, NULL);
    gtk_tree_view_column_set_min_width(toggleColumn, 60);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), toggleColumn);
    GtkCellRenderer* textRenderer1 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column1 = gtk_tree_view_column_new_with_attributes("侦察设备", textRenderer1, "text", 1, NULL);
    gtk_tree_view_column_set_min_width(column1, 120);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column1);
    GtkCellRenderer* textRenderer2 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column2 = gtk_tree_view_column_new_with_attributes("测向数据 (方位角)", textRenderer2, "text", 2, NULL);
    gtk_tree_view_column_set_min_width(column2, 150);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column2);
    GtkCellRenderer* textRenderer3 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column3 = gtk_tree_view_column_new_with_attributes("定位数据 (经纬度)", textRenderer3, "text", 3, NULL);
    gtk_tree_view_column_set_min_width(column3, 200);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column3);
    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, FALSE, 1, "侦察设备1", 2, "方位角: 45°", 3, "坐标: (116.5, 39.9)", -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, FALSE, 1, "侦察设备2", 2, "方位角: 130°", 3, "坐标: (116.4, 39.8)", -1);
    GtkWidget* scrollWin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrollWin), treeView);
    gtk_box_pack_start(GTK_BOX(dataBox), scrollWin, TRUE, TRUE, 0);
    return container;
} 