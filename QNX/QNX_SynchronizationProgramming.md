# QNX®  Neutrino POSIX线程同步编程

## 线程的同步

线程在进程中共享的继承资源有：Timers，Channels，Connections, Memory Access, File pointers / descriptors, Signal Handlers。

![](.\pic\share_resource.png)

虽然线程本地存储可以避免线程访问共享数据，但是线程之间的大部分数据始终还是共享的。在涉及到对共享数据进行读写操作时，就必须使用同步机制，否则就会造成线程们哄抢共享数据的结果，这会把你的数据弄的七零八落理不清头绪。

在这章节中以下结果工具能帮我们解决这些问题：

>* 互斥量（Mutexes）
>* 条件变量（Condvars）
>* 信号量（Semaphore）
>* 原子操作（Atomic Operations）



## 互斥量（Mutexes）

大部分多线程程序需要在线程间共享数据。如果两个线程同时访问共享数据就可能会有问，因为一个线程可能在另一个线程修改共享数据的过程中使用该数据，并认为共享数据保持末变。
使线程同步最通用和常用的方法就是确保对相同数据的内存访问“互斥地”进行，即一次只能允许一个线程写数据，其他线程必须等待。
同步不仅仅在修改数据时重要， 当线程需要读取其他线程写入的数据时，而且数据写入的顺序也有影响时，同样需要同步。

### 创建和销毁互斥量

Pthreads 的互斥量用 `pthread_mutex_t` 类型的变量来表示。不能拷贝互斥量，拷贝的互斥量是不确定的，但可以拷贝指向互斥量的指针。

大部分时间互斥量在函数体外，如果有其他文件使用互斥量，声明为外部类型，如果仅在本文将内使用，则将其声明为静态类型。可以使用宏 `PTHREAD_WTEX_INZTIALIZER` 来声明具有默认属性的静态互斥量，静态初始化的互斥量不需要主动释放。

下面程序演示了一个静态创建互斥量的程序，该程序 `main` 函数为空，不会产生任何结果。

```c
#include <pthread.h>
#include "errors.h"

typedef struct my_struct_tag {
    pthread_mutex_t mutex;
    int             value;
} my_struct_t;

my_struct_t data = {PTHREAD_MUTEX_INITIALIZER, 0};

int main()
{
    return 0;
}
```

如果要初始化一个非缺省属性的互斥量， 必须使用动态初始化。如当使用 `malloc` 动态分配一个包含互斥量的数据结构时，应该使用 `pthread_nutex_init` 调用来动态的初始化互斥量。当不需要互斥量时，应该调用 `pthread_mutex_destory` 来释放它。另外，如果想保证每个互斥量在使用前被初始化，而且只被初始化一次。可以在创建任何线程之前初始化它，如通过调用 `pthread_once`。

```c
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

下面程序演示了动态地初始化一个互斥量：

```c
#include <pthread.h>
#include "errors.h"

typedef struct my_struct_tag {
    pthread_mutex_t mutex;
    int             value;
} my_struct_t;

int main()
{
    my_struct_t *data;
    int status;

    data = malloc(sizeof(my_struct_t));
    if (data == NULL)
        errno_abort("Allocate structure");

    status = pthread_mutex_init(&data->mutex, NULL);
    if (status != 0)
        err_abort(status, "Init mutex");

    status = pthread_mutex_destroy(&data->mutex);
    if (status != 0)
        err_abort(status, "Destroy mutex");

    free(data);

    return status;
}
```

### 加锁和解锁互斥量

最简单的情况下使用互斥量通过调用 `pthread_mutex_lock` 或 `pthread_mutex_trylock` 锁住互斥量，处理共享数据，然后调用 `pthread_mutex_unlock` 解锁互斥量。为确保线程能够读取一组变量的一致的值，需要在任何读写这些变量的代码段周围锁住互斥量。
当调用线程己经锁住互斥量之后，就不能再加锁一个线程己经锁住互斥量之后，试图这样做的结果可能是返回错误(EDEADLK)，或者可能陷入“自死锁”，使线程永远等待下去。同样，你也不能解锁一个已经解锁的互斥量，不能解锁一个由其他线程锁住的互斥量。

```c
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

下面程序 pthread_mutex.c 是 alarm_thread.c 的一个改进版本，该程序效果如下：

