#pragma once

#include "../views/DataSelectionView.h"
#include <string>
#include <vector>
#include <map>

class DataSelectionController {
private:
    DataSelectionController();
    ~DataSelectionController();
    
    // 禁止拷贝
    DataSelectionController(const DataSelectionController&) = delete;
    DataSelectionController& operator=(const DataSelectionController&) = delete;

    // 视图引用
    DataSelectionView* m_view;

public:
    static DataSelectionController& getInstance();
    
    // 初始化控制器
    void init(DataSelectionView* view);
    
    // 获取视图
    DataSelectionView* getView() const;
    
    // 获取关联任务
    std::vector<std::vector<std::string>> getRelatedTasks(int radiationId);
    
    // 删除选中的数据项
    void deleteSelectedItems(DataSelectionView* view);
    
    // 显示任务详情
    std::map<std::string, std::string> showTaskDetails(int taskId, const std::string& taskType);
    
    // 向数据库录入数据
    bool importData(DataSelectionView* view, bool isSingle, const std::vector<std::string>& values, 
                  const std::vector<int>& deviceIds, int radiationId, const std::string& positioningAlgorithm);
};
