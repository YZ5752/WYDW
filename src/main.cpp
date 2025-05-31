#include <gtk/gtk.h>
#include <iostream>
#include <string>
#include "../include/ui_manager.h"
#include "../include/db_connector.h"

int main(int argc, char* argv[]) {
    // 初始化数据库连接
    DBConnector& dbConnector = DBConnector::getInstance();
    if (!dbConnector.init("PassiveLocationDSN", "root", "")) {  // 使用root用户，空密码
        std::cerr << "Failed to connect to database" << std::endl;
        return 1;
    }
    
    // 确保数据库表已创建 (现在这个函数会跳过实际的表创建)
    if (!dbConnector.createTables()) {
        std::cerr << "Failed to verify database tables" << std::endl;
        return 1;
    }
    
    // 初始化UI
    UIManager& uiManager = UIManager::getInstance();
    if (!uiManager.initUI(argc, argv)) {
        std::cerr << "Failed to initialize UI" << std::endl;
        return 1;
    }
    
    // 运行主循环
    uiManager.run();
    
    // 关闭数据库连接
    dbConnector.close();
    
    return 0;
} 