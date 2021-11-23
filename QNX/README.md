# QNX学习日志



以[BSP Directory](https://community.qnx.com/sf/wiki/do/viewPage/projects.bsp/wiki/BSPAndDrivers)为例

![git_bash](./pic/ReferenceBSPforQNXProduct.PNG)



```shell
$ tree -L 3
.
├── Makefile
├── images
│   ├── Makefile
│   ├── ifs-mx6sx-sabre-sdb.raw
│   └── ipl-mx6sx-sabre-sdb.bin
├── install
├── manifest
├── prebuilt
│   ├── armle-v7
│   │   ├── bin
│   │   ├── boot
│   │   ├── lib
│   │   ├── sbin
│   │   └── usr
│   └── usr
│       └── include
├── readme.txt
├── source.xml
└── src
    ├── Makefile
    ├── hardware
    │   ├── Makefile
    │   ├── deva
    │   ├── devb
    │   ├── devc
    │   ├── devnp
    │   ├── i2c
    │   ├── ipl
    │   ├── startup
    │   └── support
    ├── lib
    │   ├── Makefile
    │   ├── dma
    │   └── idlcommon.mk
    └── utils
        ├── Makefile
        └── g

25 directories, 12 files
```



**/src**

/src 目录包含 BSP 附带的所有源代码。源代码是组织成子目录，例如 /hardware、/lib、/utils 等。当BSP 构建完成后，BSP 内的所有源代码（包括这些中的源代码子目录）都会被构建。

**/prebuilt**

所有BSP 组件预构建并且存储在/prebuilt 目录。如果你在构建 BSP完成后，/prebuilt 的目录内容复制到 BSP 的 /install 目录。

**/install**

在构建 BSP 之前 /install 目录是空的。在构建 BSP 时，/prebuilt 目录的全部内容都复制到 /install 目录中。生成 IFS 映像时，/install 目录是首先被扫描在 BSP 的构建文件中指定的组件。如果生成 IFS 镜像进程在 /install 目录中找不到组件，它会扫描 QNX SDP 6.6 主机的组件的/target 目录。

> 如果您想通过更改或构建任何 BSP 源代码来修改 BSP 组件，你必须在构建相关的源代码时使用 make install 命令。此命令将导致您修改的组件复制到 BSP的 /install 目录中，覆盖由BSP提供的预构建的版本。

**/images**

/images是存放镜像的位置：

• BSP 的默认构建文件是在BSP 构建后生成
• 任何启动映像（例如 IFS 映像、IPL 二进制文件等）被生成。

