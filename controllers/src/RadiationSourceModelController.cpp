#include "../RadiationSourceModelController.h"
#include <iostream>

// 单例实现
RadiationSourceModelController& RadiationSourceModelController::getInstance() {
    static RadiationSourceModelController instance;
    return instance;
}

// 构造函数
RadiationSourceModelController::RadiationSourceModelController() : m_view(nullptr) {
}

// 析构函数
RadiationSourceModelController::~RadiationSourceModelController() {
}

// 初始化控制器
void RadiationSourceModelController::init(RadiationSourceModelView* view) {
    m_view = view;
    loadSourceData();
}

// 加载辐射源数据
void RadiationSourceModelController::loadSourceData() {
    if (!m_view) return;
    
    // 从DAO加载辐射源数据
    m_sources = RadiationSourceDAO::getInstance().getAllRadiationSources();
    
    // 更新视图
    m_view->updateSourceList(m_sources);
}

// 添加辐射源
void RadiationSourceModelController::addSource(const RadiationSource& source) {
    // 调用DAO添加辐射源
    int sourceId = -1;
    bool success = RadiationSourceDAO::getInstance().addRadiationSource(source, sourceId);
    
    if (success && sourceId > 0) {
        // 重新加载数据以更新视图
        loadSourceData();
    } else {
        std::cerr << "添加辐射源失败" << std::endl;
    }
}

// 编辑辐射源
void RadiationSourceModelController::editSource(const RadiationSource& source) {
    // 调用DAO更新辐射源
    bool success = RadiationSourceDAO::getInstance().updateRadiationSource(source);
    
    if (success) {
        // 重新加载数据以更新视图
        loadSourceData();
    } else {
        std::cerr << "更新辐射源失败" << std::endl;
    }
}

// 删除辐射源
void RadiationSourceModelController::deleteSource(int sourceId) {
    // 调用DAO删除辐射源
    bool success = RadiationSourceDAO::getInstance().deleteRadiationSource(sourceId);
    
    if (success) {
        // 重新加载数据以更新视图
        loadSourceData();
    } else {
        std::cerr << "删除辐射源失败" << std::endl;
    }
}

// 显示编辑对话框
void RadiationSourceModelController::showEditDialog(int sourceId) {
    if (!m_view) return;
    
    RadiationSource source;
    
    // 如果是编辑现有辐射源
    if (sourceId > 0) {
        // 查找对应ID的辐射源
        for (const auto& s : m_sources) {
            if (s.getRadiationId() == sourceId) {
                source = s;
                break;
            }
        }
    }
    
    // 创建并显示编辑对话框
    GtkWidget* dialog = m_view->createEditDialog(source);
    
    // 对话框处理逻辑在视图层实现
    // ...
}

// 获取视图
RadiationSourceModelView* RadiationSourceModelController::getView() const {
    return m_view;
} 