# QNX®  Neutrino POSIX线程编程

### 介绍

什么是线程？我们必须首先了解线程操作的边界。

当计算机程序从某个存储区加载到计算机内存中并开始执行时，它就变成了一个进程。一个进程可以由一个处理器或一组处理器执行。内存中的进程描述包含重要信息，例如跟踪程序中当前位置的程序计数器（即当前正在执行的指令）、寄存器、变量存储、文件句柄、信号等。

线程是程序中的一系列此类指令，可以独立于其他代码执行。

![posix_threads](D:\mygit\work-notes\QNX\pic\posix_threads.png)

上图概念性地显示了线程位于同一进程地址空间内，因此，进程的内存描述中存在的大部分信息可以跨线程共享。有些信息是无法复制的，例如堆栈（每个线程指向不同内存区域的堆栈指针）、寄存器和线程特定的数据。这一信息足以允许独立于程序的主线程和程序内的一个或多个其他线程来调度线程。



## 建立和使用线程

### 线程标识符

程序中使用线程标识符 ID 来表示线程。线程 ID 属于封装的 pthreadLt 类型。
为建立线程，你需要在程序中声明一个 `pthread_t` 类型的变量。 如果只需在某个函数中使用线程 ID，或者函数直到线程终止时才返回，则可以将线程 ID 声明为自动存储变量，不过大部分时间内， 线程 ID 保存在共享变量中（静态或外部）， 或者保存在堆空间的结构体中。

```c
pthread_t thread;
```

### 创建线程

通过向 `pthread_create` 函数传送线程函数地址和线程函数调用的参数来创建线程。线程函数应该只有一个 `void *` 类型参数，并返回相同的类型值。
当创建线程时，`pthread_create` 函数返回一个 `pthread_t` 类型的线程 ID, 并保存在 thread 参数中。 通过这个线程 ID， 程序可以引用该线程。

```c
int pthread_create(pthread_t *thread, const pthreae_attr_t *attr, void *(*start)(void *), void *arg);
```

### 获得自己的线程ID

线程可以通过调用 `pthread_self` 来获得自身的 ID。除非线程的创建者或者线程本身将线程 ID 保存于某处，否则不可能获得一个线程的 ID。要对线程进行任何操作都必须通过线程 ID。

```c
pthread_t pthread_self(void);
```

### 比较线程

可以使用 `pthread_equal` 函数来比较两个线程 ID,只能比较二者是否相同。比较两个线程 ID 谁大谁小是没有任何意义的，因为线程 ID 之间不存在顺序。如果两个线程 ID 表示同一个线程，则 `pthread_equal` 函数返回非零值，否则返回零值。

```c
int pthread_equal(pthread_t tl, pthread_t t2);  // 相等返回非0值
```

### 分离线程

如果要创建一个从不需要控制的线程，可以是用属性（attribute）来建立线程以使它可分离的。如果不想等待创建的某个线程，而且知道不再需要控制它，可以使用 `pthread_detach` 函数来分离它。
分离一个正在运行的线程不会对线程带来任何影响，仅仅是通知系统当该线程结束时，其所属资源可以被回收。

```c
int pthread_detach(pthread_t thread);
```

### 退出线程

当 C 程序运行时，首先运行 `main` 函数。在线程代码中， 这个特殊的执行流被称为 “**初始线程**” 或 “**主线程**”。 你可以在初始线程中做任何你能在普通线程中做的事情。也可以调用 `pthread_exit` 来终止自己。

```c
int pthread_exit(void *value_ptr);
```

### 取消线程

外部发送终止信号给指定线程，如果成功则返回0，否则返回非0。发送成功并不意味着线程会终止。
另外，如果一个线程被回收，终止线程的 ID 可能被分配给其他新的线程，使用该 ID 调用 `pthread_cancel` 可能就会取消一个不同的线程， 而不是返回 ESRCH 错误。

```c
int pthread_cancel(pthread_t thread);
```

### 等待线程结束

如果需要获取线程的返回值，或者需要获知其何时结束，应该调用 `pthread_join` 函数。 `pthread_join` 函数将阻塞其调用者直到指定线程终止。然后，可以选择地保存线程的返回值。调
用 `pthread_join` 函数将自动分离指定的线程。线程会在返回时被回收，回收将释放所有在线程终止时未释放的系统和进程资源，包栝保存线程返回值的内存空间、堆栈、保存寄存器状态的内存空间等。所以，在线程终止后上述资源就不该被访问了。

```c
int pthread_join(pthread_t thread, void **value_ptr);
```

## 线程生命周期

线程有四种基本状态：

- 就绪（Ready）状态。线程能够运行，但在等待可用的处理器，可能刚刚启动，或刚刚从阻塞中恢复，或者被其他线程抢占。
- 运行（Running）状态。线程正在运行，在多处器系统中，可能有多个线程处于运行态线程由于等待处理器外的其他条件无法运行，如条件变量的改变、加锁互斥量或 I/O 操作结束。
- 阻塞（Blocked）状态。线程由于等待处理器外的其他条件无法运行，如条件变量的改变、加锁互斥量或 I/O 操作结束。
- 终止（Terminated）状态。线程从起始函数中返回，或调用 pthread_exit，或者被取消，终止自己并完成所有资源清理。不是被分离，也不是被连接，一且线程被分离或者连接，它就可以被收回。

