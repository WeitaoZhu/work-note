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

在QNX中用法一样。但是QNX不支持多线程的进程fork。



### 进程的执行： exec*()

在fork完成后，我们得到了一个与父进程几乎完全相同的子进程，可是在很多时候’用户需要子进程执行与父进程完全不同的任务。当我们调用fork()创建了一个进程之后，通常将子进程替换成新的进程映象，这可以用exec系列的函数来进行。当然，exec系列的函数也可以将当前进程替换掉。

**函数原型**

```c
int execl(const char *path, const char *arg, ...);
int execlp(const char *file, const char *arg, ...);
int execle(const char *path, const char *arg, ..., char * const envp[]);
int execv(const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
int execvpe(const char *file, char *const argv[], char *const envp[]);
```

**参数说明**

>path：可执行文件的路径名字
>arg：可执行程序所带的参数，第一个参数为可执行文件名字，没有带路径且arg必须以NULL结束
>file：如果参数file中包含/，则就将其视为路径名，否则就按 PATH环境变量，在它所指定的各目录中搜寻可执行文件。
>
>exec族函数参数极难记忆和分辨，函数名中的字符会给我们一些帮助：
>l : 使用参数列表
>p：使用文件名，并从PATH环境进行寻找可执行文件
>v：应先构造一个指向各参数的指针数组，然后将该数组的地址作为这些函数的参数。
>e：多了envp[]数组，使用新的环境变量代替调用进程的环境变量

**fork()和exec()函数实例**

```c++
int main(int argc, char *argv[])
{
    pid_t pid;
    char *arg[] = {"ls", "-l", NULL}; 
 
    cout << "This is main process, PID is " << getpid() << endl;
    pid = fork();
    if (pid < 0){
        cout << "fork error..." << endl;
        exit(-1);
    }
    else if (pid == 0){//This is the child process
       cout << "This is child process, PID is " << getpid() << endl;
       //execl("/bin/ls", "ls", "-l", NULL); 
       //execlp("ls", "ls", "-l", NULL);
       //execle("/bin/ls", "ls", "-l", NULL, NULL);
       //execv("/bin/ls", arg);
       //execvp("ls", arg);
       execve("/bin/ls", arg, NULL);//上面的六个函数的运行结果都是一样的
       exit(11);//将子进程的退出码设置为11
    }
    else{//This is the main process
        cout << "This is main process waiting for the exit of child process." << endl;
        int child_status;
        pid = wait(&child_status);
        cout << "This is main process. The child status is " << child_status << ", and child pid is " << pid << ", WIFEXITED(child_status) is " << WIFEXITED(child_status) << ", WEXITSTATUS(child_status) is " << WEXITSTATUS(child_status) << endl;
    }
 
    exit(0);
}
```

显示结果如下：

```shell
bspserver@ubuntu:~/workspace/posix_threads/bin$ ./fork_exec 
This is main process, PID is 6429
This is main process waiting for the exit of child process.
This is child process, PID is 6430

total 10
-rwxrwxr-x 1 bspserver bspserver 23496 Dec  6 22:03 alarm_cond
-rwxrwxr-x 1 bspserver bspserver 20672 Dec  9 00:56 alarm_fork
-rwxrwxr-x 1 bspserver bspserver 22392 Dec  6 22:03 alarm_mutex
-rwxrwxr-x 1 bspserver bspserver 21048 Dec  6 22:03 alarm_thread
-rwxrwxr-x 1 bspserver bspserver 39736 Dec  9 03:04 fork_exec
-rwxrwxr-x 1 bspserver bspserver 21840 Dec  9 01:19 posix_spawn
-rwxrwxr-x 1 bspserver bspserver 21400 Dec  6 22:03 pthread_barriers
-rwxrwxr-x 1 bspserver bspserver 22080 Dec  6 22:03 pthread_rwlock
-rwxrwxr-x 1 bspserver bspserver 21416 Dec  6 22:03 pthread_semaphore
-rwxrwxr-x 1 bspserver bspserver 21176 Dec  6 22:03 pthread_spinlock

This is main process. The child status is 0, and child pid is 6430, WIFEXITED(child_status) is 1, WEXITSTATUS(child_status) is 0

```

