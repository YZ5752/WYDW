#pragma once

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include "../models/RadiationSourceModel.h"

class RadiationSourceModelView {
public:
    RadiationSourceModelView();
    ~RadiationSourceModelView();
    
    // 创建辐射源模型UI
    GtkWidget* createView();
    
    // 创建辐射源编辑对话框
    GtkWidget* createEditDialog(const RadiationSource& source = RadiationSource());
    
    // 更新辐射源列表
    void updateSourceList(const std::vector<RadiationSource>& sources);
    
    // 获取当前选中的辐射源ID
    int getSelectedSourceId() const;
    
    // 从编辑对话框获取辐射源数据
    RadiationSource getSourceFromDialog(GtkWidget* dialog) const;
    
    // 获取视图控件
    GtkWidget* getView() const;

private:
    GtkWidget* m_view;
    GtkWidget* m_sourceList;
    std::vector<RadiationSource> m_sources;
}; 