下面是线程的状态转换图：

![pipeline](D:\myBooks\QNX\POSIX标准文档\POSIX-threads-programming-tutorials-master\doc\images\lifecycle.png)

下面程序展示了一个线程使用的完整生命周期实例：

```c
/*
 *
 *   threads-example.c: Program to demonstrate Pthreads in C
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

void *ethread (void *arg);

char ret_status [10] [100];

int main (int argc, char **argv)
{
    pthread_t tid [10];
    int i, r;
    void *status;
    char buffer [128];

    // Create 10 threads
    int thread_no [10];
    for (i = 0; i < 10; i++) {
        thread_no [i] = i;
        if ((r = pthread_create (&tid [i], NULL, ethread, (void *) &thread_no [i])) != 0) {
            strerror_r (r, buffer, sizeof (buffer));
            fprintf (stderr, "Error = %d (%s)\n", r, buffer); exit (1);
        }
    }
    if ((r = pthread_cancel (tid [9])) != 0) {
        strerror_r (r, buffer, sizeof (buffer));
        fprintf (stderr, "Error = %d (%s)\n", r, buffer); exit (1);
    }
    // Wait for threads to terminate
    for (i = 0; i < 10; i++) {
        if ((r = pthread_join (tid [i], &status)) != 0) {
            strerror_r (r, buffer, sizeof (buffer));
            fprintf (stderr, "Error = %d (%s)\n", r, buffer); exit (1);
        }

        if (status == PTHREAD_CANCELED)
            printf ("i = %d, status = CANCELED\n", i);
        else
            printf ("i = %d, status = %s\n", i, (char *) status);
    }
    exit (0);
}

// ethread: example thread
void *ethread (void *arg)
{
    int my_id = *((int *) arg);

    // Take a nap
    sleep (1);

    // say hello and terminate
    printf ("Thread %d: Hello World!\n", my_id);

    sprintf (ret_status [my_id], "Thread %d: %d", my_id, my_id + 10);

    if (my_id == 9) sleep (10);

    // pass your id to the thread waiting for you to terminate
    // using pthread_join.
    pthread_exit (ret_status [my_id]);
}
```

程序运行结果如下：

```shell
bspserver@ubuntu:~/workspace/bin$ ./threads-example 
Thread 8: Hello World!
Thread 6: Hello World!
Thread 5: Hello World!
Thread 4: Hello World!
Thread 3: Hello World!
Thread 0: Hello World!
Thread 7: Hello World!
Thread 2: Hello World!
Thread 1: Hello World!
i = 0, status = Thread 0: 10
i = 1, status = Thread 1: 11
i = 2, status = Thread 2: 12
i = 3, status = Thread 3: 13
i = 4, status = Thread 4: 14
i = 5, status = Thread 5: 15
i = 6, status = Thread 6: 16
i = 7, status = Thread 7: 17
i = 8, status = Thread 8: 18
i = 9, status = CANCELED
```

上面程序中 `pthread_create` 创建线程后，线程处于就绪状态。受调度机制的限制，新线程可能在就绪状态下停留一段时间才被执行。
当处理器选中一个就绪线程执行它时，该线程进入运行态。通常这意味着某个其他线程被阻塞或者被时间片机制抢占，处理器会保存被阻塞（或抢占）线程的环境并恢复下二个就绪线程的环境。
主线程在调用 `pthread_join` 进入阻塞状态，等待它创建的线程运行结束。
当调用 `pthread_exit` 退出线程或调用 `pthread_cancel` 取消线程时， 线程在调用完清理过程后也将进入终止态。而主线程等到创建的线程终止后重新运行直到结束。



## 线程属性设置

### 线程属性

线程具有属性，用pthread_attr_t表示，在对该结构进行处理之前必须进行初始化，在使用后需要对其去除初始化。
调用pthread_attr_init之后，pthread_t结构所包含的内容就是操作系统实现支持的线程所有属性的默认值。
如果要去除对pthread_attr_t结构的初始化，可以调用pthread_attr_destroy函数。如果pthread_attr_init实现时为属性对象分配了动态内存空间，pthread_attr_destroy还会用无效的值初始化属性对象，因此如果经pthread_attr_destroy去除初始化之后的pthread_attr_t结构被pthread_create函数调用，将会导致其返回错误。

```C
typedef struct
{
       int                       detachstate;   // 线程的分离状态
       int                       schedpolicy;   // 线程调度策略
       structsched_param         schedparam;    // 线程的调度参数
       int                       inheritsched;  // 线程的继承性
       int                       scope;         // 线程的作用域
       size_t                    guardsize;     // 线程栈末尾的警戒缓冲区大小
       int                       stackaddr_set; // 线程的栈设置
       void*                     stackaddr;     // 线程栈的位置
       size_t                    stacksize;     // 线程栈的大小
} pthread_attr_t;
```

