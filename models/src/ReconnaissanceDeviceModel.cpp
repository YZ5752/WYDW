#include "../ReconnaissanceDeviceModel.h"
#include <iostream>
#include <cstdio>

// ReconnaissanceDevice类实现 - 侦察设备模型（对应数据库表reconnaissance_device_models）
ReconnaissanceDevice::ReconnaissanceDevice()
    : m_deviceId(0),                 // 设备ID（主键）
      m_deviceName(""),              // 设备名称
      m_isStationary(true),          // 是否为固定设备，默认为固定
      m_baselineLength(0),           // 基线长度（米）
      m_noisePsd(0),                 // 噪声功率谱密度（dBm/Hz）
      m_sampleRate(0),               // 采样速率（GHz）
      m_freqRangeMin(0),             // 侦收频率范围下限（GHz）
      m_freqRangeMax(0),             // 侦收频率范围上限（GHz）
      m_angleAzimuthMin(0),          // 方位角下限（度，0~360）
      m_angleAzimuthMax(360),          // 方位角上限（度，0~360）
      m_angleElevationMin(-90),        // 俯仰角下限（度，-90~90）
      m_angleElevationMax(90),        // 俯仰角上限（度，-90~90）
      m_movementSpeed(0),            // 运动速度（米/秒）
      m_movementAzimuth(0),          // 运动方位角（度，正北为0，顺时针）
      m_movementElevation(0),        // 运动俯仰角（度，水平面为0，向上为正）
      m_longitude(0),                // 经度（度，范围-180~180）
      m_latitude(0),                 // 纬度（度，范围-90~90）
      m_altitude(0),                 // 高度（米，海拔）
      m_createdAt(""),               // 记录创建时间
      m_updatedAt("") {              // 记录更新时间
         
}
// 构造函数，初始化设备参数为默认值
ReconnaissanceDevice::~ReconnaissanceDevice() {
}

// Getter和Setter方法
// 设备ID
void ReconnaissanceDevice::setDeviceId(int id) { m_deviceId = id; }
int ReconnaissanceDevice::getDeviceId() const { return m_deviceId; }

// 设备名称
void ReconnaissanceDevice::setDeviceName(const std::string& name) { m_deviceName = name; }
std::string ReconnaissanceDevice::getDeviceName() const { return m_deviceName; }

// 是否为固定的（非移动的）
// 如果设置为固定，则所有运动参数必须为0
void ReconnaissanceDevice::setIsStationary(bool s) { 
    m_isStationary = s; 
    // 如果设置为固定设备，自动将运动参数重置为0
    if (s) {
        m_movementSpeed = 0;
        m_movementAzimuth = 0;
        m_movementElevation = 0;
    }
}
bool ReconnaissanceDevice::getIsStationary() const { return m_isStationary; }

// 基线长度
void ReconnaissanceDevice::setBaselineLength(float v) { m_baselineLength = v; }
float ReconnaissanceDevice::getBaselineLength() const { return m_baselineLength; }

// 侦收频率范围下限（单位：GHz）
void ReconnaissanceDevice::setFreqRangeMin(float v) { m_freqRangeMin = v; }
float ReconnaissanceDevice::getFreqRangeMin() const { return m_freqRangeMin; }

// 侦收频率范围上限（单位：GHz）
// 注意：根据数据库触发器，上限必须大于下限
void ReconnaissanceDevice::setFreqRangeMax(float v) { 
    if (v <= m_freqRangeMin) {
        std::cerr << "错误：侦收频率范围上限必须大于下限" << std::endl;
        return;
    }
    m_freqRangeMax = v; 
}
float ReconnaissanceDevice::getFreqRangeMax() const { return m_freqRangeMax; }

// 方位角下限（单位：度，范围0~360）
void ReconnaissanceDevice::setAngleAzimuthMin(float v) { m_angleAzimuthMin = v; }
float ReconnaissanceDevice::getAngleAzimuthMin() const { return m_angleAzimuthMin; }

// 方位角上限（单位：度，范围0~360）
// 注意：根据数据库触发器，上限必须大于下限
void ReconnaissanceDevice::setAngleAzimuthMax(float v) { 
    if (v <= m_angleAzimuthMin) {
        std::cerr << "错误：方位角上限必须大于下限" << std::endl;
        return;
    }
    m_angleAzimuthMax = v; 
}
float ReconnaissanceDevice::getAngleAzimuthMax() const { return m_angleAzimuthMax; }

// 俯仰角下限（单位：度，范围-90~90）
void ReconnaissanceDevice::setAngleElevationMin(float v) { m_angleElevationMin = v; }
float ReconnaissanceDevice::getAngleElevationMin() const { return m_angleElevationMin; }

