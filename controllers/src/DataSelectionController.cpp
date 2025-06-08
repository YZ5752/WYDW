#include "../DataSelectionController.h"
#include <fstream>
#include <iostream>
#include <algorithm>

// 单例实现
DataSelectionController& DataSelectionController::getInstance() {
    static DataSelectionController instance;
    return instance;
}

// 构造函数
DataSelectionController::DataSelectionController() : m_view(nullptr) {
}

// 析构函数
DataSelectionController::~DataSelectionController() {
}

// 初始化控制器
void DataSelectionController::init(DataSelectionView* view) {
    m_view = view;
    loadData();
}

// 加载数据
void DataSelectionController::loadData() {
    if (!m_view) return;
    
    // TODO: 从数据库或文件加载数据
    // m_dataItems = ...;
    
    // 更新视图
    // m_view->updateDataList(m_dataItems);
}

// 筛选数据
void DataSelectionController::filterData(const std::string& condition) {
    if (!m_view) return;
    
    if (condition.empty()) {
        // 如果条件为空，显示所有数据
        m_view->updateDataList(m_dataItems);
        return;
    }
    
    // 根据条件筛选数据
    std::vector<std::string> filteredItems;
    for (const auto& item : m_dataItems) {
        // 简单的字符串匹配，实际应用中可能需要更复杂的筛选逻辑
        if (item.find(condition) != std::string::npos) {
            filteredItems.push_back(item);
        }
    }
    
    // 更新视图显示筛选后的数据
    m_view->updateDataList(filteredItems);
}

// 导入数据
void DataSelectionController::importData(const std::string& filePath) {
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
        std::cerr << "无法打开文件: " << filePath << std::endl;
        return;
    }
    
    // 清空现有数据
    m_dataItems.clear();
    
    // 读取文件数据
    std::string line;
    while (std::getline(inFile, line)) {
        if (!line.empty()) {
            m_dataItems.push_back(line);
        }
    }
    
    inFile.close();
    
    // 更新视图
    if (m_view) {
        m_view->updateDataList(m_dataItems);
    }
}

// 导出数据
void DataSelectionController::exportData(const std::vector<std::string>& selectedItems, const std::string& filePath) {
    std::ofstream outFile(filePath);
    if (!outFile.is_open()) {
        std::cerr << "无法打开文件: " << filePath << std::endl;
        return;
    }
    
    // 写入选中的数据项
    for (const auto& item : selectedItems) {
        outFile << item << "\n";
    }
    
    outFile.close();
}

// 获取视图
DataSelectionView* DataSelectionController::getView() const {
    return m_view;
} 