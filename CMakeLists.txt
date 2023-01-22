# CMake 最低版本号要求
cmake_minimum_required(VERSION 2.8.12)

# 项目信息
project(turbo-charge)

# 配置环境
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "aarch64")

# 设置编译链接参数
set(CMAKE_C_FLAGS "-g0 -O2 -W -Wall -fvisibility=hidden -flto -fdata-sections -ffunction-sections")
set(CMAKE_EXE_LINKER_FLAGS "-g0 -O2 -static -fvisibility=hidden -flto -ffixed-x18 -Wl,-O2,--strip-all,--hash-style=both,--gc-sections,--exclude-libs,ALL")

# 指定生成目标
add_executable(turbo-charge turbo-charge.c)
