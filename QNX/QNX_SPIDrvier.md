# QNX® Neutrino SPI驱动框架与代码分析

本文主要描述QNX SPI Drvier的相关内容，并以Texas Instruments DRA71x Jacinto6 Cortex A15处理器为例讲解

在[QNX® Neutrino I2C驱动框架与代码分析](./QNX_I2CDrvier.md)，我们具体分析了I2C驱动框架与源码的实现，同样在QNX系统里，也提供了SPI驱动框架。



## 目录结构与组成部分

下面是Texas.Instruments.DRA71x.Jacinto6.Entry.BSP.for.QNX.SDP.6.6包目录，我们只展开跟SPI相关目录内容：

```shell
.
├── Makefile
├── images
│   ├── Makefile
│   └── mkflashimage.sh
├── install
├── manifest
├── prebuilt
│   ├── armle-v7
│   │   ├── lib
│   │   │   └── dll
│   │   └── usr
│   │       └── lib
│   │           ├── libspi-master.a
│   │           └── libutil.a
│   └── usr
│       └── include
└── src
    ├── Makefile
    ├── hardware
    │   ├── spi
    │   │   ├── Makefile
    │   │   ├── master
    │   │   │   ├── Makefile
    │   │   │   ├── _spi_close_ocb.c
    │   │   │   ├── _spi_create_instance.c
    │   │   │   ├── _spi_devctl.c
    │   │   │   ├── _spi_devlock.c
    │   │   │   ├── _spi_dlload.c
    │   │   │   ├── _spi_init_iofunc.c
    │   │   │   ├── _spi_iomsg.c
    │   │   │   ├── _spi_iomsg_cmdread.c
    │   │   │   ├── _spi_iomsg_dmaxchange.c
    │   │   │   ├── _spi_iomsg_read.c
    │   │   │   ├── _spi_iomsg_write.c
    │   │   │   ├── _spi_iomsg_xchange.c
    │   │   │   ├── _spi_main.c
    │   │   │   ├── _spi_ocb.c
    │   │   │   ├── _spi_read.c
    │   │   │   ├── _spi_slogf.c
    │   │   │   ├── _spi_write.c
    │   │   │   ├── arm
    │   │   │   ├── common.mk
    │   │   │   ├── module.tmpl
    │   │   │   ├── project.xml
    │   │   │   └── proto.h
    │   │   └── omap4430
    │   │       ├── Makefile
    │   │       ├── arm
    │   │       ├── clock_toggle.c
    │   │       ├── clock_toggle.h
    │   │       ├── common.mk
    │   │       ├── config.c
    │   │       ├── context_restore.c
    │   │       ├── context_restore.h
    │   │       ├── intr.c
    │   │       ├── module.tmpl
    │   │       ├── omap4430spi.c
    │   │       ├── omap4430spi.h
    │   │       ├── pinfo.mk
    │   │       ├── project.xml
    │   │       ├── reg_map_init.c
    │   │       ├── sdma.c
    │   │       ├── sdma.h
    │   │       ├── spi-omap4430.use
    │   │       └── wait.c
    │   ├── startup
    │   └── support
    └── utils
        ├── Makefile
        └── r

```

I2C框架由以下部分组成:

