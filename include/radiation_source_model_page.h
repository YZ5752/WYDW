#pragma once

#include <gtk/gtk.h>
#include "radiation_source_model.h"

class RadiationSourceModelPage {
public:
    // 创建辐射源模型页面
    static GtkWidget* create();
    
    // 回调函数
    static void onAddSource(GtkWidget* widget, gpointer data);
    static void onEditSource(GtkWidget* widget, gpointer data);
    static void onDeleteSource(GtkWidget* widget, gpointer data);
    
    // 更新辐射源列表
    static void updateSourceList(GtkWidget* list);
};