```c
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define PTHREAD_MUTEX_ENABLE 0
// Global resource accessible to all threads
int global_resource;
// Mutex protecting the resource
pthread_mutex_t mutex;

// Threading routine which increments the resource 10 times and prints
// it after every increment
void* thread_inc (void* arg)
{
    // Pointer to mutex is passed as an argument
    pthread_mutex_t* mutex = arg;
    #if PTHREAD_MUTEX_ENABLE
    // Execute the following code without interrupts, all the way to the
    // point B
    if (0 != (errno = pthread_mutex_lock(mutex)))
    {
        perror("pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }
    #endif
    for (int i = 0; i < 10; i++)
    {
        global_resource++;
        printf("Increment: %d\n", global_resource);
        // Make this thread slower, so the other one
        // can do more work
        sleep(1);
    }

    printf("Thread inc finished.\n");
    #if PTHREAD_MUTEX_ENABLE
    // Point B:
    if (0 != (errno = pthread_mutex_unlock(mutex)))
    {
        perror("pthread_mutex_unlock failed");
        exit(EXIT_FAILURE);
    }
    #endif
    return NULL;
}

// Threading routine which decrements the resource 10 times and prints
// it after every decrement
void* thread_dec (void* arg)
{
    // Pointer to mutex is passed as an argument
    pthread_mutex_t* mutex = arg;
    #if PTHREAD_MUTEX_ENABLE
    if (0 != (errno = pthread_mutex_lock(mutex)))
    {
        perror("pthread_mutex_lock failed");
        exit(EXIT_FAILURE);
    }
    #endif
    for (int i = 0; i < 10; i++)
    {
        global_resource--;
        printf("Decrement: %d\n", global_resource);
        sleep(1);
    }

    printf("Thread dec finished.\n");
    #if PTHREAD_MUTEX_ENABLE
    // Point B:
    if (0 != (errno = pthread_mutex_unlock(mutex)))
    {
        perror("pthread_mutex_unlock failed");
        exit(EXIT_FAILURE);
    }
    #endif
    return NULL;
}

int main (int argc, char** argv)
{
    pthread_t threads[2];
    #if PTHREAD_MUTEX_ENABLE
    pthread_mutex_t mutex;

    // Create a mutex with the default parameters
    if (0 != (errno = pthread_mutex_init(&mutex, NULL)))
    {
        perror("pthread_mutex_init() failed");
        return EXIT_FAILURE;
    }
    #endif
    if (0 != (errno = pthread_create(&threads[0], NULL, thread_inc, &mutex)))
    {
        perror("pthread_create() failed");
        return EXIT_FAILURE;
    }

    if (0 != (errno = pthread_create(&threads[1], NULL, thread_dec, &mutex)))
    {
        perror("pthread_create() failed");
        return EXIT_FAILURE;
    }

    // Wait for threads to finish
    for (int i = 0; i < 2; i++)
    {
        if (0 != (errno = pthread_join(threads[i], NULL))) {
            perror("pthread_join() failed");
            return EXIT_FAILURE;
        }
    }
    #if PTHREAD_MUTEX_ENABLE
    // Both threads are guaranteed to be finished here, so we can safely
    // destroy the mutex
    if (0 != (errno = pthread_mutex_destroy(&mutex)))
    {
        perror("pthread_mutex_destroy() failed");
        return EXIT_FAILURE;
    }
    #endif
    return EXIT_SUCCESS;
}
```

代码运行结果如下：

```shell
bspserver@ubuntu:~/workspace/bin$ ./pthread_mutex 
Decrement: -1
Increment: 0
Increment: 1
Decrement: -1
Increment: 0
Decrement: -1
Increment: 0
Decrement: -1
Decrement: -2
Increment: -1
Decrement: -2
Increment: -1
Decrement: -2
Increment: -1
Decrement: -2
Increment: -1
Decrement: -2
Increment: -1
Decrement: -2
Increment: -1
Thread dec finished.
Thread inc finished.
```

你会发现线程运行的结果并没有按照我们预期的结果输出。

将`#define PTHREAD_MUTEX_ENABLE 0`修改为`#define PTHREAD_MUTEX_ENABLE 1`。对整个线程进行加锁处理。

运行结果如下：

```shell
root@ubuntu:/home/bspserver/workspace/POSIX-threads-programming-tutorials-master/bin# ./pthread_mutex 
Decrement: -1
Decrement: -2
Decrement: -3
Decrement: -4
Decrement: -5
Decrement: -6
Decrement: -7
Decrement: -8
Decrement: -9
Decrement: -10
Thread dec finished.
Increment: -9
Increment: -8
Increment: -7
Increment: -6
Increment: -5
Increment: -4
Increment: -3
Increment: -2
Increment: -1
Increment: 0
Thread inc finished.
```



### 非阻塞式互斥量锁

当调用 `pthread_mutex_lock` 加锁互斥量时，如果此时互斥量己经被锁住，则调用线程将被阻塞。通常这是你希望的结果，但有时你可能希望如果互斥量己被锁住，则执行另外的代码路线，你的程序可能做其他一些有益的工作而不仅仅是等待。为此，Pthreads 提供了 `pthread_mutex_trylock` 函数，当调用互斥量己被锁住时调用该函数将返回错误代码 `EBUSY`。

下列实例程序 `trylock.c` 使用 `pthread_mutex_trylock` 函数来间歇性地报告计数器的值， 不过仅当它对计数器的访问与计数线程没有发生冲突时才报告：

