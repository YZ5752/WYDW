#include "../include/radiation_source_model.h"
#include <iostream>
#include <cstdio>
#include <utility>

// RadiationSource实现
RadiationSource::RadiationSource()
    : m_radiationId(0),
      m_radiationName(""),
      m_isStationary(true),
      m_deviceType("雷达站"),
      m_transmitPower(100.0),
      m_scanPeriod(5.0),
      m_carrierFrequency(0.0),
      m_freqMin(1000.0),
      m_freqMax(2000.0),
      m_workSector(90.0),
      m_azimuthStart(0.0),
      m_azimuthEnd(360.0),
      m_elevationStart(-90.0),
      m_elevationEnd(90.0),
      m_movementSpeed(0.0),
      m_movementAzimuth(0.0),
      m_movementElevation(0.0),
      m_longitude(0.0),
      m_latitude(0.0),
      m_altitude(0.0) {
}

RadiationSource::~RadiationSource() {
}

void RadiationSource::setRadiationId(int id) {
    m_radiationId = id;
}

int RadiationSource::getRadiationId() const {
    return m_radiationId;
}

void RadiationSource::setRadiationName(const std::string& name) {
    m_radiationName = name;
}

std::string RadiationSource::getRadiationName() const {
    return m_radiationName;
}

void RadiationSource::setIsStationary(bool stationary) {
    m_isStationary = stationary;
}

bool RadiationSource::getIsStationary() const {
    return m_isStationary;
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

void RadiationSource::setCarrierFrequency(double freq) {
    m_carrierFrequency = freq;
}

double RadiationSource::getCarrierFrequency() const {
    return m_carrierFrequency;
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

void RadiationSource::setAzimuthRange(double start, double end) {
    m_azimuthStart = start;
    m_azimuthEnd = end;
}

std::pair<double, double> RadiationSource::getAzimuthRange() const {
    return std::make_pair(m_azimuthStart, m_azimuthEnd);
}

void RadiationSource::setElevationRange(double start, double end) {
    m_elevationStart = start;
    m_elevationEnd = end;
}

std::pair<double, double> RadiationSource::getElevationRange() const {
    return std::make_pair(m_elevationStart, m_elevationEnd);
}

void RadiationSource::setMovementSpeed(double speed) {
    m_movementSpeed = speed;
}

double RadiationSource::getMovementSpeed() const {
    return m_movementSpeed;
}

void RadiationSource::setMovementAzimuth(double azimuth) {
    m_movementAzimuth = azimuth;
}

double RadiationSource::getMovementAzimuth() const {
    return m_movementAzimuth;
}

void RadiationSource::setMovementElevation(double elevation) {
    m_movementElevation = elevation;
}

double RadiationSource::getMovementElevation() const {
    return m_movementElevation;
}

void RadiationSource::setLongitude(double longitude) {
    m_longitude = longitude;
}

double RadiationSource::getLongitude() const {
    return m_longitude;
}

void RadiationSource::setLatitude(double latitude) {
    m_latitude = latitude;
}

double RadiationSource::getLatitude() const {
    return m_latitude;
}

void RadiationSource::setAltitude(double altitude) {
    m_altitude = altitude;
}

double RadiationSource::getAltitude() const {
    return m_altitude;
}

// 数据库操作函数实现（简化版）
std::vector<RadiationSource> getAllRadiationSources() {
    return std::vector<RadiationSource>();
}

bool addRadiationSource(const RadiationSource& source) {
    return true;
}

bool updateRadiationSource(const RadiationSource& source) {
    return true;
}

bool deleteRadiationSource(int sourceId) {
    return true;
}
