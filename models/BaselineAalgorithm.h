#pragma once

#include "ReconnaissanceDeviceModel.h"
#include "RadiationSourceModel.h"
#include <vector>
#include <string>

struct LocationResult;

class BaselineAalgorithm {
public:
    static BaselineAalgorithm& getInstance();

    LocationResult runSimulation(const ReconnaissanceDevice& device,
                                 const RadiationSource& source,
                                 int simulationTime,
                                 const std::string& techSystem);
private:
    BaselineAalgorithm() = default;
    ~BaselineAalgorithm() = default;
    BaselineAalgorithm(const BaselineAalgorithm&) = delete;
    BaselineAalgorithm& operator=(const BaselineAalgorithm&) = delete;
};


