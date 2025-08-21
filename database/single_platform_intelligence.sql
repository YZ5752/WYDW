SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

DROP TABLE IF EXISTS `single_platform_intelligence`;
CREATE TABLE `single_platform_intelligence` (
  `intelligence_id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '主键，自增',
  `radiation_id` int(10) unsigned NOT NULL COMMENT '关联辐射源ID',
  `radiation_name` varchar(50) NOT NULL COMMENT '关联辐射源名称',
  `target_longitude` decimal(9,6) NOT NULL COMMENT '目标经度（度），范围-180~180',
  `target_latitude` decimal(9,6) NOT NULL COMMENT '目标纬度（度），范围-90~90',
  `target_altitude` double NOT NULL COMMENT '目标高度（米，大地高）',
  `target_angle` decimal(8,5) NOT NULL COMMENT '测向数据（度，正北为0，顺时针）',
  PRIMARY KEY (`intelligence_id`) USING BTREE,
  KEY `radiation_id` (`radiation_id`) USING BTREE,
  CONSTRAINT `single_platform_intelligence_ibfk_1` FOREIGN KEY (`radiation_id`) REFERENCES `radiation_source_models` (`radiation_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC COMMENT='单平台情报数据表';

-- 插入示例情报数据
INSERT INTO `single_platform_intelligence` 
(radiation_id, radiation_name, target_longitude, target_latitude, 
 target_altitude, target_angle) 
VALUES
(1, '固定辐射源A', 116.500000, 39.950000, 200, 45.00000);

SET FOREIGN_KEY_CHECKS = 1;