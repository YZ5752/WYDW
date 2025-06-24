#pragma once

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include <iostream> 

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

    // 从数据库获取与指定辐射源 ID 关联的任务数据。
    std::vector<std::vector<std::string>> getRelatedTasks(int radiationId);

    // 根据辐射源 ID 更新目标数据列表。
    void updateTaskList(int radiationId);

    // 删除选中的数据项
    void deleteSelectedItems();

    // 录入选中的数据项到数据库
    void importSelectedItems();

    // 删除按钮点击事件回调函数，触发删除选中数据项操作。
    static void onDeleteButtonClicked(GtkWidget* widget, gpointer user_data);

    // 录入按钮点击事件回调函数，触发录入选中数据项到数据库操作。
    static void onImportButtonClicked(GtkWidget* widget, gpointer user_data);

    // 显示任务详情对话框
    void showTaskDetailsDialog(int taskId, const std::string& taskType);

private:
    GtkWidget* m_view;
    GtkWidget* m_dataList;
    GtkWidget* m_filterEntry;
    GtkWidget* m_startTimeEntry;
    GtkWidget* m_endTimeEntry;
    GtkWidget* m_targetCombo;
    // 获取选中的数据项
    std::vector<std::vector<std::string>> getSelectedData() const;
}; 