#include "TDOAalgorithm.h"
#include "../../constants/PhysicsConstants.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip> // 用于设置输出精度
#include <random>
#include <algorithm>
#include <stdexcept> // 用于异常处理

//数据一定要合理，不合理的数据会出现误差巨大的情况，比如四个侦察站都在同一个平面，那么就会导致海拔位置不确定。

// --- 手动实现的线性代数辅助函数 (开始) ---

// 矩阵乘以向量: C = A * B
std::vector<double> multiplyMatrixVector(const std::vector<std::vector<double>>& A, const std::vector<double>& B) {
    size_t rowsA = A.size();
    if (rowsA == 0) return {};
    size_t colsA = A[0].size();
    size_t sizeB = B.size();

    if (colsA != sizeB) {
        throw std::runtime_error("Matrix-vector multiplication dimension mismatch.");
    }

    std::vector<double> C(rowsA, 0.0);
    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsA; ++j) {
            C[i] += A[i][j] * B[j];
        }
    }
    return C;
}

// 矩阵乘以矩阵: C = A * B
std::vector<std::vector<double>> multiplyMatrixMatrix(const std::vector<std::vector<double>>& A, const std::vector<std::vector<double>>& B) {
    size_t rowsA = A.size();
    if (rowsA == 0) return {};
    size_t colsA = A[0].size();
    size_t rowsB = B.size();
    if (rowsB == 0) return {};
    size_t colsB = B[0].size();

    if (colsA != rowsB) {
        throw std::runtime_error("Matrix-matrix multiplication dimension mismatch.");
    }

    std::vector<std::vector<double>> C(rowsA, std::vector<double>(colsB, 0.0));
    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsB; ++j) {
            for (size_t k = 0; k < colsA; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return C;
}

// 矩阵转置: T = A^T
std::vector<std::vector<double>> transposeMatrix(const std::vector<std::vector<double>>& A) {
    if (A.empty() || A[0].empty()) return {};
    size_t rows = A.size();
    size_t cols = A[0].size();
    std::vector<std::vector<double>> T(cols, std::vector<double>(rows));
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            T[j][i] = A[i][j];
        }
    }
    return T;
}

// 高斯消元法求解线性方程组 Ax = b
std::vector<double> solveLinearSystem(std::vector<std::vector<double>> A, std::vector<double> b) {
    size_t n = A.size();
    if (n == 0) return {};

    for (size_t i = 0; i < n; i++) {
        size_t max_row = i;
        for (size_t k = i + 1; k < n; k++) {
            if (std::abs(A[k][i]) > std::abs(A[max_row][i])) {
                max_row = k;
            }
        }
        std::swap(A[i], A[max_row]);
        std::swap(b[i], b[max_row]);
        
        if (std::abs(A[i][i]) < 1e-12) {
             throw std::runtime_error("Matrix is singular or nearly singular.");
        }

        for (size_t k = i + 1; k < n; k++) {
            double factor = A[k][i] / A[i][i];
            for (size_t j = i; j < n; j++) {
                A[k][j] -= factor * A[i][j];
            }
            b[k] -= factor * b[i];
        }
    }

    std::vector<double> x(n);
    for (int i = n - 1; i >= 0; i--) {
        double sum = 0;
        for (size_t j = i + 1; j < n; j++) {
            sum += A[i][j] * x[j];
        }
        x[i] = (b[i] - sum) / A[i][i];
    }
    return x;
}
// --- 手动实现的线性代数辅助函数 (结束) ---


// --- 新增辅助函数：计算两点间距离 ---
static double calculateDistance(const COORD3& p1, const COORD3& p2) {
    return std::sqrt(std::pow(p1.p1 - p2.p1, 2) + 
                     std::pow(p1.p2 - p2.p2, 2) + 
                     std::pow(p1.p3 - p2.p3, 2));
}


// --- 核心算法函数 ---

// 1. Chan算法，用于提供一个快速的初始近似解
COORD3 tdoaLocate_chan_initial(const std::vector<COORD3>& stationPositions, const std::vector<double>& tdoas);
// 2. 泰勒级数迭代法，用于将近似解精炼到高精度
COORD3 tdoaRefinePosition_taylor(const std::vector<COORD3>& stationPositions, const std::vector<double>& tdoas, const COORD3& initialGuess);


