# QNX®  Neutrino I2C驱动框架与代码分析

本文主要描述QNX I2C Drvier的相关内容，并以Texas Instruments DRA71x Jacinto6 Cortex A15处理器为例讲解

 I2C 是经常用到的一种总线协议，它只占用两个IO口资源，分别是SCL时钟信号线与SDA数据线，两根线就能将连接与总线上的设备实现数据通信，由于它的简便的构造设计，于是成为一种较为常用的通信方式。在QNX系统里，也提供了I2C驱动框架。



## 目录结构与组成部分

下面是Texas.Instruments.DRA71x.Jacinto6.Entry.BSP.for.QNX.SDP.6.6包目录，我们只展开跟I2C相关目录内容：

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
│   │   │       └── devu-omap5-xhci.so
│   │   └── usr
│   │       └── lib
│   │           ├── libfs-flash3.a
│   │           ├── libi2c-master.a
│   │           ├── libio-char.a
│   │           ├── libspi-master.a
│   │           ├── libspi-masterS.a
│   │           └── libutil.a
│   └── usr
│       └── include
│           ├── avb.h
│           ├── cam.h
│           ├── fs
│           ├── hw
│           │   └── i2c.h
│           ├── module.h
│           ├── netdrvr
│           └── xpt.h
├── readme.txt
├── source.xml
└── src
    ├── Makefile
    ├── hardware
    │   ├── Makefile
    │   ├── deva
    │   ├── devb
    │   ├── devc
    │   ├── devi
    │   ├── devnp
    │   ├── etfs
    │   ├── flash
    │   ├── i2c
    │   │   ├── Makefile
    │   │   ├── common.mk
    │   │   └── omap35xx
    │   │       ├── Makefile
    │   │       ├── Usemsg
    │   │       ├── Usemsg-omap4
    │   │       ├── arm
    │   │       ├── bus_recover.c
    │   │       ├── bus_speed.c
    │   │       ├── clock_toggle.c
    │   │       ├── common.mk
    │   │       ├── context_restore.c
    │   │       ├── context_restore.h
    │   │       ├── fini.c
    │   │       ├── info.c
    │   │       ├── init.c
    │   │       ├── lib.c
    │   │       ├── module.tmpl
    │   │       ├── offsets.h
    │   │       ├── options.c
    │   │       ├── project.xml
    │   │       ├── proto.h
    │   │       ├── recv.c
    │   │       ├── reg_map_init.c
    │   │       ├── reset.c
    │   │       ├── send.c
    │   │       ├── slave_addr.c
    │   │       ├── version.c
    │   │       └── wait.c
    │   ├── ipl
    │   ├── mtouch
    │   ├── spi
    │   ├── startup
    │   └── support
    └── utils
        ├── Makefile
        └── r

