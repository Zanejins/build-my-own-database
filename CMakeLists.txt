cmake_minimum_required(VERSION 3.23)
project(build-my-own-database)

# c++14规范
set(CMAKE_CXX_STANDARD 14)

# 添加头文件
include_directories(include)

# 添加源文件
aux_source_directory(./src DIR_SRCS)

# 所有需要编译的可执行文件
add_executable(build-my-own-database ${DIR_SRCS})
