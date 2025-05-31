#include "../include/ui_manager.h"
#include <iostream>

// 绘制地图的回调函数
static gboolean drawMapCallback(GtkWidget* widget, cairo_t* cr, gpointer data) {
    // 绘制蓝色背景
    cairo_set_source_rgb(cr, 0.2, 0.4, 0.8);
    cairo_rectangle(cr, 0, 0, 600, 600);
    cairo_fill(cr);
    
    // 绘制"地图"文字
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 24);
    
    cairo_text_extents_t extents;
    const char* text = "地图";
    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr, 300 - extents.width/2, 300 + extents.height/2);
    cairo_show_text(cr, text);
    
    return FALSE;
}

// 单例实现
UIManager& UIManager::getInstance() {
    static UIManager instance;
    return instance;
}

UIManager::UIManager()
    : m_mainWindow(nullptr),
      m_singlePlatformPage(nullptr),
      m_multiPlatformPage(nullptr),
      m_dataDisplayPage(nullptr),
      m_evaluationPage(nullptr) {
}

UIManager::~UIManager() {
    if (m_mainWindow) {
        gtk_widget_destroy(m_mainWindow);
    }
}

bool UIManager::initUI(int argc, char** argv) {
    gtk_init(&argc, &argv);
    
    // 创建主窗口
    m_mainWindow = createMainWindow();
    if (!m_mainWindow) {
        return false;
    }
    
    // 创建各页面
    m_singlePlatformPage = createSinglePlatformUI();
    m_multiPlatformPage = createMultiPlatformUI();
    m_dataDisplayPage = createDataDisplayUI();
    m_evaluationPage = createEvaluationUI();
    
    // 创建一个笔记本控件用于页面切换
    GtkWidget* notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(m_mainWindow), notebook);
    
    // 添加页面到笔记本
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), m_singlePlatformPage, 
                             gtk_label_new("单平台雷达侦察"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), m_multiPlatformPage, 
                             gtk_label_new("多平台协同侦察"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), m_dataDisplayPage, 
                             gtk_label_new("数据显示与分选"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), m_evaluationPage, 
                             gtk_label_new("协同定位评估"));
    
    // 显示所有控件
    gtk_widget_show_all(m_mainWindow);
    
    return true;
}

void UIManager::run() {
    gtk_main();
}

GtkWidget* UIManager::createMainWindow() {
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    gtk_window_set_title(GTK_WINDOW(window), "无源侦察协同定位仿真系统");
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    return window;
}