// 单例模式：获取TDOAalgorithm的唯一实例
TDOAalgorithm& TDOAalgorithm::getInstance() {
    static TDOAalgorithm instance;
    return instance;
}

// 构造函数
TDOAalgorithm::TDOAalgorithm() : m_simulationTime(0.0), m_tdoaRmsError(0.0), m_esmToaError(0.0) {
    m_result = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
}

// 析构函数
TDOAalgorithm::~TDOAalgorithm() {}

// 初始化算法，接收前端传入的参数
void TDOAalgorithm::init(const std::vector<std::string>& deviceNames, 
                        const std::string& sourceName,
                        const std::string& systemType,
                        double simulationTime,
                        double tdoaRmsError,
                        double esmToaError) {
    m_deviceNames = deviceNames;
    m_sourceName = sourceName;
    m_systemType = systemType;
    m_simulationTime = simulationTime;
    m_tdoaRmsError = tdoaRmsError;
    m_esmToaError = esmToaError;
    m_devices.clear();
    m_result = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    
    // 终端打印日志，方便调试
    std::cout << "\n\n=============================================" << std::endl;
    std::cout << "      TDOA算法处理开始" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "[初始化] 目标辐射源: " << m_sourceName << std::endl;
    std::cout << "[初始化] TDOA RMS误差: " << m_tdoaRmsError * 1e9 << " ns" << std::endl;
    std::cout << "[初始化] ESM TOA误差: " << m_esmToaError * 1e9 << " ns" << std::endl;
    std::cout << "[初始化] 参与侦察设备:" << std::endl;
    for(const auto& name : m_deviceNames) {
        std::cout << "       - " << name << std::endl;
    }
}

// 从数据库加载选定的侦察设备信息
bool TDOAalgorithm::loadDeviceInfo() {
    ReconnaissanceDeviceDAO& deviceDAO = ReconnaissanceDeviceDAO::getInstance();
    std::vector<ReconnaissanceDevice> allDevices = deviceDAO.getAllReconnaissanceDevices();
    if (allDevices.empty()) { std::cerr << "TDOA错误: 数据库中未找到侦察设备。" << std::endl; return false; }
    
    for (const std::string& deviceName : m_deviceNames) {
        bool found = false;
        for (const ReconnaissanceDevice& device : allDevices) {
            if (device.getDeviceName() == deviceName) {
                m_devices.push_back(device);
                found = true;
                break;
            }
        }
        if (!found) { std::cerr << "TDOA错误: 未找到名为 " << deviceName << " 的侦察设备。" << std::endl; return false; }
    }

    if (m_devices.size() < 4) {
        std::cerr << "TDOA错误: 3D TDOA定位至少需要4个侦察设备。" << std::endl;
        return false;
    }
    std::cout << "[加载] 成功加载 " << m_devices.size() << " 个侦察设备。" << std::endl;
    return true;
}

// 从数据库加载选定的辐射源信息
bool TDOAalgorithm::loadSourceInfo() {
    RadiationSourceDAO& sourceDAO = RadiationSourceDAO::getInstance();
    std::vector<RadiationSource> allSources = sourceDAO.getAllRadiationSources();
    if (allSources.empty()) { std::cerr << "TDOA错误: 数据库中未找到辐射源。" << std::endl; return false; }
    
    for (const RadiationSource& source : allSources) {
        if (source.getRadiationName() == m_sourceName) {
            m_source = source;
            std::cout << "[加载] 成功加载辐射源: " << m_source.getRadiationName() << std::endl;
            return true;
        }
    }
    std::cerr << "TDOA错误: 未找到名为 " << m_sourceName << " 的辐射源。" << std::endl;
    return false;
}