Posix线程中的线程属性pthread_attr_t主要包括分离属性、调度策略属性、调度策略优先级设置、继承属性、堆栈地址、scope属性、堆栈大小。在pthread_create中，把第二个参数设置为NULL的话，将采用默认的属性配置。

>1. **分离属性：detachstate**，如果设置为PTHREAD_CREATE_DETACHED 则新线程不能用pthread_join()来同步，且在退出时自行释放所占用的资源。缺省为PTHREAD_CREATE_JOINABLE状态。这个属性也可以在线程创建并运行以后用pthread_detach()来设置，而一旦设置为PTHREAD_CREATE_DETACH状态（不论是创建时设置还是运行时设置）则不能再恢复到PTHREAD_CREATE_JOINABLE状态。使用pthread_attr_setdetachstate()设置
>2. **调度属性：schedpolicy**，表示新线程的调度策略，主要包括SCHED_OTHER（正常、非实时）、SCHED_RR（实时、轮转法）和SCHED_FIFO（实时、先入先出）三种，缺省为SCHED_OTHER，后两种调度策略仅对超级用户有效。运行时可以用过pthread_attr_setschedpolicy ()来改变。
>3. **调度策略优先级设置：schedparam**，一个struct sched_param结构，目前仅有一个sched_priority整型变量表示线程的优先级。这个参数仅当调度策略为实时（即SCHED_RR或SCHED_FIFO）时才有效，并可以在运行时通过pthread_setschedparam()函数来改变，缺省为0。
>4. **继承属性：inheritsched**，有两种值可供选择：PTHREAD_EXPLICIT_SCHED和PTHREAD_INHERIT_SCHED，前者表示新线程使用显式指定调度策略和调度参数（即attr中的值），而后者表示继承调用者线程的值。通过pthread_attr_setinheritsched()设置，缺省为PTHREAD_EXPLICIT_SCHED
>5. **scope属性：scope**，表示线程间竞争CPU的范围，也就是说线程优先级的有效范围。POSIX的标准中定义了两个值：PTHREAD_SCOPE_SYSTEM和PTHREAD_SCOPE_PROCESS，前者表示与系统中所有线程一起竞争CPU时间，后者表示仅与同进程中的线程竞争CPU。通过pthread_attr_setscope()设置



设置线程属性流程如下：

```C
pthread_attr_t attr;
pthread_attr_init(&attr); //线程属性初始化配置系统默认线程属性
... /* set up the pthread_attr_t structure */ //设置将要设置的线程属性
pthread_create (&tid, &attr, &func, &arg);
```

设置线程属性函数如下：

```C
//– initializing, destroying
pthread_attr_init(), 
pthread_attr_destroy()
//– setting it up
pthread_attr_setdetachstate(), 
pthread_attr_setinheritsched(), 
pthread_attr_setschedparam(), 
pthread_attr_setschedpolicy(), 
pthread_attr_setstackaddr(), 
pthread_attr_setstacksize()
```

设置线程调度算法和优先级流程如下：

```C
//– setting both:
struct sched_param param;
pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED);
param.sched_priority = 15;
pthread_attr_setschedparam (&attr, &param);
pthread_attr_setschedpolicy (&attr, SCHED_RR);
pthread_create (NULL, &attr, func, arg);

//– setting priority only:
struct sched_param param;
pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED);
param.sched_priority = 15;
pthread_attr_setschedparam (&attr, &param);
pthread_attr_setschedpolicy (&attr, SCHED_NOCHANGE);
pthread_create (NULL, &attr, func, arg);
```

配置线程栈流程如下：

```C
// – to set the maximum size:
pthread_attr_setstacksize (&attr, size);
// – to provide your own buffer for the stack:
pthread_attr_setstackaddr (&attr, addr);

// Thread stack allocation can be automatic:
size = 0; // default size
addr = NULL; // OS allocates

// partly automatic:
size = desired_size;
addr = NULL; // OS allocates

// or totally manual:
size = sizeof (*stack_ptr);
addr = stack_ptr;
// Your stack size should be the sum of:
PTHREAD_STACK_MIN + platform_required_amount_for_code;
```



参考文献：

《[现代操作系统：原理与实现](https://download.csdn.net/download/v6543210/21349580?ops_request_misc=%7B%22request%5Fid%22%3A%22163910363516780265495730%22%2C%22scm%22%3A%2220140713.130102334.pc%5Fdownload.%22%7D&request_id=163910363516780265495730&biz_id=1&utm_medium=distribute.pc_search_result.none-task-download-2~download~first_rank_v2~times_rank-1-21349580.pc_v2_rank_dl_v1&utm_term=现代操作系统%3A原理与实现&spm=1018.2226.3001.4451.1)》机械工业出版社 作者是陈海波、夏虞斌 等著。

[Programming with POSIX Threads](https://download.csdn.net/download/janesshang/10910991)



