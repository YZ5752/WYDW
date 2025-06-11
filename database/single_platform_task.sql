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

 Date: 07/06/2025 12:32:24
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for single_platform_task
-- ----------------------------
DROP TABLE IF EXISTS `single_platform_task`;
CREATE TABLE `single_platform_task`  (
  `task_id` int(0) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键，自增',
  `tech_system` enum('INTERFEROMETER','TDOA') CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '技术体制：干涉仪或时差体制',
  `device_id` int(0) UNSIGNED NOT NULL COMMENT '关联侦察设备模型ID（仅移动设备）',
  `radiation_id` int(0) UNSIGNED NOT NULL COMMENT '关联辐射源模型ID（仅固定辐射源）',
  `execution_time` float NOT NULL COMMENT '仿真执行时长（秒）',
  `target_longitude` decimal(9, 6) NOT NULL COMMENT '目标经度（度），范围-180~180',
  `target_latitude` decimal(9, 6) NOT NULL COMMENT '目标纬度（度），范围-90~90',
  `target_altitude` double NOT NULL COMMENT '目标高度（米，大地高）',
  `target_angle` decimal(8, 5) NOT NULL COMMENT '测向数据（度，正北为0，顺时针）',
  `angle_error` decimal(8, 6) NOT NULL COMMENT '测向误差（度），正数',
  `created_at` datetime(0) NULL DEFAULT CURRENT_TIMESTAMP(0) COMMENT '任务创建时间，默认当前时间',
  PRIMARY KEY (`task_id`) USING BTREE,
  INDEX `device_id`(`device_id`) USING BTREE,
  INDEX `radiation_id`(`radiation_id`) USING BTREE,
  CONSTRAINT `single_platform_task_ibfk_1` FOREIGN KEY (`device_id`) REFERENCES `reconnaissance_device_models` (`device_id`) ON DELETE RESTRICT ON UPDATE RESTRICT,
  CONSTRAINT `single_platform_task_ibfk_2` FOREIGN KEY (`radiation_id`) REFERENCES `radiation_source_models` (`radiation_id`) ON DELETE RESTRICT ON UPDATE RESTRICT
) ENGINE = InnoDB AUTO_INCREMENT = 3 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci COMMENT = '单平台仿真任务表' ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of single_platform_task
-- ----------------------------
INSERT INTO `single_platform_task` VALUES (1, 'INTERFEROMETER', 5, 1, 3600, 116.500000, 39.950000, 200, 45.00000, 0.500000, '2025-06-06 15:24:07');
INSERT INTO `single_platform_task` VALUES (2, 'TDOA', 6, 1, 1800, 120.200000, 30.300000, 150, 120.00000, 0.800000, '2025-06-06 15:24:07');

-- ----------------------------
-- Triggers structure for table single_platform_task
-- ----------------------------
-- 修改分隔符为 $$
DELIMITER $$

-- 删除旧触发器（如果存在）
DROP TRIGGER IF EXISTS `before_insert_single_platform_task`$$

-- 创建插入前校验触发器
CREATE TRIGGER `before_insert_single_platform_task` 
BEFORE INSERT ON `single_platform_task` 
FOR EACH ROW 
BEGIN
    DECLARE device_stationary BOOLEAN;
    DECLARE radiation_stationary BOOLEAN;
    
    -- 获取关联侦察设备的is_stationary状态
    SELECT is_stationary INTO device_stationary
    FROM reconnaissance_device_models
    WHERE device_id = NEW.device_id;
    
    -- 获取关联辐射源的is_stationary状态
    SELECT is_stationary INTO radiation_stationary
    FROM radiation_source_models
    WHERE radiation_id = NEW.radiation_id;
    
    -- 1. 校验侦察设备必须为运动设备（is_stationary = FALSE）
    IF device_stationary = TRUE THEN
        SIGNAL SQLSTATE '45000' 
        SET MESSAGE_TEXT = '错误：单平台任务只能关联运动侦察设备';
    END IF;
    
    -- 2. 校验辐射源必须为固定辐射源（is_stationary = TRUE）
    IF radiation_stationary = FALSE THEN
        SIGNAL SQLSTATE '45000' 
        SET MESSAGE_TEXT = '错误：单平台任务只能关联固定辐射源';
    END IF;
END$$

-- 删除旧触发器（如果存在）
DROP TRIGGER IF EXISTS `before_update_single_platform_task`$$

-- 创建更新前校验触发器
CREATE TRIGGER `before_update_single_platform_task` 
BEFORE UPDATE ON `single_platform_task` 
FOR EACH ROW 
BEGIN
    DECLARE device_stationary BOOLEAN;
    DECLARE radiation_stationary BOOLEAN;
    
    -- 仅在校验字段变更时触发，提升性能
    IF NEW.device_id != OLD.device_id OR NEW.radiation_id != OLD.radiation_id THEN
        -- 获取关联侦察设备的is_stationary状态
        SELECT is_stationary INTO device_stationary
        FROM reconnaissance_device_models
        WHERE device_id = NEW.device_id;
        
        -- 获取关联辐射源的is_stationary状态
        SELECT is_stationary INTO radiation_stationary
        FROM radiation_source_models
        WHERE radiation_id = NEW.radiation_id;
        
        -- 1. 校验侦察设备必须为运动设备（is_stationary = FALSE）
        IF device_stationary = TRUE THEN
            SIGNAL SQLSTATE '45000' 
            SET MESSAGE_TEXT = '错误：单平台任务只能关联运动侦察设备';
        END IF;
        
        -- 2. 校验辐射源必须为固定辐射源（is_stationary = TRUE）
        IF radiation_stationary = FALSE THEN
            SIGNAL SQLSTATE '45000' 
            SET MESSAGE_TEXT = '错误：单平台任务只能关联固定辐射源';
        END IF;
    END IF;
END$$

-- 恢复默认分隔符
DELIMITER ;

-- 启用外键检查
SET FOREIGN_KEY_CHECKS = 1;

ALTER TABLE single_platform_task
ADD COLUMN max_positioning_distance FLOAT COMMENT '最远定位距离（米）' AFTER target_altitude,
ADD COLUMN positioning_time FLOAT COMMENT '定位时间（秒）' AFTER max_positioning_distance,
ADD COLUMN positioning_accuracy DECIMAL(8,6) COMMENT '定位精度（米）' AFTER positioning_time,
ADD COLUMN direction_finding_accuracy DECIMAL(8,6) COMMENT '测向精度（度）' AFTER angle_error;