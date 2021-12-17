# QNX®  Neutrino 进程间通信编程之Message-passing/Pules

### 介绍

Interprocess Communication（IPC，进程间通信）在QNX Neutrino从一个嵌入式实时系统向一个全面的POSIX系统转变起着至关重要的作用。IPC是将在内核中提供各种服务的进程内聚在一起的粘合剂。在QNX中，消息传递是IPC的主要形式，也提供了其他的形式，除非有特殊的说明，否则这些形式也都是基于本地消息传递而实现的。

将更高级别的 IPC 服务（如通过我们的消息传递实现的管道和 FIFO）与其宏内核对应物进行比较的基准测试表明性能相当。

QNX Neutrino提供以下形式的IPC：

| Service:             | Implemented in:  |
| -------------------- | ---------------- |
| Message-passing      | Kernel           |
| Pules                | Kernel           |
| Signals              | Kernel           |
| Event Delivery       | External process |
| POSIX message queues | External process |
| Shared memory        | Process manager  |
| Pipes                | External process |
| FIFOs                | External process |

设计人员可以根据带宽要求，排队需求，网络透明度等选择这些服务。权衡可能很复杂，但灵活性很实用。

### **Message-passing**

比较传统的IPC方式是基于主从式构架（client-server），并且是双向通信。

![qnx_msg_normal](./pic/qnx_msg_normal.png)



![qnx_msg_ipc](./pic/qnx_msg_ipc.png)

再仔细来看的话，就是每一个process里面都有一个thread来负责通信。当一个线程在等待回信的时候，就会傻傻的等待，什么都不做了。直到收到回复信息。

**客户端线程**

![clientthread_states](.\pic\clientthread_states.PNG)

> * 客户端线程调用`MsgSend()`后，如果服务器线程还没调用`MsgReceive()`，客户端线程状态则为`SEND blocked`，一旦服务器线程调用了`MsgReceive()`，客户端线程状态变为`REPLY blocked`，当服务器线程执行`MsgReply()`后，客户端线程状态就变成了`READY`；
>
> * 如果客户端线程调用`MsgSend()`后，而服务器线程正阻塞在`MsgReceive()`上， 则客户端线程状态直接跳过`SEND blocked`，直接变成`REPLY blocked`；
>
> * 当服务器线程失败、退出、或者消失了，客户端线程状态变成`READY`，此时`MsgSend()`会返回一个错误值。

**服务器线程**

![serverthread_status](.\pic\serverthread_status.PNG)

>* 服务器线程调用`MsgReceive()`时，当没有线程给它发送消息，它的状态为`RECEIVE blocked`，当有线程发送时变为`READY`；
>
>* 服务器线程调用`MsgReceive()`时，当已经有其他线程给它发送过消息，`MsgReceive()`会立马返回，而不会阻塞；
>
>* 服务器线程调用`MsgReply()`时，不会阻塞；

客户端线程和服务器线程在时间主线里显示如下：

![qnx_msg_process](./pic/qnx_msg_process.png)

下面列出两种场景**Receive before Send**和**Send before Receive**

服务器线程MsgReceive发生在客户线程MsgSend之前

![msg_receive_send](./pic/msg_receive_send.png)

客户线程MsgSend发生在服务器线程MsgReceive之前

![msg_send_receive](./pic/msg_send_receive.png)

由上面两个场景看客户线程MsgSend和服务器线程MsgReceive直接影响Message-passing性能。



Servers收到信息在通道上，Clients通过connection连接上channel，来发送信息。

![msg_channel_connection](./pic/msg_channel_connection.png)

一个进程可以有多个connections连接到另一个进程的channel上，是个多对一的关系。

![msg_mutli_channel_connection](./pic/msg_mutli_channel_connection.png)

**Message-passing编程流程如下**

