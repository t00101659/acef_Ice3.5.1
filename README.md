# acef_Ice3.5.1

本Ice源码主要基于3.5.1开源版本基础上保证对接口不变的情况下，对现有代码的不足和问题进行优化更新

可以支持windows，linux，arm，android等多个平台

OneKeyBuild.sh 可以完成linux，arm和android编译包括第三方依赖组件编译

OneKeyBuild_amd64.bat 为Windows编译脚本，主要适配VIS2013版本

OneKeyBuildIce.sh  支持预定第三方依赖文件编译Ice源码

本版本为了研发产品商用源码修改较多，主要修改如下几个特定：

1、主机多网卡检测，由于代理适配器发布代理在多个IP之上，部分IP不通，系统需要检测哪个IP可用，避免每个IP都去访问导致访问可能超时；

2、Ice日志优化和统一，原有ice日志输出策略很简单，运行时间长导致日志文件超大，同时日志信息较少，无法辅助问题定位，需要增加日志统一输出，同时增加业务层日志策略统一控制，废弃原有ice日志机制，默认不输出

3、增加3.6的ACM特性包括Router，icenode，icegrid，ice，考虑版本稳定和接口兼容，需要单独移植3.6的ACM特性，提供代理活动连接心跳检测，这样业务上层就不需要单独实现心跳检测，只需要捕获ACM连接回调即可；

4、增加一键式编译脚本

5、部分BUG修改；
