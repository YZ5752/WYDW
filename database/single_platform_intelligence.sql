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

 Date: 07/06/2025 12:32:29
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for single_platform_intelligence
-- ----------------------------
DROP TABLE IF EXISTS `single_platform_intelligence`;
CREATE TABLE `single_platform_intelligence`  (
  `intelligence_id` int(0) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键，自增',
  `radiation_id` int(0) UNSIGNED NOT NULL COMMENT '关联辐射源ID',
  `radiation_name` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '关联辐射源名称',
  `target_longitude` decimal(9, 6) NOT NULL COMMENT '目标经度（度），范围-180~180',
  `target_latitude` decimal(9, 6) NOT NULL COMMENT '目标纬度（度），范围-90~90',
  `target_altitude` double NOT NULL COMMENT '目标高度（米，大地高）',
  `target_angle` decimal(8, 5) NOT NULL COMMENT '测向数据（度，正北为0，顺时针）',
  PRIMARY KEY (`intelligence_id`) USING BTREE,
  INDEX `radiation_id`(`radiation_id`) USING BTREE,
  CONSTRAINT `single_platform_intelligence_ibfk_1` FOREIGN KEY (`radiation_id`) REFERENCES `radiation_source_models` (`radiation_id`) ON DELETE RESTRICT ON UPDATE RESTRICT
) ENGINE = InnoDB AUTO_INCREMENT = 2 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci COMMENT = '单平台情报数据表' ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of single_platform_intelligence
-- ----------------------------
INSERT INTO `single_platform_intelligence` VALUES (1, 1, '固定辐射源A', 116.500000, 39.950000, 200, 45.00000);

SET FOREIGN_KEY_CHECKS = 1;