>– Server:
>
>• creates a channel (***ChannelCreate**()*)
>
>• waits for a message (***MsgReceive**()*)
>
>• performs processing
>
>• sends reply (***MsgReply**()*)
>
>• goes back for more -> waits for a message (***MsgReceive**()*)
>
>– Client:
>
>• attaches to channel (***ConnectAttach**()*)
>
>• sends message (***MsgSend**()*)
>
>• processes reply

**函数原型**

```C
int ChannelCreate( unsigned flags );
int ChannelDestroy( int chid );
int name_open( const char * name, int flags );
int name_close( int coid );

int ConnectAttach( uint32_t nd, pid_t pid, int chid, unsigned index, int flags );
int ConnectDetach( int coid );
name_attach_t * name_attach( dispatch_t * dpp, const char * path, unsigned flags );
int name_detach( name_attach_t * attach, unsigned flags );

long MsgSend( int coid, const void* smsg, size_t sbytes, void* rmsg, size_t rbytes );
int MsgReceive( int chid, void * msg, size_t bytes, struct _msg_info * info );
int MsgReply( int rcvid, long status, const void* msg, size_t bytes );

ssize_t MsgWrite( int rcvid, const void* msg, size_t size, size_t offset );
ssize_t MsgRead( int rcvid,  void* msg, size_t bytes,  size_t offset );
```

详细命令使用请看以下命令链接：

Message-passing API

