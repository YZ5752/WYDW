#pragma once

// 坐标点模型
struct Coordinate {
    double x;
    double y;
    double z;
    
    Coordinate() : x(0), y(0), z(0) {}
    Coordinate(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
}; 