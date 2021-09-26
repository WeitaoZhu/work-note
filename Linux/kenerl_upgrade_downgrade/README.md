# Ubuntu Linux内核版本升级或降级到指定版本（基于ubuntu 16.04示例）  

本文以ubuntu 16.04.12系统为例，内核版本为4.15.0-112-generic，降级到内核版本为4.4.0-119-generic。

当前系统版本信息：

```bash
bspserver@bspserver-Hinge-tech:~$ uname -r
4.15.0-112-generic
bspserver@bspserver-Hinge-tech:~$ lsb_release -a
No LSB modules are available.
Distributor ID:	Ubuntu
Description:	Ubuntu 16.04.7 LTS
Release:	16.04
Codename:	xenial
```

## 1. 查看当前系统已安装的内核镜像

```bash
bspserver@bspserver-Hinge-tech:~$ dpkg --get-selections |grep linux-image
linux-image-4.15.0-112-generic			install
linux-image-generic-hwe-16.04			install
```

##  2. 查看指定版本的linu镜像包

这里以4.4.0-119版本内核为例：

```bash
bspserver@bspserver-Hinge-tech:~$ apt-cache search linux| grep 4.4.0-119
linux-cloud-tools-4.4.0-119 - Linux kernel version specific cloud tools for version 4.4.0-119
linux-cloud-tools-4.4.0-119-generic - Linux kernel version specific cloud tools for version 4.4.0-119
linux-cloud-tools-4.4.0-119-lowlatency - Linux kernel version specific cloud tools for version 4.4.0-119
linux-headers-4.4.0-119 - Header files related to Linux kernel version 4.4.0
linux-headers-4.4.0-119-generic - Linux kernel headers for version 4.4.0 on 64 bit x86 SMP
linux-headers-4.4.0-119-lowlatency - Linux kernel headers for version 4.4.0 on 64 bit x86 SMP
linux-image-4.4.0-119-generic - Linux kernel image for version 4.4.0 on 64 bit x86 SMP
linux-image-4.4.0-119-lowlatency - Linux kernel image for version 4.4.0 on 64 bit x86 SMP
linux-image-extra-4.4.0-119-generic - Linux kernel extra modules for version 4.4.0 on 64 bit x86 SMP
linux-signed-image-4.4.0-119-generic - Signed kernel image generic
linux-signed-image-4.4.0-119-lowlatency - Signed kernel image lowlatency
linux-tools-4.4.0-119 - Linux kernel version specific tools for version 4.4.0-119
linux-tools-4.4.0-119-generic - Linux kernel version specific tools for version 4.4.0-119
linux-tools-4.4.0-119-lowlatency - Linux kernel version specific tools for version 4.4.0-119
bspserver@bspserver-Hinge-tech:~$ 
```

***注意***

①、如果查询不到，可以在/etc/apt/sources.list配置中，加入其他源

在sources.list 最后一行添加以下内容

```bash
deb http://security.ubuntu.com/ubuntu trusty-security main
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-updates main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-backports main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-security main restricted universe multiverse
```

②、更新镜像

```bash
sudo apt-get update
```

## 3. 安装上面查到的指定版本内核的linux系统头文件及镜像：

```bash
bspserver@bspserver-Hinge-tech:~$ sudo apt-get install linux-headers-4.4.0-119-generic linux-image-4.4.0-119-generic linux-image-extra-4.4.0-119-generic
Reading package lists... Done
Building dependency tree       
Reading state information... Done
The following additional packages will be installed:
  linux-headers-4.4.0-119
Suggested packages:
  fdutils linux-doc-4.4.0 | linux-source-4.4.0 linux-tools
The following NEW packages will be installed:
  linux-headers-4.4.0-119 linux-headers-4.4.0-119-generic linux-image-4.4.0-119-generic linux-image-extra-4.4.0-119-generic
0 upgraded, 4 newly installed, 0 to remove and 180 not upgraded.
Need to get 69.2 MB of archives.
After this operation, 301 MB of additional disk space will be used.
Do you want to continue? [Y/n] y
Get:1 http://cn.archive.ubuntu.com/ubuntu xenial-updates/main amd64 linux-headers-4.4.0-119 all 4.4.0-119.143 [10.0 MB]
Get:2 http://cn.archive.ubuntu.com/ubuntu xenial-updates/main amd64 linux-headers-4.4.0-119-generic amd64 4.4.0-119.143 [816 kB] 
Get:3 http://cn.archive.ubuntu.com/ubuntu xenial-updates/main amd64 linux-image-4.4.0-119-generic amd64 4.4.0-119.143 [22.0 MB] 
Get:4 http://cn.archive.ubuntu.com/ubuntu xenial-updates/main amd64 linux-image-extra-4.4.0-119-generic amd64 4.4.0-119.143 [36.4 MB] 
......
......
Setting up linux-image-extra-4.4.0-119-generic (4.4.0-119.143) ...
run-parts: executing /etc/kernel/postinst.d/apt-auto-removal 4.4.0-119-generic /boot/vmlinuz-4.4.0-119-generic
run-parts: executing /etc/kernel/postinst.d/initramfs-tools 4.4.0-119-generic /boot/vmlinuz-4.4.0-119-generic
update-initramfs: Generating /boot/initrd.img-4.4.0-119-generic
run-parts: executing /etc/kernel/postinst.d/pm-utils 4.4.0-119-generic /boot/vmlinuz-4.4.0-119-generic
run-parts: executing /etc/kernel/postinst.d/unattended-upgrades 4.4.0-119-generic /boot/vmlinuz-4.4.0-119-generic
run-parts: executing /etc/kernel/postinst.d/update-notifier 4.4.0-119-generic /boot/vmlinuz-4.4.0-119-generic
run-parts: executing /etc/kernel/postinst.d/zz-update-grub 4.4.0-119-generic /boot/vmlinuz-4.4.0-119-generic
Generating grub configuration file ...
Warning: Setting GRUB_TIMEOUT to a non-zero value when GRUB_HIDDEN_TIMEOUT is set is no longer supported.
Found linux image: /boot/vmlinuz-4.15.0-112-generic
Found initrd image: /boot/initrd.img-4.15.0-112-generic
Found linux image: /boot/vmlinuz-4.4.0-119-generic
Found initrd image: /boot/initrd.img-4.4.0-119-generic
Found memtest86+ image: /boot/memtest86+.elf
Found memtest86+ image: /boot/memtest86+.bin
done
bspserver@bspserver-Hinge-tech:~$ 
```

