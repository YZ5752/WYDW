#include "../ReconnaissanceDeviceModelController.h"
#include "../ApplicationController.h"
#include <iostream>

// 单例实例
static ReconnaissanceDeviceModelController* s_instance = nullptr;

ReconnaissanceDeviceModelController& ReconnaissanceDeviceModelController::getInstance() {
    if (!s_instance) {
        s_instance = new ReconnaissanceDeviceModelController();
    }
    return *s_instance;
}

ReconnaissanceDeviceModelController::ReconnaissanceDeviceModelController() : m_view(nullptr) {
    g_print("创建侦察设备模型控制器\n");
}

ReconnaissanceDeviceModelController::~ReconnaissanceDeviceModelController() {
    g_print("销毁侦察设备模型控制器\n");
}

void ReconnaissanceDeviceModelController::init(ReconnaissanceDeviceModelView* view) {
    m_view = view;
    loadDeviceData();
}

void ReconnaissanceDeviceModelController::loadDeviceData() {
    g_print("加载侦察设备数据...\n");
    
    // 从数据库加载数据
    m_devices = ReconnaissanceDeviceDAO::getInstance().getAllReconnaissanceDevices();
    
    // 更新视图
    if (m_view) {
        m_view->updateDeviceList(m_devices);
    }
}

void ReconnaissanceDeviceModelController::addDevice(const ReconnaissanceDevice& device) {
    g_print("添加侦察设备: %s\n", device.getDeviceName().c_str());
    
    // 保存到数据库
    int deviceId;
    ReconnaissanceDeviceDAO::getInstance().addReconnaissanceDevice(device, deviceId);
    
    // 重新加载数据
    loadDeviceData();
}

void ReconnaissanceDeviceModelController::editDevice(const ReconnaissanceDevice& device) {
    g_print("编辑侦察设备: %s (ID: %d)\n", device.getDeviceName().c_str(), device.getDeviceId());
    
    // 更新数据库
    ReconnaissanceDeviceDAO::getInstance().updateReconnaissanceDevice(device);
    
    // 重新加载数据
    loadDeviceData();
}

void ReconnaissanceDeviceModelController::deleteDevice(int deviceId) {
    g_print("删除侦察设备 ID: %d\n", deviceId);
    
    // 从数据库删除
    ReconnaissanceDeviceDAO::getInstance().deleteReconnaissanceDevice(deviceId);
    
    // 重新加载数据
    loadDeviceData();
}

void ReconnaissanceDeviceModelController::showEditDialog(int deviceId) {
    if (!m_view) {
        g_print("视图未初始化，无法显示编辑对话框\n");
        return;
    }
    
    // 准备设备数据
    ReconnaissanceDevice device;
    
    if (deviceId >= 0) {
        // 编辑现有设备
        for (const auto& d : m_devices) {
            if (d.getDeviceId() == deviceId) {
                device = d;
                break;
            }
        }
    }
    
    // 创建并显示对话框
    GtkWidget* dialog = m_view->createEditDialog(device);
    
    // 运行对话框
    int result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        // 获取对话框中的设备数据
        ReconnaissanceDevice updatedDevice = m_view->getDeviceFromDialog(dialog);
        
        if (deviceId >= 0) {
            // 保持原ID
            updatedDevice.setDeviceId(deviceId);
            editDevice(updatedDevice);
        } else {
            // 新设备
            addDevice(updatedDevice);
        }
    }
    
    gtk_widget_destroy(dialog);
}

ReconnaissanceDeviceModelView* ReconnaissanceDeviceModelController::getView() const {
    return m_view;
}