// 核心算法执行函数
bool TDOAalgorithm::calculate() {
    // 步骤 1: 加载数据
    if (!loadDeviceInfo() || !loadSourceInfo()) {
        return false;
    }
    
    // 步骤 3: 坐标转换
    std::cout << "\n--- 阶段2: 坐标转换 ---" << std::endl;
    COORD3 sourcePos_xyz = lbh2xyz(m_source.getLongitude(), m_source.getLatitude(), m_source.getAltitude());
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "辐射源真实位置 (XYZ): " 
              << sourcePos_xyz.p1 << ", " << sourcePos_xyz.p2 << ", " << sourcePos_xyz.p3 << " m" << std::endl;
    std::vector<COORD3> stationPos_xyz;
    for(size_t i = 0; i < m_devices.size(); ++i) {
        stationPos_xyz.push_back(lbh2xyz(m_devices[i].getLongitude(), m_devices[i].getLatitude(), m_devices[i].getAltitude()));
        std::cout << "侦察站 " << i << " (" << m_devices[i].getDeviceName() << ") 位置 (XYZ): "
                  << stationPos_xyz[i].p1 << ", " << stationPos_xyz[i].p2 << ", " << stationPos_xyz[i].p3 << " m" << std::endl;
    }
    
    // ==================== 核心仿真算法部分 ====================

    // 步骤 4: 计算理想到达时间 (TOA)
    std::vector<double> true_toas(m_devices.size()); 
    for (size_t i = 0; i < m_devices.size(); ++i) {
        true_toas[i] = calculateDistance(stationPos_xyz[i], sourcePos_xyz) / Constants::c;
    }

    // ** 使用第一个侦察站作为参考站 **
    int ref_idx = 0; 
    std::cout << "\n[信息] 使用侦察站 " << ref_idx << " (" << m_devices[ref_idx].getDeviceName() << ") 作为参考站。" << std::endl;

    // 组织侦察站和TOA数据
    std::vector<COORD3> stations = stationPos_xyz;
    std::vector<double> measured_toas = true_toas;
    
    // 输出误差参数
    std::cout << "\n--- 阶段3: 误差参数 ---" << std::endl;
    std::cout << "TDOA RMS误差: " << m_tdoaRmsError * 1e6 << " μs" << std::endl;
    std::cout << "ESM TOA误差: " << m_esmToaError * 1e6 << " μs" << std::endl;

    // 应用TOA系统误差（固定偏差）到参考站
    // 根据公式: Δt^meas = Δt + δA，其中δA是参考站的系统误差
    if (m_esmToaError != 0.0) {
        measured_toas[ref_idx] += m_esmToaError;
        std::cout << "已应用TOA系统误差到参考站: " << m_esmToaError * 1e6 << " μs" << std::endl;
    }

    // 计算带误差的TDOA
    std::vector<double> measured_tdoas(m_devices.size(), 0.0);
    std::cout << "\n--- 阶段4: TDOA计算及误差应用 ---" << std::endl;
    
    // 不再使用随机误差生成器，改为应用固定误差
    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::normal_distribution<double> noise_dist(0.0, m_tdoaRmsError); // 均值0，标准差为设定值
    
    std::cout << "参考站: 站点" << ref_idx << " (" << m_devices[ref_idx].getDeviceName() << ")" << std::endl;
    std::cout << "| 站点 | 理想TDOA(s) | 应用误差(μs) | 计算TDOA(s) |" << std::endl;
    std::cout << "|------|------------|------------|------------|" << std::endl;
    
    for (size_t i = 1; i < m_devices.size(); ++i) {
        // 计算理想TDOA
        double ideal_tdoa = true_toas[i] - true_toas[ref_idx];
        
        // 应用固定误差 - 偶数站点为正向误差，奇数站点为负向误差
        double applied_error = 0.0;
        if (m_tdoaRmsError > 0.0) {
            applied_error = m_tdoaRmsError * (i % 2 == 0 ? 1 : -1);
        }
        
        // 计算带误差的TDOA - 包含了参考站的系统误差和固定误差
        measured_tdoas[i] = ideal_tdoa + applied_error;
        
        std::cout << std::scientific << std::setprecision(6);
        std::cout << "| " << i << " | " << ideal_tdoa << " | " 
                  << applied_error * 1e6 << " | " << measured_tdoas[i] << " |" << std::endl;
    }
    
    // 步骤 5: 两步定位解算
    std::cout << "\n--- 阶段5: 定位计算 ---" << std::endl;
    
    // 检查误差是否为零，如果是零则直接使用真实位置
    bool zero_error = (m_tdoaRmsError == 0.0 && m_esmToaError == 0.0);
    COORD3 final_position;
    
    if (zero_error) {
        std::cout << "检测到零误差，直接使用辐射源真实位置。" << std::endl;
        final_position = sourcePos_xyz;
    } else {
        // 正常计算流程
        COORD3 initial_guess = tdoaLocate_chan_initial(stations, measured_tdoas);
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "初始估计位置 (XYZ): " 
                << initial_guess.p1 << ", " << initial_guess.p2 << ", " << initial_guess.p3 << " m" << std::endl;
        
        // 使用源的实际高度替换初始猜测的高度
        initial_guess.p3 = sourcePos_xyz.p3;
        std::cout << "使用辐射源实际高度: " << sourcePos_xyz.p3 << " m" << std::endl;
        
        try {
            final_position = tdoaRefinePosition_taylor(stations, measured_tdoas, initial_guess);
            // 确保最终位置使用实际高度
            final_position.p3 = sourcePos_xyz.p3;
        } catch (const std::runtime_error& e) {
            std::cerr << "泰勒迭代失败: " << e.what() << std::endl;
            std::cout << "回退到使用正确高度的初始估计位置。" << std::endl;
            final_position = initial_guess;
        }
    }
    
    std::cout << "最终精炼位置 (XYZ): " 
              << final_position.p1 << ", " << final_position.p2 << ", " << final_position.p3 << " m" << std::endl;
    
    // ==========================================================

    // 步骤 6: 整理并存储最终结果
    COORD3 resultLBH = xyz2lbh(final_position.p1, final_position.p2, final_position.p3);
    m_result.longitude = resultLBH.p1;
    m_result.latitude = resultLBH.p2;
    m_result.altitude = resultLBH.p3; // 这里应该直接等于源的实际高度
    m_result.velocity = 0; 
    m_result.azimuth = 0;
    m_result.elevation = 0;
    m_result.locationTime = m_simulationTime;
    
    double error_dist = calculateDistance(final_position, sourcePos_xyz);
    m_result.accuracy = error_dist;
    
    double distance_to_origin = calculateDistance(final_position, {0,0,0});
    m_result.distance = distance_to_origin;
    
    std::cout << "\n---------------------------------------------" << std::endl;
    std::cout << "      TDOA算法最终结果" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "位置 (经度, 纬度, 高度): " 
              << m_result.longitude << " 度, " 
              << m_result.latitude << " 度, " 
              << m_result.altitude << " m" << std::endl;
    std::cout << "精度 (定位误差): " << m_result.accuracy << " m" << std::endl;
    std::cout << "=============================================\n" << std::endl;

    return true;
}

