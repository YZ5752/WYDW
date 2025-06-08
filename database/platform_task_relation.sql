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

 Date: 07/06/2025 12:33:01
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for platform_task_relation
-- ----------------------------
DROP TABLE IF EXISTS `platform_task_relation`;
CREATE TABLE `platform_task_relation`  (
  `relation_id` int(0) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键，自增',
  `simulation_id` int(0) UNSIGNED NOT NULL COMMENT '关联多平台任务ID',
  `device_id` int(0) UNSIGNED NOT NULL COMMENT '关联侦察设备模型ID（固定/运动均可）',
  PRIMARY KEY (`relation_id`) USING BTREE,
  INDEX `simulation_id`(`simulation_id`) USING BTREE,
  INDEX `device_id`(`device_id`) USING BTREE,
  CONSTRAINT `platform_task_relation_ibfk_1` FOREIGN KEY (`simulation_id`) REFERENCES `multi_platform_task` (`task_id`) ON DELETE CASCADE ON UPDATE RESTRICT,
  CONSTRAINT `platform_task_relation_ibfk_2` FOREIGN KEY (`device_id`) REFERENCES `reconnaissance_device_models` (`device_id`) ON DELETE CASCADE ON UPDATE RESTRICT
) ENGINE = InnoDB AUTO_INCREMENT = 8 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci COMMENT = '多平台任务与侦察设备关联表' ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of platform_task_relation
-- ----------------------------
INSERT INTO `platform_task_relation` VALUES (1, 1, 1);
INSERT INTO `platform_task_relation` VALUES (2, 1, 2);
INSERT INTO `platform_task_relation` VALUES (3, 1, 3);
INSERT INTO `platform_task_relation` VALUES (4, 1, 4);
INSERT INTO `platform_task_relation` VALUES (5, 2, 5);
INSERT INTO `platform_task_relation` VALUES (6, 2, 6);
INSERT INTO `platform_task_relation` VALUES (7, 2, 7);

SET FOREIGN_KEY_CHECKS = 1;
