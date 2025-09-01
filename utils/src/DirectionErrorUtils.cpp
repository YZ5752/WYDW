#include "../DirectionErrorUtils.h"
#include "../CoordinateTransform.h"
#include "../SNRValidator.h"
#include "../../constants/PhysicsConstants.h"
#include <cmath>
#include <algorithm>
#include <iostream>

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
	// 日志输出：圆锥效应查表细节
	std::cout << "[DF-IF] 圆锥效应查表: 输入 azimuth(deg)=" << azimuthDegAbs
	          << ", elevation(deg)=" << elevationDegAbs
	          << "; 归一化 alpha(deg)=" << alpha
	          << ", beta(deg)=" << beta
	          << "; 命中索引 ai=" << ai << ", bi=" << bi
	          << "; σ_β(deg)=" << sigma_beta << std::endl;
	return sigma_beta; // 单位：度
}

std::vector<double> calculateInterferometerErrors(
	const ReconnaissanceDevice& device,
	const RadiationSource& source,
	double azimuthDeg,
	double elevationDeg) {
	std::vector<double> errors;
	// 1) 对中误差 Δem、姿态误差 σ_α，来自常量
	std::cout << "[DF-IF] 设备=" << device.getDeviceName()
	          << "(id=" << device.getDeviceId() << ")"
	          << ", 入参: azimuth(deg)=" << azimuthDeg
	          << ", elevation(deg)=" << elevationDeg << std::endl;
	double delta_em   = INTERFEROMETER_ALIGNMENT_ERROR;   // 度
	double sigma_alpha= INTERFEROMETER_ATTITUDE_ERROR;    // 度
	std::cout << "[DF-IF] 常量: Δem(deg)=" << delta_em
	          << ", σ_α(deg)=" << sigma_alpha << std::endl;
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
	std::cout << "[DF-IF] 阵列误差: d(m)=" << d
	          << ", f(Hz)=" << source.getCarrierFrequency()*1e9
	          << ", λ(m)=" << lambda
	          << ", cosθ=" << cos_theta
	          << ", σ_φ(deg)=" << INTERFEROMETER_PHASE_ERROR
	          << ", σ_θ(deg)=" << sigma_theta_deg << std::endl;
	// 4) 综合测向误差 Δθ = sqrt(σ_α^2 + σ_β^2 + σ_θ^2 + Δem^2)
	double total = std::sqrt(sigma_alpha*sigma_alpha + sigma_beta*sigma_beta +
	                        sigma_theta_deg*sigma_theta_deg + delta_em*delta_em);
	double sigma_sum = std::sqrt(sigma_alpha*sigma_alpha + sigma_beta*sigma_beta + sigma_theta_deg*sigma_theta_deg);
	std::cout << "[DF-IF] 合成: std(deg)=sqrt(σ_α^2+σ_β^2+σ_θ^2)=" << sigma_sum
	          << ", total_with_mean(deg)=" << total
	          << " (输出: 均值=Δem, 标准差=std)" << std::endl;
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
	
	// 基线长度验证：TDOA需要足够长的基线
	if (baselineLength < 10.0) {
		std::cout << "[TDOA-WARN] 基线长度过短: " << baselineLength 
		          << "m < 10m，TDOA体制建议基线长度≥10m" << std::endl;
		// 使用最小基线长度进行计算
		baselineLength = 10.0;
	}
	
	// 入射角验证：避免接近90度的情况
	double incidentAngleDeg = incidentAngleRad * RAD2DEG;
	if (std::abs(std::abs(incidentAngleDeg) - 90.0) < 5.0) {
		std::cout << "[TDOA-WARN] 入射角接近90度: " << incidentAngleDeg 
		          << "°，可能导致误差放大，限制在85°以内" << std::endl;
		// 限制入射角在85度以内
		if (incidentAngleDeg > 0) {
			incidentAngleRad = 85.0 * DEG2RAD;
		} else {
			incidentAngleRad = -85.0 * DEG2RAD;
		}
	}

	// 调试输出：输入参数
	std::cout << "[TDOA] baselineLength(m)=" << baselineLength
	          << ", incidentAngle(deg)=" << (incidentAngleRad * RAD2DEG)
	          << ", estDistance(m)=" << estimatedDistance
	          << ", carrierFreq(Hz)=" << carrierFreqHz
	          << ", phaseErr(deg)=" << phaseErrorDeg
	          << ", bandwidth(Hz)=" << bandwidthHz
	          << ", SNR(linear)=" << snrLinear
	          << ", fs(Hz)=" << samplingRateHz << std::endl;
	
	// 1) σ_τφ = (1/(360°·f0))·σ_φ
	double sigma_tau_phi = (phaseErrorDeg / 360.0) / carrierFreqHz; // 秒
	
	// 2) σ_τn = 0.175 / (Bv · sqrt(2·SNR))
	double sigma_tau_n = 0.175 / (bandwidthHz * std::sqrt(2.0 * std::max(1e-9, snrLinear)));
	
	// 3) σ_τd = 2 / (2√3 · fs)
	double sigma_tau_d = 2.0 / (2.0 * std::sqrt(3.0) * samplingRateHz);
	
	// 综合时差测量误差 σ_τ
	double sigma_tau = std::sqrt(sigma_tau_phi*sigma_tau_phi + sigma_tau_n*sigma_tau_n + sigma_tau_d*sigma_tau_d);

	// 调试输出：各分量与合成时差误差
	std::cout << "[TDOA] sigma_tau_phi(s)=" << sigma_tau_phi
	          << ", sigma_tau_n(s)=" << sigma_tau_n
	          << ", sigma_tau_d(s)=" << sigma_tau_d
	          << ", sigma_tau(s)=" << sigma_tau << std::endl;
	
	// 按图中流程输出：σ_τ、（占位保持索引）、σ_τφ、σ_τn、σ_θ
	errors.push_back(sigma_tau);          // σ_τ（秒）
	errors.push_back(0.0);                // 占位，保持与现有调用约定的索引
	errors.push_back(sigma_tau_phi);      // σ_τφ（秒）
	errors.push_back(sigma_tau_n);        // σ_τn（秒）
	
	// σ_θ = (c/(d·|cosθ|))·σ_τ   -> 度
	// 工程护栏：提高 |cosθ| 下限，防止 θ≈90° 时病态放大
	double cosTheta = std::cos(incidentAngleRad);
	double absCosTheta = std::abs(cosTheta);
	if (absCosTheta < 0.10) absCosTheta = 0.10; // 约等于 θ <= ~84.3°
	double sigma_theta_deg = (c / (baselineLength * absCosTheta)) * sigma_tau * RAD2DEG;
	if (sigma_theta_deg < 0.0) sigma_theta_deg = -sigma_theta_deg;

	// 调试输出：几何与角度误差
	std::cout << "[TDOA] cosTheta=" << cosTheta
	          << ", absCosTheta(clamped)=" << absCosTheta
	          << ", sigma_theta(deg)=" << sigma_theta_deg << std::endl;
	errors.push_back(sigma_theta_deg);
	return errors;
}

