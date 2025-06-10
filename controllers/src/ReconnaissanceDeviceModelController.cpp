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
    
    try {
        // 从数据库加载数据
        m_devices = ReconnaissanceDeviceDAO::getInstance().getAllReconnaissanceDevices();
        g_print("成功加载 %zu 个侦察设备\n", m_devices.size());
        
        // 更新视图
        if (m_view) {
            m_view->updateDeviceList(m_devices);
        } else {
            g_print("视图未初始化，无法更新设备列表\n");
        }
    } catch (const std::exception& e) {
        g_print("加载侦察设备数据异常: %s\n", e.what());
    } catch (...) {
        g_print("加载侦察设备数据时发生未知异常\n");
    }
}

void ReconnaissanceDeviceModelController::addDevice(const ReconnaissanceDevice& device) {
    g_print("添加侦察设备: %s\n", device.getDeviceName().c_str());
    
    // 保存到数据库
    int deviceId = -1;
    bool success = ReconnaissanceDeviceDAO::getInstance().addReconnaissanceDevice(device, deviceId);
    
    if (success) {
        g_print("成功添加侦察设备，ID: %d\n", deviceId);
    } else {
        g_print("添加侦察设备失败\n");
    }
    
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
    
    if (deviceId > 0) {
        // 编辑现有设备
        bool found = false;
        for (const auto& d : m_devices) {
            if (d.getDeviceId() == deviceId) {
                device = d;
                found = true;
                break;
            }
        }
        
        if (!found) {
            // 如果在内存中找不到，尝试从数据库加载
            device = ReconnaissanceDeviceDAO::getInstance().getReconnaissanceDeviceById(deviceId);
            if (device.getDeviceId() <= 0) {
                g_print("无法找到ID为%d的侦察设备\n", deviceId);
                return;
            }
        }
    } else {
        // 新增设备时，显式创建一个新的设备对象（重置为默认值）
        device = ReconnaissanceDevice();
        g_print("创建新的侦察设备对象\n");
    }
    
    // 创建并显示对话框
    GtkWidget* dialog = m_view->createEditDialog(device);
    
    bool keepDialogOpen = true;
    while (keepDialogOpen) {
        // 运行对话框
        int result = gtk_dialog_run(GTK_DIALOG(dialog));
        
        if (result == GTK_RESPONSE_OK) {
            // 获取对话框中的设备数据
            ReconnaissanceDevice updatedDevice = m_view->getDeviceFromDialog(dialog);
            
            // 数据验证
            std::string errorMsg;
            bool isValid = true;
            
            // 验证频率范围
            if (updatedDevice.getFreqRangeMax() <= updatedDevice.getFreqRangeMin()) {
                errorMsg = "错误：侦收频率范围上限必须大于下限";
                isValid = false;
            }
            // 验证方位角范围
            else if (updatedDevice.getAngleAzimuthMax() <= updatedDevice.getAngleAzimuthMin()) {
                errorMsg = "错误：方位角上限必须大于下限";
                isValid = false;
            }
            // 验证俯仰角范围
            else if (updatedDevice.getAngleElevationMax() <= updatedDevice.getAngleElevationMin()) {
                errorMsg = "错误：俯仰角上限必须大于下限";
                isValid = false;
            }
            // 验证固定设备的运动参数
            else if (updatedDevice.getIsStationary() && 
                    (updatedDevice.getMovementSpeed() != 0 || 
                     updatedDevice.getMovementAzimuth() != 0 || 
                     updatedDevice.getMovementElevation() != 0)) {
                errorMsg = "错误：固定设备的运动速度、方位角和俯仰角必须为0";
                isValid = false;
            }
            
            if (isValid) {
                if (deviceId > 0) {
                    // 保持原ID
                    updatedDevice.setDeviceId(deviceId);
                    editDevice(updatedDevice);
                } else {
                    // 新设备
                    addDevice(updatedDevice);
                }
                keepDialogOpen = false;
            } else {
                // 显示错误信息
                GtkWidget* errorDialog = gtk_message_dialog_new(
                    GTK_WINDOW(dialog),
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_ERROR,
                    GTK_BUTTONS_OK,
                    "%s", errorMsg.c_str()
                );
                gtk_dialog_run(GTK_DIALOG(errorDialog));
                gtk_widget_destroy(errorDialog);
                // 保持对话框打开
            }
        } else {
            // 用户取消
            keepDialogOpen = false;
        }
    }
    
    // 清理并销毁对话框
    gtk_widget_destroy(dialog);
}

ReconnaissanceDeviceModelView* ReconnaissanceDeviceModelController::getView() const {
    return m_view;
}