// 俯仰角上限（单位：度，范围-90~90）
// 注意：根据数据库触发器，上限必须大于下限
void ReconnaissanceDevice::setAngleElevationMax(float v) { 
    if (v <= m_angleElevationMin) {
        std::cerr << "错误：俯仰角上限必须大于下限" << std::endl;
        return;
    }
    m_angleElevationMax = v; 
}
float ReconnaissanceDevice::getAngleElevationMax() const { return m_angleElevationMax; }

// 噪声功率谱密度（单位：dBm/Hz）
void ReconnaissanceDevice::setNoisePsd(float v) { m_noisePsd = v; }
float ReconnaissanceDevice::getNoisePsd() const { return m_noisePsd; }

// 采样速率（单位：GHz）- 必须大于信号频率的2倍
void ReconnaissanceDevice::setSampleRate(float v) { m_sampleRate = v; }
float ReconnaissanceDevice::getSampleRate() const { return m_sampleRate; }

// 运动速度（单位：米/秒）
// 注意：根据数据库触发器，如果是固定设备，速度必须为0
void ReconnaissanceDevice::setMovementSpeed(float v) { 
    if (m_isStationary && v != 0) {
        std::cerr << "错误：固定设备的运动速度必须为0" << std::endl;
        return;
    }
    m_movementSpeed = v; 
}
float ReconnaissanceDevice::getMovementSpeed() const { return m_movementSpeed; }

// 运动方位角（单位：度，正北为0，顺时针）
// 注意：根据数据库触发器，如果是固定设备，方位角必须为0
void ReconnaissanceDevice::setMovementAzimuth(float v) { 
    if (m_isStationary && v != 0) {
        std::cerr << "错误：固定设备的运动方位角必须为0" << std::endl;
        return;
    }
    m_movementAzimuth = v; 
}
float ReconnaissanceDevice::getMovementAzimuth() const { return m_movementAzimuth; }

// 运动俯仰角（单位：度，水平面为0，向上为正）
// 注意：根据数据库触发器，如果是固定设备，俯仰角必须为0
void ReconnaissanceDevice::setMovementElevation(float v) { 
    if (m_isStationary && v != 0) {
        std::cerr << "错误：固定设备的运动俯仰角必须为0" << std::endl;
        return;
    }
    m_movementElevation = v; 
}
float ReconnaissanceDevice::getMovementElevation() const { return m_movementElevation; }

// 经度（单位：度，范围-180~180）
void ReconnaissanceDevice::setLongitude(double v) { 
    if (v < -180 || v > 180) {
        std::cerr << "警告：经度超出有效范围(-180~180)" << std::endl;
    }
    m_longitude = v; 
}
double ReconnaissanceDevice::getLongitude() const { return m_longitude; }

// 纬度（单位：度，范围-90~90）
void ReconnaissanceDevice::setLatitude(double v) { 
    if (v < -90 || v > 90) {
        std::cerr << "警告：纬度超出有效范围(-90~90)" << std::endl;
    }
    m_latitude = v; 
}
double ReconnaissanceDevice::getLatitude() const { return m_latitude; }

// 高度（单位：米，海拔）
void ReconnaissanceDevice::setAltitude(double v) { m_altitude = v; }
double ReconnaissanceDevice::getAltitude() const { return m_altitude; }

// 创建时间
void ReconnaissanceDevice::setCreatedAt(const std::string& time) { m_createdAt = time; }
std::string ReconnaissanceDevice::getCreatedAt() const { return m_createdAt; }

// 更新时间
void ReconnaissanceDevice::setUpdatedAt(const std::string& time) { m_updatedAt = time; }
std::string ReconnaissanceDevice::getUpdatedAt() const { return m_updatedAt; }

// 设备类型（固定或移动）
std::string ReconnaissanceDevice::getDeviceTypeString() const {
    return m_isStationary ? "固定设备" : "移动设备";
}

// 频率范围
std::string ReconnaissanceDevice::getFreqRangeString() const {
    char buf[64];
    snprintf(buf, sizeof(buf), "%.2f-%.2f", m_freqRangeMin, m_freqRangeMax);
    return std::string(buf);
}

// 获取角度范围的格式化字符串，包含方位角和俯仰角
std::string ReconnaissanceDevice::getAngleRangeString() const {
    char buf[128];
    snprintf(buf, sizeof(buf), "方位角：%.0f-%.0f，俯仰角：%.0f-%.0f",
             m_angleAzimuthMin, m_angleAzimuthMax, m_angleElevationMin, m_angleElevationMax);
    return std::string(buf);
}