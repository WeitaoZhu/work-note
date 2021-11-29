# QNX®  Neutrino OS镜像

### 介绍

OS镜像是指包含OS、可执行文件、与程序相关的数据等的文件。

本文解释如何:

- 使用 mkifs 构建OS镜像 (看“[Building an OS image](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.building/topic/os_images/image_build.html)”)
- 构建闪存文件系统映像或嵌入式事务文件系统映像 (看“[Building a flash filesystem image](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.building/topic/os_images/nto_flash.html)”)
- 将可引导的操作系统映像和文件系统映像合并为一个可以传输到目标系统的映像 (看“[Combining multiple image files](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.building/topic/os_images/multiple_images_combine.html)”)

#### 镜像类型

OS镜像可以是不可引导的，也可以是可引导的，在嵌入式QNX中，镜像可能是以下几种类型：

| Image type                            | Description                                                  | Created by                                                   |
| ------------------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| OS image                              | A bootable or non-bootable structure that contains files     | [mkifs](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.utilities/topic/m/mkifs.html) |
| Flash filesystem image                | A structure that can be used in a read-only, read/write, or read/write/reclaim flash filesystem | [mkefs](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.utilities/topic/m/mkefs.html) |
| Embedded transaction filesystem image | A binary image file containing the [ETFS](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.sys_arch/topic/fsys_ETFS.html) as a sequence of transactions | [mketfs](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.utilities/topic/m/mketfs.html) |

#### 什么是可引导镜像？

可引导镜像指包含了startup和procnto的镜像，IPL，Boot Rom或BIOS（X86）可以将控制权传递给它。更详细的信息可参考“[The boot process](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.building/topic/intro/intro_startup_sequence.html)” and “[Initial Program Loaders (IPLs)](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.building/topic/ipl/ipl_about.html)”。

一般的可引导镜像如下所示：

![bootable_image](./pic/bootable_image.PNG)

对于有些系统，可能需要与配置相关的设置，通常将提供不可引导的镜像。将这个镜像看作是第二个“文件系统”，包含一些额外的文件。由于是不可引导的，所以通常不包括操作系统以及startup文件。

#### QNX OS镜像

QNX OS镜像通常包括OS（procnto）、startup代码、startup及OS需要用到的库文件（libc.so）、以及访问基本硬件所需的驱动代码或其他代码和文件。

通常，一个小的嵌入式系统中只会有一个OS镜像，在QNX系统中，包含的组件：

- startup代码： startup-*；
- OS内核和进程管理器： procnto；
- 驱动和管理器：io-pkt，devnp-ncm.so，devc-ser8250，devb-eide，esh (embedded shell)，ksh等；
- 应用程序；
- 数据文件；
   其他设备的驱动程序、用户应用程序以及所需的库和数据文件通常会放在另一个文件系统中。

对于大多数嵌入式系统，最好保持OS镜像尽可能小，将在初始启动时不需要的应用程序甚至驱动放到另一个文件系统中，比如NAND或NOR文件系统。

#### IFS（Image Filesystem）

引导之后，QNX OS镜像的内容作为文件系统显示在/proc/boot中，实际上，考虑OS镜像的一种好方法是将其视为一个小型文件系统，在QNX中，OS镜像通常被称为IFS（image filesystem）。这个IFS有一个小的目录结构和文件，目录用于告诉OS内核和进程管理器procnto OS镜像中文件的名称和位置。比如：

```shell
# cd /proc/boot
# ls
.script     ping        cat       data1        pidin       
                                          
ksh         ls          ftp       procnto      devc-ser8250-abc123
# cat data1
```

#### 镜像大小

通常小的OS镜像，好处也是显而易见的：

> - 节省RAM内存；
> - 更快的boot时间；
> - 更稳定，通常OS镜像的组件越少，引导失败的概率就越低；

如果嵌入式系统中有硬盘驱动或闪存（比如MMC/SD），可以在OS镜像文件中包含适当的文件系统驱动程序（比如devb-eide，devb-mmcsd-board_name），并从脚本中调用驱动程序来访问其中的数据。
如果嵌入式系统中有Flash设备，可以使用这个Flash设备来存储OS镜像，并直接从Flash引导系统。

### Deciding what goes on the target

根据系统的需求，需要将IPL、OS镜像和文件系统的组合传送到目标板上。这些组合包括：

> - the IPL
> - the boot image
> - the boot image and flash filesystem
> - the boot image and other image filesystem
> - some other combination of the above

![image_embed](./pic/image_embed.PNG)

如果目标开发板支持Flash设备，可以将引导镜像和Flash文件系统（或两者结合）写入到目标板的Flash设备上，如果想从Flash启动的话，这一步是必须的。要组合镜像，可以使用diskimage工具（看“[Combining multiple image files](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.building/topic/os_images/multiple_images_combine.html)”）来操作。在开发阶段，可能需要下载工具，将image固化到Flash设备中。

### Building an OS image

OS镜像是由mkifs(make image filesystem)程序来创建的，它接受来自两个主要来源的信息：命令行和buildfile文件。可以修改buildfile来配置OS镜像。

下图显示了 mkifs 如何使用构建文件中的规则来组合操作系统和用户可执行文件、共享对象和数据文件来创建操作系统映像，该映像可以传输到目标系统并启动。

![image_creation](./pic/image_creation.PNG)

