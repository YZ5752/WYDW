SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

DROP TABLE IF EXISTS `multi_platform_intelligence`;
CREATE TABLE `multi_platform_intelligence` (
  `intelligence_id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '主键，自增',
  `radiation_id` int(10) unsigned NOT NULL COMMENT '关联辐射源ID',
  `radiation_name` varchar(50) NOT NULL COMMENT '关联辐射源名称',
  `target_longitude` decimal(9,6) NOT NULL COMMENT '目标经度（度），范围-180~180',
  `target_latitude` decimal(9,6) NOT NULL COMMENT '目标纬度（度），范围-90~90',
  `target_altitude` double NOT NULL COMMENT '目标高度（米，大地高）',
  `movement_speed` float NOT NULL COMMENT '运动速度（米/秒），固定设备默认0',
  `movement_azimuth` decimal(5,2) NOT NULL COMMENT '运动方位角（度，正北为0，顺时针），固定设备默认0',
  `movement_elevation` decimal(4,2) NOT NULL COMMENT '运动俯仰角（度，水平面为0，向上为正），固定设备默认0',
  PRIMARY KEY (`intelligence_id`) USING BTREE,
  KEY `radiation_id` (`radiation_id`) USING BTREE,
  CONSTRAINT `multi_platform_intelligence_ibfk_1` FOREIGN KEY (`radiation_id`) REFERENCES `radiation_source_models` (`radiation_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC COMMENT='多平台情报数据表（记录多平台协同定位的辐射源综合结果）';

-- 插入示例情报数据
INSERT INTO `multi_platform_intelligence` 
(radiation_id, radiation_name, target_longitude, target_latitude, 
 target_altitude, movement_speed, movement_azimuth, movement_elevation) 
VALUES
(1, '固定辐射源A', 116.500000, 39.950000, 200, 0, 0.00, 0.00),
(2, '移动辐射源A', 118.810000, 32.110000, 100, 30, 180.00, 0.00);

SET FOREIGN_KEY_CHECKS = 1;