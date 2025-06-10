#pragma once

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include "../models/ReconnaissanceDeviceModel.h"

class ReconnaissanceDeviceModelView {
public:
    ReconnaissanceDeviceModelView();
    ~ReconnaissanceDeviceModelView();
    
    // 创建侦察设备模型UI
    GtkWidget* createView();
    
    // 设置容器控件
    void setContainer(GtkWidget* container);
    
    // 设置树视图控件
    void setTreeView(GtkWidget* treeView);
    
    // 创建侦察设备编辑对话框
    GtkWidget* createEditDialog(const ReconnaissanceDevice& device = ReconnaissanceDevice());
    
    // 更新侦察设备列表
    void updateDeviceList(const std::vector<ReconnaissanceDevice>& devices);
    
    // 获取当前选中的侦察设备ID
    int getSelectedDeviceId() const;
    
    // 从编辑对话框获取侦察设备数据
    ReconnaissanceDevice getDeviceFromDialog(GtkWidget* dialog) const;
    
    // 显示设备详情对话框
    void showDeviceDetailsDialog(int deviceId);
    
    // 获取视图控件
    GtkWidget* getView() const;

private:
    GtkWidget* m_view;
    GtkWidget* m_deviceList;
    std::vector<ReconnaissanceDevice> m_devices;
}; 