hardware/spi/omap4430/*    硬件接口

```shell
├── hardware
│   ├── spi
│   │   ├── Makefile
│   │   ├── master
│   │   └── omap4430
│   │       ├── Makefile
│   │       ├── arm
│   │       ├── clock_toggle.c
│   │       ├── clock_toggle.h
│   │       ├── common.mk
│   │       ├── config.c
│   │       ├── context_restore.c
│   │       ├── context_restore.h
│   │       ├── intr.c
│   │       ├── module.tmpl
│   │       ├── omap4430spi.c
│   │       ├── omap4430spi.h
│   │       ├── pinfo.mk
│   │       ├── project.xml
│   │       ├── reg_map_init.c
│   │       ├── sdma.c
│   │       ├── sdma.h
│   │       ├── spi-omap4430.use
│   │       └── wait.c
```

hardware/spi/master 资源管理器层

```shell
├── hardware
│   ├── spi
│   │   ├── Makefile
│   │   ├── master
│   │   │   ├── Makefile
│   │   │   ├── _spi_close_ocb.c
│   │   │   ├── _spi_create_instance.c
│   │   │   ├── _spi_devctl.c
│   │   │   ├── _spi_devlock.c
│   │   │   ├── _spi_dlload.c
│   │   │   ├── _spi_init_iofunc.c
│   │   │   ├── _spi_iomsg.c
│   │   │   ├── _spi_iomsg_cmdread.c
│   │   │   ├── _spi_iomsg_dmaxchange.c
│   │   │   ├── _spi_iomsg_read.c
│   │   │   ├── _spi_iomsg_write.c
│   │   │   ├── _spi_iomsg_xchange.c
│   │   │   ├── _spi_main.c
│   │   │   ├── _spi_ocb.c
│   │   │   ├── _spi_read.c
│   │   │   ├── _spi_slogf.c
│   │   │   ├── _spi_write.c
│   │   │   ├── arm
│   │   │   ├── common.mk
│   │   │   ├── module.tmpl
│   │   │   ├── project.xml
│   │   │   └── proto.h
│   │   └── omap4430
```



## 硬件管理接口

这是实现 SPI 主设备特定于硬件的底层功能的代码的接口。`spi_funcs_t` 结构是一个指向函数的指针表，您可以为特定于硬件的底层模块提供这些函数。 高级应用层代码调用这些函数。

```C
typedef struct {
    size_t  size;   /* size of this structure */
    void*   (*init)( void *hdl, char *options );
    void    (*fini)( void *hdl );
    int     (*drvinfo)( void *hdl, spi_drvinfo_t *info );
    int     (*devinfo)( void *hdl, uint32_t device, spi_devinfo_t *info );
    int     (*setcfg)( void *hdl, uint16_t device, spi_cfg_t *cfg );
    void*   (*xfer)( void *hdl, uint32_t device, uint8_t *buf, int *len );
    int     (*dma_xfer)( void *hdl, uint32_t device, spi_dma_paddr_t *paddr, int len );
} spi_funcs_t;
```

底层模块中必须有一个函数表目录，它必须命名为spi_drv_entry。 应用代码或者资源管理器代码查找此符号名称以查找底层模块的函数表。

### 底层模块句柄结构

SPIDEV 结构是底层模块必须返回给高层代码的句柄。 您可以扩展结构，但 SPIDEV 必须位于顶部。 当高层代码调用底层函数时，此句柄也会传递给底层驱动程序。

```C
typedef struct _spidev_entry {
    iofunc_attr_t   attr;
    void        *hdl;       /* Pointer to high-level handle */
    void        *lock;      /* Pointer to lock list */
} SPIDEV;
```

### init函数

init函数初始化主接口。该函数的原型是:

```C
void *init( void *hdl, char *options );
/*
* Parameters:
* (in)     hdl         Handle returned from init()  init函数返回的句柄；
* (in)     options     A pointer to the command-line arguments for the low-level module 底层模块命令行参数
```

该函数必须返回一个句柄，它是指向底层模块的 SPIDEV 的指针，如果发生错误，则返回 NULL。

### fini函数

fini函数清理驱动程序并释放与给定句柄关联的所有内存。该函数的原型是:

```c
void (*fini)(void *hdl);
```

### drvinfo函数

drvinfo函数返回有关驱动程序的信息。该函数的原型是:

```C
int drvinfo( void *hdl, spi_drvinfo_t *info );
/* 
hdl            init函数返回的句柄
info
一个指向spi_drvinfo_t结构的指针，函数应该在该结构中存储信息:
typedef struct {
        uint32_t    version;
        char        name[16];   // Driver name
        uint32_t    feature;
#define SPI_FEATURE_DMA         (1 << 31)
#define SPI_FEATURE_DMA_ALIGN   0xFF
      } spi_drvinfo_t; 
*/
```

### devinfo函数

devinfo函数返回有关驱动程序的信息。该函数的原型是:

```C
int devinfo( void *hdl, uint32_t device, spi_devinfo_t *info );
/*  
hdl            init函数返回的句柄
device         设备 ID。 您可以使用 SPI_DEV_DEFAULT 或它来选择当前设备； 否则选择下一个设备。
info
一个指向spi_drvinfo_t结构的指针，函数应该在该结构中存储信息:
typedef struct {
    uint32_t    device;     // Device ID 
    char        name[16];   // Device description 
    spi_cfg_t   cfg;        // Device configuration
} spi_devinfo_t;
*/
```

### setcfg函数

setcfg 函数更改 SPI 总线上特定设备的配置。该函数的原型是:

```C
int setcfg( void *hdl, uint16_t device, spi_cfg_t *cfg );
/* 
* Parameters:
* (in)     hdl         Handle returned from init()  init函数返回的句柄；
* (in)     device      The device ID.   设备 ID。 
* (in)     cfg         A pointer to the configuration structure. This structure is defined as:       
指向配置结构的指针。 该结构定义为
typedef struct {
    uint32_t    mode;
    uint32_t    clock_rate;
} spi_cfg_t;
*/
```

### xfer函数

xfer 函数启动传输、接收或交换事务。该函数的原型是:

```C
void *xfer( void *hdl, uint32_t device, uint8_t *buf, int *len );
/* 
* xfer.
* Parameters:
* (in)     hdl         Handle returned from init()   init函数返回的句柄
* (in)     device      The device ID.   设备 ID。
* (in)     buf         Buffer for received data      指向要发送的数据缓冲区的指针；
* (in)     len         Length in bytes of buf        发送的数据的长度(以字节为单位)；
*/
```

该函数必须返回一个指向接收/交换缓冲区的指针，并在 len 指向的位置存储底层模块已发送、接收或交换的数据的字节长度。 高层代码检查长度以确定事务是否成功。

### dma_xfer函数

dma_xfer 函数启动 DMA 发送、接收或交换事务。该函数的原型是:

```C
int *dma_xfer( void *hdl, uint32_t device, spi_dma_paddr_t *paddr, int len );
/* 
* dma_xfer.
* Parameters:
* (in)     hdl         Handle returned from init()   init函数返回的句柄
* (in)     device      The device ID.   设备 ID。
* (in)     paddr       指向 DMA 缓冲区地址的指针，其定义为：
                        typedef struct {
                            uint64_t    rpaddr;
                            uint64_t    wpaddr;
                        } spi_dma_paddr_t;
                        rpaddr 和 wpaddr 是物理地址。
* (in)     len         Length in bytes of buf        发送的数据的长度(以字节为单位)；
*/
```

此函数必须返回 DMA 已成功传输的字节数，如果发生错误，则返回 -1。 管理 DMA 缓冲区是应用程序的责任。



## API library 库的接口

libspi-master 库提供了一个接口来调解对 SPI 主设备的访问。 资源管理器层注册一个设备名称（通常是/dev/spi0）。 应用程序通过使用 <hw/spi-master.h> 中声明的函数访问 SPI 主设备。库函数包含如下：

>- [*spi_open()*](http://www.qnx.com/developers/docs/6.5.0/topic/com.qnx.doc.neutrino_technotes/spi_overview.html?cp=13_10_10#spi_open)
>- [*spi_close()*](http://www.qnx.com/developers/docs/6.5.0/topic/com.qnx.doc.neutrino_technotes/spi_overview.html?cp=13_10_10#spi_close)
>- [*spi_setcfg()*](http://www.qnx.com/developers/docs/6.5.0/topic/com.qnx.doc.neutrino_technotes/spi_overview.html?cp=13_10_10#spi_setcfg)
>- [*spi_getdevinfo()*](http://www.qnx.com/developers/docs/6.5.0/topic/com.qnx.doc.neutrino_technotes/spi_overview.html?cp=13_10_10#spi_getdevinfo)
>- [*spi_getdrvinfo()*](http://www.qnx.com/developers/docs/6.5.0/topic/com.qnx.doc.neutrino_technotes/spi_overview.html?cp=13_10_10#spi_getdrvinfo)
>- [*spi_read()*](http://www.qnx.com/developers/docs/6.5.0/topic/com.qnx.doc.neutrino_technotes/spi_overview.html?cp=13_10_10#spi_read)
>- [*spi_write()*](http://www.qnx.com/developers/docs/6.5.0/topic/com.qnx.doc.neutrino_technotes/spi_overview.html?cp=13_10_10#spi_write)
>- [*spi_xchange()*](http://www.qnx.com/developers/docs/6.5.0/topic/com.qnx.doc.neutrino_technotes/spi_overview.html?cp=13_10_10#spi_xchange)
>- [*spi_cmdread()*](http://www.qnx.com/developers/docs/6.5.0/topic/com.qnx.doc.neutrino_technotes/spi_overview.html?cp=13_10_10#spi_cmdread)
>- [*spi_dma_xchange()*](http://www.qnx.com/developers/docs/6.5.0/topic/com.qnx.doc.neutrino_technotes/spi_overview.html?cp=13_10_10#spi_dma_xchange)



### spi_open函数

spi_open() 函数让应用程序连接到 SPI 资源管理器。 这个函数的原型是：

```C
int spi_open( const char *path );
/* 
* dma_xfer.
* Parameters:
* (in)     path         The path name of the SPI device, usually /dev/spi0.   SPI 设备的路径名，通常为 /dev/spi0。
*/
```

此函数返回一个文件描述符，如果打开失败，则返回 -1。

### spi_close函数

spi_close() 函数让应用程序连接到 SPI 资源管理器。 这个函数的原型是：

```C
int spi_close( int fd );
/* 
* spi_close.
* Parameters:
* (in)     fd   The file descriptor that the spi_open() function returned. spi_open() 函数返回的文件描述符。。
*/
```

### spi_setcfg函数

spi_setcfg() 函数设置 SPI 总线上特定设备的配置。这个函数的原型是：

```C
int spi_setcfg( int fd, uint32_t device, spi_cfg_t *cfg );
/* 
* spi_close.
* Parameters:
* (in)     fd          The file descriptor that the spi_open() function returned. spi_open() 函数返回的文件描述符。
* (in)     device      The device ID.   设备 ID。 
* (in)     cfg         A pointer to the configuration structure. This structure is defined as:       
指向配置结构的指针。 该结构定义为
typedef struct {
    uint32_t    mode;
    uint32_t    clock_rate;
} spi_cfg_t;
*/
```

### spi_getdevinfo函数

spi_getdevinfo() 函数获取 SPI 总线上特定设备的信息。这个函数的原型是：

```C
int spi_getdevinfo( int fd, uint32_t device, spi_devinfo_t *devinfo );
/*  
fd             spi_open() 函数返回的文件描述符。
device         设备 ID。 设备 ID 或 SPI_DEV_ID_NONE 以选择第一个设备。 
                        如果指定了设备ID，则可以与SPI_DEV_DEFAULT 进行OR 来选择指定的设备； 否则，选择下一个设备。
devinfo
一个指向spi_devinfo_t结构的指针，函数应该在该结构中存储信息:
typedef struct {
    uint32_t    device;     // Device ID 
    char        name[16];   // Device description 
    spi_cfg_t   cfg;        // Device configuration
} spi_devinfo_t;
*/
```

### spi_getdrvinfo函数

spi_getdrvinfo() 函数获取低级模块的驱动程序信息。这个函数的原型是：

```C
int spi_getdrvinfo( int fd, spi_drvinfo_t *drvinfo );
/*  
fd             spi_open() 函数返回的文件描述符。
drvinfo
一个指向spi_drvinfo_t结构的指针，函数应该在该结构中存储信息:
typedef struct {
        uint32_t    version;
        char        name[16];   // Driver name
        uint32_t    feature;
      } spi_drvinfo_t; 
*/
```

### spi_read函数

spi_read() 函数从 SPI 总线上的特定设备读取数据。这个函数的原型是：

```C
int spi_read( int fd, uint32_t device, void *buf, int len );
/* 
* spi_read.
* Parameters:
* (in)     fd          The file descriptor that the spi_open() function returned. spi_open() 函数返回的文件描述符。
* (in)     device      The device ID.   设备 ID。
* (out)    buf         A pointer to the read buffer. 指向要读取的数据缓冲区的指针；
* (in)     len         Length in bytes of buf        读取的数据的长度(以字节为单位)；
该函数返回它成功从设备读取的数据字节数。 如果发生错误，函数返回 -1 并设置 errno：
EIO
The read from the device failed, or a hardware error occurred.
EINVAL
The device ID is invalid, or you're trying to unlock a device that isn't locked.
ENOMEM
Insufficient memory.
EPERM
The device is locked by another connection.
*/
```

如果此函数返回的字节数与其要求函数读取的字节数不同，则 SPI 驱动程序通常会将其视为错误。

### spi_write函数

spi_write() 函数从 SPI 总线上的特定设备读取数据。这个函数的原型是：

```C
int spi_write( int fd, uint32_t device, void *buf, int len );
/* 
* spi_write.
* Parameters:
* (in)     fd          The file descriptor that the spi_open() function returned. spi_open() 函数返回的文件描述符。
* (in)     device      The device ID.   设备 ID。
                       最多具有以下标志之一的设备 ID（可选）：
                       SPI_DEV_LOCK
                       SPI_DEV_UNLOCK
* (in)     buf         A pointer to the write buffer. 指向要发送的数据缓冲区的指针；
* (in)     len         Length in bytes of buf        发送的数据的长度(以字节为单位)；
该函数返回它成功从设备读取的数据字节数。 如果发生错误，函数返回 -1 并设置 errno：
EIO
The read from the device failed, or a hardware error occurred.
EINVAL
The device ID is invalid, or you're trying to unlock a device that isn't locked.
ENOMEM
Insufficient memory.
EPERM
The device is locked by another connection.
*/
```

如果此函数返回的字节数与其要求函数写入的字节数不同，则 SPI 驱动程序通常会将其视为错误。

### spi_xchange函数

spi_exchange() 函数在特定设备和 SPI 主设备之间交换数据。该函数的原型是:

```C
int spi_xchange( int fd, uint32_t device, void *wbuf,  void *rbuf, int len );
/* 
* spi_xchange.
* Parameters:
* (in)     fd          The file descriptor that the spi_open() function returned. spi_open() 函数返回的文件描述符。
* (in)     device      The device ID.   设备 ID。
                       最多具有以下标志之一的设备 ID（可选）：
                       SPI_DEV_LOCK
                       SPI_DEV_UNLOCK
* (in)     wbuf        A pointer to the send buffer.      指向要发送的数据缓冲区的指针；
* (out)    rbuf        A pointer to the receive buffer.   指向要接收的数据缓冲区的指针；
* (in)     len         Length in bytes of buf        发送的数据的长度(以字节为单位)
该函数返回它成功从设备读取的数据字节数。 如果发生错误，函数返回 -1 并设置 errno：
EIO
The read from the device failed, or a hardware error occurred.
EINVAL
The device ID is invalid, or you're trying to unlock a device that isn't locked.
ENOMEM
Insufficient memory.
EPERM
The device is locked by another connection.
*/
```

该函数必须返回一个指向接收/交换缓冲区的指针，并在 len 指向的位置存储底层模块已发送、接收或交换的数据的字节长度。 高层代码检查长度以确定事务是否成功。

### spi_cmdread函数

spi_cmdread() 函数向 SPI 总线上的特定设备发送命令，然后从中读取数据。该函数的原型是:

```C
int spi_cmdread( int fd, uint32_t device, void *cbuf, int16_t clen, void *rbuf, int rlen );
/* 
* spi_cmdread.
* Parameters:
* (in)     fd          The file descriptor that the spi_open() function returned. spi_open() 函数返回的文件描述符。
* (in)     device      The device ID.   设备 ID。
                       最多具有以下标志之一的设备 ID（可选）：
                       SPI_DEV_LOCK
                       SPI_DEV_UNLOCK
* (in)     cbuf        A pointer to the command buffer.                           
* (in)     wbuf        A pointer to the send buffer.      指向要发送的数据缓冲区的指针；
* (out)    rbuf        A pointer to the receive buffer.   指向要接收的数据缓冲区的指针；
* (in)     len         Length in bytes of buf        发送的数据的长度(以字节为单位)
该函数返回它成功从设备读取的数据字节数。 如果发生错误，函数返回 -1 并设置 errno：
EIO
The read from the device failed, or a hardware error occurred.
EINVAL
The device ID is invalid, or you're trying to unlock a device that isn't locked.
ENOMEM
Insufficient memory.
EPERM
The device is locked by another connection.
*/
```

如果此函数返回的字节数与其要求函数读取的字节数不同，则 SPI 驱动程序通常会将其视为错误。

### spi_xchange函数

spi_dma_xchange() 函数使用 DMA 在 SPI 主设备和 SPI 设备之间交换数据。该函数的原型是:

```C
int spi_dma_xchange( int fd, uint32_t device, void *wbuf,  void *rbuf, int len );
/* 
* spi_dma_xchange.
* Parameters:
* (in)     fd          The file descriptor that the spi_open() function returned. spi_open() 函数返回的文件描述符。
* (in)     device      The device ID.   设备 ID。
                       最多具有以下标志之一的设备 ID（可选）：
                       SPI_DEV_LOCK
                       SPI_DEV_UNLOCK
* (in)     wbuf        A pointer to the send buffer, or NULL if there's no data to send.
* (out)    rbuf        A pointer to the receive buffer, or NULL if there's no data to receive.
* (in)     len         Length in bytes of buf        发送的数据的长度(以字节为单位)
该函数返回它成功从设备读取的数据字节数。 如果发生错误，函数返回 -1 并设置 errno：
EIO
The write to the device failed, or a hardware error occurred.
EINVAL
The device ID is invalid, or you're trying to unlock a device that isn't locked, or the buffer address is invalid.
ENOMEM
Insufficient memory.
EPERM
The device is locked by another connection.
ENOTSUP
DMA isn't supported.
*/
```

如果此函数返回的字节数与它要求该函数交换的字节数不同，则 SPI 驱动程序通常会将其视为错误。



## 资源管理器设计

在QNX下开发驱动程序，最主要的工作除了了解底层硬件具体工作流程外，就是建立一个能与操作系统兼容且支持POSIX的Resource manger框架了。在任何一段程序的执行过程中一段都是从main函数开始的，然而在操作系统中的main函数还传递了两个参数：int argc, char argv，这两个参数是用来传递从shell命令行或者buildfile中传来对Resource manger具体参数的，使用options (int argc, char argv);函数实现，所以这个函数在main函数中最开始的位置，可以开发的driver具有不同可选的特性，提供使用的便利性。

> 第一步：[书写options](#第一步：书写options) (int argc, char **argv);的具体实现；
>
> 第二步：[建立一个上下文切换句柄dpp = dispatch_create()](#第二步：建立一个上下文切换句柄);这个东东主要用在mainloop中产生一个block特性，可以让我们等待接受消息；
>
> 第三步：[iofunc初始化]()。这一步是将自己实现的函数与POSIX层函数进行接口，解析从read、write、devctl等函数传来的消息进行解析，以实现底层与应用层函数之间的交互，通过io_funcs.read = io_read，io_funcs.write = io_write，进行函数重载；
>
> 第四步：注册设备名，使设备在命名空间中产生相应的名称，这一点是整个过程的关键了，形如  pathID = resmgr_attach (dpp, &rattr, "/dev/Null",_FTYPE_ANY, 0, &connect_funcs, &io_funcs, &ioattr)，这样不仅注册了一个设备名，还让系统知道了我们实习的IO函数对应关系；
>
> 第五步：为之前创建的上下文句柄分配空间，例如ctp = dispatch_context_alloc (dpp);为了第六步使用；
>
> 第六步：通过不断循环等待dispatch_block()来调用MsgReceive()使Resource manger处于receive block状态，以接收上层发送来的消息，通过dispatch_handler (ctp)去调用我们自己定义的IO函数。



下面我拿SPI启动脚本和源码来分析一下：

QNX系统启动后，执行一系列脚本命令，然后加载SPI驱动。

```shell
spi-master -u0 -d omap4430 base=0x4809A000,bitrate=125000,clock=48000000,irq=66,force=1,channel=2,sdma=1
```

当QNX执行该脚本时，会自动到指定目录搜索是否存在spi-master驱动，然后后面一串是参数设置。这一串参数就像[Linux](https://so.csdn.net/so/search?from=pc_blog_highlight&q=Linux)设备树一样，指定相关的硬件参数。具体参数意义在驱动里在详细解释。

```C
Syntax:
    spi-master -d omap4430 [option[,option ...]] ... &

