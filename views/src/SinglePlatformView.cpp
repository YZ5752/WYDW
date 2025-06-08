#include "../SinglePlatformView.h"
#include "../components/MapView.h"
#include <gtk/gtk.h>
#include <string>

SinglePlatformView::SinglePlatformView() :
    m_view(nullptr),
    m_algoCombo(nullptr),
    m_radarCombo(nullptr),
    m_sourceCombo(nullptr),
    m_timeEntry(nullptr),
    m_dirDataValue(nullptr),
    m_locDataValue(nullptr),
    m_errorTable(nullptr) {
}

SinglePlatformView::~SinglePlatformView() {
}

GtkWidget* SinglePlatformView::createView() {
    // 创建主容器
    m_view = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(m_view), 15);
    
    // 左侧：地图显示区域
    GtkWidget* mapFrame = gtk_frame_new("地图");
    gtk_widget_set_size_request(mapFrame, 800, 700);
    gtk_box_pack_start(GTK_BOX(m_view), mapFrame, TRUE, TRUE, 0);
    
    // 显示地图
    MapView mapView;
    GtkWidget* mapWidget = mapView.create();
    gtk_container_add(GTK_CONTAINER(mapFrame), mapWidget);
    
    // 右侧：参数设置和结果区域
    GtkWidget* rightBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(m_view), rightBox, FALSE, FALSE, 0);
    
    // 技术体制选择
    GtkWidget* algoFrame = gtk_frame_new("技术体制");
    gtk_box_pack_start(GTK_BOX(rightBox), algoFrame, FALSE, FALSE, 0);
    GtkWidget* algoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(algoFrame), algoBox);
    gtk_container_set_border_width(GTK_CONTAINER(algoBox), 10);
    
    m_algoCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_algoCombo), "干涉仪体制");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_algoCombo), "时差体制");
    gtk_combo_box_set_active(GTK_COMBO_BOX(m_algoCombo), 0);
    gtk_box_pack_start(GTK_BOX(algoBox), m_algoCombo, TRUE, TRUE, 5);
    
    // 雷达设备模型选择
    GtkWidget* radarFrame = gtk_frame_new("侦察设备模型");
    gtk_box_pack_start(GTK_BOX(rightBox), radarFrame, FALSE, FALSE, 0);
    GtkWidget* radarBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(radarFrame), radarBox);
    gtk_container_set_border_width(GTK_CONTAINER(radarBox), 10);
    
    m_radarCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_radarCombo), "侦察设备1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_radarCombo), "侦察设备2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(m_radarCombo), 0);
    gtk_box_pack_start(GTK_BOX(radarBox), m_radarCombo, TRUE, TRUE, 5);
    
    // 辐射源模型选择
    GtkWidget* sourceFrame = gtk_frame_new("辐射源模型");
    gtk_box_pack_start(GTK_BOX(rightBox), sourceFrame, FALSE, FALSE, 0);
    GtkWidget* sourceBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(sourceFrame), sourceBox);
    gtk_container_set_border_width(GTK_CONTAINER(sourceBox), 10);
    
    m_sourceCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_sourceCombo), "辐射源1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_sourceCombo), "辐射源2");
    gtk_combo_box_set_active(GTK_COMBO_BOX(m_sourceCombo), 0);
    gtk_box_pack_start(GTK_BOX(sourceBox), m_sourceCombo, TRUE, TRUE, 5);
    
    // 仿真执行时间输入框
    GtkWidget* timeFrame = gtk_frame_new("仿真参数");
    gtk_box_pack_start(GTK_BOX(rightBox), timeFrame, FALSE, FALSE, 0);
    GtkWidget* timeBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(timeFrame), timeBox);
    gtk_container_set_border_width(GTK_CONTAINER(timeBox), 6);
    
    GtkWidget* timeLabel = gtk_label_new("仿真执行时间(s):");
    gtk_box_pack_start(GTK_BOX(timeBox), timeLabel, FALSE, FALSE, 5);
    
    m_timeEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(m_timeEntry), "10");
    gtk_box_pack_start(GTK_BOX(timeBox), m_timeEntry, TRUE, TRUE, 5);
    
    // 开始按钮
    GtkWidget* startButton = gtk_button_new_with_label("开始");
    gtk_widget_set_size_request(startButton, -1, 40);
    gtk_box_pack_start(GTK_BOX(rightBox), startButton, FALSE, FALSE, 10);
    
    // 结果区域
    GtkWidget* resultFrame = gtk_frame_new("仿真结果");
    gtk_box_pack_start(GTK_BOX(rightBox), resultFrame, TRUE, TRUE, 0);
    GtkWidget* resultBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(resultFrame), resultBox);
    gtk_container_set_border_width(GTK_CONTAINER(resultBox), 10);
    
    GtkWidget* table = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(table), 5);
    gtk_grid_set_column_spacing(GTK_GRID(table), 10);
    gtk_box_pack_start(GTK_BOX(resultBox), table, TRUE, TRUE, 0);
    
    GtkWidget* dirDataLabel = gtk_label_new("测向数据");
    gtk_widget_set_halign(dirDataLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), dirDataLabel, 0, 1, 1, 1);
    
    GtkWidget* locDataLabel = gtk_label_new("定位数据");
    gtk_widget_set_halign(locDataLabel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(table), locDataLabel, 0, 2, 1, 1);
    
    m_dirDataValue = gtk_label_new("--");
    gtk_widget_set_halign(m_dirDataValue, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), m_dirDataValue, 1, 1, 1, 1);
    
    m_locDataValue = gtk_label_new("--");
    gtk_widget_set_halign(m_locDataValue, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(table), m_locDataValue, 1, 2, 1, 1);
    
    // 误差结果区域
    GtkWidget* errorFrame = gtk_frame_new("误差分析");
    gtk_box_pack_start(GTK_BOX(rightBox), errorFrame, TRUE, TRUE, 0);
    GtkWidget* errorBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(errorFrame), errorBox);
    gtk_container_set_border_width(GTK_CONTAINER(errorBox), 10);
    
    m_errorTable = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(m_errorTable), 5);
    gtk_grid_set_column_spacing(GTK_GRID(m_errorTable), 10);
    gtk_box_pack_start(GTK_BOX(errorBox), m_errorTable, TRUE, TRUE, 0);
    
    // 初始化误差表格
    updateErrorTable("干涉仪体制");
    
    return m_view;
}

