# CMake 最低版本号要求
cmake_minimum_required(VERSION 2.8.12)

# 项目信息
project(turbo-charge)

# 配置环境
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "aarch64")

# 设置编译链接参数
set(CMAKE_C_FLAGS "-O3 -fvisibility=hidden -flto -Wall -Werror -fdata-sections -ffunction-sections -fno-omit-frame-pointer -D_GNU_SOURCE")
set(CMAKE_EXE_LINKER_FLAGS "-static -O3 -fvisibility=hidden -flto -Wl,-O3,--strip-all -ffixed-x18 -Wl,--hash-style=both -fPIE -Wl,-exclude-libs,ALL -Wl,--gc-sections")

# 指定生成目标
add_executable(turbo-charge turbo-charge.c)
