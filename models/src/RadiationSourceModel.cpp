#include "../RadiationSourceModel.h"
#include <iostream>
#include <cstdio>
#include <utility>

// RadiationSource类实现 - 辐射源模型（对应数据库表radiation_source_models）
RadiationSource::RadiationSource()
    : m_radiationId(0),               // 辐射源ID（主键）
      m_radiationName(""),            // 辐射源名称
      m_isStationary(true),           // 是否为固定辐射源，默认TRUE
      m_transmitPower(0),             // 发射功率（千瓦）
      m_scanPeriod(0),                // 扫描周期（秒）
      m_carrierFrequency(0),          // 载波频率（GHz）
      m_azimuthStart(0),              // 工作扇区方位角起始角度（度，正北为0，顺时针）
      m_azimuthEnd(360),              // 工作扇区方位角终止角度（度）
      m_elevationStart(-90),          // 工作扇区俯仰角起始角度（度，水平面为0，向上为正）
      m_elevationEnd(90),             // 工作扇区俯仰角终止角度（度）
      m_movementSpeed(0),             // 运动速度（米/秒），固定设备默认0
      m_movementAzimuth(0),           // 运动方位角（度，正北为0，顺时针），固定设备默认0
      m_movementElevation(0),         // 运动俯仰角（度，水平面为0，向上为正），固定设备默认0
      m_longitude(0),                 // 辐射源经度（度），范围-180~180
      m_latitude(0),                  // 辐射源纬度（度），范围-90~90
      m_altitude(0),                  // 辐射源高度（米，海拔）
      m_createdAt(""),                // 记录创建时间，默认当前时间
      m_lastUpdated("") {             // 最后更新时间，自动更新
}
// 构造函数，初始化辐射源参数为默认值
RadiationSource::~RadiationSource() {
}
// Getter和Setter方法
// 辐射源ID
void RadiationSource::setRadiationId(int id) {m_radiationId = id;}
int RadiationSource::getRadiationId() const {return m_radiationId;}

// 辐射源名称
void RadiationSource::setRadiationName(const std::string& name) {m_radiationName = name;}
std::string RadiationSource::getRadiationName() const {return m_radiationName;}

// 是否为固定的（非移动的）
// 注意：根据数据库触发器，如果设置为固定，则所有运动参数必须为0
void RadiationSource::setIsStationary(bool stationary) {
    m_isStationary = stationary;
    // 如果设置为固定辐射源，自动将运动参数重置为0
    if (stationary) {
        m_movementSpeed = 0.0;
        m_movementAzimuth = 0.0;
        m_movementElevation = 0.0;
    }
}
bool RadiationSource::getIsStationary() const {return m_isStationary;}

// 发射功率（单位：千瓦）
void RadiationSource::setTransmitPower(double power) {m_transmitPower = power;}
double RadiationSource::getTransmitPower() const {return m_transmitPower;}

// 扫描周期（单位：秒）
void RadiationSource::setScanPeriod(double period) {m_scanPeriod = period;}
double RadiationSource::getScanPeriod() const {return m_scanPeriod;}

// 载波频率（单位：GHz）
void RadiationSource::setCarrierFrequency(double freq) {m_carrierFrequency = freq;}
double RadiationSource::getCarrierFrequency() const {return m_carrierFrequency;}

//工作扇区方位角下限
void RadiationSource::setAzimuthStart(double start) {m_azimuthStart = start;}
double RadiationSource::getAzimuthStart() const {return m_azimuthStart;}

//工作扇区方位角上限
// 注意：根据数据库触发器，上限必须大于下限
void RadiationSource::setAzimuthEnd(double end) {
    if (end <= m_azimuthStart) {
        std::cerr << "错误：工作扇区方位角上限必须大于下限" << std::endl;
        return;
    }
    m_azimuthEnd = end;
}
double RadiationSource::getAzimuthEnd() const {return m_azimuthEnd;}

// 工作扇区俯仰角下限
void RadiationSource::setElevationStart(double start) {m_elevationStart = start;}
double RadiationSource::getElevationStart() const {return m_elevationStart;}

// 工作扇区俯仰角上限
// 注意：根据数据库触发器，上限必须大于下限
void RadiationSource::setElevationEnd(double end) {
    if (end <= m_elevationStart) {
        std::cerr << "错误：工作扇区俯仰角上限必须大于下限" << std::endl;
        return;
    }
    m_elevationEnd = end;
}
double RadiationSource::getElevationEnd() const {return m_elevationEnd;}

//运动速度
// 注意：根据数据库触发器，如果是固定辐射源，速度必须为0
void RadiationSource::setMovementSpeed(double speed) {
    if (m_isStationary && speed != 0) {
        std::cerr << "错误：固定辐射源的运动参数必须为0" << std::endl;
        return;
    }
    m_movementSpeed = speed;
}
double RadiationSource::getMovementSpeed() const {return m_movementSpeed;}

// 设置运动方位角（单位：度，正北为0，顺时针）
// 注意：根据数据库触发器，如果是固定辐射源，方位角必须为0
void RadiationSource::setMovementAzimuth(double azimuth) {
    if (m_isStationary && azimuth != 0) {
        std::cerr << "错误：固定辐射源的运动参数必须为0" << std::endl;
        return;
    }
    m_movementAzimuth = azimuth;
}
double RadiationSource::getMovementAzimuth() const {return m_movementAzimuth;}
// 设置运动俯仰角（单位：度，水平面为0，向上为正）
// 注意：根据数据库触发器，如果是固定辐射源，俯仰角必须为0
void RadiationSource::setMovementElevation(double elevation) {
    if (m_isStationary && elevation != 0) {
        std::cerr << "错误：固定辐射源的运动参数必须为0" << std::endl;
        return;
    }
    m_movementElevation = elevation;
}
double RadiationSource::getMovementElevation() const {return m_movementElevation;}

// 经度（单位：度，范围-180~180）
void RadiationSource::setLongitude(double longitude) {
    if (longitude < -180 || longitude > 180) {
        std::cerr << "警告：经度超出有效范围(-180~180)" << std::endl;
    }
    m_longitude = longitude;
}
double RadiationSource::getLongitude() const {return m_longitude;}

// 纬度（单位：度，范围-90~90）
void RadiationSource::setLatitude(double latitude) {
    if (latitude < -90 || latitude > 90) {
        std::cerr << "警告：纬度超出有效范围(-90~90)" << std::endl;
    }
    m_latitude = latitude;
}
double RadiationSource::getLatitude() const {return m_latitude;}

// 高度（单位：米，海拔）
void RadiationSource::setAltitude(double altitude) {m_altitude = altitude;}
double RadiationSource::getAltitude() const {return m_altitude;}

// 创建时间
void RadiationSource::setCreatedAt(const std::string& time) {m_createdAt = time;}
std::string RadiationSource::getCreatedAt() const {return m_createdAt;}

// 最后更新时间
void RadiationSource::setLastUpdated(const std::string& time) {m_lastUpdated = time;}
std::string RadiationSource::getLastUpdated() const {return m_lastUpdated;}