从**fork()**和**fork()-exec()**实例程序，可以得出**fork()**与**fork()-exec()**创建进程的流程图如下：

<img src=".\pic\fork_process_lifesycle.png" alt="fork_process_lifesycle" style="zoom:80%;" />

### 进程创建： posix_spawn*()

posix_spawn是POSIX提供的另一种创建进程的方式，最初是为不支持fork的机器设计的。posix_spawn可以被认为是fork和exec两者功能的结合，它会使用类似于fork的方法（或者直接调用fork）获得一份进程的拷贝’然后调用exec执行。它可以用来用fork和exec代替相对复杂的“**fork-exec-wait**”方法。

虽然posix_spawn完成的任务类似于fork和exec的组合，但它的实现并不是对fork和exec的简单调用。目前，posix_spawn的性能要明显优于“fork+exec”且执行时间与原进程的内存无关。因此’当进程创建的性能比较关键时，应用程序可以选择牺牲“fork+exec”的灵活度，改用posix_spawn.

**函数原型**

```c
#include <spawn.h>
 
int posix_spawn(pid_t *restrict pid, const char *restrict path,
                const posix_spawn_file_actions_t *restrict file_actions,
                const posix_spawnattr_t *restrict attrp,
                char *const argv[restrict],
                char *const envp[restrict]);
int posix_spawnp(pid_t *restrict pid, const char *restrict file,
                const posix_spawn_file_actions_t *restrict file_actions,
                const posix_spawnattr_t *restrict attrp,
                char *const argv[restrict],
                char *const envp[restrict]);
```

**参数说明**

>* pid: 子进程 pid（pid 参数指向一个缓冲区，该缓冲区用于返回新的子进程的进程ID）
>
>* path: 可执行文件的路径 path（其实就是可以调用某些系统命令，只不过要指定其完整路径）
>
>* file_actions: 参数指向生成文件操作对象，该对象指定要在子对象之间执行的与文件相关的操作
>
>* attrp: 参数指向一个属性对象，该对象指定创建的子进程的各种属性.指向一个posix_spawnattr_t对象，也可以是NULL表示使用默认值，这个对象     使用posix_spawnattr_init初始化，使用*_destroy销毁，使用*_setflags、*_setpgroup、*_setsigdefault、*_setsigmask等设置参数，其中*_setflags可以设置POSIX_SPAWN_RESETIDS、*_SETPGROUP、*_SETSIGMASK、*_SETSIGDEFAULT等（Linux、FreeBSD、macOS分别还有其它支持的参数，这里只列出了公共部分)
>
>* argv: 参数指定在子进程中执行的程序的参数列表
>* envp: 参数指定在子进程中执行的程序的环境变量

**posix_spawn()函数实例**

