# 协同雷达侦察仿真评估系统

基于Ubuntu 18.04系统开发的协同雷达侦察仿真评估系统，使用GTK3实现界面。

## 功能简介

系统具备以下六个主要功能界面：

1. **雷达设备模型**：管理雷达设备模型，包括添加、编辑、删除功能
2. **辐射源模型**：管理辐射源模型，包括添加、编辑、删除功能
3. **单平台仿真**：进行单平台雷达侦察效能仿真分析
4. **多平台仿真**：进行多平台协同侦察效能仿真分析
5. **数据分选**：实现情报数据显示和数据分选功能
6. **仿真评估**：实现对侦察定位能力的各项指标评估

## 系统环境要求

- Ubuntu 18.04 LTS
- Visual C++ 或 g++ 8.4.0
- GTK 3.22及以上
- CMake 3.10及以上
- mysql 5.7

## 数据库配置
## 安装mysql5.7

## 修改数据库密码
```bash
sudo mysql -u root -p
ALTER USER 'root'@'localhost' IDENTIFIED BY '123456';
UPDATE mysql.user SET authentication_string=PASSWORD('123456') WHERE User='root' AND Host = 'localhost';
FLUSH PRIVILEGES;
```

## 下载dbeaver
```bash
sudo apt install openjdk-11-jre-headless
wget -O - https://dbeaver.io/debs/dbeaver.gpg.key | sudo apt-key add -
echo deb https://dbeaver.io/debs/dbeaver-ce / | sudo tee /etc/apt/sources.list.d/dbeaver.list
sudo apt update
sudo apt -y install dbeaver-ce
```

## 在dbeaver中运行database中的sql文件

## 安装依赖

在Ubuntu 18.04系统上安装所需的依赖库：

```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install libgtk-3-dev
sudo apt install pkg-config
sudo apt-get install libwebkit2gtk-4.0-dev
sudo apt install libmysqlclient-dev
```

## 编译构建

1. 克隆代码仓库：

```bash
git clone git@github.com:YZyh2519/passivelocation.git
```

2. 创建构建目录并编译：

```bash
mkdir build
cd build
cmake ..
make 
```

3. 运行程序：

```bash
./passivelocation
```

或使用CMake的自定义目标：

```bash
make run
```

## 使用说明

## git创建个人分支
```bash
git checkout -b dev
```
## 
例：
```bash
git checkout -b yz_dev
```

## git上传代码
```bash
git add .
git commit -m ''
git push -u origin dev
```
## 
例：
```bash
git add .
git commit -m '新增地图'
git push -u origin yz_dev
```

## git拉取代码
```bash
git fetch
git merge origin/main
```

### 雷达设备模型

- 点击"新增"按钮添加新的雷达设备模型
- 在弹出的对话框中输入雷达设备模型的名称、位置（经度纬度）、技术体制、基本参数、工作参数等信息
- 在列表中选择已有模型，点击"编辑"或"删除"可修改或删除模型

### 辐射源模型

- 点击"新增"按钮添加新的辐射源模型
- 在弹出的对话框中输入辐射源模型的名称、位置（经度纬度）、发射功率、扫描周期、频率范围、工作扇区等信息
- 在列表中选择已有模型，点击"编辑"或"删除"可修改或删除模型

### 单平台仿真

- 从下拉列表中选择雷达设备模型和辐射源模型
- 选择定位算法（快速定位、基线定位）
- 点击"开始"按钮进行仿真计算
- 系统将在地图上展示仿真结果，并在右侧显示威力、测向误差、参数测量误差等数据

### 多平台仿真

- 从下拉列表中选择多个雷达设备模型和辐射源模型
- 选择定位算法（时差定位、频差定位、测向定位）
- 点击"开始"按钮进行多平台协同仿真计算
- 系统将在地图上展示仿真结果

### 数据分选

- 从下拉列表中选择目标
- 在表格中查看与目标相关的测向和定位数据
- 选择需要的数据，可以进行删除、高亮显示等操作
- 点击"录入"按钮将选择的数据存入数据库

### 仿真评估

- 选择目标和评估类型（单平台或多平台）
- 点击"开始评估"按钮进行评估计算
- 系统将显示最远定位距离、定位时间、定位精度、测向精度等指标
- 并显示定位精度随时间变化的图表
- 点击"导出结果"可将评估结果保存为文本文件

