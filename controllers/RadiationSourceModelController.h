#pragma once

#include "../views/RadiationSourceModelView.h"
#include "../models/RadiationSourceDAO.h"
#include "../models/RadiationSourceModel.h"
#include <string>
#include <vector>

class ApplicationController;  // 前向声明

class RadiationSourceModelController {
public:
    friend class ApplicationController;  // 声明ApplicationController为友元类
    
    static RadiationSourceModelController& getInstance();
    
    // 初始化控制器
    void init(RadiationSourceModelView* view);
    
    // 加载辐射源数据
    void loadSourceData();
    
    // 添加辐射源
    void addSource(const RadiationSource& source);
    
    // 编辑辐射源
    void editSource(const RadiationSource& source);
    
    // 删除辐射源
    void deleteSource(int sourceId);
    
    // 显示编辑对话框
    void showEditDialog(int sourceId = -1);
    
    // 获取视图
    RadiationSourceModelView* getView() const;

private:
    RadiationSourceModelController();
    ~RadiationSourceModelController();
    
    // 禁止拷贝
    RadiationSourceModelController(const RadiationSourceModelController&) = delete;
    RadiationSourceModelController& operator=(const RadiationSourceModelController&) = delete;
    
    RadiationSourceModelView* m_view;
    std::vector<RadiationSource> m_sources;
}; 