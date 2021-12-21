# QNX®  Neutrino 进程间通信编程之Pipes/FIFOs/Message Queues

## 介绍

Interprocess Communication（IPC，进程间通信）在QNX Neutrino从一个嵌入式实时系统向一个全面的POSIX系统转变起着至关重要的作用。IPC是将在内核中提供各种服务的进程内聚在一起的粘合剂。在QNX中，消息传递是IPC的主要形式，也提供了其他的形式，除非有特殊的说明，否则这些形式也都是基于本地消息传递而实现的。

将更高级别的 IPC 服务（如通过我们的消息传递实现的管道和 FIFO）与其宏内核对应物进行比较的基准测试表明性能相当。

QNX Neutrino提供以下形式的IPC：

| Service:                 | Implemented in:      |
| ------------------------ | -------------------- |
| Message-passing          | Kernel               |
| Pules                    | Kernel               |
| Signals                  | Kernel               |
| Event Delivery           | External process     |
| **POSIX message queues** | **External process** |
| Shared memory            | Process manager      |
| **Pipes**                | **External process** |
| **FIFOs**                | **External process** |

我们基本可以认为Pipes和Message Queues都是建立在本地消息传递，通过缓冲数据并处理任何其他复杂性的服务。所以我们把POSIX IPC Pipes 与 Message Queues归为一个篇幅介绍。

## 匿名管道Pipes/命名管道FIFOs

把一个进程连接到另一个进程的一个数据流称为一个“管道”，通常是用作把一个进程的输出通过管道连接到另一个进程的输入。管道本质上是内核的一块缓存。

### 匿名管道Pipes

 匿名管道是基于文件描述符的通信方式。实现两个进程间的通信时必须通过fork创建子进程，实现父子进程之间的通信。

![unix-pipe](.\pic\unix-pipe.gif)

管道具有几个影响其用途的重要特征：

>* 管道是单向的；必须将一端指定为读取端，另一端指定为写入端。请注意，没有限制不同的进程必须读取和写入管道；相反，如果一个进程写入管道然后立即从中读取，该进程将收到自己的消息。如果两个进程需要来回交换消息，则应该使用两个管道。
>* 管道是保持秩序的；从管道接收端读取的所有数据都将与其写入管道的顺序相匹配。无法将某些数据指定为更高优先级以确保首先读取它。
>* 管道的容量有限，它们使用阻塞 I/O；如果管道已满，则对管道的任何额外写入都将阻塞该进程，直到读取了某些数据。因此，不必担心消息会被丢弃，但可能会出现性能延迟，因为写入过程无法控制何时从管道中删除字节。
>* 管道将数据作为非结构化字节流发送。交换的数据没有预定义的特征，例如可预测的消息长度。使用管道的进程必须就通信协议达成一致并适当地处理错误（例如，如果其中一个进程提前终止了通信）。
>* 小于 PIPE_BUF 指定大小的消息保证以原子方式发送。因此，如果两个进程同时写入管道，则两条消息都会被正确写入并且不会相互干扰。

 ```C
 #include <unistd.h>
 
 int pipe( int filedes[2] );
 Parameters :
 fd[0] will be the fd(file descriptor) for the read end of pipe.
 fd[1] will be the fd for the write end of pipe.
 Returns : 0 on Success. -1 on error.
 ```

以父子进程为例：创建一个子进程，子进程复制了父进程的描述符表，因此子进程也有描述符表，并且他们指向的是同一个管道，由于父子进程都能访问这个管道，就可以通信。因为管道是半双工单向通信，因此在通信前要确定数据流向：即关闭父子进程各自一端不用的读写。如果一方是读数据就关闭写的描述符。

![parent_child_process](.\pic\parent_child_process.gif)

**说明：**
   管道的读写端通过打开文件描述符来传递，因此要通信的两个进程必须从他们的公共祖先那里继承管道的文件描述符。上面的例子是父进程把文件描述符传给子进程之后父子进程之间通信。也可以父进程fork（）两次，把文件描述符传给两个子进程，然后两个子进程之间通信。总之需要通过fork()传递文件描述符使两个进程都能访问同一个管道，他们才能通信。

```C
//这是一个匿名管道实现：功能：从父进程写入数据，子进程读取数据
 #include<stdio.h>
 #include<unistd.h>
 #include<string.h>
 #include<errno.h>
 int main()
{
        int fd[2];
         //管道需要创建在创建子进程前，这样才能复制
         if(pipe(fd)<0)
         {
                 perror("pipe errno");
                 return -1;
         }
         int pid=-1;
         pid=fork();//创建子进程，对于父进程会返回子进程id，子进程会返回0，创建失败会返回0
         if(pid<0)
         {
                 perror("fork errno");
                 return -1;
         }
      else if(pid==0)
         {
                //子进程 读取数据-> fd[0]
                close(fd[1]);//fd[1]是向管道写入数据，子进程不用写入数据，需要关闭管道写入端
                char buff[1024]={0};
                read(fd[0],buff,1024);//如果管道没数据会等待，然后读取数据，默认阻塞等待直至有数据
                 printf("chlid pid=%d, buff:%s\n",(int)getpid(), buff);
                 close(fd[0]);
         }
         else
         {
                 //父进程 ：写入数据->fd[1]
                 close(fd[0]); 
                 //fd[0]是读取数据，父进程不用读取数据，需要关闭管道读取端，由于父子进程相互独立，关闭一方描述符对另一方无影响
                 printf("parent pid=%d\n", (int)getpid()) ;
                 write(fd[1],"happy day",10);
                 close(fd[1]);
         }
         return 0;
 }
```

编译运行结果如下：

```shell
bspserver@ubuntu:~/workspace/pipe$ gcc -pthread parent_child.c -o parent_child
bspserver@ubuntu:~/workspace/pipe$ ./parent_child
parent pid=11734
chlid pid=11735, buff:happy day
```

需要注意的是：

> 1、如果写端关闭，读端未关闭。则读端会停止阻塞，立即返回读取字节数为 0 。因此，可在程序中根据返回的是否是 0 来判断写端是否已经关闭。
> 2、如果读端关闭，写端还没有关闭。那么，写端继续写，操作系统会给写端进程发送一个SIGPIPE信号，终止写端程序。



### 命名管道FIFOs

命名管道：文件系统可见，是一个特殊类型（管道类型）文件，命名管道可以应用于同一主机上任意进程间通信。

