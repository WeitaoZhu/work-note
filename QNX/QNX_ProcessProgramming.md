# QNX®  Neutrino POSIX进程与多线程编程

### 介绍

由于每个操作系统提供的系统调用各不同，为了同一个应用程序在不同的操作系统上的可移植性，逐渐形成了一些可移植操作系统的接口标准。**POSIX**是**P**ortable **O**perating **S**ystem **I**nterface for Unix的简称，中文译为可移植操作系统接口，**X**表明其是对UNIX API的传承。在POSIX被提出之前，世界上存在着很多不同的UNIX操作系统，如FreeBSD, UNIX System V,Solaris, NetBSD等。这些接口各异的操作系统对应用程序开发人员造成了比较大的困扰。因此，IEEE于20世纪80年代为能同时在不同的UNIX操作系统上运行的软件定义了一套标准的操作系统API，其正式名称为**IEEE Std 1003**,国际标准的名称为**ISO/IEC 9945**。POSIX标准通常通过C library (libc)来实现。常见的libc包括glibc、musl、eglibc。Android也实现了一个名为bionic的libc。通常而言，应用程序只需要调用libc提供的接口就可以实现对操作系统功能的调用。同时也实现了应用在类UNIX系统(包括Linux)上的可移植性。不同的操作系统也可以通过对libc的移植来支持现有的应用生态。

<img src=".\pic\posix_libc.png" alt="posix_libc" style="zoom:75%;" />

在POSIX或者操作系统调用的基础上还可以封装面向不同领域的领域应用接口。为了开发的便携性(如更多可复用的功能)，人们逐渐开始为各个应用领域定义应用开发接口与软件架构。例如，面向汽车领域，一些车企联合起来定义**AUTOSAR**（**AUT**omotive **O**pen **S**ystem **AR**chitecture），以方便汽车电子平台各个部件的开发者遵循同一个标准和软件架构进行开发。随着汽车智能化引起的功能需求不断增加，AUTOSAR也逐步演进到李Adaptive AUTOSAR，并提供更为丰富的应用开发接口与软件架构。

### 接下来我们主要针对POSIX的进程和线程编译开始展开：

什么是**进程**。操作系统提出来进程的抽象：每个进程都对应于一个运行中的程序。进程可以处于以下几种状态。

- **新生状态**（**new**）: 该状态表示一个进程刚刚被创建出来，还未被初始化，不能被调度执行。在经过初始化之后，进程迁至就绪状态。
- **就绪状态**（**ready**）：该状态表示进程可以被调度执行，但还未被调度器选择。由于CPU数量可能少于进程数量，在某一时刻只有部分进程能被调度到CPU上执行。此时，系统中其他的可被调度的进程都处于预备状态。在被调度器选择执行后，进程迁移至运行状态。
- **运行状态**（**running**）：该状态表示进程正在CPU上运行。当一个进程执行—段时间后，调度器可以选择中断它的执行并重新将其放回调度队列，它就迁移至预备状态。当进程运行结束，它会迁移至终止状态。如果—个进程需要等待某些外部事件，它可以放弃CPU并迁移至阻塞状态。
- **阻塞状态**（**blocked**）：该状态表示进程需要等待外部事件（如某个I/O请求的完成）,暂时无法被调度。当进程等待的外部事件完成后,它会迁移至预备状态。     
- **终止状态**（**terminated**）：该状态表示进程已经完成了执行，且不会再被调度。

<img src=".\pic\process_modle.png" style="zoom:40%;" />

