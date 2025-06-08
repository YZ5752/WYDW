#pragma once

#include "../views/ReconnaissanceDeviceModelView.h"
#include "../models/ReconnaissanceDeviceDAO.h"
#include "../models/ReconnaissanceDeviceModel.h"
#include <string>
#include <vector>
#include <gtk/gtk.h>

class ApplicationController;  // 前向声明

class ReconnaissanceDeviceModelController {
public:
    friend class ApplicationController;  // 声明ApplicationController为友元类
    
    static ReconnaissanceDeviceModelController& getInstance();
    
    // 初始化控制器
    void init(ReconnaissanceDeviceModelView* view);
    
    // 加载侦察设备数据
    void loadDeviceData();
    
    // 添加侦察设备
    void addDevice(const ReconnaissanceDevice& device);
    
    // 编辑侦察设备
    void editDevice(const ReconnaissanceDevice& device);
    
    // 删除侦察设备
    void deleteDevice(int deviceId);
    
    // 显示编辑对话框
    void showEditDialog(int deviceId = -1);
    
    // 获取视图
    ReconnaissanceDeviceModelView* getView() const;

private:
    ReconnaissanceDeviceModelController();
    ~ReconnaissanceDeviceModelController();
    
    // 禁止拷贝
    ReconnaissanceDeviceModelController(const ReconnaissanceDeviceModelController&) = delete;
    ReconnaissanceDeviceModelController& operator=(const ReconnaissanceDeviceModelController&) = delete;
    
    ReconnaissanceDeviceModelView* m_view;
    std::vector<ReconnaissanceDevice> m_devices;
}; 