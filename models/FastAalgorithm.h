#pragma once

#include "ReconnaissanceDeviceModel.h"
#include "RadiationSourceModel.h"
#include <vector>
#include <string>

struct LocationResult;

class FastAalgorithm {
public:
    static FastAalgorithm& getInstance();

    LocationResult runSimulation(const ReconnaissanceDevice& device,
                                 const RadiationSource& source,
                                 int simulationTime,
                                 const std::string& techSystem);
private:
    FastAalgorithm() = default;
    ~FastAalgorithm() = default;
    FastAalgorithm(const FastAalgorithm&) = delete;
    FastAalgorithm& operator=(const FastAalgorithm&) = delete;
};


