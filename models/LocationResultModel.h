#pragma once

#include "CoordinateModel.h"

// 定位结果
struct LocationResult {
    Coordinate position;      // 定位坐标
    double power;            // 威力
    double directionError;   // 测向误差
    double parameterError;   // 参数测量误差
    double time;             // 定位时间
    
    LocationResult() : power(0), directionError(0), parameterError(0), time(0) {}
}; 