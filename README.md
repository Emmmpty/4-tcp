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

## 步骤
1. 首先分析main.c 文件里面的函数调用流程
 main.c 里面中需要关注的是 run_application()函数的逻辑，该函数实现了一个tcp 服务端和tcp客户端。
 
### tcp 服务器
tcp 服务端主要代码在tcp_server()函数内，服务端就主要是收数据，发ack/syn啥的

```c
// tcp server application, listens to port (specified by arg) and serves only one
// connection request
void *tcp_server(void *arg)
{
	u16 port = *(u16 *)arg;
	struct tcp_sock *tsk = alloc_tcp_sock();  //调用alloc_tcp_sock 申请套接字资源

	struct sock_addr addr;
	addr.ip = htonl(0);
	addr.port = port;
	if (tcp_sock_bind(tsk, &addr) < 0)		  //调用tcp_sock_bind(),建立 tcp_sock结构体到特定socket的绑定关系。主要把东西加到bind_hash表上
	{
		log(ERROR, "tcp_sock bind to port %hu failed", ntohs(port));
		exit(1);
	}

	if (tcp_sock_listen(tsk, 3) < 0)		 //调用listen,这个要实现
	{
		log(ERROR, "tcp_sock listen failed");
		exit(1);
	}

	log(DEBUG, "listen to port %hu.", ntohs(port));

	struct tcp_sock *csk = tcp_sock_accept(tsk);	//accept函数的实现
	log(DEBUG, "accept a connection.");

	char rbuf[BUF_SIZE];
	FILE *file = fopen("server-output.dat", "wb");
	while (1) {
		int rlen = tcp_sock_read(csk, rbuf, BUF_SIZE);	//read的实现
		if (rlen == 0) {
			log(DEBUG, "tcp_sock_read return 0, finish transmission.");
			break;
		} 
		else if (rlen > 0) {
			fwrite(rbuf, 1, rlen, file);
		}
	}

	fclose(file);
	log(DEBUG, "close this connection.");
	printf("server: close tsk.\n");
	tcp_sock_close(csk); //close实现
	
	return NULL;
}
```

### 客户端

客户端会主动发送数据

```c
// tcp client application, connects to server (ip:port specified by arg), and
// send file to it.
void *tcp_client(void *arg)
{
	struct sock_addr *skaddr = arg;

	struct tcp_sock *tsk = alloc_tcp_sock();

	if (tcp_sock_connect(tsk, skaddr) < 0)	//需要实现
	{
		log(ERROR, "tcp_sock connect to server ("IP_FMT":%hu)failed.", \
				NET_IP_FMT_STR(skaddr->ip), ntohs(skaddr->port));
		exit(1);
	}
	char buf[BUF_SIZE];
	FILE *file = fopen("client-input.dat", "rb");
	while (!feof(file)) {
        int ret_size = fread(buf, 1, BUF_SIZE, file);
        tcp_sock_write(tsk, buf, ret_size);	//需要实现

        if (ret_size < BUF_SIZE) break;
		//usleep(500000);
    }

    fclose(file);
    printf("client: close tsk.\n");
	tcp_sock_close(tsk);					//关闭连接,需要实现。

	return NULL;
}
```

2. 整体的结构
 main 里面使用raw_socket来接收原生套接字,然后把数据数据包传递给我们写的socket去处理。我们写的代码是用户层的代码，直接在用户层“模拟”出来了一个tcp协议栈。

 tcp_stack.py 也实现了一个tcp服务端和tcp客户端,但是他们使用的系统的tcp协议栈。这个文件是为了检测 我们现实的协议栈能否跟标注的协议栈进行通信。



## 实验过程

1、因为tcp_sock是tcp连接通信的核心，首先整理tcp连接代码逻辑

```c

// lookup tcp sock in established_table with key (saddr, daddr, sport, dport)

```

2、tcp_in

3、tcp_timer

## Process
1. alloc_tcp_sock, the initiate recv_window size if 65535
2. List Operate Process
  First : init_list_head   (set a list)
  
  Second: add to tail, list_add_tail (struct list_head * new,struct list_head * head)
  
          add to head
	  
	  the new item itself should be contained by a struct.
	  
  Third: get_info : call list_entry(ptr,type,member)  ,ptr should point the member of type. it return the type pointer.
  
  Delete: list_delete_entry( struct list_head * entry) ,pick off the entry from the list it linked,but not call free()