## 4. 查看当前系统中内核的启动顺序

```bas
bspserver@bspserver-Hinge-tech:~$ grep menuentry /boot/grub/grub.cfg
if [ x"${feature_menuentry_id}" = xy ]; then
  menuentry_id_option="--id"
  menuentry_id_option=""
export menuentry_id_option
menuentry 'Ubuntu' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-simple-ab51f4e0-07f5-450b-badb-2073d774beb4' {
submenu 'Advanced options for Ubuntu' $menuentry_id_option 'gnulinux-advanced-ab51f4e0-07f5-450b-badb-2073d774beb4' {
	menuentry 'Ubuntu, with Linux 4.15.0-112-generic' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-4.15.0-112-generic-advanced-ab51f4e0-07f5-450b-badb-2073d774beb4' {
	menuentry 'Ubuntu, with Linux 4.15.0-112-generic (upstart)' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-4.15.0-112-generic-init-upstart-ab51f4e0-07f5-450b-badb-2073d774beb4' {
	menuentry 'Ubuntu, with Linux 4.15.0-112-generic (recovery mode)' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-4.15.0-112-generic-recovery-ab51f4e0-07f5-450b-badb-2073d774beb4' {
	menuentry 'Ubuntu, with Linux 4.4.0-119-generic' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-4.4.0-119-generic-advanced-ab51f4e0-07f5-450b-badb-2073d774beb4' {
	menuentry 'Ubuntu, with Linux 4.4.0-119-generic (upstart)' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-4.4.0-119-generic-init-upstart-ab51f4e0-07f5-450b-badb-2073d774beb4' {
	menuentry 'Ubuntu, with Linux 4.4.0-119-generic (recovery mode)' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-4.4.0-119-generic-recovery-ab51f4e0-07f5-450b-badb-2073d774beb4' {
```

## 5. 修改内核启动顺序

如果你升级的版本比当前内核版本高的话，默认新安装的内核就是第一顺序启动的，只需重启系统就行了，否则，则需要修改配置文件，具体如下：

```bash
vi /etc/default/grub

# If you change this file, run 'update-grub' afterwards to update
# /boot/grub/grub.cfg.
# For full documentation of the options in this file, see:
#   info -f grub -n 'Simple configuration'

GRUB_DEFAULT=0
GRUB_HIDDEN_TIMEOUT=0
GRUB_HIDDEN_TIMEOUT_QUIET=true
GRUB_TIMEOUT=10
GRUB_DISTRIBUTOR=`lsb_release -i -s 2> /dev/null || echo Debian`
GRUB_CMDLINE_LINUX_DEFAULT="quiet splash"
GRUB_CMDLINE_LINUX=""

# Uncomment to enable BadRAM filtering, modify to suit your needs
# This works with Linux (no patch required) and with any kernel that obtains
# the memory map information from GRUB (GNU Mach, kernel of FreeBSD ...)
#GRUB_BADRAM="0x01234567,0xfefefefe,0x89abcdef,0xefefefef"

# Uncomment to disable graphical terminal (grub-pc only)
#GRUB_TERMINAL=console

# The resolution used on graphical terminal
# note that you can use only modes which your graphic card supports via VBE
# you can see them in real GRUB with the command `vbeinfo'
#GRUB_GFXMODE=640x480

# Uncomment if you don't want GRUB to pass "root=UUID=xxx" parameter to Linux
#GRUB_DISABLE_LINUX_UUID=true

# Uncomment to disable generation of recovery mode menu entries
#GRUB_DISABLE_RECOVERY="true"

# Uncomment to get a beep at grub start
#GRUB_INIT_TUNE="480 440 1"

```

将GRUB_DEFAULT=0修改为**GRUB_DEFAULT="Advanced options for Ubuntu>Ubuntu, with Linux 4.4.0-119-generic"**，保存退出。

## 6. 生效配置

若上面修改了配置文件，则需执行***update-grub***命令，更新grub配置：

```bash
bspserver@bspserver-Hinge-tech:~$ sudo update-grub
Generating grub configuration file ...
Warning: Setting GRUB_TIMEOUT to a non-zero value when GRUB_HIDDEN_TIMEOUT is set is no longer supported.
Found linux image: /boot/vmlinuz-4.15.0-112-generic
Found initrd image: /boot/initrd.img-4.15.0-112-generic
Found linux image: /boot/vmlinuz-4.4.0-119-generic
Found initrd image: /boot/initrd.img-4.4.0-119-generic
Found memtest86+ image: /boot/memtest86+.elf
Found memtest86+ image: /boot/memtest86+.bin
done
```

之后，**reboot**，验证是否生效。