```C
#include <spawn.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <errno.h>

#define errExit(msg)    do { perror(msg); \
                             exit(EXIT_FAILURE); } while (0)

#define errExitEN(en, msg) \
                        do { errno = en; perror(msg); \
                             exit(EXIT_FAILURE); } while (0)

char **environ;

int
main(int argc, char *argv[])
{
    pid_t child_pid;
    int s, opt, status;
    sigset_t mask;
    posix_spawnattr_t attr;
    posix_spawnattr_t *attrp;
    posix_spawn_file_actions_t file_actions;
    posix_spawn_file_actions_t *file_actionsp;

    /* Parse command-line options, which can be used to specify an
       attributes object and file actions object for the child. */

    attrp = NULL;
    file_actionsp = NULL;

    while ((opt = getopt(argc, argv, "sc")) != -1) {
        switch (opt) {
        case 'c':       /* -c: close standard output in child */

            /* Create a file actions object and add a "close"
               action to it. */

            s = posix_spawn_file_actions_init(&file_actions);
            if (s != 0)
                errExitEN(s, "posix_spawn_file_actions_init");

            s = posix_spawn_file_actions_addclose(&file_actions,
                                                  STDOUT_FILENO);
            if (s != 0)
                errExitEN(s, "posix_spawn_file_actions_addclose");

            file_actionsp = &file_actions;
            break;

        case 's':       /* -s: block all signals in child */

            /* Create an attributes object and add a "set signal mask"
               action to it. */

            s = posix_spawnattr_init(&attr);
            if (s != 0)
                errExitEN(s, "posix_spawnattr_init");
            s = posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGMASK);
            if (s != 0)
                errExitEN(s, "posix_spawnattr_setflags");

            sigfillset(&mask);
            s = posix_spawnattr_setsigmask(&attr, &mask);
            if (s != 0)
                errExitEN(s, "posix_spawnattr_setsigmask");

            attrp = &attr;
            break;
        }
    }

    /* Spawn the child. The name of the program to execute and the
       command-line arguments are taken from the command-line arguments
       of this program. The environment of the program execed in the
       child is made the same as the parent's environment. */

    s = posix_spawnp(&child_pid, argv[optind], file_actionsp, attrp,
                     &argv[optind], environ);
    if (s != 0)
        errExitEN(s, "posix_spawn");

    /* Destroy any objects that we created earlier. */

    if (attrp != NULL) {
        s = posix_spawnattr_destroy(attrp);
        if (s != 0)
            errExitEN(s, "posix_spawnattr_destroy");
    }

    if (file_actionsp != NULL) {
        s = posix_spawn_file_actions_destroy(file_actionsp);
        if (s != 0)
            errExitEN(s, "posix_spawn_file_actions_destroy");
    }

    printf("PID of child: %jd\n", (intmax_t) child_pid);

    /* Monitor status of the child until it terminates. */

    do {
        s = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);
        if (s == -1)
            errExit("waitpid");

        printf("Child status: ");
        if (WIFEXITED(status)) {
            printf("exited, status=%d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("killed by signal %d\n", WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf("stopped by signal %d\n", WSTOPSIG(status));
        } else if (WIFCONTINUED(status)) {
            printf("continued\n");
        }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    exit(EXIT_SUCCESS);
}
```

显示结果如下：

```shell
bspserver@ubuntu:~/workspace/posix_threads/bin$ ./posix_spawn date
PID of child: 8698
//子进程 date 运行结果
Thu Dec  9 17:49:21 PST 2021
//父进程检测子进程结束状态
Child status: exited, status=0

// -c 命令表示关闭了对应 data应用标准输出使用到的文件描述符
bspserver@ubuntu:~/workspace/posix_threads/bin$ ./posix_spawn -c date
date: write error: Bad file descriptor
PID of child: 8715
Child status: exited, status=1

bspserver@ubuntu:~/workspace/posix_threads/bin$ ./posix_spawn -s sleep 60 &
[1] 8720
bspserver@ubuntu:~/workspace/posix_threads/bin$ PID of child: 8721
bspserver@ubuntu:~/workspace/posix_threads/bin$ kill -KILL 8721
bspserver@ubuntu:~/workspace/posix_threads/bin$ Child status: killed by signal 9
[1]+  Done                    ./posix_spawn -s sleep 60

```

**fork()**, **fork()+exec()**, **posix_spawn()**创建的进程都是异步进程；



参考文献：

《[现代操作系统：原理与实现](https://download.csdn.net/download/v6543210/21349580?ops_request_misc=%7B%22request%5Fid%22%3A%22163910363516780265495730%22%2C%22scm%22%3A%2220140713.130102334.pc%5Fdownload.%22%7D&request_id=163910363516780265495730&biz_id=1&utm_medium=distribute.pc_search_result.none-task-download-2~download~first_rank_v2~times_rank-1-21349580.pc_v2_rank_dl_v1&utm_term=现代操作系统%3A原理与实现&spm=1018.2226.3001.4451.1)》机械工业出版社 作者是陈海波、夏虞斌 等著。

[Programming with POSIX Threads](https://download.csdn.net/download/janesshang/10910991)



