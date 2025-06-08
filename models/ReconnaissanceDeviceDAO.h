#pragma once

#include "ReconnaissanceDeviceModel.h"
#include <vector>

// 侦察设备数据访问对象 (DAO)
class ReconnaissanceDeviceDAO {
public:
    static ReconnaissanceDeviceDAO& getInstance();
    
    // 获取所有侦察设备模型
    std::vector<ReconnaissanceDevice> getAllReconnaissanceDevices();
    
    // 通过ID获取侦察设备
    ReconnaissanceDevice getReconnaissanceDeviceById(int deviceId);
    
    // 添加侦察设备
    bool addReconnaissanceDevice(const ReconnaissanceDevice& device, int& deviceId);
    
    // 更新侦察设备
    bool updateReconnaissanceDevice(const ReconnaissanceDevice& device);
    
    // 删除侦察设备
    bool deleteReconnaissanceDevice(int deviceId);

private:
    ReconnaissanceDeviceDAO();
    ~ReconnaissanceDeviceDAO();
    
    // 禁止拷贝
    ReconnaissanceDeviceDAO(const ReconnaissanceDeviceDAO&) = delete;
    ReconnaissanceDeviceDAO& operator=(const ReconnaissanceDeviceDAO&) = delete;
}; 