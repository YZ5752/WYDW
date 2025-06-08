#include "../RadiationSourceDAO.h"
#include "../DBConnector.h"
#include <iostream>

// 单例实现
RadiationSourceDAO& RadiationSourceDAO::getInstance() {
    static RadiationSourceDAO instance;
    return instance;
}

RadiationSourceDAO::RadiationSourceDAO() {
}

RadiationSourceDAO::~RadiationSourceDAO() {
}

std::vector<RadiationSource> RadiationSourceDAO::getAllRadiationSources() {
    std::cout << "RadiationSourceDAO: Getting all radiation sources..." << std::endl;
    
    // 从数据库连接器获取所有辐射源
    try {
        return DBConnector::getInstance().getAllRadiationSources();
    } catch (const std::exception& e) {
        std::cerr << "Error getting radiation sources: " << e.what() << std::endl;
        return std::vector<RadiationSource>();
    }
}

RadiationSource RadiationSourceDAO::getRadiationSourceById(int sourceId) {
    std::cout << "RadiationSourceDAO: Getting radiation source with ID " << sourceId << std::endl;
    
    // 简化实现，从获取所有辐射源中查找指定ID
    std::vector<RadiationSource> sources = getAllRadiationSources();
    for (const auto& source : sources) {
        if (source.getRadiationId() == sourceId) {
            return source;
        }
    }
    
    // 如果没有找到，返回一个空的辐射源对象
    return RadiationSource();
}

bool RadiationSourceDAO::addRadiationSource(const RadiationSource& source, int& sourceId) {
    std::cout << "RadiationSourceDAO: Adding radiation source..." << std::endl;
    
    try {
        return DBConnector::getInstance().saveRadiationSource(source, sourceId);
    } catch (const std::exception& e) {
        std::cerr << "Error adding radiation source: " << e.what() << std::endl;
        return false;
    }
}

bool RadiationSourceDAO::updateRadiationSource(const RadiationSource& source) {
    std::cout << "RadiationSourceDAO: Updating radiation source with ID " 
              << source.getRadiationId() << std::endl;
    
    // 简化实现，通过删除后添加实现更新
    int sourceId = source.getRadiationId();
    if (!deleteRadiationSource(sourceId)) {
        return false;
    }
    
    int newSourceId;
    return addRadiationSource(source, newSourceId);
}

bool RadiationSourceDAO::deleteRadiationSource(int sourceId) {
    std::cout << "RadiationSourceDAO: Deleting radiation source with ID " << sourceId << std::endl;
    
    // 简化实现，直接调用数据库连接器的SQL执行方法
    try {
        std::string sql = "DELETE FROM radiation_sources WHERE id = " + std::to_string(sourceId);
        return DBConnector::getInstance().executeSQL(sql);
    } catch (const std::exception& e) {
        std::cerr << "Error deleting radiation source: " << e.what() << std::endl;
        return false;
    }
} 