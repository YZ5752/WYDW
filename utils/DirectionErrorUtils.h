#pragma once

#include <vector>
#include "../models/ReconnaissanceDeviceModel.h"
#include "../models/RadiationSourceModel.h"

namespace DirectionErrorUtils {
	// 干涉仪体制误差计算（完全按图片与现有常量/查表实现）
	// 返回: [Δem, σ_α, σ_β, σ_θ(deg), Δθ(deg)]
	std::vector<double> calculateInterferometerErrors(
		const ReconnaissanceDevice& device,
		const RadiationSource& source,
		double azimuthDeg,
		double elevationDeg);

	// 时差体制误差计算（完全按图片公式）
	// 公式：σ_τ = sqrt(σ_τφ^2 + σ_τn^2 + σ_τd^2)
	//       σ_τφ = (1/(360°·f0))·σ_φ，σ_τn = 0.175/(Bv·sqrt(2·SNR))，σ_τd = 2/(2√3·fs)
	//       σ_θ = (c/(d·cosθ))·σ_τ  （最终输出为度）
	// 返回: [σ_τ(ns), 角位置测量误差(度,由位置精度换算,可为0), σ_φ(ns等价), 多径误差(占位), σ_θ(度), 定位误差(米)]
	std::vector<double> calculateTDOAErrors(
		double baselineLength,
		double incidentAngleRad,
		double estimatedDistance,
		// 可选物理参数，若传入<=0使用默认值
		double carrierFreqHz = 100e6,
		double phaseErrorDeg = 1.0,
		double bandwidthHz = 10e6,
		double snrLinear = 100.0,
		double samplingRateHz = 10e6);
} 