GtkWidget* UIManager::createSinglePlatformUI() {
    // 创建页面的主容器
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    // 左侧：地图显示区域
    GtkWidget* mapFrame = gtk_frame_new("单平台无源定位仿真");
    gtk_widget_set_size_request(mapFrame, 600, 700);
    gtk_box_pack_start(GTK_BOX(container), mapFrame, TRUE, TRUE, 0);
    
    // 显示地图
    showMap(mapFrame);
    
    // 右侧：参数设置区域
    GtkWidget* paramBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(container), paramBox, FALSE, FALSE, 0);
    
    // 雷达侦察设备模型参数设置
    GtkWidget* deviceFrame = gtk_frame_new("单平台雷达侦察设备模型参数设置");
    gtk_box_pack_start(GTK_BOX(paramBox), deviceFrame, FALSE, FALSE, 0);
    
    GtkWidget* deviceBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(deviceFrame), deviceBox);
    
    // 技术体制选择
    GtkWidget* techBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(deviceBox), techBox, FALSE, FALSE, 5);
    
    GtkWidget* techLabel = gtk_label_new("技术体制选择");
    gtk_box_pack_start(GTK_BOX(techBox), techLabel, FALSE, FALSE, 5);
    
    GtkWidget* techCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(techCombo), "干涉仪体制");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(techCombo), "时差体制");
    gtk_combo_box_set_active(GTK_COMBO_BOX(techCombo), 0);
    gtk_box_pack_start(GTK_BOX(techBox), techCombo, TRUE, TRUE, 5);
    
    // 算法选择
    GtkWidget* algoBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(deviceBox), algoBox, FALSE, FALSE, 5);
    
    GtkWidget* algoLabel = gtk_label_new("算法选择");
    gtk_box_pack_start(GTK_BOX(algoBox), algoLabel, FALSE, FALSE, 5);
    
    GtkWidget* algoCombo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algoCombo), "快速定位");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algoCombo), "基线定位");
    gtk_combo_box_set_active(GTK_COMBO_BOX(algoCombo), 0);
    gtk_box_pack_start(GTK_BOX(algoBox), algoCombo, TRUE, TRUE, 5);
    
    // 辐射源模型参数设置
    GtkWidget* sourceFrame = gtk_frame_new("辐射源模型参数设置");
    gtk_box_pack_start(GTK_BOX(paramBox), sourceFrame, FALSE, FALSE, 0);
    
    GtkWidget* sourceBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(sourceFrame), sourceBox);
    
    // 发射功率
    GtkWidget* powerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(sourceBox), powerBox, FALSE, FALSE, 5);
    
    GtkWidget* powerLabel = gtk_label_new("发射功率");
    gtk_box_pack_start(GTK_BOX(powerBox), powerLabel, FALSE, FALSE, 5);
    
    GtkWidget* powerEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(powerBox), powerEntry, TRUE, TRUE, 5);
    
    // 扫描周期
    GtkWidget* periodBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(sourceBox), periodBox, FALSE, FALSE, 5);
    
    GtkWidget* periodLabel = gtk_label_new("扫描周期");
    gtk_box_pack_start(GTK_BOX(periodBox), periodLabel, FALSE, FALSE, 5);
    
    GtkWidget* periodEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(periodBox), periodEntry, TRUE, TRUE, 5);
    
    // 频率范围
    GtkWidget* freqBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(sourceBox), freqBox, FALSE, FALSE, 5);
    
    GtkWidget* freqLabel = gtk_label_new("频率范围");
    gtk_box_pack_start(GTK_BOX(freqBox), freqLabel, FALSE, FALSE, 5);
    
    GtkWidget* freqEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(freqBox), freqEntry, TRUE, TRUE, 5);
    
    // 工作扇区
    GtkWidget* sectorBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(sourceBox), sectorBox, FALSE, FALSE, 5);
    
    GtkWidget* sectorLabel = gtk_label_new("工作扇区");
    gtk_box_pack_start(GTK_BOX(sectorBox), sectorLabel, FALSE, FALSE, 5);
    
    GtkWidget* sectorEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(sectorBox), sectorEntry, TRUE, TRUE, 5);
    
    // 开始仿真按钮
    GtkWidget* startButton = gtk_button_new_with_label("按钮：开始测向");
    gtk_box_pack_start(GTK_BOX(paramBox), startButton, FALSE, FALSE, 10);
    g_signal_connect(startButton, "clicked", G_CALLBACK(onSinglePlatformSimulation), NULL);
    
    // 仿真结果区域
    GtkWidget* resultFrame = gtk_frame_new("单平台雷达侦察设备模型测向结果");
    gtk_box_pack_start(GTK_BOX(paramBox), resultFrame, FALSE, FALSE, 0);
    
    GtkWidget* resultBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(resultFrame), resultBox);
    
    // 威力
    GtkWidget* powerResBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(resultBox), powerResBox, FALSE, FALSE, 5);
    
    GtkWidget* powerResLabel = gtk_label_new("威力");
    gtk_box_pack_start(GTK_BOX(powerResBox), powerResLabel, FALSE, FALSE, 5);
    
    GtkWidget* powerResEntry = gtk_entry_new();
    gtk_widget_set_sensitive(powerResEntry, FALSE);
    gtk_box_pack_start(GTK_BOX(powerResBox), powerResEntry, TRUE, TRUE, 5);
    
    // 测向误差
    GtkWidget* dirErrorBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(resultBox), dirErrorBox, FALSE, FALSE, 5);
    
    GtkWidget* dirErrorLabel = gtk_label_new("测向误差");
    gtk_box_pack_start(GTK_BOX(dirErrorBox), dirErrorLabel, FALSE, FALSE, 5);
    
    GtkWidget* dirErrorEntry = gtk_entry_new();
    gtk_widget_set_sensitive(dirErrorEntry, FALSE);
    gtk_box_pack_start(GTK_BOX(dirErrorBox), dirErrorEntry, TRUE, TRUE, 5);
    
    // 参数测量误差
    GtkWidget* paramErrorBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(resultBox), paramErrorBox, FALSE, FALSE, 5);
    
    GtkWidget* paramErrorLabel = gtk_label_new("参数测量误差");
    gtk_box_pack_start(GTK_BOX(paramErrorBox), paramErrorLabel, FALSE, FALSE, 5);
    
    GtkWidget* paramErrorEntry = gtk_entry_new();
    gtk_widget_set_sensitive(paramErrorEntry, FALSE);
    gtk_box_pack_start(GTK_BOX(paramErrorBox), paramErrorEntry, TRUE, TRUE, 5);
    
    // 开始定位按钮
    GtkWidget* locateButton = gtk_button_new_with_label("按钮：开始定位");
    gtk_box_pack_start(GTK_BOX(paramBox), locateButton, FALSE, FALSE, 10);
    
    return container;
}

