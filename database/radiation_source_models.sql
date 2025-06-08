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

 Date: 07/06/2025 12:32:15
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for radiation_source_models
-- ----------------------------
DROP TABLE IF EXISTS `radiation_source_models`;
CREATE TABLE `radiation_source_models`  (
  `radiation_id` int(0) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键，自增',
  `radiation_name` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '辐射源名称（如 “辐射源模型1”）',
  `is_stationary` tinyint(1) NOT NULL DEFAULT 1 COMMENT '是否为固定辐射源，默认TRUE',
  `transmit_power` float NOT NULL COMMENT '发射功率（千瓦）',
  `scan_period` float NOT NULL COMMENT '扫描周期（秒）',
  `carrier_frequency` float NOT NULL COMMENT '载波频率（GHz）',
  `azimuth_start_angle` decimal(8, 5) NOT NULL DEFAULT 0.00000 COMMENT '工作扇区方位角起始角度（度，正北为0，顺时针）',
  `azimuth_end_angle` decimal(8, 5) NOT NULL DEFAULT 360.00000 COMMENT '工作扇区方位角终止角度（度），必须>起始角度',
  `elevation_start_angle` decimal(8, 5) NOT NULL DEFAULT -90.00000 COMMENT '工作扇区俯仰角起始角度（度，水平面为0，向上为正）',
  `elevation_end_angle` decimal(8, 5) NOT NULL DEFAULT 90.00000 COMMENT '工作扇区俯仰角终止角度（度），必须>起始角度',
  `movement_speed` float NOT NULL COMMENT '运动速度（米/秒），固定设备默认0',
  `movement_azimuth` decimal(5, 2) NOT NULL COMMENT '运动方位角（度，正北为0，顺时针），固定设备默认0',
  `movement_elevation` decimal(4, 2) NOT NULL COMMENT '运动俯仰角（度，水平面为0，向上为正），固定设备默认0',
  `longitude` decimal(9, 6) NOT NULL COMMENT '辐射源经度（度），范围-180~180',
  `latitude` decimal(9, 6) NOT NULL COMMENT '辐射源纬度（度），范围-90~90',
  `altitude` double NOT NULL COMMENT '辐射源高度（米，海拔）',
  `created_at` datetime(0) NULL DEFAULT CURRENT_TIMESTAMP(0) COMMENT '记录创建时间，默认当前时间',
  `last_updated` datetime(0) NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP(0) COMMENT '最后更新时间，自动更新',
  PRIMARY KEY (`radiation_id`) USING BTREE,
  UNIQUE INDEX `radiation_name`(`radiation_name`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 3 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci COMMENT = '辐射源模型表' ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of radiation_source_models
-- ----------------------------
INSERT INTO `radiation_source_models` VALUES (1, '固定辐射源A', 1, 3, 1, 8, 0.00000, 360.00000, -30.00000, 30.00000, 0, 0.00, 0.00, 63.434949, 35.264390, 1732.050808, '2025-06-06 15:13:07', '2025-06-06 15:13:07');
INSERT INTO `radiation_source_models` VALUES (2, '移动辐射源A', 0, 1, 1, 8, 0.00000, 90.00000, -30.00000, 30.00000, 15, 30.00, 0.00, 26.565051, 14.036243, 1118.033989, '2025-06-06 15:13:07', '2025-06-06 15:13:07');

-- ----------------------------
-- Triggers structure for table radiation_source_models
-- ----------------------------
-- 修改分隔符为 $$
DELIMITER $$

-- 删除旧触发器（如果存在）
DROP TRIGGER IF EXISTS `before_insert_radiation_source`$$

-- 创建插入前校验触发器
CREATE TRIGGER `before_insert_radiation_source` 
BEFORE INSERT ON `radiation_source_models` 
FOR EACH ROW 
BEGIN
    -- 1. 校验方位角范围：终止角度必须大于起始角度
    IF NEW.azimuth_end_angle <= NEW.azimuth_start_angle THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：工作扇区方位角上限必须大于下限';
    END IF;
    
    -- 2. 校验俯仰角范围：终止角度必须大于起始角度
    IF NEW.elevation_end_angle <= NEW.elevation_start_angle THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：工作扇区俯仰角上限必须大于下限';
    END IF;
    
    -- 3. 校验固定设备运动参数：必须全为0
    IF NEW.is_stationary = TRUE THEN
        IF NEW.movement_speed != 0 OR 
           NEW.movement_azimuth != 0 OR 
           NEW.movement_elevation != 0 THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：固定辐射源的运动参数必须为0';
        END IF;
    END IF;
END$$

-- 删除旧触发器（如果存在）
DROP TRIGGER IF EXISTS `before_update_radiation_source`$$

-- 创建更新前校验触发器
CREATE TRIGGER `before_update_radiation_source` 
BEFORE UPDATE ON `radiation_source_models` 
FOR EACH ROW 
BEGIN
    -- 仅在校验字段变更时触发，提升性能
    -- 方位角范围校验
    IF NEW.azimuth_end_angle != OLD.azimuth_end_angle OR NEW.azimuth_start_angle != OLD.azimuth_start_angle THEN
        IF NEW.azimuth_end_angle <= NEW.azimuth_start_angle THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：工作扇区方位角上限必须大于下限';
        END IF;
    END IF;
    
    -- 俯仰角范围校验
    IF NEW.elevation_end_angle != OLD.elevation_end_angle OR NEW.elevation_start_angle != OLD.elevation_start_angle THEN
        IF NEW.elevation_end_angle <= NEW.elevation_start_angle THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：工作扇区俯仰角上限必须大于下限';
        END IF;
    END IF;
    
    -- 固定设备运动参数校验（仅当is_stationary为TRUE时）
    IF NEW.is_stationary = TRUE THEN
        IF NEW.movement_speed != 0 OR 
           NEW.movement_azimuth != 0 OR 
           NEW.movement_elevation != 0 THEN
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '错误：固定辐射源的运动参数必须为0';
        END IF;
    END IF;
END$$

-- 恢复默认分隔符
DELIMITER ;

-- 启用外键检查
SET FOREIGN_KEY_CHECKS = 1;