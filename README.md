# 4-tcp

## 实验说明

本实验要求学生在已有代码基础上，完善其中的TODO部分，实现TCP连接管理、数据传输、丢包恢复、拥塞控制等功能，使得两个协议栈之间能够进行可靠数据传输，并根据丢包情况按照TCP newReno规则调整自己的拥塞窗口大小。附件中给了两个拓扑，每个拓扑各包含2个主机节点，其中一个拓扑中链路无丢包，仅用于测试使用，另一个拓扑中链路有一定的丢包率。基于有丢包的拓扑，主机节点h1上运行TCP Server程序(./tcp_stack server 10001)，主机h2上运行TCP Client程序(./tcp_stack client 10.0.0.1 10001)，两个协议栈之间进行数据传输。使用md5sum程序验证，Server接收到的数据应与Client发送的数据完全相同。在Client程序（数据发送方）记录每次拥塞窗口变化的时间和大小，在有丢包环境下，拥塞窗口的变化图应类似锯齿状。

需要说明的是，该实验依赖IP查找转发的功能，对应静态路由转发实验中的C文件实现，这些功能以静态链接库的形式提供给大家，其API使用方法可以参考上述C文件，里面有更详细的注释，或者参考本实验include文件下的各个头文件。


## TODO
1. tcp_sock.c
2. tcp_in.c
3. tcp_timer.c


## 实验过程

1、因为tcp_sock是tcp连接通信的核心，首先整理tcp连接代码逻辑

```c

// lookup tcp sock in established_table with key (saddr, daddr, sport, dport)

```

2、tcp_in

3、tcp_timer
