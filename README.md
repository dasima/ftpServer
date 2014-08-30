#mini版ftp服务器

#1. 搭框架
main.c: 主函数
common.h: 存放公共头文件
sysutil.* : 存放服务器相关函数
strutil.* : 存放字符串处理函数
ftp_nobody.* : 父进程相关函数
ftp_proto.* : 子进程相关函数
session.* : 会话（核心）模块

#2.
ftp_server.conf : 服务器配置文件
configure.* : 配置文件相关函数
parse_conf.* : 解析配置文件函数

#3. 
ftp_codes.h : FTP应答相关的宏
command_map.* : 命令映射函数



















