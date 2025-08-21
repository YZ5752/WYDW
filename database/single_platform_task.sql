SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

DROP TABLE IF EXISTS `single_platform_task`;
CREATE TABLE `single_platform_task` (
  `task_id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '主键，自增',
  `positioning_algorithm` enum('FAST','BASELINE') NOT NULL COMMENT '定位算法：快速定位或基线定位',
  `device_id` int(10) unsigned NOT NULL COMMENT '关联侦察设备模型ID（仅移动设备）',
  `radiation_id` int(10) unsigned NOT NULL COMMENT '关联辐射源模型ID（仅固定辐射源）',
  `execution_time` float NOT NULL COMMENT '仿真执行时长（秒）',
  `target_longitude` decimal(9,6) NOT NULL COMMENT '目标经度（度），范围-180~180',
  `target_latitude` decimal(9,6) NOT NULL COMMENT '目标纬度（度），范围-90~90',
  `target_altitude` double NOT NULL COMMENT '目标高度（米，大地高）',
  `positioning_distance` float DEFAULT NULL COMMENT '最远定位距离（米）',
  `positioning_time` float DEFAULT NULL COMMENT '定位时间（秒）',
  `positioning_accuracy` float DEFAULT NULL COMMENT '定位精度（米）',
  `azimuth` decimal(5,2) DEFAULT NULL COMMENT '方位角',
  `elevation` decimal(4,2) DEFAULT NULL COMMENT '俯仰角',
  `angle_error` decimal(8,6) NOT NULL COMMENT '测向误差（度），正数',
  `direction_finding_accuracy` float DEFAULT NULL COMMENT '测向精度（度）',
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP COMMENT '任务创建时间，默认当前时间',
  PRIMARY KEY (`task_id`) USING BTREE,
  KEY `device_id` (`device_id`) USING BTREE,
  KEY `radiation_id` (`radiation_id`) USING BTREE,
  CONSTRAINT `single_platform_task_ibfk_1` FOREIGN KEY (`device_id`) REFERENCES `reconnaissance_device_models` (`device_id`),
  CONSTRAINT `single_platform_task_ibfk_2` FOREIGN KEY (`radiation_id`) REFERENCES `radiation_source_models` (`radiation_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC COMMENT='单平台仿真任务表';

-- 插入示例任务数据
INSERT INTO `single_platform_task` 
(positioning_algorithm, device_id, radiation_id, execution_time, 
 target_longitude, target_latitude, target_altitude, positioning_distance, 
 positioning_time, positioning_accuracy, azimuth, elevation, angle_error, 
 direction_finding_accuracy, created_at) 
VALUES
('BASELINE', 4, 1, 3600, 116.500000, 39.950000, 200, 1000.0, 
 120.5, 5.0, 45.00, 0.00, 0.500000, 1.2, '2025-06-06 15:24:07'),
('FAST', 5, 1, 1800, 120.200000, 30.300000, 150, 800.0, 
 90.0, 8.0, 120.00, 0.00, 0.800000, 1.5, '2025-06-06 15:24:07');

DELIMITER $$
DROP TRIGGER IF EXISTS `before_insert_single_platform_task`$$
CREATE TRIGGER `before_insert_single_platform_task` 
BEFORE INSERT ON `single_platform_task` 
FOR EACH ROW 
BEGIN
    DECLARE device_stationary BOOLEAN;
    DECLARE radiation_stationary BOOLEAN;
    
    SELECT is_stationary INTO device_stationary
    FROM reconnaissance_device_models
    WHERE device_id = NEW.device_id;
    
    SELECT is_stationary INTO radiation_stationary
    FROM radiation_source_models
    WHERE radiation_id = NEW.radiation_id;
    
    IF device_stationary = TRUE THEN
        SIGNAL SQLSTATE '45000' 
        SET MESSAGE_TEXT = '错误：单平台任务只能关联运动侦察设备';
    END IF;
    IF radiation_stationary = FALSE THEN
        SIGNAL SQLSTATE '45000' 
        SET MESSAGE_TEXT = '错误：单平台任务只能关联固定辐射源';
    END IF;
END$$

DROP TRIGGER IF EXISTS `before_update_single_platform_task`$$
CREATE TRIGGER `before_update_single_platform_task` 
BEFORE UPDATE ON `single_platform_task` 
FOR EACH ROW 
BEGIN
    DECLARE device_stationary BOOLEAN;
    DECLARE radiation_stationary BOOLEAN;
    
    IF NEW.device_id != OLD.device_id OR NEW.radiation_id != OLD.radiation_id THEN
        SELECT is_stationary INTO device_stationary
        FROM reconnaissance_device_models
        WHERE device_id = NEW.device_id;
        
        SELECT is_stationary INTO radiation_stationary
        FROM radiation_source_models
        WHERE radiation_id = NEW.radiation_id;
        
        IF device_stationary = TRUE THEN
            SIGNAL SQLSTATE '45000' 
            SET MESSAGE_TEXT = '错误：单平台任务只能关联运动侦察设备';
        END IF;
        IF radiation_stationary = FALSE THEN
            SIGNAL SQLSTATE '45000' 
            SET MESSAGE_TEXT = '错误：单平台任务只能关联固定辐射源';
        END IF;
    END IF;
END$$
DELIMITER ;

SET FOREIGN_KEY_CHECKS = 1;