void SinglePlatformView::updateDirectionData(const std::string& data) {
    gtk_label_set_text(GTK_LABEL(m_dirDataValue), data.c_str());
}

void SinglePlatformView::updateLocationData(const std::string& data) {
    gtk_label_set_text(GTK_LABEL(m_locDataValue), data.c_str());
}

void SinglePlatformView::updateErrorTable(const std::string& techSystem) {
    // 清空表格
    GList* children = gtk_container_get_children(GTK_CONTAINER(m_errorTable));
    for (GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    // 添加表头
    GtkWidget* headerLabel1 = gtk_label_new("误差类型");
    gtk_widget_set_halign(headerLabel1, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(m_errorTable), headerLabel1, 0, 0, 1, 1);
    
    GtkWidget* headerLabel2 = gtk_label_new("误差值");
    gtk_widget_set_halign(headerLabel2, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(m_errorTable), headerLabel2, 1, 0, 1, 1);
    
    // 根据技术体制添加不同的误差项
    if (techSystem == "干涉仪体制") {
        // 干涉仪体制的误差项
        const char* errorTypes[] = {"相位误差", "方位误差", "定位误差"};
        const char* errorValues[] = {"0.1°", "1.5°", "500m"};
        
        for (int i = 0; i < 3; ++i) {
            GtkWidget* typeLabel = gtk_label_new(errorTypes[i]);
            gtk_widget_set_halign(typeLabel, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(m_errorTable), typeLabel, 0, i+1, 1, 1);
            
            GtkWidget* valueLabel = gtk_label_new(errorValues[i]);
            gtk_widget_set_halign(valueLabel, GTK_ALIGN_END);
            gtk_grid_attach(GTK_GRID(m_errorTable), valueLabel, 1, i+1, 1, 1);
        }
    } else if (techSystem == "时差体制") {
        // 时差体制的误差项
        const char* errorTypes[] = {"时间误差", "方位误差", "定位误差"};
        const char* errorValues[] = {"10ns", "2.0°", "800m"};
        
        for (int i = 0; i < 3; ++i) {
            GtkWidget* typeLabel = gtk_label_new(errorTypes[i]);
            gtk_widget_set_halign(typeLabel, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(m_errorTable), typeLabel, 0, i+1, 1, 1);
            
            GtkWidget* valueLabel = gtk_label_new(errorValues[i]);
            gtk_widget_set_halign(valueLabel, GTK_ALIGN_END);
            gtk_grid_attach(GTK_GRID(m_errorTable), valueLabel, 1, i+1, 1, 1);
        }
    }
    
    // 显示所有控件
    gtk_widget_show_all(m_errorTable);
}

std::string SinglePlatformView::getSelectedTechSystem() const {
    gchar* text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_algoCombo));
    std::string result(text ? text : "");
    g_free(text);
    return result;
}

std::string SinglePlatformView::getSelectedDevice() const {
    gchar* text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_radarCombo));
    std::string result(text ? text : "");
    g_free(text);
    return result;
}

std::string SinglePlatformView::getSelectedSource() const {
    gchar* text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_sourceCombo));
    std::string result(text ? text : "");
    g_free(text);
    return result;
}

int SinglePlatformView::getSimulationTime() const {
    const gchar* text = gtk_entry_get_text(GTK_ENTRY(m_timeEntry));
    return text ? atoi(text) : 0;
}

GtkWidget* SinglePlatformView::getView() const {
    return m_view;
}

GtkWidget* SinglePlatformView::getErrorTable() const {
    return m_errorTable;
} 