Options (to override autodetected defaults):

    "base"          /* Base address for this CSPI controller */
    "bitrate"       /* defines teh bitrate to to assigned to the spi */
    "clock"         /* defined the value of the clock source of the SPI */
    "channel"       /* defines the connected channel number (1,2,3, or 4) */
    "irq"           /* IRQ for this CSPI intereface */
    "force"         /* Force the default CSx level */
    "num_cs"        /* number of support devices on this channel (default=1)*/
    "sdma"          /* Disable/enable SDMA for SPI, 1:enable SDMA, 0:disable SDMA (default=0)*/
    "cs_delay"      /* Select clock cycles 0:0.5, 1:1.5, 2:2.5, 3:3.5 (default=1) */
    "somi"          /* determine which pin to use for somi, 0:use D0, 1:use D1 (default=0) */
    "clk_activity"  /* Clocks activity during wake-up mode period, can be 0-3 (default=0) */
    "pwr"           /* Power management setting, can be 0-2 (default=1) */
    "spidatdir"     /* Enable setting of SPIDATDIRx fields */

Notes:
    The OMAP4430 SPI controller can manage up to four devices; each can be
    configured independently using the spi_setcfg() function found in the
    spi-master library.
```

### 第一步：书写options

QNX执行spi-master后，应该执行函数入口在哪里？其实QNX早就为我们分配好了，它把所有的驱动当做应用程序，一个应用程序一样，都有一个main入口。因此分析QNX SPI驱动，可以从_spi_main.c里的这个函数开始。

```C
int main(int argc, char *argv[])
{
        spi_dev_t       *head = NULL, *tail = NULL, *dev;
        void            *drventry, *dlhdl;
        siginfo_t       info;
        sigset_t        set;
        int                     i, c, devnum = 0;

        UserParm = NULL;

        /* default permission for /dev/spi* entry */
        devperm = 0666;
        /*使能超级锁定进程的内存和请求I/O特权,让线程在具有适当特权的架构上执行in、in、out、out、cli和sti I/O操作码，
          并让它附加IRQ处理程序。*/
        if (ThreadCtl(_NTO_TCTL_IO, 0) == -1) {
                perror("ThreadCtl");
                return (!EOK);
        }
        // 通过 iofunc_func_init()函数初始化，通过连接和POSIX默认IO结构层功能。
        _spi_init_iofunc();

        while ((c = getopt(argc, argv, "u:U:P:d:")) != -1) {
                switch (c) {
                        case 'u':
                                devnum = strtol(optarg, NULL, 0);
                                break;
                        case 'U':
                                UserParm = strdup(optarg);
                                break;
                        case 'P':
                                if ( optarg ) {
                                        devperm = strtoul(optarg, NULL, 8);     // octal
                                }
                                break;
                        case 'd':
                                if ((drventry = _spi_dlload(&dlhdl, optarg)) == NULL) {
                                        perror("spi_load_driver() failed");
                                        return (-1);
                                }

                                do {
                                        if ((dev = calloc(1, sizeof(spi_dev_t))) == NULL)
                                                goto cleanup;

                                        if (argv[optind] == NULL || *argv[optind] == '-')
                                                dev->opts = NULL;
                                        else
                                                dev->opts = strdup(argv[optind]);
                                        ++optind;
                                        dev->funcs  = (spi_funcs_t *)drventry;
                                        dev->devnum = devnum++;
                                        dev->dlhdl  = dlhdl;

                                        i = _spi_create_instance(dev);

                                        if (i != EOK) {
                                                perror("spi_create_instance() failed");

                                                if (dev->opts)
                                                        free(dev->opts);
                                                free(dev);
                                                goto cleanup;
                                        }
                                        if (head) {
                                                tail->next = dev;
                                                tail = dev;
                                        }
                                        else
                                                head = tail = dev;
                                } while (optind < argc && *(optarg = argv[optind]) != '-');
                                /*
                                 * Now we only support one dll
                                 */
                                goto start_spi;
                                break;
                }
        }
start_spi:
        if (head) {
                /* background the process */
                procmgr_daemon(0, PROCMGR_DAEMON_NOCLOSE | PROCMGR_DAEMON_NODEVNULL);
                sigemptyset(&set);
                sigaddset(&set, SIGTERM);
                for (;;) {
                        if (SignalWaitinfo(&set, &info) == -1)
                                continue;
                        if (info.si_signo == SIGTERM)
                                break;
                }
        }
cleanup:
        dev=head;
        while (dev) {
                if (dev->ctp) {
                        dispatch_unblock(dev->ctp);
                }
                if (dev->drvhdl) {
                        resmgr_detach(dev->dpp, dev->id, _RESMGR_DETACH_ALL);
                        dev->funcs->fini(dev->drvhdl);
                }
                if (dev->dpp) {
                        dispatch_destroy(dev->dpp);
                }
                head = dev->next;
                if (dev->opts)
                        free(dev->opts);
                free(dev);
                dev=head;
        }
        dlclose(dlhdl);
        return (EOK);
}
```

进入main后，执行ThreadCtl(_NTO_TCTL_IO, 0)函数，该函数使能超级锁定进程的内存和请求I/O特权;让线程在具有适当特权的架构上执行in、in、out、out、cli和sti I/O操作码，并让它附加IRQ处理程序。很多操作都需要进行寄存器操作，需要采用out32 in32接口等。
 调用 _spi_init_iofunc(); 初始化连接函数，通过 iofunc_func_init()函数初始化，通过连接和POSIX默认IO结构层功能。有关默认函数的信息。

```C
int _spi_init_iofunc(void)
{
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &_spi_connect_funcs, _RESMGR_IO_NFUNCS, &_spi_io_funcs);
    _spi_io_funcs.read      = _spi_read;
    _spi_io_funcs.write     = _spi_write;
    _spi_io_funcs.devctl    = _spi_devctl;
    _spi_io_funcs.close_ocb = _spi_close_ocb;
    _spi_io_funcs.msg       = _spi_iomsg;

    return EOK;
}
```

完成后，开始解析参数命令，也就是前面提到的spi-master后的参数`-u0 -d omap4430 base=0x4809A000,bitrate=125000,clock=48000000,irq=66,force=1,channel=2,sdma=1`，其中u表示设备号，定义为spi0，-d表示加载的驱动名称链接库，调用 _spi_dlload(&dlhdl, optarg)函数，加载动态库，而寄存器基地址和中断号，不在这个参数里设置，后面在叙述。

```C
void *_spi_dlload(void **hdl, const char *optarg)
{
        char            dllpath[_POSIX_PATH_MAX + 1];
        void            *dlhdl, *entry;

        if (strchr(optarg, '/') != NULL)
                strcpy(dllpath, optarg);
        else
                sprintf(dllpath, "spi-%s.so", optarg);

        dlhdl = dlopen(dllpath, 0);
        // 获取动态链接库spi-omap4430 里的spi_drv_entry函数符号表
        if (dlhdl != NULL) {
                entry = dlsym(dlhdl, "spi_drv_entry");

                if (entry != NULL) {
                        *hdl = dlhdl;
                        return entry;
                }

                dlclose(dlhdl);
        }

        return NULL;
}
```

_spi_dlload调用[dlsym](http://www.qnx.com/developers/docs/7.1/com.qnx.doc.neutrino.lib_ref/topic/d/dlsym.html)函数，找到动态链接库内的匹配函数符号表名称spi_drv_entry。这个函数作为SPI底层驱动入口加载。在主函数参数命令里，调用      dev->funcs  = (spi_funcs_t *)drventry;和  i = _spi_create_instance(dev);实例化驱动。最终完成系列的初始化过程，进入循环。



### 第二步：建立一个上下文切换句柄

主要从_spi_create_instance(dev)函数开始，_spi_create_instance初始化线程参数，创建一个线程任务，这个线程函数为_spi_driver_thread。

```C
int _spi_create_instance(spi_dev_t *dev)
{
        pthread_attr_t          pattr;
        struct sched_param      param;

        if (NULL == (dev->dpp = dispatch_create())) {
                perror("dispatch_create() failed");
                goto failed0;
        }

        pthread_attr_init(&pattr);
        pthread_attr_setschedpolicy(&pattr, SCHED_RR);
        param.sched_priority = 21;
        pthread_attr_setschedparam(&pattr, &param);
        pthread_attr_setinheritsched(&pattr, PTHREAD_EXPLICIT_SCHED);

        // Create thread for this interface
        if (pthread_create(NULL, &pattr, (void *)_spi_driver_thread, dev) != EOK) {
                perror("pthread_create() failed");
                goto failed1;
        }

        return (EOK);

failed1:
        dispatch_destroy(dev->dpp);
failed0:

        return (-1);
}
```

_spi_driver_thread调用_spi_register_interface后进入主线程任务。

```C
static void* _spi_driver_thread(void *data)
{
	spi_dev_t	*dev = data;

	if (_spi_register_interface(data) != EOK)
		return NULL;

	while (1) {
		if ((dev->ctp = dispatch_block(dev->ctp)) != NULL)
			dispatch_handler(dev->ctp);
		else
			break;
	}

	return NULL;
}
```

### 第三，四，五步: iofunc初始化, 注册设备名与创建的上下文句柄分配空间

其中_spi_register_interface在dev目录下创建一个设备节点和初始化SPI驱动。

```C
spi_funcs_t spi_drv_entry = {
	sizeof(spi_funcs_t),
	omap4430_init,		/* init() */
	omap4430_dinit,		/* fini() */
	omap4430_drvinfo,	/* drvinfo() */
	omap4430_devinfo,	/* devinfo() */
	omap4430_setcfg,	/* setcfg() */
	omap4430_xfer,		/* xfer() */
	omap4430_dmaxfer	/* dma_xfer() */
};
```

```C
static int _spi_register_interface(void *data)
{
        spi_dev_t               *dev = data;
        SPIDEV                  *drvhdl;
        resmgr_attr_t   rattr;
        char                    devname[PATH_MAX + 1];

        if ((drvhdl = dev->funcs->init(dev, dev->opts)) == NULL) {
                free(dev->opts);
                dev->opts = NULL;
                return (!EOK);
        }

        dev->drvhdl = drvhdl;

        /* set up i/o handler functions */
        /* 资源管理器本身的一些参数，下面这个就是指定了资源管理器最多一次可以处理SPI_RESMGR_NPARTS_MIN个 iov_t */
        memset(&rattr, 0, sizeof(rattr));
        rattr.nparts_max   = SPI_RESMGR_NPARTS_MIN;
        rattr.msg_max_size = SPI_RESMGR_MSGSIZE_MIN;
        /* io_attr 其实可以想像成一个文件相关的参数，比如读写权限等等 */
        iofunc_attr_init(&drvhdl->attr, S_IFCHR | devperm, NULL, NULL);
        drvhdl->attr.mount = &_spi_mount;

        /* register device name */
        /* 建立起资源管理层，同时注册路径 */
        snprintf(devname, PATH_MAX, "/dev/spi%d", dev->devnum);
        if (-1 == (dev->id = resmgr_attach(dev->dpp, &rattr, devname, _FTYPE_ANY, 0,
                                        &_spi_connect_funcs, &_spi_io_funcs, (void *)drvhdl))) {
                perror("resmgr_attach() failed");
                goto failed1;
        }

        resmgr_devino(dev->id, &drvhdl->attr.mount->dev, &drvhdl->attr.inode);
        // /* 准备一个资源管理层的 context 以备使用 */
        if ((dev->ctp = dispatch_context_alloc(dev->dpp)) != NULL)
                return (EOK);

        perror("dispatch_context_alloc() failed");

        resmgr_detach(dev->dpp, dev->id, _RESMGR_DETACH_ALL);
failed1:
        dev->funcs->fini(drvhdl);

        return (!EOK);
}
```

**iofunc初始化**

调用 dev->funcs->init(dev, dev->opts)，这个函数指针在spi_drv_entry结构体中，在主函数里通过链接库完成指针赋值，spi_drv_entry里实现真正的SPI操作。包括初始化、配置、设备信息获取以及数据传输等等操作。

```C
if ((drvhdl = dev->funcs->init(dev, dev->opts)) == NULL) {
    free(dev->opts);
    dev->opts = NULL;
    return (!EOK);
}