```

I2C框架由以下部分组成:

hardware/i2c/*  硬件接口

```shell
├── hardware
│   ├── i2c
│   │   ├── Makefile
│   │   ├── common.mk
│   │   └── omap35xx
│   │       ├── Makefile
│   │       ├── Usemsg
│   │       ├── Usemsg-omap4
│   │       ├── arm
│   │       ├── bus_recover.c
│   │       ├── bus_speed.c
│   │       ├── clock_toggle.c
│   │       ├── common.mk
│   │       ├── context_restore.c
│   │       ├── context_restore.h
│   │       ├── fini.c
│   │       ├── info.c
│   │       ├── init.c
│   │       ├── lib.c
│   │       ├── module.tmpl
│   │       ├── offsets.h
│   │       ├── options.c
│   │       ├── project.xml
│   │       ├── proto.h
│   │       ├── recv.c
│   │       ├── reg_map_init.c
│   │       ├── reset.c
│   │       ├── send.c
│   │       ├── slave_addr.c
│   │       ├── version.c
│   │       └── wait.c
```

lib/i2c  资源管理器层

```shell
├── prebuilt
│   ├── armle-v7
│   │   ├── lib
│   │   │   └── dll
│   │   │       └── devu-omap5-xhci.so
│   │   └── usr
│   │       └── lib
│   │           ├── libfs-flash3.a
│   │           ├── libi2c-master.a
│   │           ├── libio-char.a
│   │           ├── libspi-master.a
│   │           ├── libspi-masterS.a
│   │           └── libutil.a
```

<hw/i2c.h> 定义硬件和应用程序接口的公共头文件

```shell
├── prebuilt
│   ├── armle-v7
│   │   ├── lib
│   │   │   └── dll
│   │   └── usr
│   │       └── lib
│   └── usr
│       └── include
│           ├── hw
│           │   └── i2c.h
```

2C总线最常见的应用是对从设备寄存器的低带宽、低速率访问，例如:编写音频编解码器、编程一个RTC程序、读取温度传感器数据等等。 通常，总线上只交换几个字节。可以将I2C主机实现为单线程资源管理器或专用应用程序。资源管理器接口的主要优点是:

>1、它为应用程序开发人员提供了一个清晰、易于理解的思路。
>
>2、它作为一个中介，在多个应用程序对一个或多个从设备之间进行访问，强制不同I2C接口之间的一致性。
>
>3、对于专用的i2c总线应用程序，硬件访问库更有效；硬件接口定义了这个库的接口，有助于维护和代码可移植性。

QNX I2C驱动提供了基本的硬件操作接口，其接口名称为i2c_master_funcs_t，这些接口是驱动里要求实现，包括读写、设置地址、总线速度、版本信息等等。每个接口对应于上面的一个文件.c，这样便于模块化。



## 硬件管理接口

```C
typedef struct {
    size_t size;    /* size of this structure */
    int (*version_info)(i2c_libversion_t *version);
    void *(*init)(int argc, char *argv[]);
    void (*fini)(void *hdl);
    i2c_status_t (*send)(void *hdl, void *buf, unsigned int len, 
                         unsigned int stop);
    i2c_status_t (*recv)(void *hdl, void *buf, unsigned int len, 
                         unsigned int stop);
    int (*abort)(void *hdl, int rcvid);
    int (*set_slave_addr)(void *hdl, unsigned int addr, i2c_addrfmt_t fmt);
    int (*set_bus_speed)(void *hdl, unsigned int speed, unsigned int *ospeed);
    int (*driver_info)(void *hdl, i2c_driver_info_t *info);
    int (*ctl)(void *hdl, int cmd, void *msg, int msglen, 
               int *nbytes, int *info);
    int (*bus_reset)(void *hdl);
} i2c_master_funcs_t;
```

在hardware/i2c/目录中，能找到对应函数的实现。

```shell
├── i2c
│   ├── Makefile
│   ├── common.mk
│   └── omap35xx
│       ├── Makefile
│       ├── Usemsg
│       ├── Usemsg-omap4
│       ├── arm
│       ├── bus_recover.c
│       ├── bus_speed.c       # int (*set_bus_speed)(void *hdl, unsigned int speed, unsigned int *ospeed);
│       ├── clock_toggle.c
│       ├── common.mk
│       ├── context_restore.c
│       ├── context_restore.h
│       ├── fini.c             # void (*fini)(void *hdl);
│       ├── info.c             # int (*driver_info)(void *hdl, i2c_driver_info_t *info);
│       ├── init.c             # void *(*init)(int argc, char *argv[]);
│       ├── lib.c
│       ├── module.tmpl
│       ├── offsets.h
│       ├── options.c
│       ├── project.xml
│       ├── proto.h
│       ├── recv.c             # i2c_status_t (*recv)(void *hdl, void *buf, unsigned int len,  unsigned int stop);
│       ├── reg_map_init.c
│       ├── reset.c            # int (*bus_reset)(void *hdl);
│       ├── send.c             # i2c_status_t (*send)(void *hdl, void *buf, unsigned int len, unsigned int stop);
│       ├── slave_addr.c       # int (*set_slave_addr)(void *hdl, unsigned int addr, i2c_addrfmt_t fmt);
│       ├── version.c          # int (*version_info)(i2c_libversion_t *version);
│       └── wait.c
```



## 硬件访问接口

这是整个硬件接口函数，提供了10个接口函数，分别对10个接口函数进行介绍：

### version_info函数

version_info函数来获取关于库版本的信息。这个函数的原型是：

```C
int (*version_info)(i2c_libversion_t *version);
```

version参数是指向这个函数必须填充的i2c_libversion_t结构的指针。该结构定义如下:

```C
typedef struct {
    unsigned char   major;
    unsigned char   minor;
    unsigned char   revision;
} i2c_libversion_t;
```

具体驱动实现：

```C
int omap_version_info(i2c_libversion_t *version)
{
    version->major = I2CLIB_VERSION_MAJOR;
    version->minor = I2CLIB_VERSION_MINOR;
    version->revision = I2CLIB_REVISION;
    return 0;
}
```

### init函数

init函数初始化主接口。该函数的原型是:

```C
void *(*init)(int argc, char *argv[]);
```

参数是在命令行上传递的。函数返回传递给所有其他函数的句柄，如果发生错误则返回NULL。这个函数主要根据命令行解析来进行I2C初始化。

具体驱动实现：

```C
void *omap_init(int argc, char *argv[])
{
    omap_dev_t      *dev;
        uint16_t                s;

    if (-1 == ThreadCtl(_NTO_TCTL_IO, 0)) {
        perror("ThreadCtl");
        return NULL;
    }

    dev = malloc(sizeof(omap_dev_t));
    if (!dev)
        return NULL;
        dev->speed = 100000;
    //根据命令 配置I2C相关参数包括基地址 中断号
    if (-1 == omap_options(dev, argc, argv)) {
                fprintf(stderr, "omap_options: parse I2C option failed\n");
        goto fail_free_dev;
    }
    //I2C 物理地址映射
    dev->regbase = mmap_device_io(dev->reglen, dev->physbase);
    if (dev->regbase == (uintptr_t)MAP_FAILED) {
        perror("mmap_device_io");
        goto fail_free_dev;
    }

    /* Initialize interrupt handler */
    if ((dev->chid = ChannelCreate(_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1) {
        perror("ChannelCreate");
        goto fail_unmap_io;
    }

    if ((dev->coid = ConnectAttach(0, 0, dev->chid, _NTO_SIDE_CHANNEL, 0)) == -1) {
        perror("ConnectAttach");
        goto fail_chnl_dstr;
    }

        dev->intrevent.sigev_notify   = SIGEV_PULSE;
        dev->intrevent.sigev_coid     = dev->coid;
        dev->intrevent.sigev_code     = OMAP_I2C_EVENT;
        dev->intrevent.sigev_priority = dev->intr_priority;

        /*
         * Attach interrupt
         */
        dev->iid = InterruptAttach(dev->intr, i2c_intr, dev, 0, _NTO_INTR_FLAGS_TRK_MSK);
    if (dev->iid == -1) {
        perror("InterruptAttachEvent");
        goto fail_con_dtch;
    }

        if (omap_reg_map_init(dev) == -1) {
                goto fail_intr_dtch;
        }

        if (context_restore_init(dev) == -1) {
                goto fail_ctxt_rest;
        }

        if (omap_clock_toggle_init(dev) == -1) {
                goto fail_ctxt_rest;
        }
        // I2C 系统时钟初始化
        /* Set up the fifo size - Get total size */
        omap_clock_enable(dev);
        s = (in16(dev->regbase + OMAP_I2C_BUFSTAT) >> 14) & 0x3;
        omap_clock_disable(dev);
        dev->fifo_size = 0x8 << s;
        /* Set up notification threshold as half the total available size. */
        dev->fifo_size >>=1;

        if (omap_i2c_reset(dev) == -1) {
                fprintf(stderr, "omap_i2c_reset: reset I2C interface failed\n");
                goto fail_ctxt_rest;
        }

    return dev;
//失败退出处理
fail_ctxt_rest:
    context_restore_fini(dev);
fail_intr_dtch:
    InterruptDetach(dev->iid);
fail_con_dtch:
        ConnectDetach(dev->coid);
fail_chnl_dstr:
        ChannelDestroy(dev->chid);
fail_unmap_io:
    munmap_device_io(dev->regbase, dev->reglen);
fail_free_dev:
    free(dev);
    return NULL;
}
```

### fini函数

fini函数清理驱动程序并释放与给定句柄关联的所有内存。该函数的原型是:

```c
void (*fini)(void *hdl);
```

具体源码实现：

```C
void omap_fini(void *hdl)
{
    omap_dev_t  *dev = hdl;

    omap_clock_enable(dev);
    out16(dev->regbase + OMAP_I2C_CON, 0);
    out16(dev->regbase + OMAP_I2C_IE, 0);
    omap_clock_disable(dev);
    InterruptDetach(dev->iid); // 释放中断映射
    ConnectDetach(dev->coid);  // 断开消息传递(Message-passing)Channel连接
    ChannelDestroy(dev->chid); // 释放消息传递(Message-passing)Channel
    // 时钟控制物理地址映射释放
    if (dev->clkctrl_base) {
        munmap_device_io (dev->clkctrl_base, 4);
    }

    if (dev->clkstctrl_base) {
        munmap_device_io (dev->clkstctrl_base, 4);
    }

    context_restore_fini(dev);
    // 整个I2C控制器物理地址映射释放
    munmap_device_io (dev->regbase, dev->reglen);
    free (hdl);
}
```



### send函数

发送函数启动主发送。通信完成时将发送一个可选事件(可以释放数据缓冲区)。如果此函数失败，则未启动任何通信。

该函数的原型是:

```C
i2c_status_t (*send)(void *hdl, void *buf, unsigned int len, unsigned int stop);
/*
* Master send.
* Parameters:
* (in)     hdl         Handle returned from init()  init函数返回的句柄；
* (in)     buf         Buffer of data to send       指向要发送的数据缓冲区的指针；
* (in)     len         Length in bytes of buf       要发送的数据的长度(以字节为单位)；
* (in)     stop        If !0, set stop condition when send completes   要发送的数据的长度(以字节为单位)；
* Returns:
* bitmask of status bits
返回状态为：
I2C_STATUS_DONE：传输完成，并且没有错误。
I2C_STATUS_ERROR：传输错误
I2C_STATUS_NACK：没有ACK
I2C_STATUS_ARBL：失去了仲裁。
I2C_STATUS_BUSY：传输超时
I2C_STATUS_ABORT：传输终止
*/
```

具体源码实现：

```C
i2c_status_t omap_send(void *hdl, void *buf, unsigned int len, unsigned int stop)
{
    omap_dev_t      *dev = hdl;
    i2c_status_t    ret = I2C_STATUS_ERROR;
    int num_bytes;

    if (len <= 0)
        return I2C_STATUS_DONE;

    if (-1 == omap_wait_bus_not_busy(dev, stop))
        return I2C_STATUS_BUSY;

    omap_clock_enable(dev);

        dev->xlen = len;
        dev->buf = buf;
        dev->status = 0;
        dev->intexpected = 1;

    /* set slave address */
    if (dev->slave_addr_fmt == I2C_ADDRFMT_7BIT)
        out16(dev->regbase + OMAP_I2C_CON, in16(dev->regbase + OMAP_I2C_CON) & (~OMAP_I2C_CON_XSA));
    else
        out16(dev->regbase + OMAP_I2C_CON, in16(dev->regbase + OMAP_I2C_CON) | OMAP_I2C_CON_XSA);

    out16(dev->regbase + OMAP_I2C_SA, dev->slave_addr);

    /* set data count */
    out16(dev->regbase + OMAP_I2C_CNT, len);

    /* Clear the FIFO Buffers */
	out16(dev->regbase + OMAP_I2C_BUF, in16(dev->regbase + OMAP_I2C_BUF)| OMAP_I2C_BUF_RXFIF_CLR | OMAP_I2C_BUF_TXFIF_CLR);

    /* pre-fill the fifo with outgoing data */
    if (dev->xlen > dev->fifo_size)
      num_bytes = dev->fifo_size;
    else
      num_bytes = dev->xlen;
    while (num_bytes)
    {
      out8(dev->regbase + OMAP_I2C_DATA, *dev->buf++);
      dev->xlen--;
      num_bytes--;
    }

    /* set start condition */
    out16(dev->regbase + OMAP_I2C_CON,
            OMAP_I2C_CON_EN  |
            OMAP_I2C_CON_MST |
            OMAP_I2C_CON_TRX |
            OMAP_I2C_CON_STT |
            (stop? OMAP_I2C_CON_STP : 0)|
            (in16(dev->regbase + OMAP_I2C_CON)&OMAP_I2C_CON_XA));

        ret=  omap_wait_status(dev);
        omap_clock_disable(dev);
    return ret;
}
```



### recv函数

recv函数启动主接收。传输完成时将发送一个可选事件(可以使用数据缓冲区)。如果此函数失败，则未启动任何传输。

该函数的原型是:

```C
i2c_status_t (*recv)(void *hdl, void *buf, unsigned int len, unsigned int stop);
/*
* Master receive.
* Parameters:
* (in)     hdl         Handle returned from init()   init函数返回的句柄；
* (in)     buf         Buffer for received data      指向要接收的数据缓冲区的指针；
* (in)     len         Length in bytes of buf        要接收的数据的长度(以字节为单位)；
* (in)     stop        If !0, set stop condition when recv completes    如果这是非零的，函数将在发送完成时设置停止条件；
* Returns:
* bitmask of status bits
返回状态为：
I2C_STATUS_DONE：传输完成，并且没有错误。
I2C_STATUS_ERROR：传输错误
I2C_STATUS_NACK：没有ACK
I2C_STATUS_ARBL：失去了仲裁。
I2C_STATUS_BUSY：传输超时
I2C_STATUS_ABORT：传输终止
*/
```

具体源码实现：

```C
i2c_status_t omap_recv(void *hdl, void *buf, unsigned int len, unsigned int stop)
{
    omap_dev_t      *dev = hdl;
    i2c_status_t    ret;

    if (len <= 0)
        return I2C_STATUS_DONE;

    if (-1 == omap_wait_bus_not_busy(dev, stop))
        return I2C_STATUS_BUSY;

        dev->xlen = len;
        dev->buf = buf;
        dev->status = 0;
        dev->intexpected = 1;

        omap_clock_enable(dev);

    /* set slave address */
    if (dev->slave_addr_fmt == I2C_ADDRFMT_7BIT)
        out16(dev->regbase + OMAP_I2C_CON, in16(dev->regbase + OMAP_I2C_CON) & (~OMAP_I2C_CON_XSA));
    else
        out16(dev->regbase + OMAP_I2C_CON, in16(dev->regbase + OMAP_I2C_CON) | OMAP_I2C_CON_XSA);

    out16(dev->regbase + OMAP_I2C_SA, dev->slave_addr);

    /* set data count */
    out16(dev->regbase + OMAP_I2C_CNT, len);

	/* Clear the FIFO Buffers */
	out16(dev->regbase + OMAP_I2C_BUF, in16(dev->regbase + OMAP_I2C_BUF)| OMAP_I2C_BUF_RXFIF_CLR | OMAP_I2C_BUF_TXFIF_CLR);

    /* set start condition */
    out16(dev->regbase + OMAP_I2C_CON,
            OMAP_I2C_CON_EN  |
            OMAP_I2C_CON_MST |
            OMAP_I2C_CON_STT |
            (stop? OMAP_I2C_CON_STP : 0) |
            (in16(dev->regbase + OMAP_I2C_CON)&OMAP_I2C_CON_XA));

    ret=  omap_wait_status(dev);

	omap_clock_disable(dev);

    return ret;
}
```



### abort函数

中止功能迫使主程序释放总线。当停止条件被发送时，它返回。

该函数的原型是:

```C
int (*abort)(void *hdl, int rcvid);
/*
* Force the master to free the bus.
* Returns when the stop condition has been sent.
* Returns:
* 0    success
* -1   failure
*/
```

此处I2C驱动不需要进行abort操作。所以abort函数在resmgr层注册的时候为NULL。



### set_slave_addr函数

set_slave_addr函数指定目标从地址。该函数的原型是:

```C
int (*set_slave_addr)(void *hdl, unsigned int addr, i2c_addrfmt_t fmt);
/*
hdl:  init函数返回的句柄。
addr: 目标从地址。
fmt:地址的格式; I2C_ADDRFMT_7BIT 和I2C_ADDRFMT_10BIT
*/
```

具体源码实现：

```C
int omap_set_slave_addr(void *hdl, unsigned int addr, i2c_addrfmt_t fmt)
{
    omap_dev_t      *dev = hdl;

    if(fmt != I2C_ADDRFMT_7BIT && fmt != I2C_ADDRFMT_10BIT)
        return -1;

    dev->slave_addr = addr;
    dev->slave_addr_fmt = fmt;
    return 0;
}
```



### set_bus_speed函数

set_bus_speed函数指定总线速度。如果请求的总线速度无效，该函数将返回一个失败，并保持总线速度不变。

该函数的原型是:

```C
int (*set_bus_speed)(void *hdl, unsigned int speed, unsigned int *ospeed);
 /*
* Specify the bus speed.
* If an invalid bus speed is requested, this function should return
* failure and leave the bus speed unchanged.
* Parameters:
* (in)     hdl         Handle returned from init()   init函数返回的句柄；
* (in)     speed       Bus speed. Units are implementation-defined.    总线的速度。
* (out)    ospeed      Actual bus speed (if NULL, this is ignored)     NULL或指向函数应该存储实际总线速度的位置的指针。
* Returns:
* 0    success
* -1   failure
*/
```

具体源码实现：

```C
int
omap_set_bus_speed(void *hdl, unsigned int speed, unsigned int *ospeed)
{
    omap_dev_t      *dev = hdl;
    unsigned long   iclk;
    unsigned        scll_plus_sclh;
    int             scll_delta;
    int             high_adjust;
    int             low_adjust;
    int             scll_to_write;
    int             sclh_to_write;

    /* This driver support bus speed range from 8KHz to 400KHz
     * limit the low bus speed to 8KHz to protect SCLL/SCLH from overflow(large than 0xff)
     * if speed=8KHz, iclk=4MHz, then SCLL=0xf3, SCLH=0xf5
     */
    if (speed > 400000 || speed < 8000) {
        fprintf(stderr, "i2c-omap35xx:  Invalid bus speed(%d)\n", speed);
        errno = EINVAL;
        return -1;
    }

    omap_clock_enable(dev);

    /* Set the I2C prescaler register to obtain the maximum I2C bit rates
     * and the maximum period of the filtered spikes in F/S mode:
     * Stander Mode: I2Ci_INTERNAL_CLK = 4 MHz
     * Fast Mode:    I2Ci_INTERNAL_CLK = 9.6 MHz
     */
    if (speed <= 100000) {
#ifdef VARIANT_j5
        out16(dev->regbase + OMAP_I2C_PSC, 11);     // The sysclk is 48 MHz in J5 and 96 MHz in OMAP
#else
        out16(dev->regbase + OMAP_I2C_PSC, 23);     // I2Ci_INTERNAL_CLK = 4 MHz
#endif
        iclk = OMAP_I2C_ICLK;
        high_adjust = dev->high_adjust_slow;
        low_adjust = dev->low_adjust_slow;
        // in standard mode, scll is smaller than (scll_plus_sclh>>1) by 1
        scll_delta = -1;
    } else {
#ifdef VARIANT_j5
        out16(dev->regbase + OMAP_I2C_PSC, 4);      // The sysclk is 48 MHz in J5 and 96 MHz in OMAP
#else
        out16(dev->regbase + OMAP_I2C_PSC, 9);      // I2Ci_INTERNAL_CLK = 9.6 MHz
#endif
        iclk = OMAP_I2C_ICLK_9600K;
        high_adjust = dev->high_adjust_fast;
        low_adjust = dev->low_adjust_fast;
#ifdef VARIANT_omap4
        // in fast mode, scll is larger than (scll_plus_sclh>>1) by 1
        scll_delta = 1;
#else
        // At this point it is unclear whether omap3 also has this relationship
        // between SCLL and SCLH in fast mode, so we leave it alone for now to
        // avoid breakage.
        scll_delta = -1;
#endif
    }

    /* Set clock based on "speed" bps */
    scll_plus_sclh = (iclk/speed - (SCLL_BIAS + SCLH_BIAS));

    scll_to_write = (scll_plus_sclh>>1) + scll_delta;
    sclh_to_write = scll_plus_sclh - scll_to_write;
    scll_to_write += low_adjust;
    sclh_to_write += high_adjust;

    if (scll_to_write < 0) scll_to_write = 0;
    if (sclh_to_write <= 2) sclh_to_write = 3;

    out16(dev->regbase + OMAP_I2C_SCLL, scll_to_write);
    out16(dev->regbase + OMAP_I2C_SCLH, sclh_to_write);
    dev->speed = iclk / (scll_to_write + SCLL_BIAS + sclh_to_write + SCLH_BIAS);

    if (ospeed)
        *ospeed = dev->speed;

    omap_clock_disable(dev);
    return 0;
}
```



### driver_info函数

driver_info函数返回有关驱动程序的信息。该函数的原型是:

```C
 int (*driver_info)(void *hdl, i2c_driver_info_t *info);
/*
hdl            init函数返回的句柄
info
一个指向i2c_driver_info_t结构的指针，函数应该在该结构中存储信息:
typedef struct {
    uint32_t    speed_mode;
    uint32_t    addr_mode;
    uint32_t    reserved[2];
} i2c_driver_info_t;
*/
```

具体源码实现：

```C
int omap_driver_info(void *hdl, i2c_driver_info_t *info)
{

    info->speed_mode = I2C_SPEED_STANDARD | I2C_SPEED_FAST;
    info->addr_mode = I2C_ADDRFMT_7BIT | I2C_ADDRFMT_10BIT;

    return 0;
}
```



### ctl函数

ctl函数处理一个驱动程序特定的devctl()命令。该函数的原型是:

```C
int (*ctl)(void *hdl, int cmd, void *msg, int msglen, int *nbytes, int *info);
/*
* Handle a driver-specific devctl().
* Parameters:
* (in)     hdl         Handle returned from init()   init函数返回的句柄；
* (in)     cmd         Device command                设备命令；
* (i/o)    msg         Message buffer                一个指向消息缓冲区的指针。该函数可以更改缓冲区的内容；
* (in)     msglen      Length of message buffer in bytes    消息缓冲区的长度，以字节为单位；
* (out)    nbytes      Bytes to return (<= msglen)          消息缓冲区的长度，以字节为单位；
* (out)    info        Extra status information returned by devctl    指向函数可以存储devctl()返回状态信息的位置指针；
* Returns:
* EOK      success
* errno    failure
*/
```

此处I2C驱动不需要进行devctrl操作。所以ctl函数在resmgr层注册的时候为NULL。



## 共享库的接口

当I2C主设备专用于单个应用程序使用时，可以将硬件接口代码编译为库并将其链接到应用程序。

为了提高代码的可移植性，应用程序应该调用i2c_master_getfuncs()来访问特定于硬件的函数。通过功能表访问硬件库，应用程序更容易加载和管理多个硬件库。

资源管理器接口以类似的方式使用硬件库。



## 资源管理器设计

资源管理器层实现为静态链接到硬件库的库，提供了一个单线程管理器，libi2c-master。

在启动时，resmgr层执行以下操作：

i2c_master_getfuncs(&masterf)
masterf.init()
masterf.set_bus_speed()
然后，资源管理器让自己在后台运行，下面是资源管理器处理这些devctl()命令:

>DCMD_I2C_DRIVER_INFO calls masterf.driver_info().
>DCMD_I2C_SET_BUS_SPEED and DCMD_I2C_SET_SLAVE_ADDR only update the state of the current connection.
>DCMD_I2C_SEND, DCMD_I2C_RECV, DCMD_I2C_SENDRECV, DCMD_I2C_MASTER_SEND, and DCMD_I2C_MASTER_RECV

```C
if (bus_speed has changed)
    masterf.set_bus_speed()
masterf.set_slave_address()
masterf.send() or masterf.recv()
```

资源管理器线程一直占用，直到传输完成并回复客户端，可以通过向资源管理器发送SIGTERM来终止它。

实质和下面流程一样。在初始化里设置一个中断事件，这个事件将发送消息告诉主循环接收和发送数据。

```C
#include <hw/i2c.h>    
i2c_master_funcs_t  masterf;    
i2c_libversion_t    version;    
i2c_status_t        status;    void *hdl;     
i2c_master_getfuncs(&masterf, sizeof(masterf));//初始化硬件接口     masterf.version_info(&version);    
if ((version.major != I2CLIB_VERSION_MAJOR) || (version.minor > I2CLIB_VERSION_MINOR))    
{
    /* error */        
    ...    
}   
 hdl = masterf.init(...); //调用初始化函数    
 masterf.set_bus_speed(hdl, ...); //设置总线速度 
while(1)
{ 
    InterruptWait (NULL, NULL);
    masterf.set_slave_addr(hdl, ...); //设置从地址     
    status = masterf.send(hdl, ...);//发送数据    
    if (status != I2C_STATUS_DONE) {        /* error */        
    if (!(status & I2C_STATUS_DONE))         
       masterf.abort(hdl);    }    
     status = masterf.recv(hdl, ...);//接收数据    
    if (status != I2C_STATUS_DONE) 
    {        /* error */        ...    }   
    }
      masterf.fini(hdl);//完成I2C传输后，清理驱动程序并释放与给定句柄关联的所有内存
      InterruptUnmask(INTNUM, id);
}
masterf.fini(hdl);//完成I2C传输后，清理驱动程序并释放与给定句柄关联的所有内存
```



### 参考文献：

[QNX----I2C驱动框架](https://blog.csdn.net/u011996698/article/details/82857146)  

[QNX---IMX6UL I2C 驱动分析](https://blog.csdn.net/u011996698/article/details/88884581)  

[Technical Notes I2C (Inter-Integrated Circuit) Framework(qnx.com)](http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_technotes%2Fi2c_framework.html)