```c
#include <pthread.h>
#include "errors.h"

#define SPIN 10000000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long counter;
time_t end_time;

void *counter_thread(void *arg)
{
    int status;
    int spin;

    while (time(NULL) < end_time) 
    {
        status = pthread_mutex_lock(&mutex);
        if (status != 0)
            err_abort(status, "Lock mutex");

        for (spin = 0; spin < SPIN; spin++)
            counter++;

        status = pthread_mutex_unlock(&mutex);
        if (status != 0)
            err_abort(status, "Unlock mutex");

        sleep(5);
    }

    printf("Counter is %ld\n", counter);
    return NULL;
}

void *monitor_thread(void *arg)
{
    int status;
    int misses = 0;

    while (time(NULL) < end_time)
    {
        sleep(3);
        status = pthread_mutex_trylock(&mutex);
        if (status != EBUSY)
        {
            if (status != 0)
                err_abort(status, "Trylock mutex");

            printf("Counter is %ld\n", counter/SPIN);

            status = pthread_mutex_unlock(&mutex);
            if (status != 0)
                err_abort(status, "Unlock mutex");
        } else
            misses++;
    }

    printf("Monitor thread missed update %d times.\n", misses);
    return NULL;
}

int main()
{
    int status;
    pthread_t counter_thread_id;
    pthread_t monitor_thread_id;

    end_time = time(NULL) + 60; 

    status = pthread_create(&counter_thread_id, NULL, counter_thread, NULL);
    if (status != 0)
        err_abort(status, "Create counter thread");

    status = pthread_create(&monitor_thread_id, NULL, monitor_thread, NULL);
    if (status != 0)
        err_abort(status, "Create monitor thread");

    status = pthread_join(counter_thread_id, NULL);
    if (status != 0)
        err_abort(status, "Join counter thread");

    status = pthread_join(monitor_thread_id, NULL);
    if (status != 0)
        err_abort(status, "Join monitor thread");

    return 0;
}
```

代码运行结果如下：

```shell
Counter is 1
Counter is 2
Counter is 2
Counter is 3
Counter is 3
Counter is 4
Counter is 5
Counter is 5
Counter is 6
Counter is 6
Counter is 7
Counter is 8
Counter is 8
Counter is 9
Counter is 9
Counter is 10
Counter is 11
Counter is 11
Counter is 12
Counter is 12
Monitor thread missed update 0 times.
Counter is 120000000

```

将`monitor_thread`线程里的sleep(3)函数注释掉,运行结果如下：

```shell
Counter is 1
......
Counter is 12
Counter is 12
Monitor thread missed update 451 times.
Counter is 120000000
```

### 进程间的共享互斥量

>* 设置**PTHREAD_PROCESS_SHARED**标识，使得mutexes共享。
>
>* mutexes互斥量应该被放在共享内存里。
>
>* e.g.:
>
>  **pthread_mutexattr_t mutex_attr;**
>
>  **pthread_mutex_t \*mutex;**
>
>  **pthread_mutexattr_init( &mutex_attr );**
>
>  **pthread_mutexattor_setpshared( &mutex_attr,** **PTHREAD_PROCESS_SHARED);**
>
>  **mutex = (pthread_mutex_t \*)shmem_ptr;**
>
>  **pthread_mutex_init( mutex, &mutex_attr );**



共享互斥量实例如下：

```C
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>

/* Define mutex */
pthread_mutex_t mutex;
pthread_mutexattr_t mutexattr;

int main()
{
    int i = 0;
    pid_t pid;
    pthread_mutex_t *m_mutex;

    /* mutex attr Initialization */
    pthread_mutexattr_init(&mutexattr);
        
    /* modify attribute */
    pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);

    /* mutex Initialization */
    pthread_mutex_init(&mutex, &mutexattr);

    /* Share mutex */
    m_mutex  = (pthread_mutex_t*) mmap(NULL, sizeof(mutex), PROT_READ|PROT_WRITE,
                                        MAP_SHARED|MAP_ANON, -1, 0);

    pid = fork();

    if(pid == 0){
        for(; i<3; i++){
            sleep(1);
            pthread_mutex_lock(m_mutex);
            puts("This is the child process!");
            pthread_mutex_unlock(m_mutex);
        }           
    }else if(pid > 0){
        for(; i<3; i++){
            sleep(1);
            pthread_mutex_lock(m_mutex);
            puts("This is the parent process!");
            pthread_mutex_unlock(m_mutex);
        }

        /* Recycle subprocess resources */
        wait(NULL); 
    }
    
    /* Destroy mutex */
    pthread_mutexattr_destroy(&mutexattr);
    pthread_mutex_destroy(&mutex);
    
    /* Cancel mapping */
    munmap(m_mutex, sizeof(mutex)); 

    return 0;
}
```

运行结果如下：

```shell
bspserver@ubuntu:/home/bspserver/bin# ./share_mutexes
This is the parent process!
This is the child process!
This is the parent process!
This is the child process!
This is the parent process!
This is the child process!
```



### 多个互斥量与死锁

有时，一个互斥量是不够的，特别是当你的代码需要跨越软件体系内部的界限时。例如，当多个线程同时访问一个队列结构时，你需要两个互斥量，一个用来保护队列头，一个用来保护队列元素内的数据。当为多线程建立一个树型结构时，你可能需要为每个节点设置一个互斥量。

使用多个互斥量会导致复杂度的增加。最坏的情况就是死锁的发生，即两个线程分别锁住了一个互斥量而等待对方的互斥量。一个线程锁住了互斥量 A 后，加锁互斥量 B；同时另一个线程锁住了 B 而等待互斥量 A，则你的代码就产生了经典的死锁现象。

| 第一个线程                   | 第二个线程                   |
| ---------------------------- | ---------------------------- |
| pthread_mutex_lock(&mutex_a) | pthread_mutex_lock(&mutex_b) |
| pthread_mutex_lock(&mutex_b) | pthread_mutex_lock(&mutex_a) |

针对死锁，考虑以下两种通用的解决方法：

