#pragma once

#include <string>
#include <vector>
#include <gtk/gtk.h>

// 雷达侦察设备模型
class RadarDevice {
public:
    RadarDevice();
    ~RadarDevice();

    // 设置设备类型 (侦察机（移动）/雷达（固定）)
    void setDeviceType(const std::string& type);
    std::string getDeviceType() const;

    // 设置设备数量
    void setDeviceCount(int count);
    int getDeviceCount() const;

    // 设置技术体制 (时差/频差)
    void setTechnicalSystem(const std::string& system);
    std::string getTechnicalSystem() const;

    // 设置基本参数
    void setAntennaLength(double length);
    double getAntennaLength() const;

    void setNoiseLevel(double noise);
    double getNoiseLevel() const;

    // 设置工作参数
    void setWorkTimeRange(const std::string& startTime, const std::string& endTime);
    std::pair<std::string, std::string> getWorkTimeRange() const;

    void setFrequencyRange(double min, double max);
    std::pair<double, double> getFrequencyRange() const;

    void setAngleRange(double min, double max);
    std::pair<double, double> getAngleRange() const;

    // 计算威力
    double calculatePower() const;

    // 计算测向误差
    double calculateDirectionError() const;

    // 计算参数测量误差
    double calculateParameterError() const;

private:
    std::string m_deviceType;      // 设备类型
    int m_deviceCount;             // 设备数量
    std::string m_technicalSystem; // 技术体制
    double m_antennaLength;        // 天线长度
    double m_noiseLevel;           // 噪声
    std::string m_workStartTime;   // 工作开始时间
    std::string m_workEndTime;     // 工作结束时间
    double m_freqMin;              // 最小频率
    double m_freqMax;              // 最大频率
    double m_angleMin;             // 最小角度
    double m_angleMax;             // 最大角度
};

// 辐射源模型
class RadiationSource {
public:
    RadiationSource();
    ~RadiationSource();

    // 设置设备类型 (雷达站（固定）/飞机（移动）/舰船（移动）)
    void setDeviceType(const std::string& type);
    std::string getDeviceType() const;

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
    std::string m_deviceType;    // 设备类型
    double m_transmitPower;      // 发射功率
    double m_scanPeriod;         // 扫描周期
    double m_freqMin;            // 最小频率
    double m_freqMax;            // 最大频率
    double m_workSector;         // 工作扇区
}; 