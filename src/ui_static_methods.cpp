#include "../include/ui_manager.h"
#include "../include/reconnaissance_device_model.h"
#include "../include/db_connector.h"
#include "../include/radiation_source_model.h"
#include <iostream>
#include <sstream>

// 静态方法实现

// 更新雷达设备列表
void UIManager::updateReconnaissanceDeviceList(GtkWidget* list) {
    try {
        if (!list) {
            g_print("Error: Null list passed to updateReconnaissanceDeviceList\n");
            return;
        }
        
        GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        if (!model) {
            g_print("Error: Tree view has no model\n");
            return;
        }
        
        GtkListStore* store = GTK_LIST_STORE(model);
        if (!store) {
            g_print("Error: Failed to cast model to list store\n");
            return;
        }
        
        gtk_list_store_clear(store);
        
        // 从数据库获取侦察设备数据
        g_print("Fetching reconnaissance devices from database...\n");
        std::vector<ReconnaissanceDevice> devices;
        
        try {
            devices = DBConnector::getInstance().getAllReconnaissanceDevices();
            g_print("Successfully retrieved %d devices from database\n", (int)devices.size());
        } catch (const std::exception& e) {
            g_print("Exception while retrieving devices: %s\n", e.what());
            // 如果获取失败，添加一些示例数据以避免UI为空
            ReconnaissanceDevice device1;
            device1.setDeviceId(0);
            device1.setDeviceName("数据库错误 - 示例设备1");
            device1.setIsStationary(true);
            device1.setBaselineLength(2.5);
            device1.setFreqRangeMin(300);
            device1.setFreqRangeMax(18000);
            devices.push_back(device1);
        }
        
        // 如果数据库中没有数据，添加一条提示信息
        if (devices.empty()) {
            g_print("No devices found in database, adding placeholder\n");
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 
                              0, "无数据", 
                              1, "请添加侦察设备", 
                              2, "", 
                              3, "", 
                              4, "", 
                              5, "编辑",
                              6, "删除",
                              -1);
            return;
        }
        
        // 将数据库数据添加到列表中
        GtkTreeIter iter;
        for (const auto& device : devices) {
            gtk_list_store_append(store, &iter);
            
            // 将数值转换为字符串
            std::ostringstream baselineSS;
            baselineSS << device.getBaselineLength();
            
            // 设置列表数据
            gtk_list_store_set(store, &iter, 
                              0, device.getDeviceName().c_str(), 
                              1, device.getDeviceTypeString().c_str(), 
                              2, baselineSS.str().c_str(),
                              3, device.getFreqRangeString().c_str(), 
                              4, device.getAngleRangeString().c_str(), 
                              5, "编辑",  // 编辑按钮
                              6, "删除",  // 删除按钮
                              -1);
        }
        
        g_print("Device list updated with database data\n");
    } catch (const std::exception& e) {
        g_print("Exception in updateReconnaissanceDeviceList: %s\n", e.what());
    } catch (...) {
        g_print("Unknown exception in updateReconnaissanceDeviceList\n");
    }
}

// 更新辐射源列表
void UIManager::updateRadiationSourceList(GtkWidget* list) {
    try {
        if (!list) {
            g_print("Error: Null list passed to updateRadiationSourceList\n");
            return;
        }
        
        GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        if (!model) {
            g_print("Error: Tree view has no model\n");
            return;
        }
        
        GtkListStore* store = GTK_LIST_STORE(model);
        if (!store) {
            g_print("Error: Failed to cast model to list store\n");
            return;
        }
        
        gtk_list_store_clear(store);
        
        // 从数据库获取辐射源数据
        g_print("Fetching radiation sources from database...\n");
        std::vector<RadiationSource> sources;
        
        try {
            sources = DBConnector::getInstance().getAllRadiationSources();
            g_print("Successfully retrieved %d radiation sources from database\n", (int)sources.size());
        } catch (const std::exception& e) {
            g_print("Exception while retrieving radiation sources: %s\n", e.what());
            // 获取失败，添加一些示例数据
            RadiationSource source1;
            source1.setRadiationId(0);
            source1.setRadiationName("数据库错误 - 示例辐射源");
            source1.setDeviceType("雷达站");
            source1.setIsStationary(true);
            source1.setTransmitPower(100);
            sources.push_back(source1);
        }
        
        // 如果数据库中没有数据，添加一条提示信息
        if (sources.empty()) {
            g_print("No radiation sources found in database, adding placeholder\n");
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 
                              0, "无数据", 
                              1, "请添加辐射源", 
                              2, "", 
                              3, "", 
                              4, "", 
                              5, "", 
                              6, "编辑", 
                              7, "删除", 
                              -1);
            return;
        }
        
        // 将数据库数据添加到列表中
        GtkTreeIter iter;
        for (const auto& source : sources) {
            gtk_list_store_append(store, &iter);
            
            // 将数值转换为字符串
            std::ostringstream powerSS;
            powerSS << source.getTransmitPower();
            
            // 获取频率范围
            auto freqRange = source.getFrequencyRange();
            std::string freqRangeStr = std::to_string(freqRange.first) + "-" + std::to_string(freqRange.second);
            
            // 获取方位角范围
            auto azimuthRange = source.getAzimuthRange();
            std::string azimuthStr = std::to_string(azimuthRange.first) + "-" + std::to_string(azimuthRange.second);
            
            // 设备类型和状态
            std::string typeStr = source.getDeviceType();
            if (source.getIsStationary()) {
                typeStr += "（固定）";
            } else {
                typeStr += "（移动）";
            }
            
            // 设置列表数据
            gtk_list_store_set(store, &iter, 
                              0, source.getRadiationName().c_str(), 
                              1, typeStr.c_str(), 
                              2, powerSS.str().c_str(), 
                              3, "N/A",  // 无天线增益字段，设为N/A
                              4, freqRangeStr.c_str(), 
                              5, azimuthStr.c_str(), 
                              6, "编辑", 
                              7, "删除", 
                              -1);
        }
        
        g_print("Radiation source list updated with database data\n");
    } catch (const std::exception& e) {
        g_print("Exception in updateRadiationSourceList: %s\n", e.what());
    } catch (...) {
        g_print("Unknown exception in updateRadiationSourceList\n");
    }
} 