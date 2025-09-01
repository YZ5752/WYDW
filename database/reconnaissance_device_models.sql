SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

DROP TABLE IF EXISTS `reconnaissance_device_models`;
CREATE TABLE `reconnaissance_device_models` (
  `device_id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '主键，自增',
  `device_name` varchar(50) NOT NULL COMMENT '模型名称（如 "侦察模型1"）',
  `tech_system` enum('INTERFEROMETER','TDOA') NOT NULL COMMENT '技术体制：干涉仪体制或时差体制',
  `is_stationary` tinyint(1) NOT NULL DEFAULT 1 COMMENT '是否为固定侦察设备，默认TRUE',
  `baseline_length` float NOT NULL COMMENT '基线长度（米），用于单站定位系统（一个测站中两天线之间的距离）',
  `noise_psd` float NOT NULL COMMENT '噪声功率谱密度（dBm/Hz），计算时需转换为W/Hz',
  `sample_rate` float NOT NULL COMMENT '采样速率（GHz），必须大于信号频率的2倍',
  `freq_range_min` float NOT NULL COMMENT '侦收频率范围下限（GHz）',
  `freq_range_max` float NOT NULL COMMENT '侦收频率范围上限（GHz）',
  `angle_azimuth_min` decimal(5,2) NOT NULL DEFAULT '0.00' COMMENT '方位角下限（度），范围0~360',
  `angle_azimuth_max` decimal(5,2) NOT NULL DEFAULT '360.00' COMMENT '方位角上限（度），范围0~360，必须≥下限',
  `angle_elevation_min` decimal(4,2) NOT NULL DEFAULT '-90.00' COMMENT '俯仰角下限（度），范围-90~90',
  `angle_elevation_max` decimal(4,2) NOT NULL DEFAULT '90.00' COMMENT '俯仰角上限（度），范围-90~90，必须≥下限',
  `movement_speed` decimal(10,2) NOT NULL COMMENT '运动速度（米/秒），固定设备默认0',
  `movement_azimuth` decimal(5,2) NOT NULL COMMENT '运动方位角（度，正北为0，顺时针），固定设备默认0',
  `movement_elevation` decimal(4,2) NOT NULL COMMENT '运动俯仰角（度，水平面为0，向上为正），固定设备默认0',
  `longitude` decimal(9,6) NOT NULL COMMENT '设备经度（度），范围-180~180',
  `latitude` decimal(9,6) NOT NULL COMMENT '设备纬度（度），范围-90~90',
  `altitude` double NOT NULL COMMENT '设备高度（米，大地高）',
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP COMMENT '记录创建时间，默认当前时间',
  `updated_at` datetime DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP COMMENT '记录更新时间，自动更新',
  PRIMARY KEY (`device_id`) USING BTREE,
  UNIQUE KEY `device_name` (`device_name`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC COMMENT='侦察设备模型表';

-- 插入侦察设备数据（随机分配技术体制）
INSERT INTO `reconnaissance_device_models` 
(device_name, tech_system, is_stationary, baseline_length, noise_psd, sample_rate, 
 freq_range_min, freq_range_max, angle_azimuth_min, angle_azimuth_max, 
 angle_elevation_min, angle_elevation_max, movement_speed, movement_azimuth, 
 movement_elevation, longitude, latitude, altitude, created_at, updated_at) 
VALUES
-- 固定监测站A-F
('固定监测站A', IF(RAND() > 0.5, 'INTERFEROMETER', 'TDOA'), 1, 20.0, -174.0, 0.0, 0.0, 100.0, 0.10, 360.00, -90.00, 90.00, 0.00, 0.00, 0.00, 116.400000, 39.900000, 500.0, '2025-07-04 01:44:32', '2025-07-23 02:07:44'),
('固定监测站B', IF(RAND() > 0.5, 'INTERFEROMETER', 'TDOA'), 1, 10.0, -174.0, 20.0, 0.0, 15.0, 0.00, 360.00, -45.00, 90.00, 0.00, 0.00, 0.00, 116.400000, 39.600000, 500.0, '2025-06-06 14:55:27', '2025-07-23 02:06:22'),
('固定监测站C', IF(RAND() > 0.5, 'INTERFEROMETER', 'TDOA'), 1, 2.5, -174.0, 20.0, 0.0, 18.0, 0.00, 360.00, -90.00, 90.00, 0.00, 0.00, 0.00, 116.500000, 39.600000, 500.0, '2025-06-06 14:55:27', '2025-07-23 02:03:53'),
('固定监测站D', IF(RAND() > 0.5, 'INTERFEROMETER', 'TDOA'), 1, 3.7, -174.0, 20.0, 0.0, 20.0, 30.00, 360.00, -45.00, 45.00, 0.00, 0.00, 0.00, 116.250000, 39.600000, 500.0, '2025-06-06 14:55:27', '2025-07-23 02:04:28'),
('固定监测站E', IF(RAND() > 0.5, 'INTERFEROMETER', 'TDOA'), 1, 3.5, -174.0, 20.0, 0.5, 18.0, 0.00, 360.00, -90.00, 90.00, 0.00, 0.00, 0.00, 116.413384, 39.910925, 500.0, '2025-07-04 02:41:17', '2025-07-23 02:07:44'),
('固定监测站F', IF(RAND() > 0.5, 'INTERFEROMETER', 'TDOA'), 1, 3.8, -174.0, 20.0, 0.0, 15.0, 0.00, 360.00, -90.00, 90.00, 0.00, 0.00, 0.00, 104.065735, 30.659462, 500.0, '2025-07-04 02:41:17', '2025-07-23 02:03:53'),

-- 移动侦察车A-F
('移动侦察车A', IF(RAND() > 0.5, 'INTERFEROMETER', 'TDOA'), 0, 30.0, -174.0, 20.0, 0.0, 20.0, 0.00, 360.00, -90.00, 90.00, 10.00, 90.00, 0.00, 116.440000, 39.920000, 500.0, '2025-06-06 14:55:27', '2025-07-23 02:03:53'),
('移动侦察车B', IF(RAND() > 0.5, 'INTERFEROMETER', 'TDOA'), 0, 10.0, -174.0, 20.0, 0.0, 10.5, 0.00, 360.00, -90.00, 90.00, 300.00, 90.00, 20.00, 116.440000, 39.630000, 500.0, '2025-06-06 14:55:27', '2025-07-23 02:03:53'),
('移动侦察车C', IF(RAND() > 0.5, 'INTERFEROMETER', 'TDOA'), 0, 50.0, -174.0, 0.2, 0.0, 90.0, 0.00, 360.00, 0.00, 90.00, 100.00, 90.00, 0.00, 116.500000, 39.700000, 500.0, '2025-06-11 01:53:53', '2025-07-23 02:03:53'),
('移动侦察车D', IF(RAND() > 0.5, 'INTERFEROMETER', 'TDOA'), 0, 10.0, -174.0, 0.1, 0.0, 90.0, 0.00, 360.00, 0.00, 90.00, 29.00, 0.00, 0.00, 116.470000, 39.660000, 500.0, '2025-06-20 02:29:44', '2025-07-23 02:04:28'),
('移动侦察车E', IF(RAND() > 0.5, 'INTERFEROMETER', 'TDOA'), 0, 4.0, -174.0, 20.0, 0.0, 15.0, 0.00, 360.00, -90.00, 90.00, 12.00, 90.00, 0.00, 121.473701, 31.230416, 500.0, '2025-07-04 02:41:17', '2025-07-23 02:03:53'),
('移动侦察车F', IF(RAND() > 0.5, 'INTERFEROMETER', 'TDOA'), 0, 4.0, -174.0, 20.0, 0.0, 15.0, 0.00, 360.00, -90.00, 90.00, 10.00, 180.00, 0.00, 113.264435, 23.129163, 500.0, '2025-07-04 02:41:17', '2025-07-23 02:03:53');

DELIMITER $$
DROP TRIGGER IF EXISTS `before_insert_reconnaissance_device`$$
CREATE TRIGGER `before_insert_reconnaissance_device` 
BEFORE INSERT ON `reconnaissance_device_models`
FOR EACH ROW 
BEGIN
    IF NEW.freq_range_max <= NEW.freq_range_min THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：侦收频率范围上限必须大于下限';
    END IF;
    IF NEW.angle_azimuth_max <= NEW.angle_azimuth_min THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：方位角上限必须大于下限';
    END IF;
    IF NEW.angle_elevation_max <= NEW.angle_elevation_min THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：俯仰角上限必须大于下限';
    END IF;
    IF NEW.is_stationary = 1 THEN
        IF NEW.movement_speed != 0 OR NEW.movement_azimuth != 0 OR NEW.movement_elevation != 0 THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：固定设备的运动速度、方位角和俯仰角必须为0';
        END IF;
    END IF;
END$$

DROP TRIGGER IF EXISTS `before_update_reconnaissance_device`$$
CREATE TRIGGER `before_update_reconnaissance_device` 
BEFORE UPDATE ON `reconnaissance_device_models`
FOR EACH ROW 
BEGIN
    IF NEW.freq_range_max != OLD.freq_range_max OR NEW.freq_range_min != OLD.freq_range_min THEN
        IF NEW.freq_range_max <= NEW.freq_range_min THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：侦收频率范围上限必须大于下限';
        END IF;
    END IF;
    IF NEW.angle_azimuth_max != OLD.angle_azimuth_max OR NEW.angle_azimuth_min != OLD.angle_azimuth_min THEN
        IF NEW.angle_azimuth_max <= NEW.angle_azimuth_min THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：方位角上限必须大于下限';
        END IF;
    END IF;
    IF NEW.angle_elevation_max != OLD.angle_elevation_max OR NEW.angle_elevation_min != OLD.angle_elevation_min THEN
        IF NEW.angle_elevation_max <= NEW.angle_elevation_min THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：俯仰角上限必须大于下限';
        END IF;
    END IF;
    IF NEW.is_stationary = 1 THEN
        IF NEW.movement_speed != 0 OR NEW.movement_azimuth != 0 OR NEW.movement_elevation != 0 THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：固定设备的运动速度、方位角和俯仰角必须为0';
        END IF;
    END IF;
END$$
DELIMITER ;

SET FOREIGN_KEY_CHECKS = 1;