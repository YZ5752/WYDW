#include "../include/radar_model.h"
#include <iostream>

// RadarDevice实现
RadarDevice::RadarDevice()
    : m_deviceType("雷达"),
      m_deviceCount(1),
      m_technicalSystem("时差"),
      m_antennaLength(0.0),
      m_noiseLevel(0.0),
      m_workStartTime("00:00:00"),
      m_workEndTime("23:59:59"),
      m_freqMin(0.0),
      m_freqMax(0.0),
      m_angleMin(0.0),
      m_angleMax(360.0) {
}

RadarDevice::~RadarDevice() {
}

void RadarDevice::setDeviceType(const std::string& type) {
    m_deviceType = type;
}

std::string RadarDevice::getDeviceType() const {
    return m_deviceType;
}

void RadarDevice::setDeviceCount(int count) {
    m_deviceCount = count;
}

int RadarDevice::getDeviceCount() const {
    return m_deviceCount;
}

void RadarDevice::setTechnicalSystem(const std::string& system) {
    m_technicalSystem = system;
}

std::string RadarDevice::getTechnicalSystem() const {
    return m_technicalSystem;
}

void RadarDevice::setAntennaLength(double length) {
    m_antennaLength = length;
}

double RadarDevice::getAntennaLength() const {
    return m_antennaLength;
}

void RadarDevice::setNoiseLevel(double noise) {
    m_noiseLevel = noise;
}

double RadarDevice::getNoiseLevel() const {
    return m_noiseLevel;
}

void RadarDevice::setWorkTimeRange(const std::string& startTime, const std::string& endTime) {
    m_workStartTime = startTime;
    m_workEndTime = endTime;
}

std::pair<std::string, std::string> RadarDevice::getWorkTimeRange() const {
    return std::make_pair(m_workStartTime, m_workEndTime);
}

void RadarDevice::setFrequencyRange(double min, double max) {
    m_freqMin = min;
    m_freqMax = max;
}

std::pair<double, double> RadarDevice::getFrequencyRange() const {
    return std::make_pair(m_freqMin, m_freqMax);
}

void RadarDevice::setAngleRange(double min, double max) {
    m_angleMin = min;
    m_angleMax = max;
}

std::pair<double, double> RadarDevice::getAngleRange() const {
    return std::make_pair(m_angleMin, m_angleMax);
}

double RadarDevice::calculatePower() const {
    // 简化实现
    return 0.85;
}

double RadarDevice::calculateDirectionError() const {
    // 简化实现
    return 0.02;
}

double RadarDevice::calculateParameterError() const {
    // 简化实现
    return 0.03;
}

// RadiationSource实现
RadiationSource::RadiationSource()
    : m_deviceType("雷达站"),
      m_transmitPower(100.0),
      m_scanPeriod(5.0),
      m_freqMin(1000.0),
      m_freqMax(2000.0),
      m_workSector(90.0) {
}

RadiationSource::~RadiationSource() {
}

void RadiationSource::setDeviceType(const std::string& type) {
    m_deviceType = type;
}

std::string RadiationSource::getDeviceType() const {
    return m_deviceType;
}

void RadiationSource::setTransmitPower(double power) {
    m_transmitPower = power;
}

double RadiationSource::getTransmitPower() const {
    return m_transmitPower;
}

void RadiationSource::setScanPeriod(double period) {
    m_scanPeriod = period;
}

double RadiationSource::getScanPeriod() const {
    return m_scanPeriod;
}

void RadiationSource::setFrequencyRange(double min, double max) {
    m_freqMin = min;
    m_freqMax = max;
}

std::pair<double, double> RadiationSource::getFrequencyRange() const {
    return std::make_pair(m_freqMin, m_freqMax);
}

void RadiationSource::setWorkSector(double sector) {
    m_workSector = sector;
}

double RadiationSource::getWorkSector() const {
    return m_workSector;
} 