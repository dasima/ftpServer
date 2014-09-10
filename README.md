#功能：实现一个简易的FTP服务器，支持文件的上传、下载，以及断点续传
#采用多线程模型完成
#有port和pasv两种工作模式

#下面是个函数功能简单介绍
1, 基本模块：
main.c sysutil.c strutil.c session.c ftp_nobody.c ftp_proto.c common.h
其中，session模块是核心，
ftp_nobody模块实现nobody功能，主要实现控制功能，
ftp_proto模块负责一些具体的实现，实现数据传输功能
sysutil模块是系统函数模块、strutil模块是字符串处理模块，common.h用来存放公共头文件；
2，configure.c parse_conf.c ftp_codes.h
分别是配置文件模块，配置文件解析模块、FTP应答的宏模块；
3， 具体实现模块
command_map.c priv_sock.c trans_data.c trans_ctrl.c 
分别对应的是命令映射控制模块、命令请求和应答模块、数据传输模块、控制连接和数据连接模块(上传和下载限速等)
4， 其他
hash.c ftp_assist.c
哈希模块（用来控制客户端和ip连接数）、main函数实现模块
