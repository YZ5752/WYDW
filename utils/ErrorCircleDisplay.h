#pragma once
#include <vector>
#include "CoordinateTransform.h"

class MapView;

// 在地图上显示误差点
void showErrorPointsOnMap(MapView* mapView, const std::vector<COORD3>& points);

// 在地图上显示误差圆
void showErrorCircleOnMap(MapView* mapView, const COORD3& center, double radius); 