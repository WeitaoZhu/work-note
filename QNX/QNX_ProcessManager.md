# QNX®  Neutrino 进程管理器

### 简介

进程管理器可用于创建多个POSIX进程（每个进程可能包含多个POSIX线程）。在QNX Neutrino中，微内核与进程管理器一起组成`procnto`模块，所有运行时系统都需要这个模块。它的主要职责包括：

> * **进程管理 process management**，管理进程的创建 *creation*、销毁 *destruction*、属性 *attributes* 处理（用户ID和组ID）等；
>
> - **内存管理 memory management**，管理一系列的内存保护功能、共享库、进程间POSIX共享内存等；
> - **路径名管理 pathname management**，管理资源管理器可能附加到的路径名空间；

用户进程可以通过内核调用访问微内核函数，也可以通过*MsgSend\*()*向`procnto`发送消息来访问进程管理器函数。
 在`procnto`中执行线程去调用微内核的方式与其他进程中的线程完全相同，进程管理器代码和微内核共享相同的地址空间并不意味着有一套特殊的或私有的接口，系统中的所有线程共享相同的内核接口，并且在调用内核时执行特权切换。



### 进程管理 Process management

`procnto`的首要任务就是动态创建新进程，创建的进程也会依赖`procnto`提供的内存管理和路径名管理相关功能。
 进程管理包括进程创建、销毁、属性（进程ID、用户ID、组ID）管理。包含以下接口：

- `posix_spawn()`，POSIX接口，<u>*通过直接指定要加载的可执行文件来创建子进程*</u>。熟悉UNIX系统的人可能知道，这个函数相当于在`fork()`之后调用`exec*()`，但是更高效，因为不需要在像`fork()`函数中那样需要复制地址空间，而是在`exec*()`调用时直接销毁和替代；
- `spawn()`，QNX Neutrino接口，功能类似于`posix_spawn()`，使用这个接口可以控制进程的属性信息，比如文件描述符、进程组ID、信号、调度策略、调度优先级、堆栈、运行掩码（SMP系统）；
- `fork()`，POSIX接口，<u>*创建子进程，子进程与父进程共享相同的代码，并复制父进程的数据*</u>。大多数的进程资源都是继承的，不能继承的资源包括：进程ID、父进程ID、文件锁、pending信号和alarms，定时器。`fork()`函数可以在两种情况下使用：

1. 创建当前执行环境的新实例，可用`pthread_create()`替代；
2. 创建一个运行不同程序的新进程，可用`posix_spawn()`来替代；

- `vfork()`，UNIX BSD扩展接口，`vfork()`只能在单线程进程中调用。`vfork()`函数与`fork()`函数不同之处在于，它与父进程共享数据段，在调用`exec*()`或`exit()`函数之前数据都是共享的，调用`exec*()`或`exit()`函数之后父进程才能运行；
- `exec*()`，POSIX接口，
   `exec*()`系列函数，<u>*会用可执行文件加载的新进程，替换当前进程，由于调用进程被替换*</u>，因此不会有成功返回。这些函数通常用在`fork()`或`vfork()`之后，用于加载子进程。更好的方式是使用`posix_spawn()`接口。



### 内存管理 Memory management

在某些实时内核中，会在开发环境中提供内存保护支持，却很少为运行时配置提供内存保护，原因是内存和性能的损失。随着内存保护在很多嵌入式处理器中越来越普遍，内存保护的好处远远超过了它带来的性能损失，最关键的一点就是提高了软件的鲁棒性。
 内存保护对地址空间进行了隔离，避免了一个进程中的错误影响其他进程或内核。启用MMU后，操作系统可以在发生内存访问冲突时中止进程，并立刻反馈给程序员，而不是在运行一段时间后突然崩溃。

#### Memory Management Units （MMU）

典型的MMU操作方式是将物理内存划分为4KB页面，在内存中会存储一组页表，页表里存放着虚拟地址到物理地址的映射关系，CPU根据页表内容来访问物理内存。

![intelmmu](./pic/intelmmu.PNG)

​                                                                                         Virtual address mapping（on an X86）



为了提高性能，通常会使用TLB来提高页表条目的查找效率。页表条目中会对页面进行读写等权限控制。当CPU进行上下文切换的时候，如果是不同进程之间，则需要通过MMU来切换不同的地址空间，如果是在一个进程内部，则不需要这步操作。

