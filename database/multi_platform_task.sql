SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

DROP TABLE IF EXISTS `multi_platform_task`;
CREATE TABLE `multi_platform_task` (
  `task_id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '主键，自增',
  `positioning_algorithm` enum('TDOA','FDOA','DF') NOT NULL COMMENT '定位算法：时差定位、频差定位或测向定位',
  `radiation_id` int(10) unsigned NOT NULL COMMENT '关联辐射源模型ID（固定/运动均可）',
  `execution_time` float NOT NULL COMMENT '仿真执行时长（秒）',
  `target_longitude` decimal(9,6) NOT NULL COMMENT '目标经度（度），范围-180~180',
  `target_latitude` decimal(9,6) NOT NULL COMMENT '目标纬度（度），范围-90~90',
  `target_altitude` double NOT NULL COMMENT '目标高度（米，大地高）',
  `positioning_distance` float DEFAULT NULL COMMENT '定位距离（米）',
  `positioning_time` float DEFAULT NULL COMMENT '定位时间（秒）',
  `positioning_accuracy` float DEFAULT NULL COMMENT '定位精度（米）',
  `movement_speed` float NOT NULL COMMENT '运动速度（米/秒），固定设备默认0',
  `movement_azimuth` decimal(5,2) NOT NULL COMMENT '运动方位角（度，正北为0，顺时针），固定设备默认0',
  `movement_elevation` decimal(4,2) NOT NULL COMMENT '运动俯仰角（度，水平面为0，向上为正），固定设备默认0',
  `azimuth` decimal(5,2) DEFAULT NULL COMMENT '方位角',
  `elevation` decimal(4,2) DEFAULT NULL COMMENT '俯仰角',
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP COMMENT '任务创建时间，默认当前时间',
  PRIMARY KEY (`task_id`) USING BTREE,
  KEY `radiation_id` (`radiation_id`) USING BTREE,
  CONSTRAINT `multi_platform_task_ibfk_1` FOREIGN KEY (`radiation_id`) REFERENCES `radiation_source_models` (`radiation_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC COMMENT='多平台仿真任务表';

-- 插入示例任务数据
INSERT INTO `multi_platform_task` 
(positioning_algorithm, radiation_id, execution_time, target_longitude, 
 target_latitude, target_altitude, positioning_distance, positioning_time, 
 positioning_accuracy, movement_speed, movement_azimuth, movement_elevation, 
 azimuth, elevation, created_at) 
VALUES
('TDOA', 1, 7200, 116.500000, 39.950000, 200, 1500.0, 240.5, 10.0, 
 0, 0.00, 0.00, 30.50, 5.25, '2025-06-06 15:42:18'),
('FDOA', 2, 3600, 118.800000, 32.100000, 100, 1200.0, 180.0, 15.0, 
 30, 180.00, 0.00, 45.75, 2.30, '2025-06-06 15:42:18');

SET FOREIGN_KEY_CHECKS = 1;