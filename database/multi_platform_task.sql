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

 Date: 07/06/2025 12:32:40
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for multi_platform_task
-- ----------------------------
DROP TABLE IF EXISTS `multi_platform_task`;
CREATE TABLE `multi_platform_task`  (
  `task_id` int(0) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键，自增',
  `tech_system` enum('FDOA','TDOA') CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '技术体制：频差或时差体制',
  `radiation_id` int(0) UNSIGNED NOT NULL COMMENT '关联辐射源模型ID（固定/运动均可）',
  `execution_time` float NOT NULL COMMENT '仿真执行时长（秒）',
  `target_longitude` decimal(9, 6) NOT NULL COMMENT '目标经度（度），范围-180~180',
  `target_latitude` decimal(9, 6) NOT NULL COMMENT '目标纬度（度），范围-90~90',
  `target_altitude` double NOT NULL COMMENT '目标高度（米，大地高）',
  `movement_speed` float NOT NULL COMMENT '运动速度（米/秒），固定设备默认0',
  `movement_azimuth` decimal(5, 2) NOT NULL COMMENT '运动方位角（度，正北为0，顺时针），固定设备默认0',
  `movement_elevation` decimal(4, 2) NOT NULL COMMENT '运动俯仰角（度，水平面为0，向上为正），固定设备默认0',
  `created_at` datetime(0) NULL DEFAULT CURRENT_TIMESTAMP(0) COMMENT '任务创建时间，默认当前时间',
  PRIMARY KEY (`task_id`) USING BTREE,
  INDEX `radiation_id`(`radiation_id`) USING BTREE,
  CONSTRAINT `multi_platform_task_ibfk_1` FOREIGN KEY (`radiation_id`) REFERENCES `radiation_source_models` (`radiation_id`) ON DELETE RESTRICT ON UPDATE RESTRICT
) ENGINE = InnoDB AUTO_INCREMENT = 3 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci COMMENT = '多平台仿真任务表' ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of multi_platform_task
-- ----------------------------
INSERT INTO `multi_platform_task` VALUES (1, 'TDOA', 1, 7200, 116.500000, 39.950000, 200, 0, 0.00, 0.00, '2025-06-06 15:42:18');
INSERT INTO `multi_platform_task` VALUES (2, 'FDOA', 2, 3600, 118.800000, 32.100000, 100, 30, 180.00, 0.00, '2025-06-06 15:42:18');

SET FOREIGN_KEY_CHECKS = 1;

ALTER TABLE multi_platform_task
ADD COLUMN positioning_distance FLOAT COMMENT '定位距离（米）' AFTER target_altitude,
ADD COLUMN positioning_time FLOAT COMMENT '定位时间（秒）' AFTER positioning_distance,
ADD COLUMN positioning_accuracy DECIMAL(8,6) COMMENT '定位精度（米）' AFTER positioning_time;
ALTER TABLE multi_platform_task
ADD COLUMN azimuth decimal(5, 2) COMMENT '方位角' AFTER movement_elevation,
ADD COLUMN elevation decimal(4, 2) COMMENT '俯仰角' AFTER azimuth;
