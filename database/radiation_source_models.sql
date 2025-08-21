SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

DROP TABLE IF EXISTS `radiation_source_models`;
CREATE TABLE `radiation_source_models` (
  `radiation_id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '主键，自增',
  `radiation_name` varchar(50) NOT NULL COMMENT '辐射源名称（如 "辐射源模型1"）',
  `is_stationary` tinyint(1) NOT NULL DEFAULT 1 COMMENT '是否为固定辐射源，默认TRUE',
  `transmit_power` float NOT NULL COMMENT '发射功率（千瓦）',
  `scan_period` float NOT NULL COMMENT '扫描周期（秒）',
  `carrier_frequency` float NOT NULL COMMENT '载波频率（GHz）',
  `azimuth_start_angle` decimal(8,5) NOT NULL DEFAULT '0.00000' COMMENT '工作扇区方位角起始角度（度，正北为0，顺时针）',
  `azimuth_end_angle` decimal(8,5) NOT NULL DEFAULT '360.00000' COMMENT '工作扇区方位角终止角度（度），必须>起始角度',
  `elevation_start_angle` decimal(8,5) NOT NULL DEFAULT '-90.00000' COMMENT '工作扇区俯仰角起始角度（度，水平面为0，向上为正）',
  `elevation_end_angle` decimal(8,5) NOT NULL DEFAULT '90.00000' COMMENT '工作扇区俯仰角终止角度（度），必须>起始角度',
  `movement_speed` float NOT NULL COMMENT '运动速度（米/秒），固定设备默认0',
  `movement_azimuth` decimal(5,2) NOT NULL COMMENT '运动方位角（度，正北为0，顺时针），固定设备默认0',
  `movement_elevation` decimal(4,2) NOT NULL COMMENT '运动俯仰角（度，水平面为0，向上为正），固定设备默认0',
  `longitude` decimal(9,6) NOT NULL COMMENT '辐射源经度（度），范围-180~180',
  `latitude` decimal(9,6) NOT NULL COMMENT '辐射源纬度（度），范围-90~90',
  `altitude` double NOT NULL COMMENT '辐射源高度（米，大地高）',
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP COMMENT '记录创建时间，默认当前时间',
  `last_updated` datetime DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP COMMENT '最后更新时间，自动更新',
  PRIMARY KEY (`radiation_id`) USING BTREE,
  UNIQUE KEY `radiation_name` (`radiation_name`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC COMMENT='辐射源模型表';

INSERT INTO `radiation_source_models` 
(radiation_name, is_stationary, transmit_power, scan_period, carrier_frequency, 
 azimuth_start_angle, azimuth_end_angle, elevation_start_angle, elevation_end_angle, 
 movement_speed, movement_azimuth, movement_elevation, longitude, latitude, altitude, 
 created_at, last_updated) 
VALUES
('目标辐射源A', 1, 100.0, 20.0, 0.1, 0.00000, 360.00000, -90.00000, 90.00000, 
 0.0, 0.00, 0.00, 116.400000, 39.800000, 500.0, '2025-06-10 02:05:05', '2025-07-24 19:14:04'),
('目标辐射源B', 0, 100.0, 20.0, 0.1, 0.00000, 360.00000, -90.00000, 90.00000, 
 20.0, 20.00, 20.00, 116.400000, 39.800000, 500.0, '2025-07-24 03:03:23', '2025-07-24 18:41:50');

DELIMITER $$
DROP TRIGGER IF EXISTS `before_insert_radiation_source`$$
CREATE TRIGGER `before_insert_radiation_source` 
BEFORE INSERT ON `radiation_source_models` 
FOR EACH ROW 
BEGIN
    IF NEW.azimuth_end_angle <= NEW.azimuth_start_angle THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：工作扇区方位角上限必须大于下限';
    END IF;
    IF NEW.elevation_end_angle <= NEW.elevation_start_angle THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：工作扇区俯仰角上限必须大于下限';
    END IF;
    IF NEW.is_stationary = TRUE THEN
        IF NEW.movement_speed != 0 OR NEW.movement_azimuth != 0 OR NEW.movement_elevation != 0 THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：固定辐射源的运动参数必须为0';
        END IF;
    END IF;
END$$

DROP TRIGGER IF EXISTS `before_update_radiation_source`$$
CREATE TRIGGER `before_update_radiation_source` 
BEFORE UPDATE ON `radiation_source_models` 
FOR EACH ROW 
BEGIN
    IF NEW.azimuth_end_angle != OLD.azimuth_end_angle OR NEW.azimuth_start_angle != OLD.azimuth_start_angle THEN
        IF NEW.azimuth_end_angle <= NEW.azimuth_start_angle THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：工作扇区方位角上限必须大于下限';
        END IF;
    END IF;
    IF NEW.elevation_end_angle != OLD.elevation_end_angle OR NEW.elevation_start_angle != OLD.elevation_start_angle THEN
        IF NEW.elevation_end_angle <= NEW.elevation_start_angle THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：工作扇区俯仰角上限必须大于下限';
        END IF;
    END IF;
    IF NEW.is_stationary = TRUE THEN
        IF NEW.movement_speed != 0 OR NEW.movement_azimuth != 0 OR NEW.movement_elevation != 0 THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：固定辐射源的运动参数必须为0';
        END IF;
    END IF;
END$$
DELIMITER ;

SET FOREIGN_KEY_CHECKS = 1;