在管道中，只有具有血缘关系的进程才能进行通信，对于后来的命名管道，就解决了这个问题，FIFO不同于匿名管道之处在于它提供了一个路径名与之关联，以FIFO的文件形式存储在文件系统中。有名管道是一个设备文件，因此即使进程与创建FIFO的进程不存在亲缘关系，只要可以访问该路径，就能通过FIFO相互通信。值得注意的是，FIFO（first input first output）总是按照先进先出的原则工作，第一个被写入的数据首先从管道中读取。

FIFO 通过将文件名附加到管道来工作。出于这个原因，FIFO 也称为命名管道，而不是前面讨论的匿名管道。 FIFO 由一个调用 mkfifo() 的进程创建。创建后，任何进程（具有正确的访问权限）都可以通过对关联的文件名调用 open() 来访问 FIFO。一旦进程打开了文件，它们就可以使用标准的 read() 和 write() 函数进行通信。

**命名管道的创建：**

```cpp
#include <sys/types.h>
#include <sys/stat.h>
int mkfifo(const char *pathname, mode_t mode);
```

服务端进程代码如下：

```C
/*
 *
 *       server.c: Write strings in POSIX FIFOs to file
 *                 (Server process)
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
//
int main()
{
    umask(0);//将权限清0
    if(mkfifo("./mypipe",0666|S_IFIFO) < 0){//创建管道
        perror("mkfifo");
        return 1;
    }
    int fd = open("./mypipe",O_RDONLY);//打开管道
    if(fd < 0){
        perror("open");
        return 2;
    }
    char buf[1024];
    while(1){
        buf[0] = 0;
        printf("please waiting...\n");
        ssize_t s = read(fd,buf,sizeof(buf)-1);
 
        if(s > 0){
            buf[s-1] = 0;//过滤\n
            printf("Server：%s\n",buf);
        }else if(s == 0){//当客户端退出时，read返回0
            printf("client quit, Exit\n");
            break;
        }
    }
    close(fd);
    return 0;
}
```

客户端进程代码如下：

```C
/*
 *
 *       client.c: Write strings for printing in POSIX FIFOs object
 *                 
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
int main()
{
    int fd = open("./mypipe",O_WRONLY);//打开管道
    if(fd < 0){
        perror("open");
        return 1;
    }
    char buf[1024];
    while(1){
        printf("Client：");
        fflush(stdout);
        ssize_t s = read(0,buf,sizeof(buf)-1);//向管道文件中写数据
        if(s > 0){
            buf[s] = 0;//以字符串的形式写
            write(fd,buf,strlen(buf));
        }
    }
    close(fd);
    return 0;
}
```

编译运行结果如下：

服务器端

```shell
bspserver@ubuntu:~/workspace/pipe$ gcc server.c -o server
bspserver@ubuntu:~/workspace/pipe$  ./server
please waiting...
Server：hello
please waiting...
Server：world
please waiting...
client quit, Exit
bspserver@ubuntu:~/workspace/pipe$ 
```

客户端

```shell
bspserver@ubuntu:~/workspace/pipe$ gcc client.c -o client
Client：hello
Client：world
Client：^C
bspserver@ubuntu:~/workspace/pipe$ 
```

### **管道与shell命令实例**

<img src=".\pic\pipe-redirection.png" alt="pipe-redirection" style="zoom:80%;" />

在shell中输入命令：`ls -l | grep Q`，我们知道ls命令（其实也是一个进程）会把当前目录中的文件都列出来，但是它不会直接输出，而是把本来要输出到屏幕上的数据通过管道输出到grep这个进程中，作为grep这个进程的输入，然后这个进程对输入的信息进行筛选，把存在Q的信息的字符串（以行为单位）打印在屏幕上。

```shell
bspserver@ubuntu:~/workspace$ ls -l | grep Q
drwxrwxr-x  5 bspserver bspserver      4096 Nov 29 17:34 QNX_BCD
drwxrwxr-x  5 bspserver bspserver      4096 Nov 29 00:58 QNX_HMI
drwxrwxr-x  5 bspserver bspserver      4096 Nov 23 02:47 SDP_QNX660
drwxrwxr-x  5 bspserver bspserver      4096 Nov 29 01:02 SDP_QNX70
```

管道最常见的用途之一是将命令行上的多个命令链接在一起。 

例如，考虑以下命令行：`ls -l | sort -n -k 5 | tail -n 1 | awk '{print $NF}'`

