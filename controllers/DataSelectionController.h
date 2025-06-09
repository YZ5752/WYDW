#pragma once

#include "../views/DataSelectionView.h"
#include <string>
#include <vector>

class ApplicationController;  // 前向声明

class DataSelectionController {
public:
    friend class ApplicationController;  // 声明ApplicationController为友元类
    
    static DataSelectionController& getInstance();
    
    // 初始化控制器
    void init(DataSelectionView* view);
    
    // 加载数据
    void loadData();
    
    // 筛选数据
    void filterData(const std::string& condition);
    
    // 导入数据
    void importData(const std::string& filePath);
    
    // 导出数据
    void exportData(const std::vector<std::string>& selectedItems, const std::string& filePath);
    
    // 获取视图
    DataSelectionView* getView() const;

private:
    DataSelectionController();
    ~DataSelectionController();
    
    // 禁止拷贝
    DataSelectionController(const DataSelectionController&) = delete;
    DataSelectionController& operator=(const DataSelectionController&) = delete;
    
    DataSelectionView* m_view;
    std::vector<std::string> m_dataItems;
}; 