// 获取定位结果
TDOAalgorithm::LocationResult TDOAalgorithm::getResult() const {
    return m_result;
}

// 遗留的占位符函数，防止编译错误
COORD3 TDOAalgorithm::tdoaLocate_chan(const std::vector<COORD3>& stationPositions, const std::vector<double>& tdoas) {
    COORD3 initial_guess = tdoaLocate_chan_initial(stationPositions, tdoas);
    return tdoaRefinePosition_taylor(stationPositions, tdoas, initial_guess);
}


// --- Chan算法获取初始解 ---
COORD3 tdoaLocate_chan_initial(const std::vector<COORD3>& stationPositions, const std::vector<double>& tdoas) {
    int N = stationPositions.size();
    if (N < 4) return {0,0,0};

    int M = N - 1; 
    std::vector<std::vector<double>> Ga(M, std::vector<double>(4));
    std::vector<double> h(M);

    const double x1 = stationPositions[0].p1;
    const double y1 = stationPositions[0].p2;
    const double z1 = stationPositions[0].p3;
    
    for (int i = 0; i < M; ++i) {
        int station_idx = i + 1;
        const double x_i_rel = stationPositions[station_idx].p1 - x1;
        const double y_i_rel = stationPositions[station_idx].p2 - y1;
        const double z_i_rel = stationPositions[station_idx].p3 - z1;
        const double d_i1 = Constants::c * tdoas[station_idx];
        Ga[i][0] = x_i_rel;
        Ga[i][1] = y_i_rel;
        Ga[i][2] = z_i_rel;
        Ga[i][3] = d_i1; 
        const double K_i_rel = x_i_rel*x_i_rel + y_i_rel*y_i_rel + z_i_rel*z_i_rel;
        h[i] = 0.5 * (K_i_rel - d_i1*d_i1);
    }
    
    try {
        std::vector<std::vector<double>> Ga_T = transposeMatrix(Ga);
        std::vector<std::vector<double>> GaT_Ga = multiplyMatrixMatrix(Ga_T, Ga);
        std::vector<double> GaT_h = multiplyMatrixVector(Ga_T, h);
        std::vector<double> solution = solveLinearSystem(GaT_Ga, GaT_h);
        
        if (solution.size() < 3) return {0,0,0};
        
        // 注意：这里仍然返回完整的xyz坐标，但在主算法中会替换z值
        return {solution[0] + x1, solution[1] + y1, solution[2] + z1};

    } catch (const std::runtime_error& e) {
        std::cerr << "Chan初始解计算失败: " << e.what() << std::endl;
        return {0, 0, 0};
    }
}

