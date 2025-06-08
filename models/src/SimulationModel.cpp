#include "../SimulationModel.h"
#include <iostream>
#include <cmath>
#include <random>

// SinglePlatformSimulation实现
SinglePlatformSimulation::SinglePlatformSimulation() {
    m_platformPosition = {0.0, 0.0, 0.0};
    m_algorithm = "快速定位";
}

SinglePlatformSimulation::~SinglePlatformSimulation() {
}

void SinglePlatformSimulation::setRadarDevice(const ReconnaissanceDevice& device) {
    m_device = device;
}

void SinglePlatformSimulation::setRadiationSource(const RadiationSource& source) {
    m_source = source;
}

void SinglePlatformSimulation::setPlatformPosition(const Coordinate& position) {
    m_platformPosition = position;
}

LocationResult SinglePlatformSimulation::runSimulation() {
    std::cout << "Running single platform simulation..." << std::endl;
    return calculateLocation();
}

LocationResult SinglePlatformSimulation::calculateLocation() {
    LocationResult result;
    
    // 简化的定位结果计算
    result.position = {100.0, 200.0, 0.0};  // 示例坐标
    result.power = 0.95;
    result.directionError = 0.01;
    result.parameterError = 0.015;
    result.time = 1.5;  // 示例定位时间(秒)
    
    return result;
}

// MultiPlatformSimulation实现
MultiPlatformSimulation::MultiPlatformSimulation() {
    m_algorithm = "时差定位";
}

MultiPlatformSimulation::~MultiPlatformSimulation() {
}

void MultiPlatformSimulation::addPlatform(const Coordinate& position, const ReconnaissanceDevice& device) {
    m_platforms.push_back(std::make_pair(position, device));
}

void MultiPlatformSimulation::setRadiationSource(const RadiationSource& source) {
    m_source = source;
}

void MultiPlatformSimulation::setAlgorithm(const std::string& algorithm) {
    m_algorithm = algorithm;
}

LocationResult MultiPlatformSimulation::runSimulation() {
    std::cout << "Running multi-platform simulation with algorithm: " << m_algorithm << std::endl;
    return calculateCooperativeLocation();
}

LocationResult MultiPlatformSimulation::calculateCooperativeLocation() {
    LocationResult result;
    
    // 简化的协同定位结果计算
    result.position = {150.0, 250.0, 0.0};  // 示例坐标
    result.power = 0.95;  // 协同定位通常会提高威力
    result.directionError = 0.01;  // 协同定位通常会降低误差
    result.parameterError = 0.015;
    result.time = 1.0;  // 协同定位通常会缩短定位时间
    
    return result;
}

// SimulationManager实现
SimulationManager& SimulationManager::getInstance() {
    static SimulationManager instance;
    return instance;
}

SimulationManager::SimulationManager() 
    : m_mode(SimulationMode::PRE_MISSION) {
}

SimulationManager::~SimulationManager() {
}

void SimulationManager::setSimulationMode(SimulationMode mode) {
    m_mode = mode;
}

SinglePlatformSimulation& SimulationManager::getSinglePlatformSimulation() {
    return m_singleSim;
}

MultiPlatformSimulation& SimulationManager::getMultiPlatformSimulation() {
    return m_multiSim;
}

bool SimulationManager::saveResults(const LocationResult& result) {
    // 简化实现
    std::cout << "Saving simulation results..." << std::endl;
    return true;
}

std::vector<LocationResult> SimulationManager::loadResults() {
    // 简化实现
    std::cout << "Loading simulation results..." << std::endl;
    return std::vector<LocationResult>();
} 