dev->drvhdl = drvhdl;

/* set up i/o handler functions */
/* 资源管理器本身的一些参数，下面这个就是指定了资源管理器最多一次可以处理SPI_RESMGR_NPARTS_MIN个 iov_t */
memset(&rattr, 0, sizeof(rattr));
rattr.nparts_max   = SPI_RESMGR_NPARTS_MIN;
rattr.msg_max_size = SPI_RESMGR_MSGSIZE_MIN;
/* io_attr 其实可以想像成一个文件相关的参数，比如读写权限等等 */
iofunc_attr_init(&drvhdl->attr, S_IFCHR | devperm, NULL, NULL);
```

**注册设备名**

通过resmgr_attach，注册一个资源设备名为“/dev/spi0”。

```C
/* register device name */
/* 建立起资源管理层，同时注册路径 */
snprintf(devname, PATH_MAX, "/dev/spi%d", dev->devnum);
if (-1 == (dev->id = resmgr_attach(dev->dpp, &rattr, devname, _FTYPE_ANY, 0,
                                   &_spi_connect_funcs, &_spi_io_funcs, (void *)drvhdl))) {
    perror("resmgr_attach() failed");
    goto failed1;
}
```

**为之前创建的上下文句柄分配空间**

```C
// /* 准备一个资源管理层的 context 以备使用 */
if ((dev->ctp = dispatch_context_alloc(dev->dpp)) != NULL)
    return (EOK);
