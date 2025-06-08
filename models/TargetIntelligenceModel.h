#ifndef TARGET_INTELLIGENCE_MODEL_H
#define TARGET_INTELLIGENCE_MODEL_H

#include "CoordinateModel.h"
#include <string>

// 目标情报数据
class TargetIntelligence {
public:
    TargetIntelligence();
    ~TargetIntelligence();
    
    void setTargetId(int id);
    int getTargetId() const;
    
    void setTargetName(const std::string& name);
    std::string getTargetName() const;
    
    void setPosition(const Coordinate& position);
    Coordinate getPosition() const;
    
    void setFrequency(double freq);
    double getFrequency() const;
    
    void setPower(double power);
    double getPower() const;
    
    void setDetectionTime(const std::string& time);
    std::string getDetectionTime() const;
    
    void setConfidenceLevel(double level);
    double getConfidenceLevel() const;
    
private:
    int m_targetId;
    std::string m_targetName;
    Coordinate m_position;
    double m_frequency;
    double m_power;
    std::string m_detectionTime;
    double m_confidenceLevel;
};

#endif // TARGET_INTELLIGENCE_MODEL_H 