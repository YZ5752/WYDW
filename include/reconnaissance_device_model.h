#pragma once

#include <string>
#include <vector>
#include <gtk/gtk.h>

// 雷达侦察设备模型
class ReconnaissanceDevice {
public:
    ReconnaissanceDevice();
    ~ReconnaissanceDevice();

    // 字段setter/getter
    void setDeviceId(int id); int getDeviceId() const;
    void setDeviceName(const std::string& name); std::string getDeviceName() const;
    void setIsStationary(bool s); bool getIsStationary() const;
    void setBaselineLength(float v); float getBaselineLength() const;
    void setFreqRangeMin(float v); float getFreqRangeMin() const;
    void setFreqRangeMax(float v); float getFreqRangeMax() const;
    void setAngleAzimuthMin(float v); float getAngleAzimuthMin() const;
    void setAngleAzimuthMax(float v); float getAngleAzimuthMax() const;
    void setAngleElevationMin(float v); float getAngleElevationMin() const;
    void setAngleElevationMax(float v); float getAngleElevationMax() const;
    void setNoisePsd(float v); float getNoisePsd() const;
    void setSampleRate(float v); float getSampleRate() const;
    void setMovementSpeed(float v); float getMovementSpeed() const;
    void setMovementAzimuth(float v); float getMovementAzimuth() const;
    void setMovementElevation(float v); float getMovementElevation() const;
    void setLongitude(double v); double getLongitude() const;
    void setLatitude(double v); double getLatitude() const;
    void setAltitude(double v); double getAltitude() const;

    // 展示辅助
    std::string getDeviceTypeString() const;
    std::string getFreqRangeString() const;
    std::string getAngleRangeString() const;

    // 计算辅助方法
    double calculatePower() const;
    double calculateDirectionError() const;
    double calculateParameterError() const;

private:
    int m_deviceId;
    std::string m_deviceName;
    bool m_isStationary;
    float m_baselineLength;
    float m_freqRangeMin;
    float m_freqRangeMax;
    float m_angleAzimuthMin;
    float m_angleAzimuthMax;
    float m_angleElevationMin;
    float m_angleElevationMax;
    float m_noisePsd;
    float m_sampleRate;
    float m_movementSpeed;
    float m_movementAzimuth;
    float m_movementElevation;
    double m_longitude;
    double m_latitude;
    double m_altitude;
};

// 获取所有侦察设备
std::vector<ReconnaissanceDevice> getAllReconnaissanceDevices();
// 新增
bool addReconnaissanceDevice(const ReconnaissanceDevice& device);
// 编辑
bool updateReconnaissanceDevice(const ReconnaissanceDevice& device);
// 删除
bool deleteReconnaissanceDevice(int deviceId); 