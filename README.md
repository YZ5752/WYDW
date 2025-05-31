# 无源侦察协同定位仿真系统 (Passive Location Simulation System)

本项目是一个用于雷达侦察和辐射源模型仿真的系统，支持单平台和多平台协同侦察效能分析。

## 功能特点

- 雷达侦察和辐射源建模
- 单平台雷达侦察效能仿真分析
- 多平台协同侦察效能仿真分析
- 无源侦察情报数据处理与评估
- 多种仿真评估模式支持：任务前、实时、任务后

## 系统要求

- Ubuntu 18.04 LTS
- Visual C++ (在Ubuntu下可使用g++)
- GTK+ 3.0
- ODBC驱动和连接库
- CMake 3.10或更高版本

## 构建与安装

### 前提条件

安装必要的依赖项：

```bash
sudo apt-get update
sudo apt-get install build-essential cmake
sudo apt-get install libgtk-3-dev
sudo apt-get install unixodbc-dev
```

### 构建步骤


1. 克隆代码库

```bash
git clone git@github.com:YZyh2519/passivelocation.git
cd passivelocation
```

2. 创建构建目录

```bash
mkdir build
cd build
```

3. 配置和构建项目

```bash
cmake ..
make
```

4. 安装应用程序

```bash
sudo make install
```

## 数据库配置

本系统使用ODBC连接数据库。您需要设置一个名为"PassiveLocationDSN"的ODBC数据源，可以连接到MySQL、PostgreSQL或其他支持ODBC的数据库系统。

### 创建数据库表

```bash
# 对于MySQL
sudo mysql -u root passivelocation < database/schema.sql


```

### 配置ODBC

1. 安装ODBC驱动

```bash
# 对于MySQL
sudo apt-get install libmyodbc

# 对于PostgreSQL
sudo apt-get install odbc-postgresql
```

2. 编辑odbc.ini文件

```bash
sudo nano /etc/odbc.ini
```

添加以下内容：

```
[PassiveLocationDSN]
Description     = Passive Location Database
Driver          = MySQL
Server          = localhost
Database        = passive_location
Port            = 3306
User            = username
Password        = password
```

## 运行应用程序

```bash
./passivelocation
```

## 项目结构

- `include/` - 头文件
- `src/` - 源代码
- `res/` - 资源文件
- `database/` - 数据库脚本
- `doc/` - 文档



1. 雷达侦察设备模型开发
   - 在 `radar_model.h` 中定义模型接口
   - 实现不同的技术体制（干涉仪体制、时差体制）

2. 辐射源模型开发
   - 在 `radar_model.h` 中定义辐射源接口
   - 根据发射功率、扫描周期等参数实现模型

3. 仿真算法开发
   - 单平台算法：快速定位、基线定位
   - 多平台算法：时差定位、频差定位、测向定位

4. UI开发
   - 使用GTK+ 3.0构建用户界面
   - 支持地图显示、参数设置、结果展示