// 多平台协同侦察UI创建 (简化版)
GtkWidget* UIManager::createMultiPlatformUI() {
    // 创建页面的主容器
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    // 左侧：地图显示区域
    GtkWidget* mapFrame = gtk_frame_new("多平台无源定位仿真");
    gtk_widget_set_size_request(mapFrame, 600, 700);
    gtk_box_pack_start(GTK_BOX(container), mapFrame, TRUE, TRUE, 0);
    
    // 显示地图
    showMap(mapFrame);
    
    // 右侧：参数设置区域
    GtkWidget* paramBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(container), paramBox, FALSE, FALSE, 0);
    
    // 雷达侦察设备模型参数设置
    GtkWidget* deviceFrame = gtk_frame_new("多平台雷达侦察设备模型参数设置");
    gtk_box_pack_start(GTK_BOX(paramBox), deviceFrame, FALSE, FALSE, 0);
    
    // 辐射源模型参数设置
    GtkWidget* sourceFrame = gtk_frame_new("辐射源模型参数设置");
    gtk_box_pack_start(GTK_BOX(paramBox), sourceFrame, FALSE, FALSE, 0);
    
    // 开始定位按钮
    GtkWidget* locateButton = gtk_button_new_with_label("按钮：开始定位");
    gtk_box_pack_start(GTK_BOX(paramBox), locateButton, FALSE, FALSE, 10);
    
    return container;
}

// 创建数据显示UI (简化版)
GtkWidget* UIManager::createDataDisplayUI() {
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    // 显示区域
    GtkWidget* displayFrame = gtk_frame_new("侦察数据显示");
    gtk_box_pack_start(GTK_BOX(container), displayFrame, TRUE, TRUE, 0);
    
    return container;
}

// 创建数据分选UI (简化版)
GtkWidget* UIManager::createDataSelectionUI() {
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    // 分选区域
    GtkWidget* selectionFrame = gtk_frame_new("数据分选");
    gtk_box_pack_start(GTK_BOX(container), selectionFrame, TRUE, TRUE, 0);
    
    return container;
}

// 创建评估UI (简化版)
GtkWidget* UIManager::createEvaluationUI() {
    GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    // 评估区域
    GtkWidget* evalFrame = gtk_frame_new("协同定位评估");
    gtk_box_pack_start(GTK_BOX(container), evalFrame, TRUE, TRUE, 0);
    
    return container;
}

// 显示地图
void UIManager::showMap(GtkWidget* container) {
    // 创建绘图区域
    GtkWidget* drawingArea = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawingArea, 600, 600);
    gtk_container_add(GTK_CONTAINER(container), drawingArea);
    
    // 连接绘图事件
    g_signal_connect(drawingArea, "draw", G_CALLBACK(drawMapCallback), NULL);
}

// 单平台仿真回调
void UIManager::onSinglePlatformSimulation(GtkWidget* widget, gpointer data) {
    // 获取仿真参数并执行仿真
    // 在实际应用中，这里需要从UI控件获取参数值
    
    // 创建仿真对象并设置参数
    SinglePlatformSimulation& simulation = SimulationManager::getInstance().getSinglePlatformSimulation();
    
    // 执行仿真
    LocationResult result = simulation.runSimulation();
    
    // 显示结果
    // 在实际应用中，这里需要将结果显示到UI控件
    std::cout << "Simulation completed. Power: " << result.power 
              << ", Direction Error: " << result.directionError 
              << ", Parameter Error: " << result.parameterError << std::endl;
}

// 多平台仿真回调
void UIManager::onMultiPlatformSimulation(GtkWidget* widget, gpointer data) {
    // 获取仿真参数并执行仿真
    // 在实际应用中，这里需要从UI控件获取参数值
    
    // 创建仿真对象并设置参数
    MultiPlatformSimulation& simulation = SimulationManager::getInstance().getMultiPlatformSimulation();
    
    // 执行仿真
    LocationResult result = simulation.runSimulation();
    
    // 显示结果
    // 在实际应用中，这里需要将结果显示到UI控件
    std::cout << "Simulation completed. Power: " << result.power 
              << ", Direction Error: " << result.directionError 
              << ", Parameter Error: " << result.parameterError << std::endl;
}