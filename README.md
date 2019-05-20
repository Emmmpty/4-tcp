# 4-tcp

## 实验说明

本实验要求学生在已有代码基础上，完善其中的TODO部分，实现TCP连接管理、数据传输、丢包恢复、拥塞控制等功能，使得两个协议栈之间能够进行可靠数据传输，并根据丢包情况按照TCP newReno规则调整自己的拥塞窗口大小。附件中给了两个拓扑，每个拓扑各包含2个主机节点，其中一个拓扑中链路无丢包，仅用于测试使用，另一个拓扑中链路有一定的丢包率。基于有丢包的拓扑，主机节点h1上运行TCP Server程序(./tcp_stack server 10001)，主机h2上运行TCP Client程序(./tcp_stack client 10.0.0.1 10001)，两个协议栈之间进行数据传输。使用md5sum程序验证，Server接收到的数据应与Client发送的数据完全相同。在Client程序（数据发送方）记录每次拥塞窗口变化的时间和大小，在有丢包环境下，拥塞窗口的变化图应类似锯齿状。

需要说明的是，该实验依赖IP查找转发的功能，对应静态路由转发实验中的C文件实现，这些功能以静态链接库的形式提供给大家，其API使用方法可以参考上述C文件，里面有更详细的注释，或者参考本实验include文件下的各个头文件。


## TODO
1. tcp_sock.c
2. tcp_in.c
3. tcp_timer.c
4. tcp_out.c
5. tcp_sock.c 

##实验过程

1、对已有的代码逻辑进行整理，核心函数tcp_sock包含主要功能进行编写

```c

// lookup tcp sock in established_table with key (saddr, daddr, sport, dport)
struct tcp_sock *tcp_sock_lookup_established(u32 saddr, u32 daddr, u16 sport, u16 dport)
{
	fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);

	return NULL;
}

// lookup tcp sock in listen_table with key (sport)
//
// In accordance with BSD socket, saddr is in the argument list, but never used.
struct tcp_sock *tcp_sock_lookup_listen(u32 saddr, u16 sport)
{
	fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);

	return NULL;
}

// connect to the remote tcp sock specified by skaddr
//
// XXX: skaddr here contains network-order variables
// 1. initialize the four key tuple (sip, sport, dip, dport);
// 2. hash the tcp sock into bind_table;
// 3. send SYN packet, switch to TCP_SYN_SENT state, wait for the incoming
//    SYN packet by sleep on wait_connect;
// 4. if the SYN packet of the peer arrives, this function is notified, which
//    means the connection is established.
int tcp_sock_connect(struct tcp_sock *tsk, struct sock_addr *skaddr)
{
	fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);

	return -1;
}

// set backlog (the maximum number of pending connection requst), switch the
// TCP_STATE, and hash the tcp sock into listen_table
int tcp_sock_listen(struct tcp_sock *tsk, int backlog)
{
	fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);

	return -1;
}

// check whether the accept queue is full
inline int tcp_sock_accept_queue_full(struct tcp_sock *tsk)
{
	if (tsk->accept_backlog >= tsk->backlog) {
		log(ERROR, "tcp accept queue (%d) is full.", tsk->accept_backlog);
		return 1;
	}

	return 0;
}

// if accept_queue is not emtpy, pop the first tcp sock and accept it,
// otherwise, sleep on the wait_accept for the incoming connection requests
struct tcp_sock *tcp_sock_accept(struct tcp_sock *tsk)
{
	fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);

	return NULL;
}

// similar to read function, try to read from socket tsk
int tcp_sock_read(struct tcp_sock *tsk, char *buf, int size)
{
	fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);

	return 0;
}

// similar to write function, try to write to socket tsk
int tcp_sock_write(struct tcp_sock *tsk, char *buf, int size)
{
	fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);

	return -1;
}

// close the tcp sock, by releasing the resources, sending FIN/RST packet
// to the peer, switching TCP_STATE to closed
void tcp_sock_close(struct tcp_sock *tsk)
{
	fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);
}

```