```shell
bspserver@ubuntu:~/workspace$ ls -l
total 545464
drwxrwxr-x 14 bspserver bspserver      4096 Dec  8 02:48 04_Source_BareMetal
drwxrwxr-x  5 bspserver bspserver      4096 Nov 23 02:48 audio_driver
drwxrwxr-x  8 bspserver bspserver      4096 Nov 24 21:04 devs-fpdlink
drwxrwxr-x 10 bspserver bspserver      4096 Nov 23 22:14 esp-idf
-rw-rw-r--  1 bspserver bspserver 240473071 Nov 23 22:08 esp-idf-master.tar.gz
drwxrwxr-x 19 bspserver bspserver      4096 Dec 20 18:05 FFmpeg
-rw-rw-r--  1 bspserver bspserver 317400777 Dec 20 18:02 FFmpeg.tar.gz
drwxrwxr-x  2 bspserver bspserver      4096 Dec 19 18:51 github
drwxrwxr-x  6 bspserver bspserver      4096 Dec 17 01:41 linux_training
drwxrwxr-x  2 bspserver bspserver      4096 Nov 26 01:49 pcan_pytest
drwxrwxr-x  7 bspserver bspserver      4096 Dec 13 21:36 POSIX-threads-programming-tutorials-master
-rw-r--r--  1 root      root         612759 Dec 13 02:53 POSIX-threads-programming-tutorials-master.tar.gz
drwxrwxr-x  5 bspserver bspserver      4096 Nov 29 17:34 QNX_BCD
drwxrwxr-x  5 bspserver bspserver      4096 Nov 29 00:58 QNX_HMI
drwxrwxr-x  5 bspserver bspserver      4096 Nov 23 02:47 SDP_QNX660
drwxrwxr-x  5 bspserver bspserver      4096 Nov 29 01:02 SDP_QNX700
drwxrwxr-x  5 bspserver bspserver      4096 Nov 24 21:13 Upgrade_HAL
bspserver@ubuntu:~/workspace$ ls -l | sort -n -k 5
total 545464
drwxrwxr-x 10 bspserver bspserver      4096 Nov 23 22:14 esp-idf
drwxrwxr-x 14 bspserver bspserver      4096 Dec  8 02:48 04_Source_BareMetal
drwxrwxr-x 19 bspserver bspserver      4096 Dec 20 18:05 FFmpeg
drwxrwxr-x  2 bspserver bspserver      4096 Dec 19 18:51 github
drwxrwxr-x  2 bspserver bspserver      4096 Nov 26 01:49 pcan_pytest
drwxrwxr-x  5 bspserver bspserver      4096 Nov 23 02:47 SDP_QNX660
drwxrwxr-x  5 bspserver bspserver      4096 Nov 23 02:48 audio_driver
drwxrwxr-x  5 bspserver bspserver      4096 Nov 24 21:13 Upgrade_HAL
drwxrwxr-x  5 bspserver bspserver      4096 Nov 29 00:58 QNX_HMI
drwxrwxr-x  5 bspserver bspserver      4096 Nov 29 01:02 SDP_QNX700
drwxrwxr-x  5 bspserver bspserver      4096 Nov 29 17:34 QNX_BCD
drwxrwxr-x  6 bspserver bspserver      4096 Dec 17 01:41 linux_training
drwxrwxr-x  7 bspserver bspserver      4096 Dec 13 21:36 POSIX-threads-programming-tutorials-master
drwxrwxr-x  8 bspserver bspserver      4096 Nov 24 21:04 devs-fpdlink
-rw-r--r--  1 root      root         612759 Dec 13 02:53 POSIX-threads-programming-tutorials-master.tar.gz
-rw-rw-r--  1 bspserver bspserver 240473071 Nov 23 22:08 esp-idf-master.tar.gz
-rw-rw-r--  1 bspserver bspserver 317400777 Dec 20 18:02 FFmpeg.tar.gz
bspserver@ubuntu:~/workspace$ ls -l | sort -n -k 5 |  tail -n 1 
-rw-rw-r--  1 bspserver bspserver 317400777 Dec 20 18:02 FFmpeg.tar.gz
bspserver@ubuntu:~/workspace$ ls -l | sort -n -k 5 | tail -n 1 | awk '{print $NF}'
FFmpeg.tar.gz
bspserver@ubuntu:~/workspace$ 
```

此命令行创建四个链接在一起的进程。 首先，`ls` 命令打印出文件列表及其详细信息。 此列表作为输入发送到`sort`，排序基于第 5 个字段（文件大小）进行数字排序。 `tail`进程然后抓取最后一行，这是最大文件的行。 最后，awk 将打印该行的最后一个字段，即最大文件的文件名。

<img src=".\pic\ipc_pipes_shell.png" alt="ipc_pipes_shell" style="zoom:40%;" />

### 命名管道和匿名管道区别和联系：

1.区别：匿名管道用int pipe(int pipefd[2]); 创建并打开匿名管道返回描述符
命名管道用mkfifo或者 int mkfifo(const char *pathname, mode_t mode);创建，并没有打开，如果打开需要open；
匿名管道是具有亲缘关系进程间通信的媒介，而命名管道作为同一主机任意进程间通信的媒介；
匿名管道不可见文件系统，命名管道可见于文件系统，是一个特殊类型（管道类型）文件。
2.联系：匿名管道和命名管道都是内核的一块缓冲区，并且都是单向通信；另外当命名管道打开(open)后，所有特性和匿名管道一样（上文匿名管道读写规则与管道特性）：两者自带同步（临界资源访问的时序性）与互斥（临界资源同一时间的唯一访问性），管道生命周期随进程退出而结束。



## 消息队列 POSIX message queues

POSIX通过`message queues`定义一组非阻塞的消息传递机制。消息队列为命名对象，针对这些对象可以进行读取和写入，作为离散消息的优先级队列，消息队列具有比管道更多的结构，为应用程序提供了更多的通信控制。QNX Neutrino内核不包含`message queues`，它的实现在内核之外。
 QNX Neutrino提供了两种`message queues`的实现：

>- mqueue，使用mqueue资源管理的传统实现
>- mq，使用mq服务和非同步消息的替代实现

QNX消息机制与POSIX的`Message queues`有一个根本的区别：，QNX的消息机制通过内存拷贝来实现消息的传递；而POSIX的消息队列通过将消息进行存取来实现消息的传递。QNX的消息机制比POSIX的消息队列效率更高，但有时为了POSIX的灵活，需要适当的牺牲一点效率。

消息队列与文件类似，操作的接口相近。