// 动态计算TDOA误差计算所需的参数
TDOAParams calculateTDOAParams(
	const ReconnaissanceDevice& device,
	const RadiationSource& source,
	double estimatedDistance) {
	
	TDOAParams params;
	
	// 1. 相位误差：根据设备技术体制确定
	// TDOA体制通常相位误差较小，干涉仪体制相位误差较大
	if (device.getTechSystem() == "TDOA" || device.getTechSystem() == "时差体制") {
		params.phaseErrorDeg = 0.1;  // TDOA体制：相位误差0.1度（现代系统）
	} else if (device.getTechSystem() == "INTERFEROMETER" || device.getTechSystem() == "干涉仪体制") {
		params.phaseErrorDeg = 0.5;  // 干涉仪体制：相位误差0.5度
	} else {
		params.phaseErrorDeg = 0.2;  // 默认：相位误差0.2度
	}
	
	// 2. 带宽：使用设备的实际侦收频率范围
	double freqRangeMin = device.getFreqRangeMin();  // GHz
	double freqRangeMax = device.getFreqRangeMax();  // GHz
	params.bandwidthHz = (freqRangeMax - freqRangeMin) * 1e9;  // 转换为Hz
	
	// 确保带宽不为零，设置最小带宽
	if (params.bandwidthHz < 1e6) {  // 小于1MHz
		params.bandwidthHz = 10e6;    // 设置最小带宽为10MHz
	}
	
	// 3. 采样率：使用设备的实际采样率
	params.samplingRateHz = device.getSampleRate() * 1e9;  // 转换为Hz
	
	// 确保采样率满足奈奎斯特采样定理（至少是信号频率的2倍）
	double maxSignalFreq = source.getCarrierFrequency() * 1e9;  // 转换为Hz
	if (params.samplingRateHz < 2.2 * maxSignalFreq) {  // 2.2倍作为安全系数
		params.samplingRateHz = 2.2 * maxSignalFreq;
	}
	
	// 4. SNR：根据发射功率、距离、噪声功率谱密度等动态计算
	double pt = source.getTransmitPower();           // 发射功率 (kW)
	double fc = source.getCarrierFrequency();       // 载波频率 (GHz)
	double N0_W = device.getNoisePsd();             // 噪声功率谱密度 (dBm/Hz)
	
	// 将噪声功率谱密度从dBm/Hz转换为W/Hz
	double N0_dBm = N0_W;
	double N0_W_per_Hz = std::pow(10.0, (N0_dBm - 30.0) / 10.0);  // dBm/Hz -> W/Hz
	
	// 计算SNR (dB)
	double snr_dB = calculateSNR(estimatedDistance, pt, fc, N0_dBm, params.bandwidthHz / 1e9);
	
	// 转换为线性值
	params.snrLinear = std::pow(10.0, snr_dB / 10.0);
	
	// 限制SNR在合理范围内
	params.snrLinear = std::max(1.0, std::min(10000.0, params.snrLinear));
	
	// 输出调试信息
	std::cout << "[TDOA-PARAMS] 设备=" << device.getDeviceName() 
	          << ", 技术体制=" << device.getTechSystem() << std::endl;
	std::cout << "[TDOA-PARAMS] 相位误差=" << params.phaseErrorDeg 
	          << "°, 带宽=" << params.bandwidthHz/1e6 << "MHz" << std::endl;
	std::cout << "[TDOA-PARAMS] 采样率=" << params.samplingRateHz/1e6 
	          << "MHz, SNR=" << snr_dB << "dB(" << params.snrLinear << ")" << std::endl;
	
	return params;
}

} // namespace DirectionErrorUtils 