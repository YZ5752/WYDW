#ifndef SIMULATION_VALIDATOR_H
#define SIMULATION_VALIDATOR_H

#include <vector>
#include <string>

/**
 * @brief 仿真前条件验证类
 * 用于验证侦察设备与辐射源之间的各项条件是否满足仿真要求
 */
class SimulationValidator {
public:
    /**
     * @brief 构造函数
     */
    SimulationValidator();
    
    /**
     * @brief 析构函数
     */
    ~SimulationValidator();
    
    /**
     * @brief 验证所有条件
     * 包括频率、角度、信噪比等条件的验证
     * @param deviceIds 侦察设备ID列表
     * @param sourceId 辐射源ID
     * @param failMessage 失败信息输出参数
     * @return bool 验证结果，成功返回true，失败返回false
     */
    bool validateAll(const std::vector<int>& deviceIds, int sourceId, std::string& failMessage);
};

#endif // SIMULATION_VALIDATOR_H 