| Function                                                     | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [*ChannelCreate()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fc%2Fchannelcreate.html) | Create a channel to receive messages on.                     |
|                                                              |                                                              |
| [*ChannelDestroy()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fc%2Fchanneldestroy.html) | Destroy a channel.                                           |
| [*ConnectAttach()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fc%2Fconnectattach.html) | Create a connection to send messages on.                     |
| [*ConnectDetach()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fc%2Fconnectdetach.html) | Detach a connection.                                         |
| [name_open()](http://www.qnx.com/developers/docs/7.1/com.qnx.doc.neutrino.lib_ref/topic/n/name_open.html) | Open a name to connect to a server                           |
| [name_close()](http://www.qnx.com/developers/docs/7.1/com.qnx.doc.neutrino.lib_ref/topic/n/name_close.html) | Close a server connection that was opened by **name_open()** |
| [name_attach()](http://www.qnx.com/developers/docs/7.1/com.qnx.doc.neutrino.lib_ref/topic/n/name_attach.html) | Register a name in the pathname space and create a channel   |
| [name_detach()](http://www.qnx.com/developers/docs/7.1/com.qnx.doc.neutrino.lib_ref/topic/n/name_detach.html) | Remove a name from the namespace and destroy the channel     |
| [*MsgSend()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fm%2Fmsgsend.html) | Send a message and block until reply.                        |
| [MsgSendv()](http://www.qnx.com/developers/docs/7.1/index.html#com.qnx.doc.neutrino.lib_ref/topic/m/msgsendv.html) | Send a message to a channel                                  |
| [*MsgReceive()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fm%2Fmsgreceive.html) | Wait for a message.                                          |
| [MsgReceivev()](http://www.qnx.com/developers/docs/7.1/index.html#com.qnx.doc.neutrino.lib_ref/topic/m/msgreceivev.html) | Wait for a message or pulse on a channel                     |
| [*MsgReceivePulse()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fm%2Fmsgreceivepulse.html) | Wait for a tiny, nonblocking message (pulse).                |
| [*MsgReply()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fm%2Fmsgreply.html) | Reply to a message.                                          |
| [*MsgError()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fm%2Fmsgerror.html) | Reply only with an error status. No message bytes are transferred. |
| [*MsgRead()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fm%2Fmsgread.html) | Read additional data from a received message.                |
| [MsgReadv()](http://www.qnx.com/developers/docs/7.1/index.html#com.qnx.doc.neutrino.lib_ref/topic/m/msgreadv.html) | Read data from a message                                     |
| [*MsgWrite()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fm%2Fmsgwrite.html) | Write additional data to a reply message.                    |
| [MsgWritev()](http://www.qnx.com/developers/docs/7.1/index.html#com.qnx.doc.neutrino.lib_ref/topic/m/msgwritev.html) | Write a reply message                                        |
| [*MsgInfo()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fm%2Fmsginfo.html) | Obtain info on a received message.                           |
| [*MsgSendPulse()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fm%2Fmsgsendpulse.html) | Send a tiny, nonblocking message (pulse).                    |
| [*MsgDeliverEvent()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fm%2Fmsgdeliverevent.html) | Deliver an event to a client.                                |
| [*MsgKeyData()*](https://www.oschina.net/action/GoToLink?url=http%3A%2F%2Fwww.qnx.com%2Fdevelopers%2Fdocs%2F7.1%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fm%2Fmsgkeydata.html) | Key a message to allow security checks.                      |

服务器端伪代码如下：

```C
#include <sys/neutrino.h>
int chid; // channel ID
main ()
{
	int rcvid; // receive ID
	chid = ChannelCreate (0 /* or flags */);
	/* create client thread */
	...
	while (1) {
		rcvid = MsgReceive (chid, &recvmsg, rbytes, NULL);
		// process message from client here...
		MsgReply (rcvid, 0, &replymsg, rbytes);
	} 
}
```

客户端伪代码如下：

```C
#include <sys/neutrino.h>
extern int chid;
int coid; // connection id
int status;
client_thread() {
	// create the connection, typically done only once
	coid = ConnectAttach (0, 0, chid, _NTO_SIDE_CHANNEL, 0);
	...
	// at some point later we decide we want to send a message
	status = MsgSend (coid, &sendmsg, sbytes, &replymsg, rbytes);
}
```

Massage之间的**通信数据总是通过拷贝**，而不是指针的传递。

![msg_data_copy](./pic/msg_data_copy.png)

那么如何设计消息传递策略呢？

>* 定义公共消息头消息类型结构体
>* 所有消息都是同一个消息类型
>* 具有匹配每个消息类型的结构
>* 如果消息相关或它们使用共同的结构，请考虑使用消息类型和子类型
>* 定义匹配的回复结构体。如果合适，避免不同类型服务器的消息类型重叠

下面的消息传递策略伪代码帮助你理解：

```C
while(1) {
		recvid = MsgReceive( chid, &msg, sizeof(msg), NULL );
		switch( msg.hdr.type ) {
			case MSG_TYPE_1:
				handle_msg_type_1(rcvid, &msg);
				break;
		case MSG_TYPE_2:
				… 
	} 
}
```

#### 频道（Channel）与连接（Connect）实例代码

服务器：这个服务器，准备好频道后，就从频道上接收信息。如果信息是字符串”Hello“的话，这个服务器应答一个”World“字符串。如果收到的信处是字符串“Ni Hao", 那么它会应答”Zhong Guo"，其它任何消息都用MsgError()回答一个错误。

```C
// Simple server
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/neutrino.h>

int main()
{
	int chid, rcvid, status;
	char buf[128];

	if ((chid = ChannelCreate(0)) == -1) {
		perror("ChannelCreate");
		return -1;
	}

	printf("Server is ready, pid = %d, chid = %d\n", getpid(), chid);

	for (;;) {
		if ((rcvid = MsgReceive(chid, buf, sizeof(buf), NULL)) == -1) {
			perror("MsgReceive");
			return -1;
		}
	printf("Server: Received '%s'\n", buf);

	/* Based on what we receive, return some message */
	if (strcmp(buf, "Hello") == 0) {
			MsgReply(rcvid, 0, "World", strlen("World") + 1);
		} else if (strcmp(buf, "Ni Hao") == 0) {
			MsgReply(rcvid, 0, "Zhong Guo", strlen("Zhong Guo") + 1);
		} else {
			MsgError(rcvid, EINVAL);
		}
	}

	ChannelDestroy(chid);
	return 0;
}
```

服务器：这个服务器，准备好频道后，就从频道上接收信息。如果信息是字符串”Hello“的话，这个服务器应答一个”World“字符串。如果收到的信处是字符串“Ni Hao", 那么它会应答”Zhong Guo"，其它任何消息都用MsgError()回答一个错误。

```C
//simple client
#include <stdio.h>
#include <string.h>
#include <sys/neutrino.h>

int main(int argc, char **argv)
{
	pid_t spid;
	int chid, coid, i;
	char buf[128];

	if (argc < 3) {
		fprintf(stderr, "Usage: simple_client <pid> <chid>\n");
		return -1;
	}

	spid = atoi(argv[1]);
	chid = atoi(argv[2]);

	if ((coid = ConnectAttach(0, spid, chid, 0, 0)) == -1) {
		perror("ConnectAttach");
		return -1;
	}
	/* sent 3 pairs of "Hello" and "Ni Hao" */
	for (i = 0; i < 3; i++) {
		sprintf(buf, "Hello");
		printf("client: sent '%s'\n", buf);
		if (MsgSend(coid, buf, strlen(buf) + 1, buf, sizeof(buf)) != 0) {
			perror("MsgSend");
			return -1;
		}
		printf("client: returned '%s'\n", buf);

		sprintf(buf, "Ni Hao");
		printf("client: sent '%s'\n", buf);
		if (MsgSend(coid, buf, strlen(buf) + 1, buf, sizeof(buf)) != 0) {
			perror("MsgSend");
			return -1;
		}
		printf("client: returned '%s'\n", buf);
	}
	/* sent a bad message, see if we get an error */
	sprintf(buf, "Unknown");
	printf("client: sent '%s'\n", buf);
	if (MsgSend(coid, buf, strlen(buf) + 1, buf, sizeof(buf)) != 0) {
		perror("MsgSend");
		return -1;
	}
	ConnectDetach(coid);
	return 0;
}
```

分别编译后的执行结果如下：

服务器：

```shell
$ ./simple_server
Server is ready, pid = 36409378, chid = 2
Server: Received 'Hello'
Server: Received 'Ni Hao'
Server: Received 'Hello'
Server: Received 'Ni Hao'
Server: Received 'Hello'
Server: Received 'Ni Hao'
Server: Received 'Unknown'
Server: Received ''
```

客户端：

```shell
$ ./simple_client 36409378 2
client: sent 'Hello'
client: returned 'World'
client: sent 'Ni Hao'
client: returned 'Zhong Guo'
client: sent 'Hello'
client: returned 'World'
client: sent 'Ni Hao'
client: returned 'Zhong Guo'
client: sent 'Hello'
client: returned 'World'
client: sent 'Ni Hao'
client: returned 'Zhong Guo'
client: sent 'Unknown'
MsgSend: Invalid argument
```

#### name_open与name_attach实例代码

用**name_open**与**MsgSend**消息传递的客户端线程

```C
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>

#define ATTACH_POINT "myname"

/* We specify the header as being at least a pulse */
typedef struct _pulse msg_header_t;

/* Our real data comes after the header */
typedef struct _my_data {
    msg_header_t hdr;
    int data;
} my_data_t;

/*** Client Side of the code ***/
int client() 
{
    my_data_t msg;
	int msg_reply;
    int server_coid;

    if ((server_coid = name_open(ATTACH_POINT, 0)) == -1) {
		printf("client name open failed\n");
        return EXIT_FAILURE;
    }

    /* We would have pre-defined data to stuff here */
    msg.hdr.type = 0x00;
    msg.hdr.subtype = 0x00;
	msg.data = 1;

    /* Do whatever work you wanted with server connection */
    printf("client name open success, Client sending msg %d \n", msg.data);
    if (MsgSend(server_coid, &msg, sizeof(msg), &msg_reply, sizeof(msg_reply)) == -1) {
		printf("client send msg 1 error\n");    
	}
	
    printf("client receive msg 1 reply: %d \n", msg_reply);
	
	msg.hdr.type = 0x00;
    msg.hdr.subtype = 0x01;
	msg.data = 2;
	printf("client name open success, Client sending msg %d \n", msg.data);
	if (MsgSend(server_coid, &msg, sizeof(msg), &msg_reply, sizeof(msg_reply)) == -1) {
		printf("client send msg 2 error\n");    
	}
	
    printf("client receive msg 2 reply: %d \n", msg_reply);
	
    /* Close the connection */
    name_close(server_coid);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	int ret;

    if (argc < 2) {
        printf("Usage %s -s | -c \n", argv[0]);
        ret = EXIT_FAILURE;
    }
    else if (strcmp(argv[1], "-c") == 0) {
        printf("Running client ... \n");
        ret = client();
    }
	else {
        printf("Usage %s -s | -c \n", argv[0]);
        ret = EXIT_FAILURE;
    }
    return ret;
}
```

用**name_attach**与**MsgReceive，MsgReply**实现消息传递的服务器线程

```C
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>

#define ATTACH_POINT "myname"

/* We specify the header as being at least a pulse */
typedef struct _pulse msg_header_t;

/* Our real data comes after the header */
typedef struct _my_data {
    msg_header_t hdr;
    int data;
} my_data_t;

int msg_update_fail =3;
int msg_update_success =4;

/*** Server Side of the code ***/
int server() {
   name_attach_t *attach;
   my_data_t msg;

   
   my_data_t msg_reply;
   msg_reply.hdr.type = 0x00;
   msg_reply.hdr.subtype = 0x00;
   
  // my_data_t msg_replaydata;
   int rcvid;

   /* Create a local name (/dev/name/local/...) */
   if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL) {
   		printf("server name_attach error\n");
       return EXIT_FAILURE;
   }
   printf("server name_attach suceess,wait masg from client\n");

   /* Do your MsgReceive's here now with the chid */
   while (1) {
       rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);

       if (rcvid == -1) {/* Error condition, exit */
           break;
       }
       /* A message (presumable ours) received, handle */
	   switch(msg.data){

			case 1:
				printf("Server receive msg data %d \n", msg.data);
				MsgReply(rcvid, EOK, &msg_update_fail, sizeof(msg_update_fail));
				//MsgReply(UpdateReceiveId, EOK, &msg_update_fail, 0);
				break;
			case 2:
				printf("Server receive msg data %d \n", msg.data);		
				MsgReply(rcvid, EOK, &msg_update_success, sizeof(msg_update_success));
				break;
			default:
				break;
	   }
	   
       MsgReply(rcvid, EOK, 0, 0);

   }

   /* Remove the name from the space */
   name_detach(attach, 0);

   return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    int ret;

    if (argc < 2) {
        printf("Usage %s -s | -c \n", argv[0]);
        ret = EXIT_FAILURE;
    }
    else if (strcmp(argv[1], "-s") == 0) {
        printf("Running Server ... \n");
        ret = server();
    }
	else {
        printf("Usage %s -s | -c \n", argv[0]);
        ret = EXIT_FAILURE;
    }
    return ret;
}
```

**如果您想在一条消息中发送以下三个缓冲区怎么办？**

你可能通过三次`memcpy()`处理一个大的buffer。但是有个更有效的方法通过`MsgSendv()`传递指针数组。

使用**函数原型**如下：

```c
long MsgSendv( int coid,  const iov_t* siov, size_t sparts,  const iov_t* riov, size_t rparts );
int MsgReceivev( int chid, const iov_t * riov, size_t rparts, struct _msg_info * info );
ssize_t MsgReadv( int rcvid, const iov_t* riov, size_t rparts, size_t offset );
void SETIOV( iov_t *msg, void *addr, size_t len );
```

```C
typedef struct {
    void *iov_base;
    size_t iov_len;
} iov_t;

iov_t iovs [3];
```

![msg_setiov](./pic/msg_setiov.png)

实际IOVs应用实例，客户端需要将一个12KBytes的文件传给服务器端。

客户端伪代码如下：

```C
write (fd, buf, size);
effectively does:
	hdr.nbytes = size;
	SETIOV (&siov[0], &header, sizeof (header));
	SETIOV (&siov[1], buf, size);
	MsgSendv (fd, siov, 2, NULL, 0);
```

<img src="./pic/msg_data_buffer.png" alt="msg_data_buffer" style="zoom:80%;" />

实际上获得的是连续的字节流。

<img src="./pic/msg_data_getsent.png" alt="msg_data_getsent" style="zoom:80%;" />

服务器端获得接收内容是什么？

```C
// assume riov has been setup
MsgReceivev (chid, riov, 4, NULL);
```

<img src="./pic/msg_data_serverget.png" alt="msg_data_serverget" style="zoom:80%;" />

实际上我们是不知道接收的数据是多少，直到我们看见数据流的头。

```C
rcvid = MsgReceive (chid, &header,sizeof (header), NULL);
//获取到数据流的头信息后
SETIOV (iov [0], &cbuf [6], 4096);
SETIOV (iov [1], &cbuf [2], 4096);
SETIOV (iov [2], &cbuf [5], 4096);
//用MsgReadv直接根据sizeof(header)偏移把剩下的数据获取出来。
MsgReadv (rcvid, iov, 3, sizeof(header));
```

![msg_data_serverreadv](./pic/msg_data_serverreadv.png)

整个消息传递从客户端到服务器，你可以理解为下面两个流程：

![msg_ctos_iovs.png](./pic/msg_ctos_iovs.png)



![msg_stoc_iovs.png](./pic/msg_stoc_iovs.png)



**那怎么理解从服务器端拷贝数据给客户端呢？**

```C
ssize_t MsgWrite( int rcvid, const void* msg, size_t size, size_t offset );
ssize_t MsgWritev( int rcvid, const iov_t* iov, size_t parts, size_t offset );
```

MsgWrite回传数据实例如下：

![msg_stoc_writev.png](./pic/msg_stoc_writev.png)



### **Pulses脉冲**

脉冲其实更像一个短消息，也是在“连接Connection”上发送的。脉冲最大的特点是它是异步的。发送方不必要等接收方应答，直接可以继续执行。

![qnx_msg_pules](./pic/qnx_msg_pules.png)

脉冲的通信方式很特别，就像喊命令，不需要回应，执行就好了。便宜还快速，也不会发生blocking的现象。但是，这种异步性也给脉冲带来了限制。脉冲能携带的数据量有限，只有一个**8位的"code"域 (1byte)用来区分不同的脉冲，和一个32位的“value"域 (4字节)**来携带数据。脉冲最主要的用途就是用来进行“通知”(Notification)。不仅是用户程序，内核也会生成发送特殊的“系统脉冲”到用户程序，以通知某一特殊情况的发生。

```C
int MsgSendPulse ( int coid, int priority, int code, int value );
                                                |         |
                                      8bits <---|         |
                                     32bits <-------------|
```

![pulse](./pic/pulse.PNG)

>* **code** 通常用于表示“脉冲类型”的有效范围是 _PULSE_CODE_MINAVAIL 到 _PULSE_CODE_MAXAVAIL。
>
>* **priority** 就像发送线程的消息优先级一样
>
>  - 接收线程以该优先级运行
>  - 发送顺序基于优先级
>
>  要跨进程边界发送脉冲，发送者必须与接收者具有相同的有效用户 ID 或者是 root 用户

脉冲的接收比较简单，如果你知道频道上不会有别的消息，只有脉冲的话，可以用MsgReceivePulse()来只接收脉冲；
如果频道既可以接收消息，也可以接收脉冲时，就直接用MsgReceive()，只要确保接收缓冲(ReveiveBuf)至少可以容下一个脉冲（sizeof struct _pulse)就可以了。
在后一种情况下，如果MsgReceive()返回的rcvid是0，就代表接收到了一个脉冲，反之，则收到了一个消息。所以，一个既接收脉冲，又接收消息的服务器，Pulses脉冲实例伪代码如下：

```C
#include <sys/neutrino.h>

struct _pulse {
    uint16_t                    type;
    uint16_t                    subtype;
    int8_t                      code;   // <---- 8-bit code
    uint8_t                     zero[3];
    union sigval                value; // <--- 32-bit value
    int32_t                     scoid;
};

typedef union {
	struct _pulse pulse;
	// other message types you will receive
} myMessage_t;
…
myMessage_t msg;
while (1) {
	rcvid = MsgReceive (chid, &msg, sizeof(msg), NULL);
	if (rcvid == 0) {
		// it’s a pulse, look in msg.pulse… for data
        process_pulse(&msgs, &info);
		} else {
		// it’s a regular message
        process_message(&msgs, &info);
	}
}
…

```

展开**process_pulse**处理实现

```C
...
	rcvid = MsgReceive (chid, &msg, sizeof(msg), NULL);
	if (rcvid == 0) {
		// it’s a pulse, look in msg.pulse… for data
		switch (msg.pulse.code) {
		case _PULSE_CODE_UNBLOCK:
			// a kernel unblock pulse
			...
			break;
		case MY_PULSE_CODE:
			// do what's needed
			...
			break;
    } else {
    	process_message(&msgs, &info);
    }  
...
```

脉冲的发送，最直接的就是MsgSendPulse()。不过，这个函数通常只在一个进程中，用在一个线程要通知另一个线程的情形。在**跨进程**的时候，通常不会用到这个函数，而是用到下面将要提到的 **MsgDeliverEvent()**。与消息传递相比，消息传递永远是在进程间进行的。也就是说，不会有一个进程向内核发送数据的情形。而脉冲就不一样，除了用户进程间可以发脉冲以外，内核也会向用户进程发送“系统脉冲”来通知某一事件的发生。

如果您有一个频道，您可能会在该频道上接收来自 MsgSend*() 调用和脉冲的消息，但在某个时间点只想接收脉冲，使用MsgReceivePulse()则很有用。

```C
int MsgReceivePulse( int chid, void * pulse, size_t bytes, struct _msg_info * info );
```

如果进程正在接收消息和脉冲：

> - 接收顺序仍然基于优先级，使用脉冲的优先级
> - 内核将以其接收到的脉冲的优先级运行接收线程
> - 脉冲和消息可能会混合在一起

![pulse_priority](./pic/pulse_priority.PNG)

来自线程 1 的Send Pulse比在它Send message之前先到达服务器。因为脉冲的优先级比线程本身的优先级要高。

| Pulse API | send                                      | receive           |
| --------- | ----------------------------------------- | ----------------- |
|           | MsgSendPulse(coid, priority, code, value) | MsgReceivePulse() |
|           | MsgDeliverEvent()                         | MsgReceive()      |

>MsgSendPulse() 只在一个进程中的通知，用与同一个进程中一个线程要通知另一个线程的情形, 其中 code 8bits; value 32bits
>MsgDeliverEvent() 在跨进程的时候的通知
>MsgReceivePulse() 用于频道上只有pulse的接收
>MsgReceive() 用于频道上既接收message又接收pulse



参考文献：

[Programming with POSIX Threads](https://download.csdn.net/download/janesshang/10910991)

[QNC IPC---msg send receive example](https://blog.csdn.net/yuewen2008/article/details/86375835)

[QNX trying to send struct through MsgSend(), MsgReply() IPC message passing](https://stackoverflow.com/questions/66842159/qnx-trying-to-send-struct-through-msgsend-msgreply-ipc-message-passing)

[从API开始理解QNX - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/144325471)

[QNX IPC机制](https://blog.csdn.net/weixin_44280688/article/details/103234235)

[Message Passing (IPC) with Neutrino (swan.ac.uk)](https://psy.swan.ac.uk/staff/carter/qnx/tut_nto_ipc.htm)