有关mkifs 的更多信息，请参阅[Utilities]([- About This Reference (qnx.com)](http://www.qnx.com/developers/docs/7.1/index.html#com.qnx.doc.neutrino.utilities/topic/about.html))参考中的[mkifs](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.utilities/topic/m/mkifs.html) ； 有关 OS 映像构建文件的更多信息，请参阅[OS Image Buildfiles](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.building/topic/buildfiles/buildfile.html)一章。

当配置完buildfile后，假如名为shell.bld，在Host机器上，可以执行：`mkifs  shell.bld  shell.ifs`，来生成镜像文件。可以为mkifs命令指定命令行选项。
 当创建完镜像后，也可以通过dumpifs工具来查看镜像中的内容。

```shell
   Offset     Size  Name
        0      100  Startup-header flags1=0x1 flags2=0 paddr_bias=0x80000000
      100     a008  startup.*
     a108       5c  Image-header mountpoint=/
     a164      264  Image-directory
     ----     ----  Root-dirent
     ----       12  usr/lib/ldqnx.so.2 -> /proc/boot/libc.so
     ----        9  dev/console -> /dev/ser1
     a3c8       80  proc/boot/.script
     b000    4a000  proc/boot/procnto
    55000    59000  proc/boot/libc.so.4 
     ----        9  proc/boot/libc.so -> libc.so.4 
    ae000     7340  proc/boot/devc-ser8250
    b6000     4050  proc/boot/esh
    bb000     4a80  proc/boot/ls
    c0000    14fe0  proc/boot/data1
    d5000     22a0  proc/boot/data2
Checksums: image=0x94b0d37b startup=0xa3aeaf2
```

### Building a flash filesystem image

如果应用程序需要可写的文件系统，并且嵌入式系统上有Flash设备，那么可以使用QNX Flash文件系统驱动来提供与POSIX兼容的文件系统。
将文件构建到Flash文件系统中，可以简化到目标板的复制工作，在使用mkefs构建文件系统之后，不再需要逐个复制每个文件，只需要一条命令来移至镜像即可。
 构建Flash文件系统时，有两种选择：

> - 在主机上创建Flash文件系统镜像文件，然后将镜像文件写入到目标板的Flash中 (参考“[Making flash filesystems](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.building/topic/os_images/nto_flash.html#flash__make)”).；
> - 在目标板系统中运行Flash文件系统驱动程序，然后将文件复制到目标板上的Flash文件系统中；

创建Flash文件系统：
当改变目标板上Flash的大小或配置，需要重新创建Flash文件系统镜像。BSP包中包含了适当的Flash文件系统驱动的源代码和二进制文件，也提供了相关的库和头文件。
 产生Flash文件系统二进制文件：

> - 进入BSP目录中；
> - 在命令行模式下，输入make即可；

### Creating a new filesystem on your target

可以使用QNX工具来在目标板上创建一个新的文件系统。
 下边的指令是在目标系统上创建文件系统的通用指令。具体的细节可能跟各个文件系统的类型、开发板、以及创建文件系统的介质（比如DOS文件系统在X86开发板的SATA硬盘）等有关系。
 要在目标板上创建新文件系统，先连接目标板，然后在目标板上：

> - 加载合适的设备驱动，比如devb-eide；
> - 识别相关的设备；
> - 使用fdisk来初始化或编辑分区表，以删除不需要的分区，并添加要用于新文件系统的分区(看Utilities Reference的命令[fdisk](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.utilities/topic/f/fdisk.html))；
> - 使用mkfilesystem_typefs（比如，mkdosfs、mkqnx6fs）命令在适当的分区中创建文件系统；
> - 使用mount挂载具有新文件系统的分区(看Utilities Reference的命令[mount](http://www.qnx.com/developers/docs/7.0.0/com.qnx.doc.neutrino.utilities/topic/m/mount.html) )；


这些步骤之后，应该就在挂载的分区中安装了新的文件系统。

### Combining multiple image files

将IPL、mkifs创建的OS镜像、使用mkefs创建的flash文件系统镜像结合起来可能是有用的，甚至是必需的。
尤其是：

> - 有些开发板只接收单个镜像文件，因此必须组合在一起才能传送到开发板；
> - 一旦系统投入生产，单个镜像可以简化系统的制作。复制单个镜像比复制多个镜像更简单、更快，也更不容易出错；

![image_combined](./pic/image_combined.PNG)

当使用mkifs构建OS文件系统时，可以指定是将可执行文件复制到RAM中运行，还是在本地执行（XIP）。XIP通过避免将对象的代码/数据段从一个RAM位置复制到另一个RAM位置来节省一点RAM。

要启动上边的配置（XIP，并在RAM中运行），创建两个单独的镜像，然后将它们组合起来。可以使用diskimage程序来为分区介质（比如硬盘驱动器、SD卡或MMC）创建单个组合镜像，然后将镜像传输到目标板。

组合镜像中的每个部分（IPL、IFS（XIP）、EFS（在RAM中运行））必须从内存页边界开始（一般4KB），IPL代码将把执行权交给第一个镜像中的startup代码和OS。

参考文献：

[**OS Core Components** - OS Images](http://www.qnx.com/developers/docs/7.0.0/index.html#com.qnx.doc.neutrino.building/topic/os_images/images_about.html)

