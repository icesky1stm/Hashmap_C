#HashMap&List

###1.本库使用了C语言实现了一个基本的hashmap,同时实现了一个list
参考java的hashmap实现机制,实现了C基本的
*          hashmap实现，支持几种功能:
*          1.新建
*          2.销毁
*          3.加入/设置
*          4.获取
*          5.删除
*          6.判断是否存在
*          7.打印
list实现，支持几种功能:
 *          1.新建
 *          2.销毁
 *          3:.加入
 *          4.获取
 *          5.打印



###2.哈希算法
哈希算法使用了JAVA的JDK中默认的simple BKDR hash algorithm
有需要的也可以替换成暴雪的One-Way-Hash或者PHP中的time33之类的

###3.编译方法
XipHashMap.c和XipList.c可以执行make -f makefile.osc编译成libkmaplib.so

HashMap_test.c和List_test.c 可以使用mk来编译成可执行程序

我是在cygwin环境下编写的,linux下要改一下makefile中的cc -shared命令, unix类似,可以生成动态库
当然也可以直接把代码copy

###4.注意事项
1).使用的时候,调用程序请注意包含头文件hashmap.h来声明调用函数原型 ，否则可能会导致

###5.更新说明
2016.08.01更新:
    1)增加了list
    2)扩展了printf，支持回调函数自己定义打印格式。
    3)增加了hashmap和list的malloc_flag字段，可以控制是否在map或list中额外分配内存保存value
