#pragma once

#include <gtk/gtk.h>
#include <vector>
#include <string>

class MultiPlatformView {
public:
    MultiPlatformView();
    ~MultiPlatformView();
    
    // 创建多平台仿真UI
    GtkWidget* createView();
    
    // 获取视图控件
    GtkWidget* getView() const;

private:
    GtkWidget* m_view;//主视图
    GtkWidget* m_algoCombo;//技术体制
    GtkWidget* m_resultLabel;//仿真结果
    GtkWidget* m_errorLabel;//误差结果
}; 