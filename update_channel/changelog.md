# v66

### Changelog

1.不再使用bsd库函数strlcpy，使用glibc库函数strncpy并限制size

2.释放regex_t结构体，避免内存泄漏

3.优化代码