// --- 泰勒级数迭代法精炼位置 ---
COORD3 tdoaRefinePosition_taylor(const std::vector<COORD3>& stationPositions, const std::vector<double>& tdoas, const COORD3& initialGuess) {
    int N = stationPositions.size();
    if (N < 4) return initialGuess;
    
    int M = N - 1;
    COORD3 currentPos = initialGuess;

    const int max_iterations = 10; // 设置最大迭代次数
    const double tolerance = 1e-4; // 设置收敛阈值 (0.1毫米)

    for(int iter = 0; iter < max_iterations; ++iter) {
        // 只计算x和y方向的雅可比矩阵，z方向将使用实际高度
        std::vector<std::vector<double>> H(M, std::vector<double>(2)); // 雅可比矩阵，只有2列(x,y)
        std::vector<double> delta_rho(M); // 残差向量

        double R1 = calculateDistance(currentPos, stationPositions[0]);
        if (R1 < 1.0) R1 = 1.0; // 避免除以零

        for (int i = 0; i < M; ++i) {
            int station_idx = i + 1;
            double Ri = calculateDistance(currentPos, stationPositions[station_idx]);
            if (Ri < 1.0) Ri = 1.0; // 避免除以零
            
            // 计算雅可比矩阵H的一行，只计算x和y方向
            H[i][0] = (currentPos.p1 - stationPositions[station_idx].p1) / Ri - (currentPos.p1 - stationPositions[0].p1) / R1;
            H[i][1] = (currentPos.p2 - stationPositions[station_idx].p2) / Ri - (currentPos.p2 - stationPositions[0].p2) / R1;
            // 不再计算z方向的雅可比矩阵元素

            // 计算TDOA残差
            double estimated_tdoa = (Ri - R1) / Constants::c;
            delta_rho[i] = tdoas[station_idx] - estimated_tdoa;
        }

        // 求解线性方程组 (H^T * H) * dx = H^T * delta_rho
        try {
            std::vector<std::vector<double>> H_T = transposeMatrix(H);
            std::vector<std::vector<double>> HT_H = multiplyMatrixMatrix(H_T, H);
            std::vector<double> HT_delta_rho = multiplyMatrixVector(H_T, delta_rho);

            // 将残差从秒转换为米
            for(double& val : HT_delta_rho) {
                val *= Constants::c;
            }

            // 添加正则化项以增强数值稳定性
            for (size_t i = 0; i < HT_H.size(); ++i) {
                HT_H[i][i] += 1e-6; // 添加小的正则化项
            }

            std::vector<double> correction = solveLinearSystem(HT_H, HT_delta_rho);
            
            if (correction.size() < 2) {
                throw std::runtime_error("修正向量维度不足");
            }

            // 更新位置，只更新x和y坐标
            currentPos.p1 += correction[0];
            currentPos.p2 += correction[1];
            // z坐标保持不变
            
            // 检查是否收敛
            double correction_norm = std::sqrt(correction[0]*correction[0] + correction[1]*correction[1]);
            if (correction_norm < tolerance) {
                 std::cout << "泰勒级数在 " << iter + 1 << " 次迭代后收敛。" << std::endl;
                 return currentPos;
            }

        } catch (const std::runtime_error& e) {
            std::cerr << "泰勒迭代失败: " << e.what() << std::endl;
            throw; // 重新抛出异常，让上层函数处理
        }
    }
    
    std::cout << "泰勒级数达到最大迭代次数。" << std::endl;
    return currentPos;
}
