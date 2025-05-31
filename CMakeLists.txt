# 在 CMakeLists.txt 的开头添加
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

cmake_minimum_required(VERSION 3.10)
project(passivelocation)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找GTK3
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# 包含GTK3的头文件路径
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

# 包含项目头文件路径
include_directories(include)

# 查找ODBC
find_package(ODBC REQUIRED MODULE)
include_directories(${ODBC_INCLUDE_DIRS})

# 添加源文件
file(GLOB SOURCES "src/*.cpp")

# 创建可执行文件
add_executable(passivelocation ${SOURCES})

# 链接库
target_link_libraries(passivelocation ${GTK3_LIBRARIES} ${ODBC_LIBRARIES})

# 安装目标
install(TARGETS passivelocation DESTINATION bin)

# 复制资源文件
install(DIRECTORY res/ DESTINATION share/passivelocation/res)

# 复制数据库脚本
install(DIRECTORY database/ DESTINATION share/passivelocation/database)

# 添加测试
enable_testing()
# 这里可以添加测试案例 