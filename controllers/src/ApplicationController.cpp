#include "../ApplicationController.h"
#include "../../models/ReconnaissanceDeviceModel.h"
#include "../../models/RadiationSourceModel.h"
#include "../../views/ReconnaissanceDeviceModelView.h"
#include "../../views/RadiationSourceModelView.h"
#include "../../views/SinglePlatformView.h"
#include "../../views/MultiPlatformView.h"
#include "../../views/DataSelectionView.h"
#include "../../views/EvaluationView.h"
#include "../ReconnaissanceDeviceModelController.h"
#include "../RadiationSourceModelController.h"
#include "../SinglePlatformController.h"
#include "../MultiPlatformController.h"
#include "../DataSelectionController.h"
#include "../EvaluationController.h"
#include <iostream>
#include <random>

// 辅助函数，获取容器中指定索引的子控件
GtkWidget* get_child_at_index(GtkContainer* container, gint index) {
    if (!container) {
        g_print("Error: Null container passed to get_child_at_index\n");
        return NULL;
    }

    GList* children = gtk_container_get_children(container);
    GtkWidget* widget = NULL;
    
    if (children && g_list_length(children) > index) {
        widget = GTK_WIDGET(g_list_nth_data(children, index));
    } else {
        g_print("Warning: Container has %d children, but index %d was requested\n", 
               children ? g_list_length(children) : 0, index);
    }
    
    g_list_free(children);
    return widget;
}

// 单例实现
ApplicationController& ApplicationController::getInstance() {
    static ApplicationController instance;
    return instance;
}

ApplicationController::ApplicationController()
    : m_mainWindow(nullptr),
      m_notebook(nullptr),
      m_currentPage(""),
      m_singlePlatformController(nullptr),
      m_multiPlatformController(nullptr),
      m_dataSelectionController(nullptr),
      m_evaluationController(nullptr),
      m_radiationSourceModelController(nullptr),
      m_reconnaissanceDeviceModelController(nullptr),
      m_reconDeviceView(nullptr),
      m_radiationSourceView(nullptr),
      m_singlePlatformView(nullptr),
      m_multiPlatformView(nullptr),
      m_dataSelectionView(nullptr),
      m_evaluationView(nullptr) {
}

ApplicationController::~ApplicationController() {
    if (m_mainWindow) {
        gtk_widget_destroy(m_mainWindow);
    }
    
    // 释放视图对象
    delete m_reconDeviceView;
    delete m_radiationSourceView;
    delete m_singlePlatformView;
    delete m_multiPlatformView;
    delete m_dataSelectionView;
    delete m_evaluationView;
    
    // 不需要删除单例控制器
    // 它们会在程序结束时自动销毁
}

bool ApplicationController::init(int argc, char** argv) {
    g_print("Starting application initialization...\n");
    gtk_init(&argc, &argv);
    
    // 初始化数据库连接
    g_print("Initializing database connection...\n");
    if (!DBConnector::initDefaultConnection()) {
        g_print("Failed to connect to database, will continue with sample data\n");
        // 继续执行，使用示例数据
    } else {
        g_print("Database connection established successfully\n");
    }
    
    // 创建主窗口
    m_mainWindow = createMainWindow();
    if (!m_mainWindow) {
        g_print("Failed to create main window\n");
        return false;
    }
    
    // 创建笔记本控件用于页面切换
    g_print("Creating notebook for page navigation...\n");
    m_notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(m_mainWindow), m_notebook);
    
    // 初始化各个控制器和视图
    g_print("Initializing controllers and views...\n");
    
    // 创建视图实例
    m_reconDeviceView = new ReconnaissanceDeviceModelView();
    m_radiationSourceView = new RadiationSourceModelView();
    m_singlePlatformView = new SinglePlatformView();
    m_multiPlatformView = new MultiPlatformView();
    m_dataSelectionView = new DataSelectionView();
    m_evaluationView = new EvaluationView();
    
    // 初始化控制器
    g_print("Initializing controllers...\n");
    m_reconnaissanceDeviceModelController = &ReconnaissanceDeviceModelController::getInstance();
    m_reconnaissanceDeviceModelController->init(m_reconDeviceView);
    
    m_radiationSourceModelController = &RadiationSourceModelController::getInstance();
    m_radiationSourceModelController->init(m_radiationSourceView);
    
    // 创建各个页面
    GtkWidget* reconDeviceTab = gtk_label_new("侦察设备模型");
    GtkWidget* reconDevicePage = m_reconDeviceView->createView();
    gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), reconDevicePage, reconDeviceTab);
    
    GtkWidget* radiationSourceTab = gtk_label_new("辐射源模型");
    GtkWidget* radiationSourcePage = m_radiationSourceView->createView();
    gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), radiationSourcePage, radiationSourceTab);
    
    GtkWidget* singlePlatformTab = gtk_label_new("单平台仿真");
    GtkWidget* singlePlatformPage = m_singlePlatformView->createView();
    gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), singlePlatformPage, singlePlatformTab);
    
    GtkWidget* multiPlatformTab = gtk_label_new("多平台仿真");
    GtkWidget* multiPlatformPage = m_multiPlatformView->createView();
    gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), multiPlatformPage, multiPlatformTab);
    
    GtkWidget* dataSelectionTab = gtk_label_new("数据分选");
    GtkWidget* dataSelectionPage = m_dataSelectionView->createView();
    gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), dataSelectionPage, dataSelectionTab);
    
    GtkWidget* evaluationTab = gtk_label_new("仿真评估");
    GtkWidget* evaluationPage = m_evaluationView->createView();
    gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), evaluationPage, evaluationTab);
    
    // 加载数据
    g_print("Loading initial data...\n");
    m_radiationSourceModelController->loadSourceData();
    m_reconnaissanceDeviceModelController->loadDeviceData();
    
    // 显示所有控件
    g_print("Showing all widgets...\n");
    gtk_widget_show_all(m_mainWindow);
    
    g_print("Application initialization completed\n");
    return true;
}

void ApplicationController::run() {
    gtk_main();
}

GtkWidget* ApplicationController::createMainWindow() {
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    gtk_window_set_title(GTK_WINDOW(window), "无源协同定位模块");
    gtk_window_set_default_size(GTK_WINDOW(window), 1280, 800);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    return window;
}

void ApplicationController::switchToSinglePlatformPage() {
    if (m_notebook) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(m_notebook), 2); // 假设单平台页面索引为2
        m_currentPage = "single_platform";
    }
}

void ApplicationController::switchToMultiPlatformPage() {
    if (m_notebook) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(m_notebook), 3); // 假设多平台页面索引为3
        m_currentPage = "multi_platform";
    }
}

void ApplicationController::switchToDataSelectionPage() {
    if (m_notebook) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(m_notebook), 4); // 假设数据分选页面索引为4
        m_currentPage = "data_selection";
    }
}

void ApplicationController::switchToEvaluationPage() {
    if (m_notebook) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(m_notebook), 5); // 假设评估页面索引为5
        m_currentPage = "evaluation";
    }
}

void ApplicationController::switchToRadarDeviceModelPage() {
    if (m_notebook) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(m_notebook), 0); // 假设雷达设备模型页面索引为0
        m_currentPage = "radar_device_model";
    }
}

void ApplicationController::switchToRadiationSourceModelPage() {
    if (m_notebook) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(m_notebook), 1); // 假设辐射源模型页面索引为1
        m_currentPage = "radiation_source_model";
    }
}

std::string ApplicationController::getCurrentPage() const {
    return m_currentPage;
}