- 固定加锁顺序。所有需要同时加锁互斥量A和互斥量B的代码，必须首先加锁互斥量A，然后锁互斥量B。
- 试加锁和回退。在锁住某个集合中的第一个互斥量后，使用以 `pthread_mutex_trylock` 来加锁集合中的其他互斥量，如果失败则将集合中所有己加锁互斥量释放，并重新锁。
- 如果代码不变量允许先释放互斥量 1,然后再加锁互斥量 2,就可以避免同时拥有两个互斥量的需要。但是，如果存在被破坏的不变置需要锁住不变量 1，则互斥量 1 就不能被释放，直到不变量被恢复为止。在这种情况下， 你应该考虑使用回退（或者试锁-回退 ）算法。

以下程序 `backoff.c` 演示了如何使用回退算法避免互斥量死锁。程序建立了两个线程线程，一个运行函数 `lock_forward`,一个个运行函数 `lock_backward`。程序每次循环都会试图锁住三个互斥量，`lock_forward` 依次锁住互斥量1、2、3，`lock_backward`则按相反顺序加锁互斥量：

```c
#include <pthread.h>
#include "errors.h"

#define ITERATIONS 10

pthread_mutex_t mutex[3] = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER};

int backoff = 1;
int yield_flag = 0;

void *lock_forward(void *arg)
{
    int i, iterate, backoffs;
    int status;

    for (iterate = 0; iterate < ITERATIONS; iterate++)
    {
        backoffs = 0;

        for (i = 0; i < 3; i++)
        {
            if (i == 0)
            {
                status = pthread_mutex_lock(&mutex[i]);
                if (status != 0)
                    err_abort(status, "First lock");
                printf("forward lock got %d\n", i);
            }
            else
            {
                if (backoff)
                    status = pthread_mutex_trylock(&mutex[i]);
                else
                    status = pthread_mutex_lock(&mutex[i]);

                if (status == EBUSY)
                {
                    backoffs++;
                    printf("forward locker backing of at %d\n", i);
                    for (; i >= 0; i--)
                    {
                        status = pthread_mutex_unlock(&mutex[i]);
                        if (status != 0)
                            err_abort(status, "Backoff");
                    }
                }
                else
                {
                    if (status != 0)
                        err_abort(status, "Lock mutex");
                    printf("forward locker got %d\n", i);
                }
            }

            if (yield_flag)
            {
                if (yield_flag > 0)
                    sched_yield();
                else
                    sleep(1);
            }
        }

        printf("lock forward got all locks, %d backoffs\n", backoffs);
        pthread_mutex_unlock(&mutex[0]);
        pthread_mutex_unlock(&mutex[1]);
        pthread_mutex_unlock(&mutex[2]);
        sched_yield();
    }

    return NULL;
}

void *lock_backward(void *arg)
{
    int i, iterate, backoffs;
    int status;

    for (iterate = 0; iterate < ITERATIONS; iterate++)
    {
        backoffs = 0;

        for (i = 2; i >= 0; i--)
        {
            if (i == 2)
            {
                status = pthread_mutex_lock(&mutex[i]);
                if (status != 0)
                    err_abort(status, "First lock");
                printf("backward lock got %d\n", i);
            }
            else
            {
                if (backoff)
                    status = pthread_mutex_trylock(&mutex[i]);
                else
                    status = pthread_mutex_lock(&mutex[i]);

                if (status == EBUSY)
                {
                    backoffs++;
                    printf("backward locker backing of at %d\n", i);
                    for (; i < 3; i++)
                    {
                        status = pthread_mutex_unlock(&mutex[i]);
                        if (status != 0)
                            err_abort(status, "Backoff");
                    }
                }
                else
                {
                    if (status != 0)
                        err_abort(status, "Lock mutex");
                    printf("backward locker got %d\n", i);
                }
            }

            if (yield_flag)
            {
                if (yield_flag > 0)
                    sched_yield();
                else
                    sleep(1);
            }
        }

        printf("lock backward got all locks, %d backoffs\n", backoffs);
        pthread_mutex_unlock(&mutex[2]);
        pthread_mutex_unlock(&mutex[1]);
        pthread_mutex_unlock(&mutex[0]);
        sched_yield();
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t forward, backward;
    int status;

    if (argc > 1)
        backoff = atoi(argv[1]);

    if (argc > 2)
        yield_flag = atoi(argv[2]);

    status = pthread_create(&forward, NULL, lock_forward, NULL);
    if (status != 0)
        err_abort(status, "Create forward");

    status = pthread_create(&backward, NULL, lock_backward, NULL);
    if (status != 0)
        err_abort(status, "Create backward");

    pthread_exit(NULL);
}
```

如果没有特殊防范机制，这个程序很快就会死锁，如果上面程序运行 `backoff 0`，就会看到死锁现象：

```shell
$ ./bin/backoff 0
backward lock got 2
backward locker got 1
forward lock got 0
```

上面两个线程都调用 `pthread_mutex_lock` 来加锁每个互斥量，由于线程从不同的端开始，所以它们在中间遇到时就会死锁。
而使用回退算法的程序，不管运行多少次循环，上面的程序都会正常执行，而不会发生死锁现象。

## 条件变量（Condvars）Condition Variables

条件变量是用来通知共享数据状态信息的。可以使用条件变量来通知队列已空、或队列非空、或任何其他需要由线程处理的共享数据状态。

