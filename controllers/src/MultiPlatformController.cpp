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
        if (algorithm.calculate()) {
            // 获取定位结果
            auto result = algorithm.getResult();
            
            // 格式化结果
            std::stringstream ss;
            ss << std::fixed << std::setprecision(6);
            
            // 辐射源位置
            ss << "<span weight='bold'>辐射源位置</span>\n";
            ss << "经度：" << result.longitude << "°\n";
            ss << "纬度：" << result.latitude << "°\n";
            ss << "高度：" << result.altitude << " 米\n\n";
            
            // 运动参数
            ss << "<span weight='bold'>运动参数</span>\n";
            ss << "方位角：" << result.azimuth << "°\n";
            ss << "俯仰角：" << result.elevation << "°\n";
            ss << "速度：" << result.velocity << " 米/秒\n\n";
            
            // 定位参数
            ss << "<span weight='bold'>定位参数</span>\n";
            ss << "定位时间：" << result.locationTime << " 秒\n";
            ss << "定位距离：" << result.distance << " 米\n";
            ss << "定位精度：" << result.accuracy << " 米";
            
            // 更新界面显示
            if (m_view) {
                m_view->updateResult(ss.str());
            }
        } else {
            if (m_view) {
                m_view->updateResult("<span color='red'>仿真计算失败</span>");
            }
        }
    } else if (systemType == "时差体制") {
        // TODO: 实现时差定位算法
        if (m_view) {
            m_view->updateResult("<span color='blue'>时差定位算法尚未实现</span>");
        }
    } else {
        if (m_view) {
            m_view->updateResult("<span color='red'>未知的技术体制</span>");
        }
    }
} 