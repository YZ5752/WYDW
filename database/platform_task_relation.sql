SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

DROP TABLE IF EXISTS `platform_task_relation`;
CREATE TABLE `platform_task_relation` (
  `relation_id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '主键，自增',
  `simulation_id` int(10) unsigned NOT NULL COMMENT '关联多平台任务ID',
  `device_id` int(10) unsigned NOT NULL COMMENT '关联侦察设备模型ID（固定/运动均可）',
  PRIMARY KEY (`relation_id`) USING BTREE,
  KEY `simulation_id` (`simulation_id`) USING BTREE,
  KEY `device_id` (`device_id`) USING BTREE,
  CONSTRAINT `platform_task_relation_ibfk_1` FOREIGN KEY (`simulation_id`) REFERENCES `multi_platform_task` (`task_id`) ON DELETE CASCADE,
  CONSTRAINT `platform_task_relation_ibfk_2` FOREIGN KEY (`device_id`) REFERENCES `reconnaissance_device_models` (`device_id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC COMMENT='多平台任务与侦察设备关联表';

-- 插入示例关联数据
INSERT INTO `platform_task_relation` 
(simulation_id, device_id) 
VALUES
(1, 1),
(1, 2),
(1, 3),
(1, 4),
(2, 5),
(2, 6),
(2, 7);

SET FOREIGN_KEY_CHECKS = 1;