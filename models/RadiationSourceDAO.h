#pragma once

#include "RadiationSourceModel.h"
#include <vector>

// 辐射源数据访问对象 (DAO)
class RadiationSourceDAO {
public:
    static RadiationSourceDAO& getInstance();
    
    // 获取所有辐射源模型
    std::vector<RadiationSource> getAllRadiationSources();
    
    // 通过名称获取辐射源ID
    int getRadiationSourceIdByName(const std::string& name);
    
    // 通过ID获取辐射源
    RadiationSource getRadiationSourceById(int sourceId);
    
    // 添加辐射源
    bool addRadiationSource(const RadiationSource& source, int& sourceId);
    
    // 更新辐射源
    bool updateRadiationSource(const RadiationSource& source);
    
    // 删除辐射源
    bool deleteRadiationSource(int sourceId);

private:
    RadiationSourceDAO();
    ~RadiationSourceDAO();
    
    // 禁止拷贝
    RadiationSourceDAO(const RadiationSourceDAO&) = delete;
    RadiationSourceDAO& operator=(const RadiationSourceDAO&) = delete;
}; 