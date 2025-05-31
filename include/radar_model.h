#pragma once

#include <string>
#include <vector>
#include <gtk/gtk.h>

// 雷达侦察设备模型
class RadarDevice {
public:
    RadarDevice();
    ~RadarDevice();

    // 设置技术体制 (干涉仪体制/时差体制)
    void setTechnicalSystem(const std::string& system);
    std::string getTechnicalSystem() const;

    // 设置算法 (快速定位/基线定位)
    void setAlgorithm(const std::string& algorithm);
    std::string getAlgorithm() const;

    // 设置基本参数
    void setBasicParameters(const std::vector<double>& params);
    std::vector<double> getBasicParameters() const;

    // 设置工作参数
    void setWorkParameters(const std::vector<double>& params);
    std::vector<double> getWorkParameters() const;

    // 计算威力
    double calculatePower() const;

    // 计算测向误差
    double calculateDirectionError() const;

    // 计算参数测量误差
    double calculateParameterError() const;

private:
    std::string m_technicalSystem;  // 技术体制
    std::string m_algorithm;        // 算法
    std::vector<double> m_basicParams;  // 基本参数
    std::vector<double> m_workParams;   // 工作参数
};

// 辐射源模型
class RadiationSource {
public:
    RadiationSource();
    ~RadiationSource();

    // 设置发射功率
    void setTransmitPower(double power);
    double getTransmitPower() const;

    // 设置扫描周期
    void setScanPeriod(double period);
    double getScanPeriod() const;

    // 设置频率范围
    void setFrequencyRange(double min, double max);
    std::pair<double, double> getFrequencyRange() const;

    // 设置工作扇区
    void setWorkSector(double sector);
    double getWorkSector() const;

private:
    double m_transmitPower;  // 发射功率
    double m_scanPeriod;     // 扫描周期
    double m_freqMin;        // 最小频率
    double m_freqMax;        // 最大频率
    double m_workSector;     // 工作扇区
}; 