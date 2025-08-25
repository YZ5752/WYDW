#include "../DirectionErrorUtils.h"
#include "../CoordinateTransform.h"
#include "../../constants/PhysicsConstants.h"
#include <cmath>
#include <algorithm>

using namespace Constants;

namespace DirectionErrorUtils {

static double lookupConeEffect(double azimuthDegAbs, double elevationDegAbs) {
	// 复用 InterferometerPositioning 中的查表边界/表
	// 这里只写一个访问器，直接使用全局常量数组
	// 方位α归一化到[0,90]，仰角β直接取绝对值
	double alpha = fmod(std::abs(azimuthDegAbs), 180.0);
	if (alpha > 90.0) alpha = 180.0 - alpha;
	double beta  = std::abs(elevationDegAbs);
	// clamp to bounds
	double a = alpha;
	double b = beta;
	if (b > CONE_EFFECT_BETA_BOUNDS[CONE_EFFECT_BETA_LEVELS-1]) b = CONE_EFFECT_BETA_BOUNDS[CONE_EFFECT_BETA_LEVELS-1];
	if (a > CONE_EFFECT_ALPHA_BOUNDS[CONE_EFFECT_ALPHA_LEVELS-1]) a = CONE_EFFECT_ALPHA_BOUNDS[CONE_EFFECT_ALPHA_LEVELS-1];
	int bi = -1, ai = -1;
	for (int i=0;i<CONE_EFFECT_BETA_LEVELS;i++) { if (b <= CONE_EFFECT_BETA_BOUNDS[i]) { bi = i; break; } }
	for (int i=0;i<CONE_EFFECT_ALPHA_LEVELS;i++){ if (a <= CONE_EFFECT_ALPHA_BOUNDS[i]) { ai = i; break; } }
	double sigma_beta = 0.0;
	if (bi>=0 && ai>=0) sigma_beta = CONE_EFFECT_ERROR_TABLE[bi][ai];
	// 对超界进行简单比例放大（与模型一致）
	if (beta > CONE_EFFECT_BETA_BOUNDS[CONE_EFFECT_BETA_LEVELS-1] || alpha > CONE_EFFECT_ALPHA_BOUNDS[CONE_EFFECT_ALPHA_LEVELS-1]) {
		double scale = 1.0;
		if (beta > CONE_EFFECT_BETA_BOUNDS[CONE_EFFECT_BETA_LEVELS-1]) {
			double ratio = beta / CONE_EFFECT_BETA_BOUNDS[CONE_EFFECT_BETA_LEVELS-1];
			ratio = std::min(ratio, 2.0);
			scale *= ratio;
		}
		if (alpha > CONE_EFFECT_ALPHA_BOUNDS[CONE_EFFECT_ALPHA_LEVELS-1]) {
			double ratio = alpha / CONE_EFFECT_ALPHA_BOUNDS[CONE_EFFECT_ALPHA_LEVELS-1];
			ratio = std::min(ratio, 2.0);
			scale *= ratio;
		}
		sigma_beta *= scale;
	}
	return sigma_beta; // 单位：度
}

std::vector<double> calculateInterferometerErrors(
	const ReconnaissanceDevice& device,
	const RadiationSource& source,
	double azimuthDeg,
	double elevationDeg) {
	std::vector<double> errors;
	// 1) 对中误差 Δem、姿态误差 σ_α，来自常量
	double delta_em   = INTERFEROMETER_ALIGNMENT_ERROR;   // 度
	double sigma_alpha= INTERFEROMETER_ATTITUDE_ERROR;    // 度
	// 2) 圆锥效应 σ_β 查表
	double sigma_beta = lookupConeEffect(azimuthDeg, elevationDeg); // 度
	// 3) 阵列测向误差 σ_θ = (λ/(2π d cos θ))·σ_φ
	double d = std::max(1e-3, static_cast<double>(device.getBaselineLength()));
	double lambda = c / (source.getCarrierFrequency()*1e9);
	double theta_rad = azimuthDeg * DEG2RAD;
	double cos_theta = std::cos(theta_rad);
	if (std::abs(cos_theta) < 1e-6) cos_theta = 1e-6;
	double sigma_phi_rad = INTERFEROMETER_PHASE_ERROR * DEG2RAD; // 相位不一致性
	double sigma_theta_deg = (lambda / (2*M_PI*d*cos_theta)) * sigma_phi_rad * RAD2DEG;
	// 4) 综合测向误差 Δθ = sqrt(σ_α^2 + σ_β^2 + σ_θ^2 + Δem^2)
	double total = std::sqrt(sigma_alpha*sigma_alpha + sigma_beta*sigma_beta +
	                        sigma_theta_deg*sigma_theta_deg + delta_em*delta_em);
	errors.push_back(delta_em);
	errors.push_back(sigma_alpha);
	errors.push_back(sigma_beta);
	errors.push_back(sigma_theta_deg);
	errors.push_back(total);
	return errors;
}

std::vector<double> calculateTDOAErrors(
	double baselineLength,
	double incidentAngleRad,
	double estimatedDistance,
	double carrierFreqHz,
	double phaseErrorDeg,
	double bandwidthHz,
	double snrLinear,
	double samplingRateHz) {
	std::vector<double> errors;
	if (baselineLength < 1e-6) baselineLength = 1e-6;
	// 1) σ_τφ = (1/(360°·f0))·σ_φ
	double sigma_tau_phi = (phaseErrorDeg / 360.0) / carrierFreqHz; // 秒
	// 2) σ_τn = 0.175 / (Bv · sqrt(2·SNR))
	double sigma_tau_n = 0.175 / (bandwidthHz * std::sqrt(2.0 * std::max(1e-9, snrLinear)));
	// 3) σ_τd = 2 / (2√3 · fs)
	double sigma_tau_d = 2.0 / (2.0 * std::sqrt(3.0) * samplingRateHz);
	// 综合时差测量误差 σ_τ
	double sigma_tau = std::sqrt(sigma_tau_phi*sigma_tau_phi + sigma_tau_n*sigma_tau_n + sigma_tau_d*sigma_tau_d);
	errors.push_back(sigma_tau * 1e9); // ns
	// 角位置测量误差（可选，这里给0或按需要外部传入转换）
	errors.push_back(0.0);
	errors.push_back(sigma_tau_phi * 1e9); // 等价相位误差(ns)
	errors.push_back(sigma_tau_n * 1e9);   // 热噪声造成的时差(ns)
	// σ_θ = (c/(d·cosθ))·σ_τ   -> 度
	double cosTheta = std::cos(incidentAngleRad);
	if (std::abs(cosTheta) < 1e-6) cosTheta = 1e-6;
	double sigma_theta_deg = (c / (baselineLength * cosTheta)) * sigma_tau * RAD2DEG;
	// 合理约束
	sigma_theta_deg = std::clamp(sigma_theta_deg, 0.001, 30.0);
	errors.push_back(sigma_theta_deg);
	// 定位误差 ~ R·sin(σ_θ)
	double positionError = estimatedDistance * std::sin(sigma_theta_deg * DEG2RAD);
	errors.push_back(positionError);
	return errors;
}

} // namespace DirectionErrorUtils 