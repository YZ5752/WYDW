#pragma once

#include <string>
#include <vector>
#include <utility>

// 辐射源模型
class RadiationSource {
public:
    RadiationSource();
    ~RadiationSource();

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

    //工作扇区方位角下限
    void setAzimuthStart(double start);
    double getAzimuthStart() const;

    //工作扇区方位角上限
    void setAzimuthEnd(double end);
    double getAzimuthEnd() const;

    //工作扇区俯仰角下限
    void setElevationStart(double start);
    double getElevationStart() const;

    //工作扇区俯仰角上限
    void setElevationEnd(double end);
    double getElevationEnd() const;
    

    // 运动速度
    void setMovementSpeed(double speed);
    double getMovementSpeed() const;

    // 运动方位角
    void setMovementAzimuth(double azimuth);
    double getMovementAzimuth() const;

    // 运动俯仰角
    void setMovementElevation(double elevation);
    double getMovementElevation() const;

    // 经度
    void setLongitude(double longitude);
    double getLongitude() const;

    // 纬度
    void setLatitude(double latitude);
    double getLatitude() const;

    // 高度
    void setAltitude(double altitude);
    double getAltitude() const;

    // 创建时间
    void setCreatedAt(const std::string& time);
    std::string getCreatedAt() const;

    // 最后更新时间
    void setLastUpdated(const std::string& time);
    std::string getLastUpdated() const;


private:
    int m_radiationId;
    std::string m_radiationName;
    bool m_isStationary;
    double m_transmitPower;
    double m_scanPeriod;
    double m_carrierFrequency;
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
    std::string m_createdAt;
    std::string m_lastUpdated;
}; 