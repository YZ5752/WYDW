#include "../MultiPlatformController.h"
#include "../../models/ReconnaissanceDeviceDAO.h"
#include "../../models/RadiationSourceDAO.h"
#include <iostream>
#include <sstream>
#include <iomanip>

// 单例实现
MultiPlatformController& MultiPlatformController::getInstance() {
    static MultiPlatformController instance;
    return instance;
}

// 构造函数
MultiPlatformController::MultiPlatformController() : m_view(nullptr) {
}

// 析构函数
MultiPlatformController::~MultiPlatformController() {
}

// 初始化控制器
void MultiPlatformController::init(MultiPlatformView* view) {
    m_view = view;
}

// 获取视图
MultiPlatformView* MultiPlatformController::getView() const {
    return m_view;
}

void MultiPlatformController::startSimulation(const std::vector<std::string>& deviceNames,
                                            const std::string& sourceName,
                                            const std::string& systemType,
                                            double simulationTime) {
    if (systemType == "频差体制") {
        // 获取FDOA算法实例
        FDOAalgorithm& algorithm = FDOAalgorithm::getInstance();
        
        // 初始化算法参数
        algorithm.init(deviceNames, sourceName, systemType, simulationTime);
        
        // 执行算法
        bool success = algorithm.calculate();
        
        if (success) {
            FDOAalgorithm::SourcePositionResult result = algorithm.getResult();
            // 将空间直角坐标转换为大地坐标
            COORD3 resultLBH = xyz2lbh(result.position.p1, result.position.p2, result.position.p3);
            COORD3 velocityResult = velocity_xyz2lbh(resultLBH.p1, resultLBH.p2, 
                                           result.velocity.x, result.velocity.y, result.velocity.z);
            
            std::stringstream ss;
            ss << "经度：" << resultLBH.p1 << "°\n";
            ss << "纬度：" << resultLBH.p2 << "°\n";  
            ss << "高度：" << resultLBH.p3 << " 米\n";
            ss << "运动速度：" << velocityResult.p1 << " 米/秒\n";
            ss << "运动方位角：" << velocityResult.p2 << "°\n";
            ss << "运动俯仰角：" << velocityResult.p3 << "°\n\n";
            
            m_view->updateResult(ss.str());
       } else {
            m_view->updateResult("定位计算失败");
        }
    } else if (systemType == "时差体制") {
        // TODO: 实现时差定位算法
        if (m_view) {
            m_view->updateResult("<span color='blue'>时差定位算法尚未实现</span>");
        }
    } 
} 