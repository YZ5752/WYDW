/*
 Navicat Premium Data Transfer

 Source Server         : localhost_3306
 Source Server Type    : MySQL
 Source Server Version : 80039
 Source Host           : localhost:3306
 Source Schema         : passive_location

 Target Server Type    : MySQL
 Target Server Version : 80039
 File Encoding         : 65001

 Date: 07/06/2025 11:38:02
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for reconnaissance_device_models
-- ----------------------------
DROP TABLE IF EXISTS `reconnaissance_device_models`;
CREATE TABLE `reconnaissance_device_models`  (
  `device_id` int(0) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键，自增',
  `device_name` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '模型名称（如 “侦察模型1”）',
  `is_stationary` tinyint(1) NOT NULL DEFAULT 1 COMMENT '是否为固定侦察设备，默认TRUE',
  `baseline_length` float NOT NULL COMMENT '基线长度（米），用于单站定位系统（一个测站中两天线之间的距离）',
  `noise_psd` float NOT NULL COMMENT '噪声功率谱密度（dBm/Hz），计算时需转换为W/Hz',
  `sample_rate` float NOT NULL COMMENT '采样速率（GHz），必须大于信号频率的2倍',
  `freq_range_min` float NOT NULL COMMENT '侦收频率范围下限（GHz）',
  `freq_range_max` float NOT NULL COMMENT '侦收频率范围上限（GHz）',
  `angle_azimuth_min` decimal(5, 2) NOT NULL DEFAULT 0.00 COMMENT '方位角下限（度），范围0~360',
  `angle_azimuth_max` decimal(5, 2) NOT NULL DEFAULT 360.00 COMMENT '方位角上限（度），范围0~360，必须≥下限',
  `angle_elevation_min` decimal(4, 2) NOT NULL DEFAULT -90.00 COMMENT '俯仰角下限（度），范围-90~90',
  `angle_elevation_max` decimal(4, 2) NOT NULL DEFAULT 90.00 COMMENT '俯仰角上限（度），范围-90~90，必须≥下限',
  `movement_speed` decimal(10, 2) NOT NULL COMMENT '运动速度（米/秒），固定设备默认0',
  `movement_azimuth` decimal(5, 2) NOT NULL COMMENT '运动方位角（度，正北为0，顺时针），固定设备默认0',
  `movement_elevation` decimal(4, 2) NOT NULL COMMENT '运动俯仰角（度，水平面为0，向上为正），固定设备默认0',
  `longitude` decimal(9, 6) NOT NULL COMMENT '设备经度（度），范围-180~180',
  `latitude` decimal(9, 6) NOT NULL COMMENT '设备纬度（度），范围-90~90',
  `altitude` double NOT NULL COMMENT '设备高度（米，大地高）',
  `created_at` datetime(0) NULL DEFAULT CURRENT_TIMESTAMP(0) COMMENT '记录创建时间，默认当前时间',
  `updated_at` datetime(0) NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP(0) COMMENT '记录更新时间，自动更新',
  PRIMARY KEY (`device_id`) USING BTREE,
  UNIQUE INDEX `device_name`(`device_name`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 8 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci COMMENT = '侦察设备模型表' ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of reconnaissance_device_models
-- ----------------------------
INSERT INTO `reconnaissance_device_models` VALUES (1, '固定监测站A', 1, 2.5, -170, 20, 0.3, 18, 0.00, 360.00, -90.00, 90.00, 0.00, 0.00, 0.00, 0.000000, 0.089832, 5000, '2025-06-06 14:55:27', '2025-06-06 14:55:27');
INSERT INTO `reconnaissance_device_models` VALUES (2, '固定监测站B', 1, 3.8, -170, 20, 2, 15, 0.00, 150.00, -45.00, 45.00, 0.00, 0.00, 0.00, 90.000000, 0.000000, 5000, '2025-06-06 14:55:27', '2025-06-06 14:55:27');
INSERT INTO `reconnaissance_device_models` VALUES (3, '固定监测站C', 1, 2.9, -170, 20, 0.3, 12, 0.00, 360.00, -90.00, 90.00, 0.00, 0.00, 0.00, 180.000000, 0.000000, 5000, '2025-06-06 14:55:27', '2025-06-06 14:55:27');
INSERT INTO `reconnaissance_device_models` VALUES (4, '固定监测站D', 1, 3.7, -170, 20, 5, 20, 30.00, 270.00, -45.00, 45.00, 0.00, 0.00, 0.00, 0.000000, 0.000000, 5000, '2025-06-06 14:55:27', '2025-06-06 14:55:27');
INSERT INTO `reconnaissance_device_models` VALUES (5, '移动侦察车A', 0, 3, -174, 20, 0.5, 20, 0.00, 180.00, -90.00, 90.00, 10.00, 90.00, 0.00, 0.000000, 0.000000, -6356752.314245, '2025-06-06 14:55:27', '2025-06-06 14:55:27');
INSERT INTO `reconnaissance_device_models` VALUES (6, '移动侦察车B', 0, 3.6, -174, 20, 1, 10.5, 0.00, 180.00, -90.00, 90.00, 10.00, 90.00, 20.00, 0.000000, 0.071558, 800, '2025-06-06 14:55:27', '2025-06-06 14:55:27');
INSERT INTO `reconnaissance_device_models` VALUES (7, '移动侦察车C', 0, 4.2, -174, 20, 0.5, 20, 0.00, 180.00, -90.00, 90.00, 10.00, 90.00, 0.00, 90.000000, 0.000000, 600, '2025-06-06 14:55:27', '2025-06-06 14:55:27');

-- ----------------------------
-- Triggers structure for table reconnaissance_device_models
-- ----------------------------
-- 修改分隔符为 $$
DELIMITER $$

-- 删除旧触发器（如果存在）
DROP TRIGGER IF EXISTS `before_insert_reconnaissance_device`$$

-- 创建插入前校验触发器
CREATE TRIGGER `before_insert_reconnaissance_device` 
BEFORE INSERT ON `reconnaissance_device_models`
FOR EACH ROW 
BEGIN
    -- 1. 频率范围校验
    IF NEW.freq_range_max <= NEW.freq_range_min THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：侦收频率范围上限必须大于下限';
    END IF;
    
    -- 2. 方位角范围校验
    IF NEW.angle_azimuth_max <= NEW.angle_azimuth_min THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：方位角上限必须大于下限';
    END IF;
    
    -- 3. 俯仰角范围校验
    IF NEW.angle_elevation_max <= NEW.angle_elevation_min THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：俯仰角上限必须大于下限';
    END IF;
    
    -- 4. 固定设备运动参数校验
    IF NEW.is_stationary = 1 THEN
        IF NEW.movement_speed != 0 OR NEW.movement_azimuth != 0 OR NEW.movement_elevation != 0 THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：固定设备的运动速度、方位角和俯仰角必须为0';
        END IF;
    END IF;
END$$

-- 删除旧触发器（如果存在）
DROP TRIGGER IF EXISTS `before_update_reconnaissance_device`$$

-- 创建更新前校验触发器
CREATE TRIGGER `before_update_reconnaissance_device` 
BEFORE UPDATE ON `reconnaissance_device_models`
FOR EACH ROW 
BEGIN
    -- 频率范围变更时校验
    IF NEW.freq_range_max != OLD.freq_range_max OR NEW.freq_range_min != OLD.freq_range_min THEN
        IF NEW.freq_range_max <= NEW.freq_range_min THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：侦收频率范围上限必须大于下限';
        END IF;
    END IF;
    
    -- 方位角范围变更时校验
    IF NEW.angle_azimuth_max != OLD.angle_azimuth_max OR NEW.angle_azimuth_min != OLD.angle_azimuth_min THEN
        IF NEW.angle_azimuth_max <= NEW.angle_azimuth_min THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：方位角上限必须大于下限';
        END IF;
    END IF;
    
    -- 俯仰角范围变更时校验
    IF NEW.angle_elevation_max != OLD.angle_elevation_max OR NEW.angle_elevation_min != OLD.angle_elevation_min THEN
        IF NEW.angle_elevation_max <= NEW.angle_elevation_min THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：俯仰角上限必须大于下限';
        END IF;
    END IF;
    
    -- 固定设备运动参数校验
    IF NEW.is_stationary = 1 THEN
        IF NEW.movement_speed != 0 OR NEW.movement_azimuth != 0 OR NEW.movement_elevation != 0 THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：固定设备的运动速度、方位角和俯仰角必须为0';
        END IF;
    END IF;
END$$

-- 恢复默认分隔符
DELIMITER ;

-- 启用外键检查
SET FOREIGN_KEY_CHECKS = 1;