#### Memory protection at run time

在许多嵌入式系统中，会使用硬件看门狗来检测是否有软件或硬件异常，在出现异常时则进行重启。
 在内存保护系统中，有一种更好的方式，可以称为软件看门狗。当软件出现间歇性错误时，操作系统可以捕获事件，并将控制权交给用户线程，而不是直接进行内存转储。用户线程则可以有选择的做决定，而不是像硬件看门狗那样直接重启。软件看门狗（[**Software watchdog**]([- Software watchdog (qnx.com)](http://www.qnx.com/developers/docs/7.0.0/index.html#com.qnx.doc.neutrino.sys_arch/topic/proc_Watchdog.html))）可以做：

- 中止由于内存访问冲突而失败的进程，并在不关闭系统其余部分的情况下重启该进程；
- 中止失败进程以及任何相关进程，将硬件初始化为安全状态，再重启相关进程；
- 如果故障很严重，则关闭整个系统，并发出警报；

很显然，软件看门狗能更好的进行控制，还可以收集有关软件故障的信息，有利于事后的诊断。

#### Quality control

通过将嵌入式系统划分成一组协作的、受内存保护的进程，我们可以很容易重用这些组件。加上有明确的接口定义，这些进程可以放心的集成到应用程序中，确保它们不会破坏系统的整体可靠性。当然，应用程序不可能做到完全没有bug，系统应该设计成能够容忍并从故障中恢复的架构，而利用MMU提供内存保护正是朝着这个方向迈出了良好的一步。

#### Full-protection model

在全保护模型中，QNX Neutrino首先会将image中的所有代码重映射到一个新的虚拟空间中，使能MMU，设置好初始页表。这就允许`procnto`在支持MMU的环境中启动，随后，进程管理器便会接管该环境，再根据启动的进程来修改页表。

![ususpri](./pic/ususpri.PNG)

​                                                                                                 Full protection VM *(on an x86).*

##### 平台特殊限制 Platform-specific limits

| Limit                    | x86                                             | x86_64  | ARMv7                                           | AArch64   |
| ------------------------ | ----------------------------------------------- | ------- | ----------------------------------------------- | --------- |
| System RAM (theoretical) | 32 GB^a^                                        | 1 TB    | 128 GB^a^                                       | 1 TB      |
| System RAM (tested)      | 32 GB^a^                                        | 96 GB   | 4 GB^a^                                         | 8 GB      |
| Physical addressing      | 36-bit with startup's -x option, 32-bit without | 48-bitb | 40-bit with startup's -x option, 32-bit without | 48-bit^b^ |
| Number of CPUs^b^        | 32                                              | 32      | 32^c^                                           | 32^c^     |
| Virtual address space^d^ | 3.5 GB                                          | 512 GB  | 2 GB                                            | 512 GB    |

>^a^ 您需要通过指定启动的 -x 选项来启用扩展寻址。
>
>^b^ 可能会受到硬件的进一步限制
>
>^c^ 如果您使用的是第 2 版通用中断控制器 (GIC)，则限制为 8。
>
>^d^ 这些是虚拟地址空间的绝对最大限制； 您可以通过使用 setrlimit() 设置 RLIMIT_AS 资源来减少它们。

####  Locking memory

QNX Neutrino支持内存锁定，进程可以通过锁定内存来避免获取内存页的延迟。
 内存锁定分为以下几级：

- **Unlocked**，未锁定的内存可以换入换出，内存在映射时完成分配，但是不会创建页表条目。当第一次访问内存时会失败，内存管理器会进行内存的初始化并创建页表条目，此时线程的状态为`WAITPAGE`；
- **Locked**，被锁定的内存不能被换入换出，会在访问时发生页面错误；要锁定和解锁线程的一部分内存，请调用 [*mlock()*](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.lib_ref/topic/m/mlock.html)  和 [*munlock()*](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.lib_ref/topic/m/munlock.html)； 要锁定和解锁线程的所有内存，请调用 [*mlockall()*](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.lib_ref/topic/m/mlockall.html) 和 [*munlockall()*](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.lib_ref/topic/m/munlockall.html)。 内存保持锁定状态，直到进程解锁、退出或调用 exec*() 函数。 如果进程调用 fork()、posix_spawn*() 函数或 spawn*() 函数，则在子进程中释放内存锁。
- **Superlocked**，QNX Neutrino的扩展实现，不允许任何错误，所有内存都必须初始化和私有化，并且在内存映射时设置权限，覆盖整个线程的地址空间；

#### Defragmenting physical memory

就像磁盘碎片一样，程序的运行也有可能带来内存碎片问题。
 碎片整理的任务包括更改现有的内存分配和映射，以便使用不同的底层物理页面。通过交换底层的物理内存单元，操作系统可以将碎片化空间合并成连续的区域，但是在移动某些类型的内存时需要小心，因为这类内存的映射表不能被安全的修改。

- 内核分配的一对一映射的内存区域不能移动，因为操作系统不能在不更改虚拟地址的情况下更改物理地址；
- 被应用程序锁定(`mlock()/mlockall()`)的内存不能移动；
- 具有IO特权的应用程序默认会锁定所有页面，因为设备驱动通常需要物理地址；
- 目前没有移动具有互斥锁对象的内存页，互斥锁对象通过物理地址向内核注册，如果移动带有互斥锁对象的页面，则需要重新编写这些对象。



### Pathname management

`procnto`允许资源管理器通过提供标准的API接口，管理路径名空间子集作为自己的“授权域”。当一个进程打开一个文件时，兼容POSIX的open库函数会向`procnto`发送路径名消息，`procnto`会根据路径的前缀来判断由哪一个资源管理器来处理。当一个前缀被重叠注册时，会使用与最长的前缀关联的资源管理器来处理。
 启动时，`procnto`会创建以下路径名：

| Prefix          | Description                                                  |
| --------------- | ------------------------------------------------------------ |
| **/**           | Root of the filesystem.                                      |
| **/proc/boot/** | Some of the files from the boot image presented as a flat filesystem. |
| **/proc/**pid   | The running processes, each represented by its process ID (PID). For more information, see “[Controlling processes via the **/proc** filesystem](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.prog/topic/process_proc_filesystem.html)” in the Processes chapter of the QNX Neutrino Programmer's Guide. |
| **/dev/zero**   | A device that always returns zero. Used for allocating zero-filled pages using the [*mmap()*](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.lib_ref/topic/m/mmap.html) function. |
| **/dev/mem**    | A device that represents all physical memory.                |

#### Resolving pathnames

可以举个例子来说明一下最长路径名匹配，假设有以下路径名进行了注册，并有对应的模块：

| Prefix        | Description                                                  |
| ------------- | ------------------------------------------------------------ |
| **/**         | Power-Safe filesystem ([fs-qnx6.so](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.utilities/topic/f/fs-qnx6.so.html)) |
| **/dev/ser1** | Serial device manager ([devc-ser*](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.utilities/topic/utils-d.html)) |
| **/dev/ser2** | Serial device manager (devc-ser*)                            |
| **/dev/hd0**  | Raw disk volume ([devb-eide](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.utilities/topic/d/devb-eide.html)) |



下表展示了路径名解析的最长匹配规则：

| This pathname:        | matches:      | and resolves to: |
| --------------------- | ------------- | ---------------- |
| **/dev/ser1**         | **/dev/ser1** | devc-ser*        |
| **/dev/ser2**         | **/dev/ser2** | devc-ser*        |
| **/dev/ser**          | **/**         | fs-qnx6.so       |
| **/dev/hd0**          | **/dev/hd0**  | devb-eide.so     |
| **/usr/jhsmith/test** | **/**         | fs-qnx6.so       |



##### Single-device mountpoints

假设有三个服务器：

- 服务器A，QNX 文件系统，挂载点是`/`，包含两个文件`bin/true`和`bin/false`；

- 服务器B，Flash文件系统，挂载点是`/bin/`，包含文件`ls`和`echo`；

- 服务器C，产生数字的设备，挂载点是

  ```
  /dev/random
  ```

  在进程管理器内部，挂载点列表如下：

  | Mountpoint      | Server                           |
  | --------------- | -------------------------------- |
  | **/**           | Server A (Power-Safe filesystem) |
  | **/bin**        | Server B (flash filesystem)      |
  | **/dev/random** | Server C (device)                |

假设一个客户端想往服务器C发送消息，客户端的代码如下：

```c
int fd;
fd = open("/dev/random", ...);
read(fd, ...);
close(fd);
```

在这种情况下，C库代码会请求进程管理器提供处理路径`/dev/random`的服务器，进程管理器将返回服务器列表：

- 服务器C（可能性最大，最长路径匹配）
- 服务器A（可能性最小，最短路径匹配）
   根据这些信息，C库将以此与每个服务器进行联系，并发送`open`的消息和路径组件，而服务器会对路径组件进行验证：
- 服务器C收到空路径，因为请求与挂载在同一个路径下；
- 服务器A收到路径`dev/random`，因为它的挂载点是`/`；
   一旦一个服务器确认了请求，C库就不会联系其他服务器，这意味着只有服务器C拒绝请求时，才会去联系服务器A。



##### Unioned filesystem mountpoints

假设有两个服务器：

- 服务器A， QNX 4文件系统，挂载点是`/`，包含两个文件`bin/true`和`bin/false`；
- 服务器B， Flash文件系统，挂载点是`/bin`，包含两个文件`ls`和`echo`；
   两个服务器都有`/bin`路径，但是包含不同的内容，当两个服务器都挂载后，可以看到联合挂载点如下：
- `/`，服务器A；
- `/bin`，服务器A和B；
- `/bin/echo`，服务器B；
- `/bin/false`，服务器A；
- `/bin/ls`，服务器B；
- `/bin/true`，服务器A；
   当执行以下代码，路径解析跟之前一样，但是不是将返回限制成一个连接ID，而是去联系所有的服务器并询问它们对路径的处理。



```c
DIR *dirp;
dirp = opendir("/bin", ...);
closedir(dirp);
```

结果为：

- 服务器B将收到一个空路径，因为请求与挂载在同一个路径下；
- 服务器A将收到`bin`，因为挂载点是`/bin`；
   结论是，对于处理路径`/bin`的服务器（本例中A和B），我们有一组文件描述符，当调用`readdir()`时，可以依次读取实际的目录名条目。如果目录中的任何名称都是通过常规的`open`访问，那就会执行正常的解析过程，并且只能访问一个服务器。



#### Symbolic prefixes

类似于Linux系统中的链接，在QNX中，可以通过`ln -s`来建立链接，

| Command                            | Description                         |
| ---------------------------------- | ----------------------------------- |
| ln -s existing_file symbolic_link  | Create a filesystem symbolic link.  |
| ln -Ps existing_file symbolic_link | Create a prefix tree symbolic link. |

比如：

```shell
ln -Ps /net/neutron/bin /bin
```

 此时，`/bin/ls`会被替换成`/net/neutron/bin/ls`，在进行路径名匹配时，会匹配到`/net`上，而`/net`指向的是`lsm-qnet`，`lsm-qnet`资源管理器会去解析`neutron`组件，并将进一步的解析请求发送到叫`neutron`的网络节点上，从而在`neutron`节点上完成`/bin/ls`的解析。符号链接允许我们像访问本地文件系统一样访问远程文件系统。
 执行重定向不需要运行本地文件系统，无磁盘工作站的路径名组织可能如下图，本地设备（如`;dev/ser1`或`/dev/console`）将被路由到本地字符设备管理器，而对其他路径的请求将会被路由到远程文件系统。

![prfxtree](./pic/prfxtree.PNG)



#### File descriptor namesapce

文件描述符空间属于进程内部资源，资源管理器通过使用`SCOID`（Server Connection ID）和`FD`（File Descriptor）来标识`OCB`（open control block），其中在IO管理器中，使用稀疏矩阵完成三者之间的映射：

![scofdocb](./pic/scofdocb.PNG)

​                                                                                                        Sparse array



`OCB`结构包含了打开资源的活动信息，下图中表示一个进程打开文件两次，另一个进程打开同一文件一次：

![tmpfile](./pic/tmpfile.PNG)

​                                                                                        Two processes open the same file



也可以多个不同进程之间的描述符指向同一个`OCB`结构，通常是在`dup()/dup2()/fcntl()`或者`vfork()/fork()/posix_spawn()/spawn()`接口调用时产生的，此时一个进程对`OCB`操作可能会影响与之关联的进程，如下图：

![dupfd](./pic/dupfd.PNG)

​                                                                                              A process opens a file twice



参考文献：

[**OS Core Components** - Process Manager](http://www.qnx.com/developers/docs/7.0.0/index.html#com.qnx.doc.neutrino.sys_arch/topic/proc.html)