| Function                                                     | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [*mq_open()*](http://www.qnx.com/developers/docs/7.1/com.qnx.doc.neutrino.lib_ref/topic/m/mq_open.html) | Open a message queue                                         |
| [*mq_close()*](http://www.qnx.com/developers/docs/7.1/com.qnx.doc.neutrino.lib_ref/topic/m/mq_close.html) | Close a message queue                                        |
| [*mq_unlink()*](http://www.qnx.com/developers/docs/7.1/com.qnx.doc.neutrino.lib_ref/topic/m/mq_unlink.html) | Remove a message queue                                       |
| [*mq_send()*](http://www.qnx.com/developers/docs/7.1/com.qnx.doc.neutrino.lib_ref/topic/m/mq_send.html) | Add a message to the message queue                           |
| [*mq_receive()*](http://www.qnx.com/developers/docs/7.1/com.qnx.doc.neutrino.lib_ref/topic/m/mq_receive.html) | Receive a message from the message queue                     |
| [*mq_notify()*](http://www.qnx.com/developers/docs/7.1/com.qnx.doc.neutrino.lib_ref/topic/m/mq_notify.html) | Tell the calling process that a message is available on a message queue |
| [*mq_setattr()*](http://www.qnx.com/developers/docs/7.1/com.qnx.doc.neutrino.lib_ref/topic/m/mq_setattr.html) | Set message queue attributes                                 |
| [*mq_getattr()*](http://www.qnx.com/developers/docs/7.1/com.qnx.doc.neutrino.lib_ref/topic/m/mq_getattr.html) | Get message queue attributes                                 |



- **mq_open**: 创建一个新的消息队列或打开一个已存在的消息的队列。

```cpp
mqd_t mq_open(const char *name, int oflag);
mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
```

> **返回值**：成功，消息队列描述符；失败，-1
>
> 消息队列描述符用作其余消息队列函数(mq_unlink除外)的第一个参数。
>
> **name**规则：必须以一个斜杠符打头，并且不能再包含任何其他斜杠符
>
> **oflag**：O_RDONLY、O_WRONLY、O_RDWR三者之一，按位或上O_CREAT、O_EXCL
>
> **mode**：S_ISRUSR、S_ISWUSR、S_ISRGRP、S_ISWGRP、S_ISROTH、S_ISWOTH
>
> **attr**：
>
> struct mq_attr
>
> {
>
> ​    long mq_flags;//阻塞标志， 0或O_NONBLOCK
>
> ​    long mq_maxmsg;//最大消息数
>
> ​    long mq_msgsize;//每个消息最大大小
>
> ​    long mq_curmsgs;//当前消息数
>
> };



- **mq_close**: 关闭消息队列。
- **mq_unlink**: 从系统中删除消息队列。

```cpp
int mq_close(mqd_t mqd);
int mq_unlink(const char *name)
```

>//关闭已打开的消息队列
>
>**int mq_close(mqd_t mqd);**
>
>返回值：成功，0；出错，-1
>
>一个进程终止时，它的所有打开的消息队列都关闭，如同调用了mq_close。
>
>//删除消息队列的name
>
>**int mq_unlink(const char \*name);**
>
>返回值：成功，0；出错，-1
>
>每个消息队列有一个保存其当前打开的描述符数的引用计数，只有当引用计数为0时，才删除该消息队列。mq_unlink和mq_close都会让引用数减一



- **mq_send**:向消息队列中写入一条消息。

```cpp
int mq_send(mqd_t mqd, const char *ptr, size_t len, unsigned int prio);
```

>返回值：成功，0；出错，-1
>
>说明：优先级prio要小于MQ_PRIO_MAX（此值最少为32）。



- **mq_receive**:从消息队列中读取一条消息。

```cpp
ssize_t mq_receive(mqd_t mqd, char *ptr, size_t len, unsigned int *prio);
```

>- 返回值：成功，消息中字节数；出错，-1
>
>  说明：消息内存的长度len，最小要等于mq_msgsize。mq_receive总是返回消息队列中**最高优先级**的最早消息。



- **mq_getattr**:用于获取当前消息队列的属性。
- **mq_setattr**:用于设置当前消息队列的属性。

```cpp
int mq_getattr(mqd_t mqd, struct mq_attr *attr);
int mq_setattr(mqd_t mqd, const struct mq_attr *attr, struct mq_attr *oattr);
```

> 返回值：成功，0；出错，-1
>
> 消息队列先前属性返回到oattr中
>
> 消息队列有4个属性，如下：
>
> struct mq_attr
> {
>         long mq_flags;//阻塞标志， 0或O_NONBLOCK
>         long mq_maxmsg;//最大消息数
>         long mq_msgsize;//每个消息最大大小
>         long mq_curmsgs;//当前消息数
> };
> 其中，mq_setattr只能设置mq_flags属性；
>      mq_open只能设置mq_maxmsg和mq_msgsize属性，并且两个必须要同时设置；
>      mq_getattr返回全部4个属性。

<img src=".\pic\msg-queue.webp" alt="msg-queue" style="zoom:70%;" />

服务端进程代码如下：

```C
#include <stdio.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
 
#define MAXSIZE     10   //定义buf大小
#define BUFFER      8192 //定义Msg大小
 
struct MsgType{
    int len;
    char buf[MAXSIZE];
    char x;
    short y;
};
 
int main(int argc, char **argv)
{
    /*消息队列*/
    mqd_t msgq_id;
    struct MsgType msg;
    unsigned int prio = 1;
    unsigned int send_size = BUFFER;
    struct mq_attr msgq_attr;
    const char *file = "/posix";
    /*mq_open() for creating a new queue (using default attributes) */
    /*mq_open() 创建一个新的 POSIX 消息队列或打开一个存在的队列*/
    msgq_id = mq_open(file, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG, NULL);
    if(msgq_id == (mqd_t)-1)
    {
        perror("mq_open");
        exit(1);
    }
    /* getting the attributes from the queue        --  mq_getattr() */
    if(mq_getattr(msgq_id, &msgq_attr) == -1)
    {
        perror("mq_getattr");
        exit(1);
    }
    printf("Queue \"%s\":\n\t- stores at most %ld messages\n\t- \
        large at most %ld bytes each\n\t- currently holds %ld messages\n",
        file, msgq_attr.mq_maxmsg, msgq_attr.mq_msgsize, msgq_attr.mq_curmsgs);
    /*setting the attributes of the queue        --  mq_setattr() */
    /*mq_setattr() 设置消息队列的属性，设置时使用由 newattr 指针指向的 mq_attr 结构的信息。*/
    /*属性中只有标志 mq_flasgs 里的 O_NONBLOCK 标志可以更改，其它在 newattr 内的域都被忽略 */
    if(mq_setattr(msgq_id, &msgq_attr, NULL) == -1)
    {
        perror("mq_setattr");
        exit(1);
    }  
    int i = 0;
    while(i < atoi(argv[1]))
    {
        msg.len = i;
        memset(msg.buf, 0, MAXSIZE);
        sprintf(msg.buf, "0x%x", i);
        msg.x = (char)(i + 'a');
        msg.y = (short)(i + 100);
        printf("msg.len = %d, msg.buf = %s, msg.x = %c, msg.y = %d\n", msg.len, msg.buf, msg.x, msg.y);
        /*sending the message      --  mq_send() */
        /*mq_send() 把 msg_ptr 指向的消息加入由 mqdes 引用的消息队列里。*/
        /*参数 msg_len 指定消息 msg_ptr 的长度：这个长度必须小于或等于队列 mq_msgsize 属性的值。零长度的消息是允许。*/
        if(mq_send(msgq_id, (char*)&msg, sizeof(struct MsgType), prio) == -1)
        {
            perror("mq_send");
            exit(1);
        }
        i++;
        sleep(1);
    }
    msgq_attr.mq_curmsgs = msgq_attr.mq_maxmsg;
    while(msgq_attr.mq_curmsgs)
    {
        /* getting the attributes from the queue        --  mq_getattr() */
        if(mq_getattr(msgq_id, &msgq_attr) == -1)
        {
            perror("mq_getattr");
            exit(1);
        }
        sleep(1); //等待消费者进程退出
        printf("currently holds %ld messages\n", msgq_attr.mq_curmsgs);
    }
    /*closing the queue        -- mq_close() */
    /*mq_close() 关闭消息队列描述符 mqdes。如果调用进程在消息队列 mqdes 绑定了通知请求，*/
    /*那么这个请求被删除，此后其它进程就可以绑定通知请求到此消息队列。*/
    if(mq_close(msgq_id) == -1)
    {
        perror("mq_close");
        exit(1);
    }
    /*mq_unlink() 删除名为 name 的消息队列。消息队列名将被直接删除。*/
    /*消息队列本身在所有引用这个队列的描述符被关闭时销毁。*/
    if(mq_unlink(file) == -1)
    {
        perror("mq_unlink");
        exit(1);
    }
    return 0;
}
```

客户端进程代码如下：

```C
#include <stdio.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
 
#define MAXSIZE     10   //定义buf大小
#define BUFFER      8192 //定义Msg大小
 
struct MsgType{
    int len;
    char buf[MAXSIZE];
    char x;
    short y;
};
 
int main(int argc, char **argv)
{
    /*消息队列*/
    mqd_t msgq_id;
    struct MsgType msg;
    unsigned int sender;
    struct mq_attr msgq_attr;
    unsigned int recv_size = BUFFER;
    const char *file = "/posix";
 
    /* mq_open() for opening an existing queue */
    msgq_id = mq_open(file, O_RDWR);
    if(msgq_id == (mqd_t)-1)
    {
        perror("mq_open");
        exit(1);
    }
    /* getting the attributes from the queue        --  mq_getattr() */
    if(mq_getattr(msgq_id, &msgq_attr) == -1)
    {
        perror("mq_getattr");
        exit(1);
    }
    printf("Queue \"%s\":\n\t- stores at most %ld messages\n\t- \
        large at most %ld bytes each\n\t- currently holds %ld messages\n",
        file, msgq_attr.mq_maxmsg, msgq_attr.mq_msgsize, msgq_attr.mq_curmsgs);
 
    if(recv_size < msgq_attr.mq_msgsize)
        recv_size = msgq_attr.mq_msgsize;
 
    int i = 0;
    while(i < atoi(argv[1])) //运行一个consumenr，为 10 ，同时运行两个consumer进程，为 5 
    {
        msg.len = -1;
        memset(msg.buf, 0, MAXSIZE);
        msg.x = ' ';
        msg.y = -1;
        /* getting a message */
        /*mq_receive() 从由描述符 mqdes 引用的队列时删除优先级最高的最老的消息，并把放置到 msg_ptr 的缓存区内。*/
        /*参数 msg_len 指定缓冲区 msg_ptr 的大小：它必须大于队列的 mq_msgsize 属性(参数 mq_getattr)。*/
        /*如果 prio 不是 NULL，那么它指向的内存用于返回收到消息相关的优先级。*/
        if (mq_receive(msgq_id, (char*)&msg, recv_size, &sender) == -1)
        {
            perror("mq_receive");
            exit(1);
        }
        printf("msg.len = %d, msg.buf = %s, msg.x = %c, msg.y = %d\n", msg.len, msg.buf, msg.x, msg.y);
        i++;
        sleep(2);
    }
 
    if(mq_close(msgq_id) == -1)
    {
        perror("mq_close");
        exit(1);
    }
    return 0;
}
```

代码编译运行结果如下：

```shell
bspserver@ubuntu:~/workspace/posix_message_queue$ gcc -pthread client.c -lrt -o client
bspserver@ubuntu:~/workspace/posix_message_queue$ gcc -pthread server.c -lrt -o server
bspserver@ubuntu:~/workspace/linux_training/kernel/process-courses/posix_message_queue$ ./server 5
Queue "/posix":
	- stores at most 10 messages
	-         large at most 8192 bytes each
	- currently holds 0 messages
msg.len = 0, msg.buf = 0x0, msg.x = a, msg.y = 100
msg.len = 1, msg.buf = 0x1, msg.x = b, msg.y = 101
msg.len = 2, msg.buf = 0x2, msg.x = c, msg.y = 102
msg.len = 3, msg.buf = 0x3, msg.x = d, msg.y = 103
msg.len = 4, msg.buf = 0x4, msg.x = e, msg.y = 104
currently holds 5 messages
currently holds 5 messages
currently holds 4 messages
currently holds 4 messages
currently holds 3 messages
currently holds 3 messages
currently holds 2 messages
currently holds 2 messages
currently holds 1 messages
currently holds 1 messages
currently holds 0 messages
# 再看一个终端查看消息队列的属性
bspserver@ubuntu:~$ ls /dev/mqueue/ -al
total 0
drwxrwxrwt  2 root      root        60 Dec 20 23:11 .
drwxr-xr-x 20 root      root      4160 Dec 20 17:55 ..
-rwxrwx---  1 bspserver bspserver   80 Dec 20 23:11 posix
bspserver@ubuntu:~$ cat /dev/mqueue/posix 
QSIZE:100        NOTIFY:0     SIGNO:0     NOTIFY_PID:0     
bspserver@ubuntu:~$ ls /proc/sys/fs/mqueue/
msg_default      msg_max    msgsize_default  msgsize_max      queues_max       
bspserver@ubuntu:~$ cat /proc/sys/fs/mqueue/*
10
10
8192
8192
256
bspserver@ubuntu:~$ 
# 再开一个终端运行客户端进程
bspserver@ubuntu:~/workspace/posix_message_queue$ ./client 5
Queue "/posix":
	- stores at most 10 messages
	-         large at most 8192 bytes each
	- currently holds 5 messages
msg.len = 0, msg.buf = 0x0, msg.x = a, msg.y = 100
msg.len = 1, msg.buf = 0x1, msg.x = b, msg.y = 101
msg.len = 2, msg.buf = 0x2, msg.x = c, msg.y = 102
msg.len = 3, msg.buf = 0x3, msg.x = d, msg.y = 103
msg.len = 4, msg.buf = 0x4, msg.x = e, msg.y = 104
```

运行一个server进程，为 20 ，同时运行两个client进程，为 5， 还有一个client进程为10的情况。
当发送的总消息个数大于mqueue最大个数时，消息队列会一直等待到队列不为满的时候再往队列里发数据。

```shell
bspserver@ubuntu:~/workspace/posix_message_queue$ ./server 20
Queue "/posix":
	- stores at most 10 messages
	-         large at most 8192 bytes each
	- currently holds 0 messages
msg.len = 0, msg.buf = 0x0, msg.x = a, msg.y = 100
msg.len = 1, msg.buf = 0x1, msg.x = b, msg.y = 101
msg.len = 2, msg.buf = 0x2, msg.x = c, msg.y = 102
msg.len = 3, msg.buf = 0x3, msg.x = d, msg.y = 103
msg.len = 4, msg.buf = 0x4, msg.x = e, msg.y = 104
msg.len = 5, msg.buf = 0x5, msg.x = f, msg.y = 105
msg.len = 6, msg.buf = 0x6, msg.x = g, msg.y = 106
msg.len = 7, msg.buf = 0x7, msg.x = h, msg.y = 107
msg.len = 8, msg.buf = 0x8, msg.x = i, msg.y = 108
msg.len = 9, msg.buf = 0x9, msg.x = j, msg.y = 109
# 如果没有进程从mqueue里取数据，此时mqueue为满。mq_send 无法再往mqueue发送数据。
msg.len = 10, msg.buf = 0xa, msg.x = k, msg.y = 110 
msg.len = 11, msg.buf = 0xb, msg.x = l, msg.y = 111
msg.len = 12, msg.buf = 0xc, msg.x = m, msg.y = 112
msg.len = 13, msg.buf = 0xd, msg.x = n, msg.y = 113
msg.len = 14, msg.buf = 0xe, msg.x = o, msg.y = 114
msg.len = 15, msg.buf = 0xf, msg.x = p, msg.y = 115
msg.len = 16, msg.buf = 0x10, msg.x = q, msg.y = 116
msg.len = 17, msg.buf = 0x11, msg.x = r, msg.y = 117
msg.len = 18, msg.buf = 0x12, msg.x = s, msg.y = 118
msg.len = 19, msg.buf = 0x13, msg.x = t, msg.y = 119
currently holds 5 messages
currently holds 4 messages
currently holds 3 messages
currently holds 2 messages
currently holds 1 messages
currently holds 1 messages
currently holds 0 messages
# 三个client进程分别取出数据
bspserver@ubuntu:~/workspace/posix_message_queue$ ./client 5 
Queue "/posix":
	- stores at most 10 messages
	-         large at most 8192 bytes each
	- currently holds 9 messages
msg.len = 6, msg.buf = 0x6, msg.x = g, msg.y = 106
msg.len = 9, msg.buf = 0x9, msg.x = j, msg.y = 109
msg.len = 12, msg.buf = 0xc, msg.x = m, msg.y = 112
msg.len = 15, msg.buf = 0xf, msg.x = p, msg.y = 115
msg.len = 17, msg.buf = 0x11, msg.x = r, msg.y = 117

bspserver@ubuntu:~/workspace/posix_message_queue$ ./client 5
Queue "/posix":
	- stores at most 10 messages
	-         large at most 8192 bytes each
	- currently holds 10 messages
msg.len = 3, msg.buf = 0x3, msg.x = d, msg.y = 103
msg.len = 5, msg.buf = 0x5, msg.x = f, msg.y = 105
msg.len = 8, msg.buf = 0x8, msg.x = i, msg.y = 108
msg.len = 11, msg.buf = 0xb, msg.x = l, msg.y = 111
msg.len = 14, msg.buf = 0xe, msg.x = o, msg.y = 114

bspserver@ubuntu:~/workspace/posix_message_queue$ ./client 10
Queue "/posix":
	- stores at most 10 messages
	-         large at most 8192 bytes each
	- currently holds 10 messages
msg.len = 0, msg.buf = 0x0, msg.x = a, msg.y = 100
msg.len = 1, msg.buf = 0x1, msg.x = b, msg.y = 101
msg.len = 2, msg.buf = 0x2, msg.x = c, msg.y = 102
msg.len = 4, msg.buf = 0x4, msg.x = e, msg.y = 104
msg.len = 7, msg.buf = 0x7, msg.x = h, msg.y = 107
msg.len = 10, msg.buf = 0xa, msg.x = k, msg.y = 110
msg.len = 13, msg.buf = 0xd, msg.x = n, msg.y = 113
msg.len = 16, msg.buf = 0x10, msg.x = q, msg.y = 116
msg.len = 18, msg.buf = 0x12, msg.x = s, msg.y = 118
msg.len = 19, msg.buf = 0x13, msg.x = t, msg.y = 119
```



Posix消息队列容许 异步事件通知，以告知何时有一个消息放置到某个空消息队列中，这种通知有两种方式可以选择:

- 产生一个信号
- 创建一个线程来执行一个指定的函数



### 信号实例

带信号通知的客户端进程代码如下：

```C
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <mqueue.h>
 
#define MAXSIZE     10   //定义buf大小
#define BUFFER      8192 //定义Msg大小
 
struct MsgType{
    int len;
    char buf[MAXSIZE];
    char x;
    short y;
};
 
int main(int argc, char **argv){
    /*消息队列*/
    mqd_t msgq_id;
    struct MsgType msg;
    unsigned int prio = 1;
    struct mq_attr msgq_attr;
    const char *file = "/posix";
    printf("\r\n");
    printf("(CLIENT) My PID: %d\n\n", getpid());
    /*mq_open() for creating a new queue (using default attributes) */
    /*mq_open() 创建一个新的 POSIX 消息队列或打开一个存在的队列*/
    msgq_id = mq_open(file, O_RDWR | O_CREAT, 0777, NULL);
    if(msgq_id == (mqd_t)-1)
    {
        perror("mq_open");
        exit(1);
    }
    /* getting the attributes from the queue        --  mq_getattr() */
    if(mq_getattr(msgq_id, &msgq_attr) == -1)
    {
        perror("mq_getattr");
        exit(1);
    }
    printf("Queue \"%s\":\n\t- stores at most %ld messages\n\t- \
        large at most %ld bytes each\n\t- currently holds %ld messages\n",
        file, msgq_attr.mq_maxmsg, msgq_attr.mq_msgsize, msgq_attr.mq_curmsgs);
    int i = 0;
    while(i < atoi(argv[1]))
    {
        msg.len = i;
        memset(msg.buf, 0, MAXSIZE);
        sprintf(msg.buf, "0x%x", i);
        msg.x = (char)(i + 'a');
        msg.y = (short)(i + 100);
        //printf("msg.len = %d, msg.buf = %s, msg.x = %c, msg.y = %d\n", msg.len, msg.buf, msg.x, msg.y);
        /*sending the message      --  mq_send() */
        /*mq_send() 把 msg_ptr 指向的消息加入由 mqdes 引用的消息队列里。*/
        /*参数 msg_len 指定消息 msg_ptr 的长度：这个长度必须小于或等于队列 mq_msgsize 属性的值。零长度的消息是允许。*/
        if(mq_send(msgq_id, (char*)&msg, sizeof(struct MsgType), prio) == -1)
        {
            perror("mq_send");
            exit(1);
        }
        i++;
        sleep(1);  
    }
    msgq_attr.mq_curmsgs = msgq_attr.mq_maxmsg;
    while(msgq_attr.mq_curmsgs)
    {
        /* getting the attributes from the queue        --  mq_getattr() */
        if(mq_getattr(msgq_id, &msgq_attr) == -1)
        {
            perror("mq_getattr");
            exit(1);
        }
        sleep(1); //等待消费者进程退出
        printf("currently holds %ld messages\n", msgq_attr.mq_curmsgs);
    }
    return 0;
}
```

带信号通知的服务端进程代码如下：

```C
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <mqueue.h>
 
#define MAXSIZE     10   //定义buf大小
#define BUFFER      8192 //定义Msg大小
 
struct MsgType{
    int len;
    char buf[MAXSIZE];
    char x;
    short y;
};
mqd_t msgq_id;   
void message_callback(int signum, siginfo_t* info, void* context);
 
int main(int argc, char **argv){
 
    struct sigevent se;
    struct sigaction sa;
    printf("\r\n");
    printf("(SERVER) My PID: %d\n\n", getpid());
     
    msgq_id = mq_open("/posix", O_RDWR | O_CREAT, 0777, NULL);
    for(;;)
    {
        se.sigev_notify = SIGEV_SIGNAL;
        se.sigev_signo = SIGUSR1;
        mq_notify(msgq_id, &se);
         
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = &message_callback;
        sigemptyset(&(sa.sa_mask));
        if(sigaction(SIGUSR1, &sa, NULL) == -1){
            perror("sigaction");
            exit(1);
        }
        pause();
    }
    mq_close(msgq_id);
    return 0;
}
 
void message_callback(int signum, siginfo_t* info, void* context){
    struct MsgType msg;
    unsigned int sender;
    unsigned int recv_size = BUFFER;
     
    if(signum == SIGUSR1){
        if(info->si_code == SI_MESGQ){
            msg.len = -1;
            memset(msg.buf, 0, MAXSIZE);
            msg.x = ' ';
            msg.y = -1;   
             
            printf("Message received from PID %d:\n", info->si_pid);
            /* getting a message */
            /*mq_receive() 从由描述符 mqdes 引用的队列时删除优先级最高的最老的消息，并把放置到 msg_ptr 的缓存区内。*/
            /*参数 msg_len 指定缓冲区 msg_ptr 的大小：它必须大于队列的 mq_msgsize 属性(参数 mq_getattr)。*/
            /*如果 prio 不是 NULL，那么它指向的内存用于返回收到消息相关的优先级。*/
            if (mq_receive(msgq_id, (char*)&msg, recv_size, &sender) == -1)
            {
                perror("mq_receive");
                exit(1);
            }
            printf("msg.len = %d, msg.buf = %s, msg.x = %c, msg.y = %d\n", msg.len, msg.buf, msg.x, msg.y);
        }
    }
}
```

代码编译运行结果如下：

```shell
bspserver@ubuntu:~/workspace/posix_message_queue$ ./mq_notify_signal_server 20  & 
[1] 12735
bspserver@ubuntu:~/workspace/posix_message_queue$ 
(SERVER) My PID: 12735

./mq_notify_signal_client 5 &
[2] 12736
bspserver@ubuntu:~/workspace/posix_message_queue$ 
(CLIENT) My PID: 12736

Queue "/posix":
	- stores at most 10 messages
	-         large at most 8192 bytes each
	- currently holds 0 messages
Message received from PID 12736:
msg.len = 0, msg.buf = 0x0, msg.x = a, msg.y = 100
./mq_notify_signal_client 5 &Message received from PID 12736:
msg.len = 1, msg.buf = 0x1, msg.x = b, msg.y = 101

[3] 12737
bspserver@ubuntu:~/workspace/posix_message_queue$ 
(CLIENT) My PID: 12737

Queue "/posix":
	- stores at most 10 messages
	-         large at most 8192 bytes each
	- currently holds 0 messages
Message received from PID 12737:
msg.len = 0, msg.buf = 0x0, msg.x = a, msg.y = 100
Message received from PID 12736:
msg.len = 2, msg.buf = 0x2, msg.x = c, msg.y = 102
Message received from PID 12737:
msg.len = 1, msg.buf = 0x1, msg.x = b, msg.y = 101
Message received from PID 12736:
msg.len = 3, msg.buf = 0x3, msg.x = d, msg.y = 103
Message received from PID 12737:
msg.len = 2, msg.buf = 0x2, msg.x = c, msg.y = 102
Message received from PID 12736:
msg.len = 4, msg.buf = 0x4, msg.x = e, msg.y = 104
Message received from PID 12737:
msg.len = 3, msg.buf = 0x3, msg.x = d, msg.y = 103
Message received from PID 12737:
msg.len = 4, msg.buf = 0x4, msg.x = e, msg.y = 104
currently holds 0 messages
currently holds 0 messages

[2]-  Done                    ./mq_notify_signal_client 5
[3]+  Done                    ./mq_notify_signal_client 5
bspserver@ubuntu:~/workspace/linux_training/kernel/process-courses/posix_message_queue$ 
```

### 指定的函数mq_notify实例

这种通知通过调用**mq_notify**建立

```cpp
#include <mqueue.h>
int mq_notify(mqd_t mqdes, const struct sigevent* notification);
```

>该函数为指定队列建立或删除异步事件通知
>
>1. 如果notification参数为非空，那么当前进程希望在有一个消息到达所指定的先前为空的对列时得到通知。
>2. 如果notification参数为空，而且当前进程被注册为接收指定队列的通知，那么已存在的注册将被撤销。
>3. 任意时刻只有一个进程可以被注册为接收某个给定队列的通知。
>4. 当有一个消息到达先前为空的消息队列，而且已有一个进程被注册为接收该队列的通知时，只有在没有任何线程阻塞在该队列的mq_receive调用中的前提下，通知才会发出。即说明，在mq_receive调用中的阻塞比任何通知的注册都优先。
>5. 当前通知被发送给它的注册进程时，其注册将被撤销。该进程必须再次调用mq_notify以重新注册。
>
>```cpp
>union sigval
>{
>    int  sival_int;  /*整数值*/
>    void *sival_ptr; /*指针值*/
>};
>struct sigevent
>{
>    int sigev_notify; /*通知类型：SIGEV_NONE、SIGEV_SIGNAL、SIGEV_THREAD*/
>    int sigev_signo; /*信号值*/
>    union sigval sigev_value; /*传递给信号处理函数或线程的信号值*/
>    void (*sigev_notify_function)(union sigval); /*线程处理函数*/
>    pthread_attr_t *sigev_notify_attributes; /*线程属性*/
>};
>sigev_notify 的取值有3个：
>SIGEV_NONE：消息到达不会发出通知
>SIGEV_SIGNAL：以信号方式发送通知，当设置此选项时，sigev_signo 设置信号的编号，且只有当信号为实时信号时才可以通过sigev_value顺带数据，参考这里。
>SIGEV_THREAD：以线程方式通知，当设置此选项时，sigev_notify_function 即一个函数指针，sigev_notify_attributes 即线程的属性
>```

```C
#include <stdio.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define MAXSIZE     10   //定义buf大小
#define BUFFER      8192 //定义Msg大小

mqd_t mqdes;
struct MsgType{
    int len;
    char buf[MAXSIZE];
    char x;
    short y;
};

static void                     /* Thread start function */
message_callback(union sigval sv)
{
    struct mq_attr attr;
	struct MsgType msg;
    unsigned int recv_size = BUFFER;	
    ssize_t nr;
    mqd_t mqdes = *((mqd_t *) sv.sival_ptr);


    /* Determine maximum msg size; allocate buffer to receive msg */


    if (mq_getattr(mqdes, &attr) == -1) {
        perror("mq_getattr");
        exit(EXIT_FAILURE);
    }

	msg.len = -1;
	memset(msg.buf, 0, MAXSIZE);
	msg.x = ' ';
	msg.y = -1;	

	/* getting a message */
	/*mq_receive() 从由描述符 mqdes 引用的队列时删除优先级最高的最老的消息，并把放置到 msg_ptr 的缓存区内。*/
	/*参数 msg_len 指定缓冲区 msg_ptr 的大小：它必须大于队列的 mq_msgsize 属性(参数 mq_getattr)。*/ 
	/*如果 prio 不是 NULL，那么它指向的内存用于返回收到消息相关的优先级。*/
	if (mq_receive(mqdes, (char*)&msg, attr.mq_msgsize, NULL) == -1) 
	{
		perror("mq_receive");
		exit(1);
	}
	printf("mq_receive: thread_id = %d, msg.len = %d, msg.buf = %s, msg.x = %c, msg.y = %d\n",getpid(), msg.len, msg.buf, msg.x, msg.y);
    exit(EXIT_SUCCESS);         /* Terminate the process */
}


int
main(int argc, char *argv[])
{
    struct sigevent sev;	
    struct MsgType msg;	
    mqd_t mqdes = mq_open("/posix", O_CREAT | O_RDWR, 0777, NULL);
    if (mqdes == (mqd_t) -1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

	bzero(&sev, sizeof(sev));
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = message_callback;
    sev.sigev_notify_attributes = NULL;
    sev.sigev_value.sival_ptr = &mqdes;   /* Arg. to thread func. */

	msg.len = atoi(argv[1]);
	memset(msg.buf, 0, MAXSIZE);
	sprintf(msg.buf, "0x%x", atoi(argv[1]));
	msg.x = (char)(atoi(argv[1]) + 'a');
	msg.y = (short)(atoi(argv[1]) + 100);
	printf("mq_send: thread_id = %d, msg.len = %d, msg.buf = %s, msg.x = %c, msg.y = %d\n",getpid(), msg.len, msg.buf, msg.x, msg.y);
	if (mq_notify(mqdes, &sev) == -1) {
		perror("mq_notify");
		exit(EXIT_FAILURE);
	}
	sleep(1);
	/*sending the message      --  mq_send() */
	/*mq_send() 把 msg_ptr 指向的消息加入由 mqdes 引用的消息队列里。*/
	/*参数 msg_len 指定消息 msg_ptr 的长度：这个长度必须小于或等于队列 mq_msgsize 属性的值。零长度的消息是允许。*/
	if(mq_send(mqdes, (char*)&msg, sizeof(struct MsgType), 1) == -1)
	{
		perror("mq_send");
		exit(1);
	}
	pause();    /* Process will be terminated by thread function */	

    exit(EXIT_SUCCESS);	
}
```

代码编译运行结果如下：

```shell
bspserver@ubuntu:~/workspace/posix_message_queue$ gcc -pthread  mq_notify_thread.c -o mq_notify_thread -lrt
bspserver@ubuntu:~/workspace/posix_message_queue$ ./mq_notify_thread 20
mq_send: thread_id = 12904, msg.len = 20, msg.buf = 0x14, msg.x = u, msg.y = 120
mq_receive: thread_id = 12904, msg.len = 20, msg.buf = 0x14, msg.x = u, msg.y = 120
bspserver@ubuntu:~/workspace/posix_message_queue$ 
```



参考文献：

[Programming with POSIX Threads](https://download.csdn.net/download/janesshang/10910991)  

[Pipe redirection in Linux: Named and Unnamed Pipes (linuxhandbook.com)](https://linuxhandbook.com/pipe-redirection/)  

[Pipes and FIFOs — Computer Systems Fundamentals](https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/Pipes.html)  

[CMP320 Operating Systems Lecture 07, 08 Operating System Concepts](https://slideplayer.com/slide/253497/)  

[POSIX message queues in Linux - SoftPrayog](https://www.softprayog.in/programming/interprocess-communication-using-posix-message-queues-in-linux)  

[**System Architecture** - Interprocess Communication (IPC)](http://www.qnx.com/developers/docs/7.1/index.html#com.qnx.doc.neutrino.sys_arch/topic/ipc.html)

