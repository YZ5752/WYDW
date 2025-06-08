#pragma once

#include <string>
#include <vector>
#include <gtk/gtk.h>

// 雷达侦察设备模型
class ReconnaissanceDevice {
public:
    ReconnaissanceDevice();
    ~ReconnaissanceDevice();

    // 设备id
    void setDeviceId(int id); 
    int getDeviceId() const;

    //设备名称
    void setDeviceName(const std::string& name); 
    std::string getDeviceName() const;

    //是否固定
    void setIsStationary(bool s); 
    bool getIsStationary() const;

    //基线长度
    void setBaselineLength(float v); 
    float getBaselineLength() const;

    //噪声功率谱密度
    void setNoisePsd(float v); 
    float getNoisePsd() const;

    //采样速率
    void setSampleRate(float v); 
    float getSampleRate() const;

    //侦收频率范围下限
    void setFreqRangeMin(float v); 
    float getFreqRangeMin() const;

    //侦收频率范围上限
    void setFreqRangeMax(float v); 
    float getFreqRangeMax() const;

    //方位角下限
    void setAngleAzimuthMin(float v); 
    float getAngleAzimuthMin() const;

    //方位角上限
    void setAngleAzimuthMax(float v); 
    float getAngleAzimuthMax() const;

    //俯仰角下限
    void setAngleElevationMin(float v); 
    float getAngleElevationMin() const;

    //俯仰角上限
    void setAngleElevationMax(float v); 
    float getAngleElevationMax() const;  

     //运动速度
    void setMovementSpeed(float v);
    float getMovementSpeed() const;

    //运动方位角
    void setMovementAzimuth(float v); 
    float getMovementAzimuth() const;

    //运动俯仰角
    void setMovementElevation(float v); 
    float getMovementElevation() const;

    //经度
    void setLongitude(double v); 
    double getLongitude() const;

    //纬度
    void setLatitude(double v); 
    double getLatitude() const;

    //高度
    void setAltitude(double v); 
    double getAltitude() const;

    //创建时间
    void setCreatedAt(const std::string& time); 
    std::string getCreatedAt() const;

    //更新时间
    void setUpdatedAt(const std::string& time); 
    std::string getUpdatedAt() const;

    // 展示辅助
    std::string getDeviceTypeString() const;
    std::string getFreqRangeString() const;
    std::string getAngleRangeString() const;


private:
    int m_deviceId;
    std::string m_deviceName;
    bool m_isStationary;
    float m_baselineLength;
    float m_noisePsd;
    float m_sampleRate;
    float m_freqRangeMin;
    float m_freqRangeMax;
    float m_angleAzimuthMin;
    float m_angleAzimuthMax;
    float m_angleElevationMin;
    float m_angleElevationMax;
    float m_movementSpeed;
    float m_movementAzimuth;
    float m_movementElevation;
    double m_longitude;
    double m_latitude;
    double m_altitude;
    std::string m_createdAt;
    std::string m_updatedAt;
};

// 获取所有侦察设备
std::vector<ReconnaissanceDevice> getAllReconnaissanceDevices();
// 新增
bool addReconnaissanceDevice(const ReconnaissanceDevice& device);
// 编辑
bool updateReconnaissanceDevice(const ReconnaissanceDevice& device);
// 删除
bool deleteReconnaissanceDevice(int deviceId); 