当一个线程互斥地访问其享状态时，它可能发现在其他线程改变状态之前它什么也做不了。即没有破坏不变量，但是线程就是对当前状态不感兴趣。例如，一个处理队列的线程发现队列为空时，它只能等恃，直到有一个节点被添加进队列中。

条件变置不提供互斥，需要一个互斥量来同步对共享数据的访问。

例如，您可以有一个计数器，一旦达到某个计数，您就希望激活一个线程。 一旦计数器达到限制，激活的线程（或多个线程）将等待条件变量。 活动线程在此条件变量上发出信号以通知其他线程在此条件变量上等待/休眠； 从而导致等待线程唤醒。 如果要通知所有等待条件变量的线程唤醒，也可以使用广播机制。 从概念上讲，这是由下图用伪代码建模的。

![condition_wait](.\pic\condition_wait.png)

### 创建和释放条件变量

程序中由 `pthread_cond_t` 类型的变量来表示条件变量。如果声明了一个使用默认属性值的静态条件变量，则需要要使用 `PTHREAD_COND_TNTTIALIZER` 宏初始化，这样初始化的条件变量不必主动释放。

```c
pthread_cond_tcond = PTHREAD_COND_INITIALIZER;
```

下面时一个**静态初始化条件变量**的实例：

```c
#include <pthread.h>
#include "errors.h"

typedef struct my_struct_tag
{
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int             value;
} my_struct_t;

my_struct_t data = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0};

int main(int argc, char const *argv[])
{
    return 0;
}
```

有时无法静态地初始化一个条件变量，例如，当使用 `malloc` 分配一个包含条件变量的结构时，这时，你需要调用 `pthread_cond_init` 来动态地初始化条件变量。当动态初始化条件变量时，应该在不需要它时调用 `pthread_cond_destory` 来释放它。

```c
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *condattr);
int pthread_cond_destroy(pthread_cond_t *cond);
```

下面是一个**动态初始化条件变量**的实例:

```c
#include <pthread.h>
#include "errors.h"

typedef struct my_struct_tag
{
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int             value;
} my_struct_t;

int main(int argc, char const *argv[])
{
    my_struct_t *data;
    int status;
 
    data = malloc(sizeof(my_struct_t));
    if (data == NULL)
        errno_abort("Allocate structure");

    status = pthread_mutex_init(&data->mutex, NULL);
    if (status != 0)
        err_abort(status, "Init mutex");

    status = pthread_cond_init(&data->cond, NULL);
    if (status != 0)
        err_abort(status, "Init condition");

    status = pthread_cond_destroy(&data->cond);
    if (status != 0)
        err_abort(status, "Destroy condition");

    status = pthread_mutex_destroy(&data->mutex);
    if (status != 0)
        err_abort(status, "Destroy mutex");

    free(data);

    return status;
}
```

### 等待条件变量和唤醒等待线程

每个条件变量必须与一个特定的互斥量、一个谓词条件相关联。当线程等待条件变量时，它必须轉相关互斥量锁住。记住，在阻寒线程之前，条件变量等待操作将解锁互斥量；而在重新返回线程之前，会再次锁住互斥量。

所有并发地（同时）等待同一个条件变量的线程心须指定同一个相关互斥量。例如，Pthreads不允许线程1使用互斥量 A 等待条件变量 A，而线程2使用互斥量 B 等待条件变量 A。不过，以下情况是十分合理的：线程1使用互斥量 A 等待条件变量 A，而线程2使用互斥量 A 等待条件变量 B。即，任何条件变量在特定时刻只能与一个互斥量相关联，而互斥量则可以同时与多个条件变过关联。

```c
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond *cond, pthread_mutex_t *mutex, struct timespec *expiration);
```

使用这些函数时适当的锁定和解锁相关的互斥量是非常重要的。如： 

>* 调用pthread_cond_wait()前锁定互斥量失败可能导致线程不会阻塞。
>* 调用pthread_cond_signal()后解锁互斥量失败可能会不允许相应的pthread_cond_wait()函数结束（保存阻塞）。 

一旦有线程为某个谓词在等待一个条件变量，你可能需要唤醒它。Pthreads 提供了两种方式唤醒等待的线程：一个是“发信号”，一个是“广播”。发信号只唤醒一个等待该条件变量的线程，而广播将唤醒所有等待该条件变量的线程。

广播与发信号真正的区别是效率：广播将唤醒额外的等待线程，而这些线程会检测自己的谓词然后继续等待，通常，不能用发信号代替广播。“当有什么疑惑的时候，就使用广播”。

```c
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
```

更深一步理解，条件变量是一种事件机制。**由一类线程来控制“事件”的发生，另外一类线程等待“事件”的发生**。为了实现这种机制，条件变量必须是共享于线程之间的全局变量。而且，**条件变量也需要与互斥锁同时使用**。

>控制“事件”发生的接口是`pthread_cond_signal()`或`pthread_cond_broadcast()`； 
>等待“事件”发生的接口是`pthead_cond_wait()`或`pthread_cond_timedwait()`。 

