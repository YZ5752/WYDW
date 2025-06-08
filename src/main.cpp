#include <gtk/gtk.h>
#include <iostream>
#include <string>
#include <cstdlib>  // 用于 exit 函数
#include "../include/ui_manager.h"
#include "../include/db_connector.h"

// 错误处理函数
void handle_error(const char* message) {
    std::cerr << "ERROR: " << message << std::endl;
    g_print("ERROR: %s\n", message);
}

int main(int argc, char* argv[]) {
    g_print("Starting application...\n");

    try {
        // 初始化数据库连接
        g_print("Initializing database connection...\n");
        DBConnector& dbConnector = DBConnector::getInstance();
        if (!dbConnector.init("127.0.0.1", "root", "123456", "passive_location", 3306)) {
            handle_error("Failed to connect to database");
            return 1;
        }
        
        // 确保数据库表已创建 (现在这个函数会跳过实际的表创建)
        g_print("Verifying database tables...\n");
        if (!dbConnector.createTables()) {
            handle_error("Failed to verify database tables");
            return 1;
        }
        
        // 初始化UI
        g_print("Initializing UI...\n");
        UIManager& uiManager = UIManager::getInstance();
        
        // 添加更多错误处理
        try {
            if (!uiManager.initUI(argc, argv)) {
                handle_error("Failed to initialize UI");
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception during UI initialization: " << e.what() << std::endl;
            g_print("Exception during UI initialization: %s\n", e.what());
            return 1;
        } catch (...) {
            handle_error("Unknown exception during UI initialization");
            return 1;
        }
        
        // 运行主循环
        g_print("Starting GTK main loop...\n");
        uiManager.run();
        
        // 关闭数据库连接
        g_print("Closing database connection...\n");
        dbConnector.close();
        
        g_print("Application terminated normally\n");
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        g_print("Unhandled exception: %s\n", e.what());
        return 1;
    } catch (...) {
        handle_error("Unknown unhandled exception");
        return 1;
    }
} 