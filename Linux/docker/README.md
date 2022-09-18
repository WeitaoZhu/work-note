# Ubuntu16.04/18.04/20.04下安装Docker

Docker Engine-Community 支持以下的 Ubuntu 版本：

- Xenial 16.04 (LTS)
- Bionic 18.04 (LTS)
- Focal    20.04(LTS)
- 或者更高版本

## 1. 使用官方安装脚本自动安装

**安装命令如下：**

```bash
curl -fsSL https://get.docker.com | bash -s docker --mirror Aliyun
```

命令log如下：

```bash
$ curl -fsSL https://get.docker.com | bash -s docker --mirror Aliyun
# Executing docker install script, commit: 93d2499759296ac1f9c510605fef85052a2c32be

DEPRECATION WARNING
    This Linux distribution (ubuntu xenial) reached end-of-life and is no longer supported by this script.
    No updates or security fixes will be released for this distribution, and users are recommended
    to upgrade to a currently maintained version of ubuntu.

Press Ctrl+C now to abort this script, or wait for the installation to continue.
+ sudo -E sh -c 'apt-get update -qq >/dev/null'
+ sudo -E sh -c 'DEBIAN_FRONTEND=noninteractive apt-get install -y -qq apt-transport-https ca-certificates curl >/dev/null'
+ sudo -E sh -c 'curl -fsSL "https://mirrors.aliyun.com/docker-ce/linux/ubuntu/gpg" | gpg --dearmor --yes -o /usr/share/keyrings/docker-archive-keyring.gpg'
+ sudo -E sh -c 'echo "deb [arch=amd64 signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://mirrors.aliyun.com/docker-ce/linux/ubuntu xenial stable" > /etc/apt/sources.list.d/docker.list'
+ sudo -E sh -c 'apt-get update -qq >/dev/null'
+ sudo -E sh -c 'DEBIAN_FRONTEND=noninteractive apt-get install -y -qq --no-install-recommends  docker-ce-cli docker-scan-plugin docker-ce >/dev/null'
+ version_gte 20.10
+ '[' -z '' ']'
+ return 0
+ sudo -E sh -c 'DEBIAN_FRONTEND=noninteractive apt-get install -y -qq docker-ce-rootless-extras >/dev/null'
+ sudo -E sh -c 'docker version'
Client: Docker Engine - Community
 Version:           20.10.7
 API version:       1.41
 Go version:        go1.13.15
 Git commit:        f0df350
 Built:             Wed Jun  2 11:56:47 2021
 OS/Arch:           linux/amd64
 Context:           default
 Experimental:      true

Server: Docker Engine - Community
 Engine:
  Version:          20.10.7
  API version:      1.41 (minimum version 1.12)
  Go version:       go1.13.15
  Git commit:       b0f5bc3
  Built:            Wed Jun  2 11:54:58 2021
  OS/Arch:          linux/amd64
  Experimental:     false
 containerd:
  Version:          1.4.6
  GitCommit:        d71fcd7d8303cbf684402823e425e9dd2e99285d
 runc:
  Version:          1.0.0-rc95
  GitCommit:        b9ee9c6314599f1b4a7f497e1f1f856fe433d3b7
 docker-init:
  Version:          0.19.0
  GitCommit:        de40ad0

================================================================================

To run Docker as a non-privileged user, consider setting up the
Docker daemon in rootless mode for your user:

    dockerd-rootless-setuptool.sh install

Visit https://docs.docker.com/go/rootless/ to learn about rootless mode.


To run the Docker daemon as a fully privileged service, but granting non-root
users access, refer to https://docs.docker.com/go/daemon-access/

WARNING: Access to the remote API on a privileged Docker daemon is equivalent
         to root access on the host. Refer to the 'Docker daemon attack surface'
         documentation for details: https://docs.docker.com/go/attack-surface/

================================================================================
```

从log可以看出安装是基于ubuntu xenial，docker版本为20.10.7。



## 2. 手动安装

**卸载旧版本**

Docker 的旧版本被称为 docker，docker.io 或 docker-engine 。如果已安装，请卸载它们：

```bash
sudo apt-get remove docker docker-engine docker.io containerd runc
```

命令log如下：

