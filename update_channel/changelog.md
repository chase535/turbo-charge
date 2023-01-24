### Changelog
1.使用Github Actions进行静态交叉编译(CMake + aarch64-linux-musl-gcc)，自编译的aarch64-linux-musl-gcc，项目地址：[aarch64-linux-musl-gcc](https://github.com/chase535/aarch64-linux-musl-gcc)
2.部分代码重构，不再使用popen向系统传递命令，完全移除shell命令
3.减少内存占用
4.解决跳电问题(酷安@零漓殇度)
5.主程序文件直接放在模块目录下执行，不再复制到/system/bin
6.添加编译链接参数
7.更好的代码逻辑
8.统一一处变量类型
9.添加一个充电状态判断
10.循环间隔时间5秒
11.修复内存泄漏问题
12.修复电池温度少于两位数以及负数时出现的BUG
13.大量减小文件体积
14.优化代码
15.在模块安装时及模块的description中添加卸载模块的提示

**恢复半更新状态，我并不会适配不支持的机型，但您可通过提交PR来主动适配，亦可通过提交PR来优化程序**

已经开启CI构建，可在Actions页面下载CI构建版本以获取最新测试版，**不保证CI构建版本的功能性及稳定性**
