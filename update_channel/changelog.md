### Changelog

1.在模块安装以及程序运行时判断必要文件是否存在，若不存在，则证明不适配此手机，模块安装失败/程序强制停止运行

2.电量低于20%时开启阶梯充电，以解决健康状态不太好的电池在低电量时充电断充的问题（酷安@来回拉扯 提供问题产生原因，只经过我手机的测试，阈值为15%，所以就设置为20%）

3.添加Magisk内部的更新通道，使用Cloudflare做CDN加速，国内劣质网络环境也能正常检测、下载更新

4.重写安装脚本，以适配使用Magisk自带的util_functions.sh脚本进行安装（要求Magisk版本大于20.4）