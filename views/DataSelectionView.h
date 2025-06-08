#pragma once

#include <gtk/gtk.h>
#include <string>
#include <vector>

class DataSelectionView {
public:
    DataSelectionView();
    ~DataSelectionView();
    
    // 创建数据选择UI
    GtkWidget* createView();
    
    // 更新数据列表
    void updateDataList(const std::vector<std::string>& dataItems);
    
    // 获取选择的数据项
    std::vector<std::string> getSelectedItems() const;
    
    // 获取数据筛选条件
    std::string getFilterCondition() const;
    
    // 获取开始时间
    std::string getStartTime() const;
    
    // 获取结束时间
    std::string getEndTime() const;
    
    // 获取视图控件
    GtkWidget* getView() const;

private:
    GtkWidget* m_view;
    GtkWidget* m_dataList;
    GtkWidget* m_filterEntry;
    GtkWidget* m_startTimeEntry;
    GtkWidget* m_endTimeEntry;
}; 