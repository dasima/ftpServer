# 简易的FTP服务器

## 简介

功能：实现一个简易的FTP服务器，支持文件的上传、下载，以及断点续传

- 采用多线程模型完成

- 有port和pasv两种工作模式

## 函数功能简单介绍

1. 基本模块：
main.c sysutil.c strutil.c session.c ftp_nobody.c ftp_proto.c common.h
其中，session模块是核心，
ftp_nobody 模块实现nobody功能，主要实现控制功能，
ftp_proto 模块负责一些具体的实现，实现数据传输功能
sysutil模块是系统函数模块、strutil模块是字符串处理模块，common.h用来存放公共头文件；
2. configure.c parse_conf.c ftp_codes.h
分别是配置文件模块，配置文件解析模块、FTP应答的宏模块；
3. 具体实现模块
command_map.c priv_sock.c trans_data.c trans_ctrl.c
分别对应的是命令映射控制模块、命令请求和应答模块、数据传输模块、控制连接和数据连接模块(上传和下载限速等)
4. 其他
hash.c ftp_assist.c
哈希模块（用来控制客户端和ip连接数）、main函数实现模块

## 使用

1. 修改配置信息

    根据需求修改配置文件 `ftpserver.conf` 中的值，确保防火墙打开相应的端口，确保主机 `21` 端口没被占用

2. 编译

    Linux 系统下，切换到源码所在目录， 输入命令: `make`

3. 运行

    输入 `./ftpServer`，即可运行程序

4. 客户端连接

    使用 `ftp` 客户端，连接当前主机，登录的用户名和密码为当前主机的用户名及密码
