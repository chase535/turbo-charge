# CMake最低版本号要求
cmake_minimum_required(VERSION 3.14)

# 项目信息
project(turbo-charge)

# 配置环境
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "aarch64")

# 发行版本
set(CMAKE_BUILD_TYPE "Release")

# 设置编译链接参数
set(CMAKE_C_FLAGS "-g0 -O2 -W -Wall -Wno-format-truncation -Wno-unused-result -Werror -fvisibility=hidden -flto -fno-omit-frame-pointer -fdata-sections -ffunction-sections")
set(CMAKE_EXE_LINKER_FLAGS "-g0 -O2 -static -fvisibility=hidden -flto -ffixed-x18 -Wl,-O2,--strip-all,--hash-style=both,--gc-sections,--exclude-libs,ALL")

# 查找mimalloc包
if(DEFINED ENV{MIMALLOC_VERSION})
  message(STATUS "'MIMALLOC_VERSION' is defined in the environment variable, find mimalloc version ENV{MIMALLOC_VERSION}")
  find_package(MIMALLOC NO_SYSTEM_ENVIRONMENT_PATH ENV{MIMALLOC_VERSION})
else()
  message(STATUS "'MIMALLOC_VERSION' is not defined in the environment variable, find any usable mimalloc version")
  find_package(MIMALLOC NO_SYSTEM_ENVIRONMENT_PATH)
endif()
if(MIMALLOC_FOUND)
  message(STATUS "Find mimalloc installed at: ${MIMALLOC_LIBRARY_DIR} (${MIMALLOC_VERSION_DIR}), use mimalloc")
  add_compile_definitions(MI_MALLOC)
else(MIMALLOC_FOUND)
  message(STATUS "Could not find mimalloc, use malloc")
endif(MIMALLOC_FOUND)

# 添加头文件目录
include_directories(./inc)

# 指定生成目标
aux_source_directory(./src SRC_LIST)
add_executable(turbo-charge ${SRC_LIST})

# 链接静态mimalloc库
if(MIMALLOC_FOUND)
  target_link_libraries(turbo-charge PUBLIC mimalloc-static)
endif(MIMALLOC_FOUND)