下面实例展示了如何等待条件变量，唤醒正在睡眠的等待线程。
线程 `wait_thread` 等待指定时间后，设置 `value` 值后，发送信号给条件变量。
主线程调用 `pthread_cond_timedwait` 函数等待最多2秒，如果 `hibernation` 大于2秒则条件变量等待将会超时，返回 `ETIMEOUT`;
如果 `hibernation` 设置为2秒，则主线程与 `wait_thread` 线程发生竞争，每次运行结果可能不同；
如果 `hibertnation` 设置少于2秒，则条件变量等待永远不会超时。

```c
#include <pthread.h>
#include <time.h>
#include "errors.h"

#define ENABLE_RETRY 0

typedef struct my_struct_tag
{
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int             value;
} my_struct_t;

my_struct_t data = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0};

int hibernation = 1;

void *wait_thread(void *arg)
{
    int status;

    sleep(hibernation);

    status = pthread_mutex_lock(&data.mutex);
    if (status != 0)
        err_abort(status, "Lock mutex");

    data.value = 1;

    status = pthread_cond_signal(&data.cond);
    if (status != 0)
        err_abort(status, "Signal condition");
    printf("Child process data.cond %d data.value %d.\n",data.cond,data.value);
    status = pthread_mutex_unlock(&data.mutex);
    if (status != 0)
        err_abort(status, "Unlock mutex");

    return NULL;
}

int main(int argc, char *argv[])
{
    int status;
    pthread_t wait_thread_id;
    struct timespec timeout;

    if (argc > 1)
        hibernation = atoi(argv[1]);

    status = pthread_create(&wait_thread_id, NULL, wait_thread, NULL);
    if (status != 0)
        err_abort(status, "Create wait thread");

    timeout.tv_sec = time(NULL) + 2;
    timeout.tv_nsec = 0;

    status = pthread_mutex_lock(&data.mutex);
    if (status != 0)
        err_abort(status, "Lock mutex");

retry:
    while (data.value == 0) {
        status = pthread_cond_timedwait(&data.cond, &data.mutex, &timeout);
        if (status == ETIMEDOUT) {
            printf("Parent process data.cond %d data.value %d.\n",data.cond,data.value);
            printf("Condition wait time out.\n");
            #if ENABLE_RETRY
            sleep(1);
            goto retry;
            #else
            break;
            #endif
        } else if (status != 0)
            err_abort(status, "Wait on condition");
    }
    printf("Parent process data.cond %d data.value %d.\n",data.cond,data.value);
    status = pthread_mutex_unlock(&data.mutex);
    if (status != 0)
        err_abort(status, "Unlock mutex");

    return 0;
}
```

代码运行结果如下：

```shell
root@ubuntu:/home/bspserver/workspace/bin# ./cond 1
Child process data.cond 1 data.value 1.
Parent process data.cond 1 data.value 0.
//由于`wait_thread` 线程延时设置为2s, 主线程与 `wait_thread` 线程发生竞争
root@ubuntu:/home/bspserver/workspace/bin# ./cond 2
Parent process data.cond 0 data.value 0.
Condition wait time out.
Parent process data.cond 0 data.value 1.

//将`#define ENABLE_RETRY 0`修改为`#define ENABLE_RETRY 1`
//主线程与 `wait_thread` 线程发生竞争,但是主线程有retry机制
bspserver@ubuntu:~/workspace/POSIX-threads-programming-tutorials-master/bin$ ./cond 2
Parent process data.cond 0 data.value 0.
Condition wait time out.
Child process data.cond 1 data.value 1.
Parent process data.cond 1 data.value 0.
```

### 闹钟实例最终版本

之前采用 `mutex` 实现的闹钟版本并不完美，它必须在处理完当前闹铃后，才能检测其他闹铃请求是否已经被加入了列表，即使新的请求
的到期时间比当前请求早。例如， 首先输入命令行 `10 message1`， 然后输入 `5 message2`，那么程序是无法预知后面5秒的闹钟加入到列表中来了，只能先处理完10秒的闹钟，才能继续处理后面的内容。

我们可以增加条件变量的使用来解决这个问题，新的版本使用一个超时条件变量操作代替睡眠操作，以等待闹钟到时。
当主线程在列表中添加了一个新的请求时，将发信号给条件变量，立刻唤醒 `alarm_thread` 线程。`alarm_thread` 线程可以重排等待的闹铃请求，然后重新等待。

你可以在我的gitee开源仓库[`alarm_cond.c`](https://gitee.com/msntec/posix-threads-programming/blob/master/src/chapter03/alarm_cond.c)获取源代码实现。

运行结果如下：

```shell
bspserver@ubuntu:~/workspace/bin$ ./alarm_cond 
Alarm> 10 Hello World~
Alarm> 3 I Love U~
Alarm> (3) I Love U~
(10) Hello World~
```

### 条件变量进程共享属性

>条件变量属性初始化
>
>* int pthread_condattr_destroy(pthread_condattr_t *attr);
>* int pthread_condattr_init(pthread_condattr_t *attr);
>
>设置条件变量属性
>
>* int pthread_condattr_getpshared(const pthread_condattr_t *restrict attr,int *restrict pshared);
>* int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared);

#### 进程间的共享条件变量

>* 设置**PTHREAD_PROCESS_SHARED**标识，使得condvar共享。
>
>* condvar互斥量应该被放在共享内存里。
>
>* e.g.:
>
>  **pthread_condattr_t cond_attr;**
>
>  **pthread_cond_t \*cond;**
>
>  **pthread_condattr_init( &cond_attr );**
>
>  **pthread_condattr_setpshared( &cond_attr,** **PTHREAD_PROCESS_SHARED);**
>
>  **cond = (pthread_cond_t \*)shmem_ptr;**
>
>  **pthread_cond_init(cond, &cond_attr );**

共享条件变量实例如下：

```C
#include<stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<pthread.h>

