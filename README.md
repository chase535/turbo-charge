# turbo-charge

**开始上班，随缘更新**

酷安@诺鸡鸭

## 模块功能

删除温控，关闭阶梯式充电，持续修改电池温度及充电电流，尽可能的让手机满血快充。配置文件在/data/adb/turbo-charge/option.txt，可以更改一些参数，日志文件为同目录的log.txt

## 使用到的工具

1.使用自己构建的[aarch64-linux-musl-gcc](https://github.com/chase535/aarch64-linux-musl-gcc)交叉编译工具链进行编译链接，静态链接更高效，程序大小相较于使用基于glibc函数库的gnu-gcc生成的程序来说大量减少

2.使用[mimalloc](https://github.com/microsoft/mimalloc)代替原版malloc家族，以略微增加程序大小为代价，解决musl函数库malloc家族执行效率低的短板，提高程序运行效率

3.使用[CMake](https://cmake.org)自动生成Makefile文件（别问，问就是懒，不想学Makefile）

## 适配

**我并不会适配不支持的机型，但您可通过提交PR来主动适配，亦可通过提交PR来优化程序**

适配需要文件，但问题就在这，我也不知道需要哪些文件，之前收集过很多不适配的手机的各种文件，但是发现并没有需要的文件，而且没有的很彻底，所以我无从下手，无法进行适配

## CI构建

已经开启CI构建，可在[Actions页面](https://github.com/chase535/turbo-charge/actions)下载CI构建版本以获取最新测试版，**不保证CI构建版本的功能性及稳定性**

## 开源协议

程序遵循**AGPLv3**开源协议
