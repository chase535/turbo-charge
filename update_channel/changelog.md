# v65

### Changelog

1.cmake可通过定义环境变量MIMALLOC_VERSION来查找指定版本的mimalloc

2.将bypass_charge.txt文件中的内容加载至内存，仅在文件发生修改时才重新加载，而不是每次循环都直接读取本地文件，减少系统资源消耗

3.修复潜在的内存溢出问题

4.优化代码，提高程序运行效率