# 项目MVC架构说明

本项目采用MVC (Model-View-Controller) 架构进行组织，将代码按照职责分成三个主要部分。

## 架构概述

### 模型层 (Model)
- 位于 `models/` 目录
- 负责数据存储、检索和业务逻辑
- 包含数据库实体、数据访问对象 (DAO)
- 完全封装与数据库的交互

### 视图层 (View)
- 位于 `views/` 目录
- 负责UI展示和用户交互
- 包含各个页面的视图实现
- 可复用UI组件位于 `views/components/` 子目录

### 控制器层 (Controller)
- 位于 `controllers/` 目录
- 接收用户输入，调用模型处理数据，处理业务逻辑
- 更新视图显示
- 每个页面对应一个控制器

## 详细结构

### 模型层
```
models/
├── src/                            # 模型实现文件
│   ├── DBConnector.cpp             # 数据库连接器实现
│   ├── DataProcessorModel.cpp      # 数据处理器实现
│   ├── RadiationSourceModel.cpp    # 辐射源模型实现
│   ├── ReconnaissanceDeviceModel.cpp # 侦察设备模型实现
│   └── SimulationModel.cpp         # 模拟模型实现
├── CoordinateModel.h               # 坐标模型
├── DataProcessorModel.h            # 数据处理器
├── DBConnector.h                   # 数据库连接器
├── LocationResultModel.h           # 定位结果模型
├── RadiationSourceDAO.h            # 辐射源数据访问对象
├── RadiationSourceModel.h          # 辐射源模型
├── ReconnaissanceDeviceDAO.h       # 侦察设备数据访问对象
├── ReconnaissanceDeviceModel.h     # 侦察设备模型
├── SimulationManager.h             # 模拟管理器
├── SimulationModel.h               # 模拟模型
└── TargetIntelligenceModel.h       # 目标情报模型
```

### 视图层
```
views/
├── components/                     # 可复用UI组件
│   ├── src/
│   │   └── MapView.cpp             # 地图视图组件实现
│   └── MapView.h                   # 地图视图组件
├── src/                            # 视图实现文件
│   ├── DataSelectionView.cpp       # 数据选择视图实现
│   ├── EvaluationView.cpp          # 评估视图实现
│   ├── MultiPlatformView.cpp       # 多平台视图实现
│   ├── RadiationSourceModelView.cpp # 辐射源模型视图实现
│   └── SinglePlatformView.cpp      # 单平台视图实现
├── DataSelectionView.h             # 数据选择视图
├── EvaluationView.h                # 评估视图
├── MultiPlatformView.h             # 多平台视图
├── RadiationSourceModelView.h      # 辐射源模型视图
├── ReconnaissanceDeviceModelView.h # 侦察设备模型视图
└── SinglePlatformView.h            # 单平台视图
```

### 控制器层
```
controllers/
├── src/                            # 控制器实现文件
│   ├── ApplicationController.cpp   # 应用程序控制器实现
│   ├── DataSelectionController.cpp # 数据选择控制器实现
│   ├── EvaluationController.cpp    # 评估控制器实现
│   ├── MultiPlatformController.cpp # 多平台控制器实现
│   ├── RadiationSourceModelController.cpp # 辐射源模型控制器实现
│   ├── ReconnaissanceDeviceModelController.cpp # 侦察设备模型控制器实现
│   └── SinglePlatformController.cpp # 单平台控制器实现
├── ApplicationController.h         # 应用程序控制器（主控制器）
├── DataSelectionController.h       # 数据选择控制器
├── EvaluationController.h          # 评估控制器
├── MultiPlatformController.h       # 多平台控制器
├── RadiationSourceModelController.h # 辐射源模型控制器
├── ReconnaissanceDeviceModelController.h # 侦察设备模型控制器
└── SinglePlatformController.h      # 单平台控制器
```

## 文件命名规范

- 头文件（.h）：采用大驼峰命名法，如 `RadiationSourceModel.h`
- 实现文件（.cpp）：与对应的头文件同名，如 `RadiationSourceModel.cpp`
- 所有文件按照功能分类放在对应的目录中

## 通信流程

1. 用户在视图层进行操作
2. 视图层将操作传递给控制器层
3. 控制器层调用模型层处理数据
4. 模型层返回处理结果给控制器层
5. 控制器层更新视图层显示
