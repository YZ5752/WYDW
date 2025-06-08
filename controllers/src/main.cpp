#include "../ApplicationController.h"
#include <iostream>

int main(int argc, char** argv) {
    // 初始化应用程序控制器
    if (!ApplicationController::getInstance().init(argc, argv)) {
        std::cerr << "应用程序初始化失败" << std::endl;
        return 1;
    }
    
    // 运行应用程序
    ApplicationController::getInstance().run();
    
    return 0;
} 