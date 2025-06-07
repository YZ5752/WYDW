#include "../include/evaluation_page.h"
#include "../include/ui_manager.h"
#include <gtk/gtk.h>

GtkWidget* createEvaluationUI() {
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(container), 15);
    GtkWidget* titleLabel = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(titleLabel), "<span font='16' weight='bold'>协同定位评估</span>");
    gtk_box_pack_start(GTK_BOX(container), titleLabel, FALSE, FALSE, 5);
    GtkWidget* paramFrame = gtk_frame_new("评估参数");
    gtk_box_pack_start(GTK_BOX(container), paramFrame, FALSE, FALSE, 0);
    GtkWidget* paramBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_container_add(GTK_CONTAINER(paramFrame), paramBox);
    gtk_container_set_border_width(GTK_CONTAINER(paramBox), 10);
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
    GtkWidget* typeBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(paramBox), typeBox, TRUE, TRUE, 0);
    GtkWidget* typeLabel = gtk_label_new("评估类型");
    gtk_widget_set_halign(typeLabel, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(typeBox), typeLabel, FALSE, FALSE, 0);
    GtkWidget* singleRadio = gtk_radio_button_new_with_label(NULL, "单平台");
    gtk_box_pack_start(GTK_BOX(typeBox), singleRadio, FALSE, FALSE, 0);
    GtkWidget* multiRadio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(singleRadio), "多平台");
    gtk_box_pack_start(GTK_BOX(typeBox), multiRadio, FALSE, FALSE, 0);
    GtkWidget* startButton = gtk_button_new_with_label("开始评估");
    gtk_widget_set_size_request(startButton, 100, 35);
    gtk_box_pack_start(GTK_BOX(paramBox), startButton, FALSE, FALSE, 0);
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
    GtkWidget* resultsBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(container), resultsBox, TRUE, TRUE, 0);
    GtkWidget* tableFrame = gtk_frame_new("评估结果");
    gtk_widget_set_size_request(tableFrame, 400, 500);
    gtk_box_pack_start(GTK_BOX(resultsBox), tableFrame, FALSE, FALSE, 0);
    GtkWidget* tableBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(tableFrame), tableBox);
    gtk_container_set_border_width(GTK_CONTAINER(tableBox), 10);
    GtkWidget* table = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(table), 10);
    gtk_grid_set_column_spacing(GTK_GRID(table), 15);
    gtk_box_pack_start(GTK_BOX(tableBox), table, TRUE, TRUE, 0);
    GtkWidget* headerLabel1 = gtk_label_new("指标");
    gtk_widget_set_halign(headerLabel1, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), headerLabel1, 0, 0, 1, 1);
    GtkWidget* headerLabel2 = gtk_label_new("数值");
    gtk_widget_set_halign(headerLabel2, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), headerLabel2, 1, 0, 1, 1);
    const char* metrics[] = {"最远定位距离", "定位时间", "定位精度", "测向精度"};
    for (int i = 0; i < 4; i++) {
        GtkWidget* metricLabel = gtk_label_new(metrics[i]);
        gtk_widget_set_halign(metricLabel, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(table), metricLabel, 0, i+1, 1, 1);
        GtkWidget* valueLabel = gtk_label_new("--");
        gtk_widget_set_halign(valueLabel, GTK_ALIGN_END);
        gtk_grid_attach(GTK_GRID(table), valueLabel, 1, i+1, 1, 1);
    }
    GtkWidget* exportButton = gtk_button_new_with_label("导出结果");
    gtk_box_pack_start(GTK_BOX(tableBox), exportButton, FALSE, FALSE, 5);
    GtkWidget* chartFrame = gtk_frame_new("定位精度随时间变化");
    gtk_box_pack_start(GTK_BOX(resultsBox), chartFrame, TRUE, TRUE, 0);
    GtkWidget* drawingArea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(chartFrame), drawingArea);
    return container;
} 