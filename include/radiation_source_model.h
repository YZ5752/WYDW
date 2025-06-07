#pragma once

#include <string>
#include <vector>
#include <utility>

// 辐射源模型
class RadiationSource {
public:
    RadiationSource();
    ~RadiationSource();

    // 设备类型
    void setDeviceType(const std::string& type);
    std::string getDeviceType() const;

    // 辐射源ID
    void setRadiationId(int id);
    int getRadiationId() const;

    // 辐射源名称
    void setRadiationName(const std::string& name);
    std::string getRadiationName() const;

    // 是否固定设备
    void setIsStationary(bool stationary);
    bool getIsStationary() const;

    // 发射功率（千瓦）
    void setTransmitPower(double power);
    double getTransmitPower() const;

    // 扫描周期（秒）
    void setScanPeriod(double period);
    double getScanPeriod() const;

    // 载波频率（GHz）
    void setCarrierFrequency(double freq);
    double getCarrierFrequency() const;

    // 频率范围
    void setFrequencyRange(double min, double max);
    std::pair<double, double> getFrequencyRange() const;

    // 工作扇区（度）
    void setWorkSector(double sector);
    double getWorkSector() const;

    // 方位角范围（度）
    void setAzimuthRange(double start, double end);
    std::pair<double, double> getAzimuthRange() const;

    // 俯仰角范围（度）
    void setElevationRange(double start, double end);
    std::pair<double, double> getElevationRange() const;

    // 运动参数
    void setMovementSpeed(double speed);
    double getMovementSpeed() const;
    void setMovementAzimuth(double azimuth);
    double getMovementAzimuth() const;
    void setMovementElevation(double elevation);
    double getMovementElevation() const;

    // 位置
    void setLongitude(double longitude);
    double getLongitude() const;
    void setLatitude(double latitude);
    double getLatitude() const;
    void setAltitude(double altitude);
    double getAltitude() const;

private:
    int m_radiationId;
    std::string m_radiationName;
    bool m_isStationary;
    std::string m_deviceType;
    double m_transmitPower;
    double m_scanPeriod;
    double m_carrierFrequency;
    double m_freqMin;
    double m_freqMax;
    double m_workSector;
    double m_azimuthStart;
    double m_azimuthEnd;
    double m_elevationStart;
    double m_elevationEnd;
    double m_movementSpeed;
    double m_movementAzimuth;
    double m_movementElevation;
    double m_longitude;
    double m_latitude;
    double m_altitude;
};

// 数据库操作函数
std::vector<RadiationSource> getAllRadiationSources();
bool addRadiationSource(const RadiationSource& source);
bool updateRadiationSource(const RadiationSource& source);
bool deleteRadiationSource(int sourceId);
