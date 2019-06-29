# keeper
守护进程
##在Windows服务器上以系统服务形式间接启动自定义程序进程, 并守护自定义进程,在其意外退出时重新运行
#安装
1.配置WindowsService.conf(json格式)文件
WindowsService.conf内容示例:

{"FilePath":"my_test.exe","ServiceName":"MyTest","ServiceDisplayName":"My Test","ServiceDescription":"Windows Service Bind My Test Exe"}

FilePath: 服务运行时启动的进程文件
ServiceName: 服务安装名称
ServiceDisplayName: 服务显示名称
ServiceDescription: 服务描述
2.服务安装
以管理员权限运行bin/install.cmd
3.服务卸载
以管理员权限运行bin/uninstall.cmd