当一个程序或者进程被加载到内存里，它会被分配一个进程标识符（**Process IDentifier, PID**）。进程具有独立的虚拟内存空间。进程内存空间布局包括用户栈(**Stack**），代码库(**Shared libraries**)，用户堆(**Heap**)，数据段(**Data**)和代码段(**Text**)。还包含打开的文件描述符（**File Descriptor, fd**）表，用户ID，组ID，定时器或者其他更多的资源。

<img src=".\pic\process_resource.png" alt="process_resource" style="zoom:60%;" />

线程在进程里运行，一个进程至少有一个线程在运行。进程里的所有线程享有进程所有资源。

<img src=".\pic\process_threads.png" alt="process_threads" style="zoom:60%;" />

每个线程都有它自己独立的栈控件，进程的其他资源共享。

<img src=".\pic\process_layout.png" alt="process_layout" style="zoom:80%;" />

下面有几个进程创建方式：

>- fork(),vfork()
>
>​    通过拷贝创建一个进程
>
>- exec*()
>
>通过加载一个程序触发创建一个进程
>
>- posix_spawn(),spawn*()



#### 进程创建： fork()

 Linux的进程创建方式非常有趣。在Linux中，进程一般是通过调用`fork` 接口 ，从已有的进程中“分裂”出来的。由于是从别的进程“分裂”而来，`fork` 接口非常简单,不接收任何参数,返回值是当前进程的PID。当—个进程调用`fork` 时,操作系统会为该进程创建—个几乎一模一样的新进程。我们—般将调用`fork` 的进程称为父进程,将新创建的进程称为子进程. 当`fork` 刚刚完成时，两个进程的内存、寄存器、程序计数器等状态都完全—致；但它们是完全独立的两个进程，拥有不同的PID与虚拟内存空间，在`fork` 完成后它们会各自独立地执行,互不干扰。

 为每个命令使用 `fork` 调用生成一个独立的子进程来处理闹钟。`fork` 版本是异步方式的的一种实现，该程序可以随时输入命令行，它们被彼此独立地执行。 新版本并不比同步版本复杂多少。
该版本的主要难点在于对所有己终止子进程的 `reap`。如果程序不做这个工作，则要等到程序退出的时候由系统回收，通常回收子进程的方法是调用某个 `wait` 系列函数。在本例中，我们调用 waitpid 函数，并设置 WNOHANG（父进程不必挂等待子进程的结束）。



下面使用一个简单的闹钟实例程序来演示基本的异歩编程方法。该程序循环接受用户输入信息，直到出错或者输入完毕，用户输入的每行信息中，第一部分是闹钟等待的时间（ 以秒为单位），第二部分是闹钟时间到迖时显示的文本消息。

```c
#include <sys/types.h>
#include <wait.h>
#include "errors.h"

int main(void) {
        pid_t pid;
        int seconds;
        char line[128];
        char message[64];

        while(1) {
                printf("Alarm> ");
                if (fgets(line, sizeof(line), stdin) == NULL)
                        exit(0);

                if (strlen(line) <= 1)
                        continue;

                if (sscanf(line, "%d %64[^\n]", &seconds, message) < 2) {
                        fprintf(stderr, "Bad command\n");
                } else {
                        pid = fork();
                        if (pid == -1)
                                errno_abort("Fork");

                        if (pid == (pid_t)0) {
                                //子进程
                                sleep(seconds);
                                pid = getpid();
                                printf("Child process pid is (%d)\n",pid);
                                printf("(%d) %s\n", seconds, message);
                                exit(0);
                        } else {
                                //父进程
                                pid = getpid();
                                printf("Parent process pid is (%d)\n",pid);
                                do {
                                        pid = waitpid((pid_t)-1, NULL, WNOHANG);
                                        if (pid == (pid_t) -1)
                                                errno_abort("Wait for child");
                                } while(pid != (pid_t)0);
                        }
                }
        }
}
```

输入命令`10 I Love China!`,表示10秒后子进程会发送`(10) I Love China!`字符,查看终端log如下：

```shell
bspserver@ubuntu:~/workspace/posix_threads/bin$ ./alarm_fork 
Alarm> 10 I Love China!
Parent process pid is (4966)
Alarm> 
Child process pid is (4967)
(10) I Love China!
```

使用`top`命令查询进程的状态变化：

```shell
top - 00:11:42 up  6:51,  2 users,  load average: 0.03, 0.04, 0.00
Tasks: 299 total,   1 running, 298 sleeping,   0 stopped,   0 zombie
%Cpu(s):  0.0 us,  0.2 sy,  0.0 ni, 99.8 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
MiB Mem :   3896.7 total,   2252.9 free,    890.2 used,    753.5 buff/cache
MiB Swap:   2048.0 total,   2048.0 free,      0.0 used.   2771.1 avail Mem 

    PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND                                        4376 root      20   0   37076   8860   7224 S   0.0   0.2   0:00.00 cupsd                                          4377 root      20   0  178400  12512  10940 S   0.0   0.3   0:00.00 cups-browsed                                  4379 lp        20   0   15336   6540   5740 S   0.0   0.2   0:00.00 dbus                                          4659 root      20   0       0      0      0 I   0.0   0.0   0:00.01 kworker/u256:0-events_power_efficient          4670 root      20   0       0      0      0 I   0.0   0.0   0:00.00 kworker/u256:1-events_unbound                  4699 bspserv+  20   0  369764  22732  14972 S   0.0   0.6   0:00.04 tracker-store                                  4966 bspserv+  20   0    2648    564    480 S   0.0   0.0   0:00.00 alarm_fork                                    4967 bspserv+  20   0    2648    108      0 S   0.0   0.0   0:00.00 alarm_fork  
```

你会发现子进程在发送`(10) I Love China!`字符之后，子进程的什么周期结束之后。进程状态由`S`变成了`Z`。`Z`意思**僵尸进程(zombie)**.

一般父进程没有调用`wait`操作，或者还没来得及调用`wait`操作，就算子进程已经终止了，它的所占用的资源也不会完全释放，这种情况进程称为**僵尸进程(zombie)**。

```shell
top - 00:11:42 up  6:51,  2 users,  load average: 0.03, 0.04, 0.00
Tasks: 299 total,   1 running, 298 sleeping,   0 stopped,   1 zombie
%Cpu(s):  0.0 us,  0.2 sy,  0.0 ni, 99.8 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
MiB Mem :   3896.7 total,   2252.9 free,    890.2 used,    753.5 buff/cache
MiB Swap:   2048.0 total,   2048.0 free,      0.0 used.   2771.1 avail Mem 

    PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND                                        4376 root      20   0   37076   8860   7224 S   0.0   0.2   0:00.00 cupsd                                          4377 root      20   0  178400  12512  10940 S   0.0   0.3   0:00.00 cups-browsed                                  4379 lp        20   0   15336   6540   5740 S   0.0   0.2   0:00.00 dbus                                          4659 root      20   0       0      0      0 I   0.0   0.0   0:00.01 kworker/u256:0-events_power_efficient          4670 root      20   0       0      0      0 I   0.0   0.0   0:00.00 kworker/u256:1-events_unbound                  4699 bspserv+  20   0  369764  22732  14972 S   0.0   0.6   0:00.04 tracker-store 
   4966 bspserv+  20   0    2648    564    480 S   0.0   0.0   0:00.00 alarm_fork                                    4967 bspserv+  20   0       0      0      0 Z   0.0   0.0   0:00.00 alarm_fork   
```

也可通过`ps -e -o stat,ppid,pid,cmd|egrep '^[Zz]'`命令查询是否有**僵尸进程(zombie)**。

>ps：ps命令用于获取当前系统的进程信息.
>
>-e：参数用于列出所有的进程
>
>-o：参数用于设定输出格式，这里只输出进程的stat(状态信息)、ppid(父进程pid)、pid（当前进程的pid)，cmd(即进程的可执行文件。
>
>egrep：是linux下的正则表达式工具
>
>`'^[Zz]'`：这是正则表达式，^表示第一个字符的位置，[Zz]，表示z或者大写的Z字母，即表示第一个字符为Z或者z开头的进程数据，只所以这样是因为僵尸进程的状态信息以Z或者z字母开头。

```shell
bspserver@ubuntu:~$ ps -e -o stat,ppid,pid,cmd|egrep '^[Zz]'
STAT    PPID     PID CMD
Z+      4966    4967 [alarm_fork] <defunct>
```

为什么会出现**僵尸进程(zombie)**，大家可以带着这个问题思考一下样例代码是不是有问题？

从闹钟fork实例程序，可以得出fork创建进程的流程图如下：

<img src=".\pic\fork_process_lifesycle.png" alt="fork_process_lifesycle" style="zoom:80%;" />



在QNX中用法一样。但是QNX不支持多线程的进程fork。







参考文献：

[**OS Core Components** - OS Images](http://www.qnx.com/developers/docs/7.0.0/index.html#com.qnx.doc.neutrino.building/topic/os_images/images_about.html)

