# QNX®  Neutrino 进程间通信编程

### 介绍

Interprocess Communication（IPC，进程间通信）在QNX Neutrino从一个嵌入式实时系统向一个全面的POSIX系统转变起着至关重要的作用。IPC是将在内核中提供各种服务的进程内聚在一起的粘合剂。在QNX中，消息传递是IPC的主要形式，也提供了其他的形式，除非有特殊的说明，否则这些形式也都是基于本地消息传递而实现的。

将更高级别的 IPC 服务（如通过我们的消息传递实现的管道和 FIFO）与其宏内核对应物进行比较的基准测试表明性能相当。

QNX Neutrino提供以下形式的IPC：

| Service:             | Implemented in:  |
| -------------------- | ---------------- |
| Message-passing      | Kernel           |
| Pules                | Kernel           |
| Signals              | Kernel           |
| POSIX message queues | External process |
| Shared memory        | Process manager  |
| Pipes                | External process |
| FIFOs                | External process |

设计人员可以根据带宽要求，排队需求，网络透明度等选择这些服务。权衡可能很复杂，但灵活性很实用。





参考文献：

《[现代操作系统：原理与实现](https://download.csdn.net/download/v6543210/21349580?ops_request_misc=%7B%22request%5Fid%22%3A%22163910363516780265495730%22%2C%22scm%22%3A%2220140713.130102334.pc%5Fdownload.%22%7D&request_id=163910363516780265495730&biz_id=1&utm_medium=distribute.pc_search_result.none-task-download-2~download~first_rank_v2~times_rank-1-21349580.pc_v2_rank_dl_v1&utm_term=现代操作系统%3A原理与实现&spm=1018.2226.3001.4451.1)》机械工业出版社 作者是陈海波、夏虞斌 等著。

[Programming with POSIX Threads](https://download.csdn.net/download/janesshang/10910991)



