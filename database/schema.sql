-- 创建雷达设备模型表
CREATE TABLE radar_devices (
    device_id INT PRIMARY KEY AUTO_INCREMENT,
    device_type VARCHAR(50) NOT NULL,  -- 类型：侦察机（移动）/雷达（固定）
    name VARCHAR(100) NOT NULL,
    technical_system VARCHAR(50) NOT NULL,  -- 技术体制：时差/频差
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    coordinate_id INT,
    FOREIGN KEY (coordinate_id) REFERENCES coordinates(coordinate_id)
);

-- 创建雷达设备基本参数表
CREATE TABLE radar_basic_params (
    param_id INT PRIMARY KEY AUTO_INCREMENT,
    device_id INT NOT NULL,
    antenna_length DOUBLE,       -- 天线长度
    noise_level DOUBLE,          -- 噪声
    FOREIGN KEY (device_id) REFERENCES radar_devices(device_id) ON DELETE CASCADE
);

-- 创建雷达设备工作参数表
CREATE TABLE radar_work_params (
    param_id INT PRIMARY KEY AUTO_INCREMENT,
    device_id INT NOT NULL,
    work_start_time TIME,        -- 工作开始时间
    work_end_time TIME,          -- 工作结束时间
    freq_min DOUBLE NOT NULL,    -- 侦收的频率范围最小值
    freq_max DOUBLE NOT NULL,    -- 侦收的频率范围最大值
    angle_min DOUBLE,            -- 角度范围最小值
    angle_max DOUBLE,            -- 角度范围最大值
    FOREIGN KEY (device_id) REFERENCES radar_devices(device_id) ON DELETE CASCADE
);

-- 创建辐射源模型表
CREATE TABLE radiation_sources (
    source_id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    device_type VARCHAR(50) NOT NULL,  -- 类型：雷达站（固定）/飞机（移动）/舰船（移动）
    transmit_power DOUBLE NOT NULL,    -- 发射功率
    scan_period DOUBLE NOT NULL,       -- 扫描周期
    freq_min DOUBLE NOT NULL,          -- 频率范围最小值
    freq_max DOUBLE NOT NULL,          -- 频率范围最大值
    work_sector DOUBLE NOT NULL,       -- 工作扇区
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    coordinate_id INT,
    FOREIGN KEY (coordinate_id) REFERENCES coordinates(coordinate_id)
);

-- 创建坐标点表
CREATE TABLE coordinates (
    coordinate_id INT PRIMARY KEY AUTO_INCREMENT,
    x DOUBLE NOT NULL,
    y DOUBLE NOT NULL,
    z DOUBLE NOT NULL
);

-- 创建平台表
CREATE TABLE platforms (
    platform_id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    coordinate_id INT NOT NULL,
    device_id INT NOT NULL,
    FOREIGN KEY (coordinate_id) REFERENCES coordinates(coordinate_id),
    FOREIGN KEY (device_id) REFERENCES radar_devices(device_id)
);

-- 创建仿真结果表
CREATE TABLE simulation_results (
    result_id INT PRIMARY KEY AUTO_INCREMENT,
    simulation_type VARCHAR(50) NOT NULL,  -- 仿真类型：单平台/多平台
    device_id INT NOT NULL,                -- 雷达设备ID
    source_id INT NOT NULL,                -- 辐射源ID
    coordinate_id INT NOT NULL,            -- 定位坐标ID
    power DOUBLE NOT NULL,                 -- 威力
    direction_error DOUBLE NOT NULL,       -- 测向误差
    parameter_error DOUBLE NOT NULL,       -- 参数测量误差
    location_time DOUBLE NOT NULL,         -- 定位时间
    simulation_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (device_id) REFERENCES radar_devices(device_id),
    FOREIGN KEY (source_id) REFERENCES radiation_sources(source_id),
    FOREIGN KEY (coordinate_id) REFERENCES coordinates(coordinate_id)
);

-- 创建基准点表
CREATE TABLE reference_points (
    point_id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    coordinate_id INT NOT NULL,
    FOREIGN KEY (coordinate_id) REFERENCES coordinates(coordinate_id)
);

-- 创建目标情报数据表
CREATE TABLE target_intelligence (
    target_id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    reference_point_id INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (reference_point_id) REFERENCES reference_points(point_id)
);

-- 创建目标定位结果表
CREATE TABLE target_locations (
    location_id INT PRIMARY KEY AUTO_INCREMENT,
    target_id INT NOT NULL,
    result_id INT NOT NULL,
    is_selected BOOLEAN DEFAULT FALSE,
    is_highlighted BOOLEAN DEFAULT FALSE,
    is_anomaly BOOLEAN DEFAULT FALSE,
    FOREIGN KEY (target_id) REFERENCES target_intelligence(target_id),
    FOREIGN KEY (result_id) REFERENCES simulation_results(result_id)
);

-- 创建评估结果表
CREATE TABLE evaluation_results (
    eval_id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,            -- 评估指标名称
    value DOUBLE NOT NULL,                 -- 评估值
    eval_type VARCHAR(50) NOT NULL,        -- 评估类型：单平台/多平台
    eval_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 创建仿真记录表
CREATE TABLE simulation_records (
    record_id INT PRIMARY KEY AUTO_INCREMENT,
    simulation_mode VARCHAR(50) NOT NULL,  -- 仿真模式：任务前/实时/任务后
    start_time TIMESTAMP NOT NULL,
    end_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    description TEXT
); 