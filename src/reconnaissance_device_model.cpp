#include "../include/reconnaissance_device_model.h"
#include <iostream>
#include <cstdio>

// ReconnaissanceDevice实现
ReconnaissanceDevice::ReconnaissanceDevice()
    : m_deviceId(0), m_isStationary(true), m_baselineLength(0),
      m_freqRangeMin(0), m_freqRangeMax(0),
      m_angleAzimuthMin(0), m_angleAzimuthMax(0),
      m_angleElevationMin(0), m_angleElevationMax(0) {}

ReconnaissanceDevice::~ReconnaissanceDevice() {
}

void ReconnaissanceDevice::setDeviceId(int id) { m_deviceId = id; }
int ReconnaissanceDevice::getDeviceId() const { return m_deviceId; }
void ReconnaissanceDevice::setDeviceName(const std::string& name) { m_deviceName = name; }
std::string ReconnaissanceDevice::getDeviceName() const { return m_deviceName; }
void ReconnaissanceDevice::setIsStationary(bool s) { m_isStationary = s; }
bool ReconnaissanceDevice::getIsStationary() const { return m_isStationary; }
void ReconnaissanceDevice::setBaselineLength(float v) { m_baselineLength = v; }
float ReconnaissanceDevice::getBaselineLength() const { return m_baselineLength; }
void ReconnaissanceDevice::setFreqRangeMin(float v) { m_freqRangeMin = v; }
float ReconnaissanceDevice::getFreqRangeMin() const { return m_freqRangeMin; }
void ReconnaissanceDevice::setFreqRangeMax(float v) { m_freqRangeMax = v; }
float ReconnaissanceDevice::getFreqRangeMax() const { return m_freqRangeMax; }
void ReconnaissanceDevice::setAngleAzimuthMin(float v) { m_angleAzimuthMin = v; }
float ReconnaissanceDevice::getAngleAzimuthMin() const { return m_angleAzimuthMin; }
void ReconnaissanceDevice::setAngleAzimuthMax(float v) { m_angleAzimuthMax = v; }
float ReconnaissanceDevice::getAngleAzimuthMax() const { return m_angleAzimuthMax; }
void ReconnaissanceDevice::setAngleElevationMin(float v) { m_angleElevationMin = v; }
float ReconnaissanceDevice::getAngleElevationMin() const { return m_angleElevationMin; }
void ReconnaissanceDevice::setAngleElevationMax(float v) { m_angleElevationMax = v; }
float ReconnaissanceDevice::getAngleElevationMax() const { return m_angleElevationMax; }
void ReconnaissanceDevice::setNoisePsd(float v) { m_noisePsd = v; }
float ReconnaissanceDevice::getNoisePsd() const { return m_noisePsd; }
void ReconnaissanceDevice::setSampleRate(float v) { m_sampleRate = v; }
float ReconnaissanceDevice::getSampleRate() const { return m_sampleRate; }
void ReconnaissanceDevice::setMovementSpeed(float v) { m_movementSpeed = v; }
float ReconnaissanceDevice::getMovementSpeed() const { return m_movementSpeed; }
void ReconnaissanceDevice::setMovementAzimuth(float v) { m_movementAzimuth = v; }
float ReconnaissanceDevice::getMovementAzimuth() const { return m_movementAzimuth; }
void ReconnaissanceDevice::setMovementElevation(float v) { m_movementElevation = v; }
float ReconnaissanceDevice::getMovementElevation() const { return m_movementElevation; }
void ReconnaissanceDevice::setLongitude(double v) { m_longitude = v; }
double ReconnaissanceDevice::getLongitude() const { return m_longitude; }
void ReconnaissanceDevice::setLatitude(double v) { m_latitude = v; }
double ReconnaissanceDevice::getLatitude() const { return m_latitude; }
void ReconnaissanceDevice::setAltitude(double v) { m_altitude = v; }
double ReconnaissanceDevice::getAltitude() const { return m_altitude; }

std::string ReconnaissanceDevice::getDeviceTypeString() const {
    return m_isStationary ? "固定设备" : "移动设备";
}
std::string ReconnaissanceDevice::getFreqRangeString() const {
    char buf[64];
    snprintf(buf, sizeof(buf), "%.2f-%.2f", m_freqRangeMin, m_freqRangeMax);
    return std::string(buf);
}
std::string ReconnaissanceDevice::getAngleRangeString() const {
    char buf[128];
    snprintf(buf, sizeof(buf), "方位角：%.2f-%.2f，俯仰角%.2f-%.2f",
             m_angleAzimuthMin, m_angleAzimuthMax, m_angleElevationMin, m_angleElevationMax);
    return std::string(buf);
}

double ReconnaissanceDevice::calculatePower() const {
    // 简化实现
    return 0.85;
}

double ReconnaissanceDevice::calculateDirectionError() const {
    // 简化实现
    return 0.02;
}

double ReconnaissanceDevice::calculateParameterError() const {
    // 简化实现
    return 0.03;
}

// 数据库操作函数
std::vector<ReconnaissanceDevice> getAllReconnaissanceDevices() {
    return std::vector<ReconnaissanceDevice>();
}

bool addReconnaissanceDevice(const ReconnaissanceDevice& device) {
    return true;
}

bool updateReconnaissanceDevice(const ReconnaissanceDevice& device) {
    return true;
}

bool deleteReconnaissanceDevice(int deviceId) {
    return true;
} 