int main(int argc,char *argv[])
{
	int shm_id1,shm_id2;
	char * shm = "a", * shm1 = "b";
	char *buf;
	pid_t pid;
	pthread_mutex_t *mutex;
	pthread_mutexattr_t mutexattr;
	//创建、打开共享内存1
	shm_id1 = shm_open(shm,O_RDWR|O_CREAT,0744);
	//调整内存空间大小
	ftruncate(shm_id1,100);
	//映射共享内存到互斥量
	mutex = (pthread_mutex_t *)mmap(NULL,100,PROT_READ|PROT_WRITE,MAP_SHARED,shm_id1,0);
 
	pthread_mutexattr_init(&mutexattr);//互斥量属性初始化
#ifdef _POSIX_THREAD_PROCESS_SHARED
	pthread_mutexattr_setpshared(&mutexattr,PTHREAD_PROCESS_SHARED);//互斥量进程共享属性设置
#endif
	pthread_mutex_init(mutex,&mutexattr);//互斥量初始化
	//创建、打开共享内存2
	shm_id2 = shm_open(shm1,O_RDWR|O_CREAT,0744);
	//调整内存空间大小
	ftruncate(shm_id2,100);
	//映射共享内存
	buf = (char *)mmap(NULL,100,PROT_READ|PROT_WRITE,MAP_SHARED,shm_id2,0);
	
	//fork()创建新进程
	pid = fork();
	if(pid==0)
	{	
		sleep(1);
		printf("I am child process\n");
		pthread_mutex_lock(mutex);
		memcpy(buf,"Hello World!",12);
		printf("child process:  buf value is %s\n",buf);		
		pthread_mutex_unlock(mutex);
	}
	if(pid>0)
	{
	
		pthread_mutex_lock(mutex);
		printf("I am parent process\n");
		memcpy(buf,"I Love China!",13);
		sleep(2);
		printf("parent process: buf value is %s\n",buf);
		pthread_mutex_unlock(mutex);
	}
	pthread_mutexattr_destroy(&mutexattr);//互斥量属性销毁
	pthread_mutex_destroy(mutex);//互斥量销毁
	munmap(mutex,100);//解除虚拟内存映射
	shm_unlink(shm);//释放共享内存
	munmap(buf,100);
	shm_unlink(shm1);
}
```

编译运行输出如下：

```shell
bspserver@ubuntu:~/workspace/src/chapter03$ gcc -pthread cond_share.c -lrt -o cond_share
-l表示链接指定库
rt应该是库名  POSIX.1b Realtime Extensions library

bspserver@ubuntu:~/workspace/src/chapter03$ ./cond_share 
I am parent process
I am child process
parent process: buf value is I Love China!
child process:  buf value is Hello World!!
```



## 信号量（Semaphore）[ˈseməfɔːr]

信号量是由 EW Dijkstra 在20世纪60年代后期设计的编程结构。Dijkstra 的模型是铁路运营。考虑一段铁路，其中存在单个轨道，在该轨道上一次只允许一列火车。

信号量同步此轨道上的行程。火车必须在进入单轨之前等待，直到信号量处于允许旅行的状态。当火车进入轨道时，信号量改变状态以防止其他列车进入轨道。离开这段赛道的火车必须再次改变信号量的状态，以允许另一列火车进入。

在计算机版本中，信号量似乎是一个简单的整数。线程等待许可继续，然后通过对信号量执行`P操作`来发出线程已经继续的信号。

线程必须等到信号量的值为正，然后通过从值中减去1来更改信号量的值。完成此操作后，线程执行`V操作`，通过向该值加1来更改信号量的值。这些操作必须以原子方式进行。在`P操作`中，信号量的值必须在值递减之前为正，从而产生一个值，该值保证为非负值，并且比递减之前的值小1。

### 信号量 API

```c
#include <semaphore.h>
sem_t *sem_open(const char *name,int oflag,···);
int sem_close(sem_t *sem);
int sem_unlink(const char *name);
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_post(sem_t *sem);
int sem_getvalue(sem_t *sem,int *valp);
//成功返回0，出错返回-1
int sem_init(sem_t *sem,int pshared,unsigned int value);
int sem_destroy(sem_t *sem);
```

信号量初始化函数中，pshared的值为零，则不能在进程之间共享信号量。如果pshared的值非零，则可以在进程之间共享信号量。值 value 之名，
其中，`sem_post` 以原子方式递增sem指向的信号量。调用后，当信号量上的任何线程被阻塞时，其中一个线程被解除阻塞。
使用 `sem_wait` 来阻塞调用线程，直到sem指向的信号量计数变为大于零，然后原子地减少计数。

#### 无名信号量的创建与销毁

```C
 #include <semaphore.h>