```

### 第六步：通过不断循环等待dispatch_block()与dispatch_handler (ctp)执行IO函数处理。

我们基本可以认为SPI驱动是用iofunc层+resmgr层已经可以构建一个完整的资源管理器。该资源管理器通过dispatch处理其他形态的信息。

![dispatch_layer](.\pic\dispatch_layer.png)

在使用 dispatch时，进行特殊的 *_attach() 挂接以后，只要把resmgr层的几个函数替换成dispath层的几个函数 就可以了，比如这样：

```text
ctp = dispatch_context_alloc(dispatch);
while (1)
{
    ctp = dispatch_block(ctp);
    dispatch_handler(ctp);
}
```

dispath_block() 相当于阻塞并等待，而 dispatch_handle() 则根据不同的挂接，调用不同的回调函数进行处理。其实在_spi_register_interface里进行了dispatch_context_alloc的操作。通过不断循环等待dispatch_block()来调用MsgReceive()使Resource manger处于receive block状态，以接收上层发送来的消息，通过dispatch_handler (ctp)去调用我们自己定义的IO函数。



### main主进程后台处理

到main函数最后将进程spi-master进程放在后台处理。

```C
start_spi:
        if (head) {
                /* background the process */
                /* 调用procmgr_daemon函数，置PROCMGR_DAEMON_NOCLOSE | PROCMGR_DAEMON_NODEVNULL标志，把该进程运行于后台。 
                */
                procmgr_daemon(0, PROCMGR_DAEMON_NOCLOSE | PROCMGR_DAEMON_NODEVNULL);
                /* 初始化一个不包含任何信号的集合 */
                sigemptyset(&set);
                /* 设置SIGTERM信号。是一个程序结束(terminate)信号, 与SIGKILL不同的是该信号可以被阻塞和 
                   处理. 通常用来要求程序自己正常退出. shell命令kill缺省产生这个信号。 */
                sigaddset(&set, SIGTERM);
                for (;;) {
                        /* 内核调用从set指定的集合中选择挂起信号。如果在调用时集合中没有挂起信号，线程将阻塞，
                           直到集合中的一个或多个信号成为挂起信号，或者直到被未阻塞的捕获信号中断。在这里主要
                           是捕获SIGTERM信号，当收到该信号，退出该驱动。 */
                        if (SignalWaitinfo(&set, &info) == -1)
                                continue;
                        if (info.si_signo == SIGTERM)
                                break;
                }
        }
```



### 参考文献：

[QNX驱动开发——Resource manger framework](https://blog.csdn.net/DanielLee_ustb/article/details/7842140?spm=1001.2101.3001.6650.14&utm_medium=distribute.pc_relevant.none-task-blog-2~default~CTRLIST~default-14.highlightwordscore&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2~default~CTRLIST~default-14.highlightwordscore)  

[QNX资源管理器——知乎](https://zhuanlan.zhihu.com/p/151033583)  

[QNX---SPI驱动分析](https://blog.csdn.net/u011996698/article/details/79536679)

[Technical Notes SPI (Serial Peripheral Interface) Framework (qnx.com)](http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_technotes%2Fspi_overview.html)

[Writing a Resource Manager](http://www.qnx.com/developers/docs/6.3.2/neutrino/prog/resmgr.html)