```bash
$ sudo apt-get remove docker docker-engine docker.io containerd runc
Reading package lists... Done
Building dependency tree       
Reading state information... Done
Package 'docker-engine' is not installed, so not removed
Package 'docker' is not installed, so not removed
Package 'containerd' is not installed, so not removed
Package 'docker.io' is not installed, so not removed
Package 'runc' is not installed, so not removed
The following packages were automatically installed and are no longer required:
  libass5 libavcodec-ffmpeg56 libavdevice-ffmpeg56 libavfilter-ffmpeg5 libavformat-ffmpeg56 libavresample-ffmpeg2 libavutil-ffmpeg54 libbluray1 libmodplug1 libopencv-core2.4v5 libopencv-imgproc2.4v5 libopenjpeg5 libpostproc-ffmpeg53 libschroedinger-1.0-0
  libsdl1.2debian libswresample-ffmpeg1 libswscale-ffmpeg3 libtbb2 libx264-148 libx265-79
Use 'sudo apt autoremove' to remove them.
0 upgraded, 0 newly installed, 0 to remove and 84 not upgraded.

```

当前称为 Docker Engine-Community 软件包 docker-ce 。

安装 Docker Engine-Community，以下介绍两种方式。

### **使用 Docker 仓库进行安装**

在新主机上首次安装 Docker Engine-Community 之前，需要设置 Docker 仓库。之后，您可以从仓库安装和更新 Docker 。

**设置仓库**

更新 apt 包索引。

```bash
$ sudo apt-get update
```

安装 apt 依赖包，用于通过HTTPS来获取仓库:

```bash
$ sudo apt-get install \
    apt-transport-https \
    ca-certificates \
    curl \
    gnupg-agent \
    software-properties-common
```

添加 Docker 的官方 GPG 密钥：

```bash
$ curl -fsSL https://mirrors.ustc.edu.cn/docker-ce/linux/ubuntu/gpg | sudo apt-key add -
```

使用以下指令设置稳定版仓库

```bash
$ sudo add-apt-repository \
   "deb [arch=amd64] https://mirrors.ustc.edu.cn/docker-ce/linux/ubuntu/ \
  $(lsb_release -cs) \
  stable"
```

**安装 Docker Engine-Community**

更新 apt 包索引。

```bash
$ sudo apt-get update
```

安装最新版本的 Docker Engine-Community 和 containerd ，或者转到下一步安装特定版本：

```bash
$ sudo apt-get install docker-ce docker-ce-cli containerd.io
```

要安装特定版本的 Docker Engine-Community，请在仓库中列出可用版本，然后选择一种安装。列出您的仓库中可用的版本：

```bash
$ apt-cache madison docker-ce

 docker-ce | 5:20.10.7~3-0~ubuntu-xenial | http://mirrors.aliyun.com/docker-ce/linux/ubuntu xenial/stable amd64 Packages
 docker-ce | 5:20.10.7~3-0~ubuntu-xenial | https://mirrors.aliyun.com/docker-ce/linux/ubuntu xenial/stable amd64 Packages
 docker-ce | 5:20.10.6~3-0~ubuntu-xenial | http://mirrors.aliyun.com/docker-ce/linux/ubuntu xenial/stable amd64 Packages
  ...
 docker-ce | 18.06.3~ce~3-0~ubuntu | http://mirrors.aliyun.com/docker-ce/linux/ubuntu xenial/stable amd64 Packages
 docker-ce | 18.06.3~ce~3-0~ubuntu | https://mirrors.aliyun.com/docker-ce/linux/ubuntu xenial/stable amd64 Packages
 docker-ce | 18.06.2~ce~3-0~ubuntu | http://mirrors.aliyun.com/docker-ce/linux/ubuntu xenial/stable amd64 Packages
  ...
 docker-ce | 17.12.1~ce-0~ubuntu | http://mirrors.aliyun.com/docker-ce/linux/ubuntu xenial/stable amd64 Packages
 docker-ce | 17.12.1~ce-0~ubuntu | https://mirrors.aliyun.com/docker-ce/linux/ubuntu xenial/stable amd64 Packages
 docker-ce | 17.12.0~ce-0~ubuntu | http://mirrors.aliyun.com/docker-ce/linux/ubuntu xenial/stable amd64 Packages
  ...
 docker-ce | 17.03.0~ce-0~ubuntu-xenial | https://mirrors.aliyun.com/docker-ce/linux/ubuntu xenial/stable amd64 Packages
```