int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
```

**参数**：
 `pshared`： if pshared == 0, 表示线程间共享信号量；if pshared != 0，表示进程间共享信号量。
 `sem`：若是线程间共享信号量，此值要设置为全局变量或动态内存（即是线程都可以访问）；若是进程间共享信号量，此值应该在共享内存里面（即是进程都可以访问）。
 `value`：初始化值。

#### 有信号量的创建与销毁

```C
#include <semaphore.h>
sem_t *sem_open(const char *name,int oflag,···);
int sem_close(sem_t *sem);
int sem_unlink(const char *name);
```

**参数**：
 `name`： 可访问的目录或文件
 `oflag`：if oflag == O_CREAT ，表示如果信号量不存在则创建信号量，存在则不创建； if oflag == O_CREAT｜O_EXCL，如果信号量存在，函数调用会出错。
 `mode`：用户可读写权限
 `value`：初始化值

#### 无名信号量 vs 有名信号量

>– 使用无名信号量，sem_post() 和 sem_wait() 直接调用信号量内核调用，而...
>– 使用命名信号量，sem_post() 和 sem_wait() 向 mqueue 发送消息，然后 mqueue 进行信号量内核调用
>– 所以无名信号量比命名信号量更快
>– 如果您在多线程进程中使用信号量，无名信号量很容易，因为信号量可以只是一个全局变量



### 信号量解决生产者与消费者问题

生产者和消费者问题是并发编程中标准的，众所周知的一个小问题。在一个缓冲区中，分为将项目放入缓冲区生产者，从缓冲区中取出项目的消费者。

在缓冲区有可用空间之前，生产者不能在缓冲区中放置东西。在生产者写入缓冲区之前，消费者不能从缓冲区中取出东西。

你可以查看源文件 [`pthread-semaphore.c`](https://gitee.com/msntec/posix-threads-programming/blob/master/src/chapter07/pthread_semaphore.c)，这是一个使用信号量解决生产者、消费者问题的实例。

```C
#include <pthread.h>
#include <semaphore.h>
#include "errors.h"

#define BUFF_SIZE 4
#define PRODUCTER_SIZE 2
#define CONSUMER_SIZE 2
#define ITERATIONS 4

typedef struct buff_type {
    int buff[BUFF_SIZE];
    int in;
    int out;
    sem_t full;
    sem_t empty;
    sem_t mutex;
} buff_t;

buff_t shared;

void *producter(void *arg)
{
    int id = *(int*)arg;
    int item;
    int count;

    for (count = 0; count < ITERATIONS; count++) {
        sem_wait(&shared.empty);
        sem_wait(&shared.mutex);

        item = count;
        shared.buff[shared.in] = item;
        shared.in++;
        shared.in %= BUFF_SIZE;

        printf("[%d] Producing %d ...\n", id, item);
        fflush(stdout);

        sem_post(&shared.mutex);
        sem_post(&shared.full);

        if (count % 2 == 1)
            sleep(1);
    }
}

void *consumer(void *arg)
{
    int id = *(int *)arg;
    int item;
    int count;

    for (count = 0; count < ITERATIONS; count++) {
        sem_wait(&shared.full);
        sem_wait(&shared.mutex);

        item = shared.buff[shared.out];
        shared.out++;
        shared.out %= BUFF_SIZE;

        printf("[%d] Consuming %d ...\n", id, item);
        fflush(stdout);

        sem_post(&shared.mutex);
        sem_post(&shared.empty);

        if (count % 2 == 1)
            sleep(1);
    }
}

int main()
{
    pthread_t producter_id, consumer_id;
    int count;
    int status;

    status = sem_init(&shared.full, 0, 0);
    if (status != 0)
        err_abort(status, "Sem init");

    status = sem_init(&shared.empty, 0, BUFF_SIZE);
    if (status != 0)
        err_abort(status, "Sem init");

    status = sem_init(&shared.mutex, 0, BUFF_SIZE);
    if (status != 0)
        err_abort(status, "Sem init");

    for (count = 0; count < PRODUCTER_SIZE; count++){
        pthread_create(&producter_id, NULL, producter, (void *)&count);
    }

    for (count = 0; count < CONSUMER_SIZE; count++) {
        pthread_create(&consumer_id, NULL, consumer, (void *)&count);
    }

    pthread_exit(NULL);
}
```

代码运行结果如下：

```shell
bspserver@ubuntu:~/workspace/bin$ ./pthread_semaphore 
[2] Producing 0 ...
[2] Consuming 0 ...
[2] Producing 1 ...
[2] Consuming 0 ...
[2] Producing 0 ...
[2] Producing 1 ...
[2] Consuming 1 ...
[2] Consuming 1 ...
[2] Producing 2 ...
[2] Producing 3 ...
[2] Consuming 3 ...
[2] Consuming 2 ...
[2] Producing 2 ...
[2] Producing 3 ...
[2] Consuming 2 ...
[2] Consuming 3 ...
```



参考文献：

[Programming with POSIX Threads Sample Code (gitee.com)](https://gitee.com/msntec/posix-threads-programming)

[Linux Tutorial: POSIX Threads](https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html)

[Programming with POSIX Threads](https://download.csdn.net/download/janesshang/10910991)

[Using Mutex to realize inter process synchronization](https://programmer.help/blogs/using-mutex-to-realize-inter-process-synchronization.html)

[Multithreaded Programming (POSIX pthreads Tutorial)](https://randu.org/tutorials/threads/)

[linux进程高级属性之同步属性](https://blog.csdn.net/weixin_42039602/article/details/83152354)
