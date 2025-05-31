#include "../include/radar_model.h"
#include <iostream>

// RadarDevice实现
RadarDevice::RadarDevice()
    : m_technicalSystem("干涉仪体制"),
      m_algorithm("快速定位") {
}

RadarDevice::~RadarDevice() {
}

void RadarDevice::setTechnicalSystem(const std::string& system) {
    m_technicalSystem = system;
}

std::string RadarDevice::getTechnicalSystem() const {
    return m_technicalSystem;
}

void RadarDevice::setAlgorithm(const std::string& algorithm) {
    m_algorithm = algorithm;
}

std::string RadarDevice::getAlgorithm() const {
    return m_algorithm;
}

void RadarDevice::setBasicParameters(const std::vector<double>& params) {
    m_basicParams = params;
}

std::vector<double> RadarDevice::getBasicParameters() const {
    return m_basicParams;
}

void RadarDevice::setWorkParameters(const std::vector<double>& params) {
    m_workParams = params;
}

std::vector<double> RadarDevice::getWorkParameters() const {
    return m_workParams;
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
    : m_transmitPower(100.0),
      m_scanPeriod(5.0),
      m_freqMin(1000.0),
      m_freqMax(2000.0),
      m_workSector(90.0) {
}

RadiationSource::~RadiationSource() {
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