使用第二列中的版本字符串安装特定版本，例如 5:20.10.6~3-0~ubuntu-xenial。所以如下VERSION_STRING=5:20.10.6~3-0~ubuntu-xenial

```bash
$ sudo apt-get install docker-ce=<VERSION_STRING> docker-ce-cli=<VERSION_STRING> containerd.io
```

安装命令log如下：

```bash
hinge@hinge-bspserver:~$ sudo apt-get install docker-ce docker-ce-cli containerd.io
Reading package lists... Done
Building dependency tree       
Reading state information... Done
The following packages were automatically installed and are no longer required:
  libass5 libavcodec-ffmpeg56 libavdevice-ffmpeg56 libavfilter-ffmpeg5 libavformat-ffmpeg56 libavresample-ffmpeg2 libavutil-ffmpeg54 libbluray1 libmodplug1 libopencv-core2.4v5 libopencv-imgproc2.4v5 libopenjpeg5 libpostproc-ffmpeg53 libschroedinger-1.0-0
  libsdl1.2debian libswresample-ffmpeg1 libswscale-ffmpeg3 libtbb2 libx264-148 libx265-79
Use 'sudo apt autoremove' to remove them.
Suggested packages:
  aufs-tools cgroupfs-mount | cgroup-lite
The following NEW packages will be installed:
  containerd.io docker-ce docker-ce-cli
0 upgraded, 3 newly installed, 0 to remove and 84 not upgraded.
Need to get 0 B/93.8 MB of archives.
After this operation, 426 MB of additional disk space will be used.
Selecting previously unselected package containerd.io.
(Reading database ... 218799 files and directories currently installed.)
Preparing to unpack .../containerd.io_1.4.6-1_amd64.deb ...
Unpacking containerd.io (1.4.6-1) ...
Selecting previously unselected package docker-ce-cli.
Preparing to unpack .../docker-ce-cli_5%3a20.10.7~3-0~ubuntu-xenial_amd64.deb ...
Unpacking docker-ce-cli (5:20.10.7~3-0~ubuntu-xenial) ...
Selecting previously unselected package docker-ce.
Preparing to unpack .../docker-ce_5%3a20.10.7~3-0~ubuntu-xenial_amd64.deb ...
Unpacking docker-ce (5:20.10.7~3-0~ubuntu-xenial) ...
Processing triggers for man-db (2.7.5-1) ...
Processing triggers for ureadahead (0.100.0-19.1) ...
ureadahead will be reprofiled on next reboot
Processing triggers for systemd (229-4ubuntu21.28) ...
Setting up containerd.io (1.4.6-1) ...
Setting up docker-ce-cli (5:20.10.7~3-0~ubuntu-xenial) ...
Setting up docker-ce (5:20.10.7~3-0~ubuntu-xenial) ...
```

如果要使用 Docker 作为非 root 用户，则应考虑使用类似以下方式将用户添加到 docker 组：

```bash
$ sudo usermod -aG docker ${USER}
```

注销用户查看是否已经加入到docker组里。

```bash
$ su ${USER}
$ id -nG
hinge adm cdrom sudo dip plugdev lpadmin sambashare docker
```

测试 Docker 是否安装成功，输入以下指令，打印出以下信息则安装成功:

```bash
$ docker run hello-world

Hello from Docker!
This message shows that your installation appears to be working correctly.

To generate this message, Docker took the following steps:
 1. The Docker client contacted the Docker daemon.
 2. The Docker daemon pulled the "hello-world" image from the Docker Hub.
    (amd64)
 3. The Docker daemon created a new container from that image which runs the
    executable that produces the output you are currently reading.
 4. The Docker daemon streamed that output to the Docker client, which sent it
    to your terminal.

To try something more ambitious, you can run an Ubuntu container with:
 $ docker run -it ubuntu bash

Share images, automate workflows, and more with a free Docker ID:
 https://hub.docker.com/

For more examples and ideas, visit:
 https://docs.docker.com/get-started/

```



```shell
docker load < weston-ubuntu1804-bionic-dokcer-20220202.tar

docker images

docker tag 6cee39171947 weston-ubuntu1804:latest



```

