# 在 CMakeLists.txt 的开头添加
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

cmake_minimum_required(VERSION 3.10)
project(passivelocation VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找GTK3
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# 查找WebKit2GTK
pkg_check_modules(WEBKIT2 REQUIRED webkit2gtk-4.0)

# 查找ODBC
find_package(ODBC REQUIRED MODULE)

# 包含头文件路径
include_directories(
    ${GTK3_INCLUDE_DIRS}
    ${WEBKIT2_INCLUDE_DIRS}
    ${ODBC_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接目录
link_directories(
    ${GTK3_LIBRARY_DIRS}
    ${WEBKIT2_LIBRARY_DIRS}
)

# 添加源文件
file(GLOB SOURCES "src/*.cpp")

# 确保包含ui_static_methods.cpp
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/ui_static_methods.cpp")

# 创建可执行文件（仅一次）
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接库
target_link_libraries(${PROJECT_NAME} 
    ${GTK3_LIBRARIES} 
    ${WEBKIT2_LIBRARIES}
    ${ODBC_LIBRARIES}
    stdc++fs  # 添加filesystem库支持
)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 安装目标
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# 复制资源文件
install(DIRECTORY res/ DESTINATION share/passivelocation/res)

# 复制数据库脚本
install(DIRECTORY database/ DESTINATION share/passivelocation/database)

# 添加测试
enable_testing()
# 这里可以添加测试案例

# 添加构建脚本
add_custom_target(run
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 显示构建信息
message(STATUS "GTK3 include dirs: ${GTK3_INCLUDE_DIRS}")
message(STATUS "WebKit2GTK include dirs: ${WEBKIT2_INCLUDE_DIRS}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID}")