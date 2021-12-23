# QNX®  Neutrino 进程间通信编程之Signals

## 介绍

Interprocess Communication（IPC，进程间通信）在QNX Neutrino从一个嵌入式实时系统向一个全面的POSIX系统转变起着至关重要的作用。IPC是将在内核中提供各种服务的进程内聚在一起的粘合剂。在QNX中，消息传递是IPC的主要形式，也提供了其他的形式，除非有特殊的说明，否则这些形式也都是基于本地消息传递而实现的。

将更高级别的 IPC 服务（如通过我们的消息传递实现的管道和 FIFO）与其宏内核对应物进行比较的基准测试表明性能相当。

QNX Neutrino提供以下形式的IPC：

| Service:             | Implemented in:  |
| -------------------- | ---------------- |
| Message-passing      | Kernel           |
| Pules                | Kernel           |
| **Signals**          | **Kernel**       |
| Event Delivery       | External process |
| POSIX message queues | External process |
| Shared memory        | Process manager  |
| Pipes                | External process |
| FIFOs                | External process |

本篇幅介绍的是POSIX IPC Signals。

## Signals



参考文献：

[Programming with POSIX Threads](https://download.csdn.net/download/janesshang/10910991)

[**System Architecture** - Interprocess Communication (IPC)](http://www.qnx.com/developers/docs/7.1/index.html#com.qnx.doc.neutrino.sys_arch/topic/ipc.html)
