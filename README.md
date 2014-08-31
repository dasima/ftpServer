#mini版ftp服务器

test_ .c : 模块测试函数</br>

#1. 搭框架
main.c: 主函数</br>
common.h: 存放公共头文件</br>
sysutil.* : 存放服务器系统相关函数</br>
strutil.* : 字符串工具封装</br>
ftp_nobody.* : 父进程相关函数</br>
ftp_proto.* : 子进程相关函数</br>
session.* : 会话（核心）模块</br>

#2.
ftp_server.conf : 服务器配置文件</br>
configure.* : 参数配置模块</br>
parse_conf.* : 解析配置文件模块</br>

#3. 
ftp_codes.h : FTP应答相关的宏定义</br>
command_map.* : FTP命令映射实现</br>


