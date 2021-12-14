# Ubuntu20.04安装配置GitLab Server

### **介绍**

[GitLab CE](https://gitlab.com/gitlab-org/gitlab-ce)或Community Edition是一个开源应用程序，主要用于托管Git存储库，以及其他与开发相关的功能，如问题跟踪。它旨在使用您自己的基础架构进行托管，并为您的开发团队提供部署内部存储库的灵活性，与用户交互的公共方式，或者为贡献者提供托管自己项目的方法。

GitLab项目使用简单的安装机制在您自己的硬件上设置GitLab实例变得相对简单。在本指南中，我们将介绍如何在虚拟机Ubuntu 20.04上安装和配置GitLab。

进入gitlab.com官方主页 菜单Product -> Intall GitLab

![gitlab.com](.\pic\gitlab.com.png)

<img src=".\pic\gitlab.install.png" alt="gitlab.install" style="zoom:80%;" />

查看“View Install instructions” -> " For Ubuntu 20.04, `arm64` packages are also available. "

### **第1步 - 安装依赖项**

```shell
sudo apt-get update
sudo apt-get install -y curl openssh-server ca-certificates tzdata perl
```

安装`postfix`为了能发送邮件。

```shell
sudo apt-get install -y postfix
```

![postfix_setting](.\pic\postfix_setting.png)

选择“Internet Site” **<*OK*>**继续。

![postfix_mail](.\pic\postfix_mail.png)

"System mail name:" 输入你服务器主机得邮箱名。

### 第2步 - 增加Gitlab版本并安装版本包

```shell
cd /tmp
curl https://packages.gitlab.com/install/repositories/gitlab/gitlab-ee/script.deb.sh | sudo bash

显示结果如下：
bspserver:/tmp$ curl https://packages.gitlab.com/install/repositories/gitlab/gitlab-ce/script.deb.sh | sudo bash
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100  5933  100  5933    0     0   7416      0 --:--:-- --:--:-- --:--:--  7416
Detected operating system as Ubuntu/focal.
Checking for curl...
Detected curl...
Checking for gpg...
Detected gpg...
Running apt-get update... done.
Installing apt-transport-https... done.
Installing /etc/apt/sources.list.d/gitlab_gitlab-ce.list...done.
Importing packagecloud gpg key... done.
Running apt-get update... done.

The repository is setup! You can now install packages.
```

您可以随意检查下载的脚本，以确保您对所需的操作感到满意。您还可以在[此处](https://packages.gitlab.com/gitlab/gitlab-ce/install)找到该脚本的托管版本：

```
less /tmp/script.deb.sh
```

一旦您对脚本的安全性感到满意，请运行安装程序：

```
sudo bash /tmp/script.deb.sh
```

该脚本将设置您的服务器以使用GitLab维护的存储库。这使您可以使用与其他系统软件包相同的软件包管理工具来管理GitLab。完成后，您可以使用以下`apt`命令安装实际的GitLab应用程序：

```
sudo apt install gitlab-ce
```

这将在您的系统上安装必要的组件。

安装显示结果如下：

```shell
bspserver:/tmp$ sudo apt install gitlab-ce
Reading package lists... Done
Building dependency tree       
Reading state information... Done
The following NEW packages will be installed:
  gitlab-ce
0 upgraded, 1 newly installed, 0 to remove and 63 not upgraded.
Need to get 972 MB of archives.
After this operation, 2,637 MB of additional disk space will be used.
Get:1 https://packages.gitlab.com/gitlab/gitlab-ce/ubuntu focal/main amd64 gitlab-ce amd64 14.5.2-ce.0 [972 MB]
Fetched 972 MB in 2min 29s (6,538 kB/s)                                                                                                          
Selecting previously unselected package gitlab-ce.
(Reading database ... 201115 files and directories currently installed.)
Preparing to unpack .../gitlab-ce_14.5.2-ce.0_amd64.deb ...
Unpacking gitlab-ce (14.5.2-ce.0) ...
Setting up gitlab-ce (14.5.2-ce.0) ...
It looks like GitLab has not been configured yet; skipping the upgrade script.

       *.                  *.
      ***                 ***
     *****               *****
    .******             *******
    ********            ********
   ,,,,,,,,,***********,,,,,,,,,
  ,,,,,,,,,,,*********,,,,,,,,,,,
  .,,,,,,,,,,,*******,,,,,,,,,,,,
      ,,,,,,,,,*****,,,,,,,,,.
         ,,,,,,,****,,,,,,
            .,,,***,,,,
                ,*,.
  


     _______ __  __          __
    / ____(_) /_/ /   ____ _/ /_
   / / __/ / __/ /   / __ `/ __ \
  / /_/ / / /_/ /___/ /_/ / /_/ /
  \____/_/\__/_____/\__,_/_.___/
  

Thank you for installing GitLab!
GitLab was unable to detect a valid hostname for your instance.
Please configure a URL for your GitLab instance by setting `external_url`
configuration in /etc/gitlab/gitlab.rb file.
Then, you can start your GitLab instance by running the following command:
  sudo gitlab-ctl reconfigure

For a comprehensive list of configuration options please see the Omnibus GitLab readme
https://gitlab.com/gitlab-org/omnibus-gitlab/blob/master/README.md

Help us improve the installation experience, let us know how we did with a 1 minute survey:
https://gitlab.fra1.qualtrics.com/jfe/form/SV_6kVqZANThUQ1bZb?installation=omnibus&release=14-5
```



**注意事项**

在安装GitLab应用程序中如果报以下错误，我们可以配置国内加速镜像。

报错信息：

```
# apt-get install gitlab-ce
Reading package lists... Done
Building dependency tree
Reading state information... Done
E: Unable to locate package gitlab-ce
```

修改安装脚本

```
sudo vim /etc/apt/sources.list.d/gitlab_gitlab-ce.list
```

OLD

```
deb https://packages.gitlab.com/gitlab/gitlab-ce/ubuntu/ focal main
deb-src https://packages.gitlab.com/gitlab/gitlab-ce/ubuntu/ focal main
```

NEW

```
deb https://mirrors.tuna.tsinghua.edu.cn/gitlab-ce/ubuntu focal main
deb-src https://mirrors.tuna.tsinghua.edu.cn/gitlab-ce/ubuntu focal main
```

再次执行

```
sudo apt update
sudo apt install gitlab-ce
```

参考链接：https://gitlab.com/gitlab-org/gitlab-foss/-/issues/2370

### **第3步 - 编辑GitLab配置文件**

在使用该应用程序之前，需要更新配置文件并运行重新配置命令。首先，打开Gitlab的配置文件：

```
sudo vim /etc/gitlab/gitlab.rb
```

顶部附近是`external_url`配置线。更新它以匹配您的域。更改`http`为`https`以便GitLab会自动将用户重定向到受Let's加密证书保护的站点：

```makefile
## GitLab URL
##! URL on which GitLab will be reachable.
##! For more details on configuring external_url see:
##! https://docs.gitlab.com/omnibus/settings/configuration.html#configuring-the-external-url-for-gitlab
##!
##! Note: During installation/upgrades, the value of the environment variable
##! EXTERNAL_URL will be used to populate/replace this value.
##! On AWS EC2 instances, we also attempt to fetch the public hostname/IP
##! address from AWS. For more details, see:
##! https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/instancedata-data-retrieval.html
external_url 'http://example.com' // 此处修改为您的域名或ip地址 external_url 'http://173.20.14.206' 
```

设置Gitlab邮箱服务配置：

```makefile
### GitLab email server settings
###! Docs: https://docs.gitlab.com/omnibus/settings/smtp.html
###! **Use smtp instead of sendmail/postfix.**

gitlab_rails['smtp_enable'] = true
gitlab_rails['smtp_address'] = "smtp.aliyun.com"
gitlab_rails['smtp_port'] = 465
gitlab_rails['smtp_user_name'] = "weitao.zhu@aliyun.com"
gitlab_rails['smtp_password'] = "your aliyun email password"
gitlab_rails['smtp_domain'] = "aliyun.com"
gitlab_rails['smtp_authentication'] = "login"
gitlab_rails['smtp_enable_starttls_auto'] = true
gitlab_rails['smtp_tls'] = true
gitlab_rails['gitilab_email_from'] = "weitao.zhu@aliyun.com"
user["git_user_email"] = "weitao.zhu@aliyun.com"
```

检查Gitlab邮箱服务配置是否修改成功：

```shell
grep "^[a-Z]" /etc/gitlab/gitlab.rb

bspserver@bspserver-hingetech:/tmp$ sudo grep "^[a-Z]" /etc/gitlab/gitlab.rb
external_url 'http://173.20.14.206'
gitlab_rails['smtp_enable'] = true
gitlab_rails['smtp_address'] = "smtp.aliyun.com"
gitlab_rails['smtp_port'] = 465
gitlab_rails['smtp_user_name'] = "weitao.zhu@aliyun.com"
gitlab_rails['smtp_password'] = "your aliyun email password"
gitlab_rails['smtp_domain'] = "aliyun.com"
gitlab_rails['smtp_authentication'] = "login"
gitlab_rails['smtp_enable_starttls_auto'] = true
gitlab_rails['smtp_tls'] = true
gitlab_rails['gitilab_email_from'] = "weitao.zhu@aliyun.com"
user["git_user_email"] = "weitao.zhu@aliyun.com"
```

保存并关闭文件。运行以下命令重新配置Gitlab：

```shell
sudo gitlab-ctl reconfigure
```

这将使用它可以找到的有关您的服务器的信息初始化GitLab。这是一个完全自动化的过程，因此您不必回答任何提示。

到最后有个root初始密码提示如下：

```shell
Notes:
Default admin account has been configured with following details:
Username: root
Password: You didn't opt-in to print initial root password to STDOUT.
Password stored to /etc/gitlab/initial_root_password. This file will be cleaned up in first reconfigure run after 24 hours.

NOTE: Because these credentials might be present in your log files in plain text, it is highly recommended to reset the password following https://docs.gitlab.com/ee/security/reset_user_password.html#reset-your-root-password.

bspserver@bspserver-hingetech:/tmp$ sudo cat  /etc/gitlab/initial_root_password
# WARNING: This value is valid only in the following conditions
#          1. If provided manually (either via `GITLAB_ROOT_PASSWORD` environment variable or via `gitlab_rails['initial_root_password']` setting in `gitlab.rb`, it was provided before database was seeded for the first time (usually, the first reconfigure run).
#          2. Password hasn't been changed manually, either via UI or via command line.
#
#          If the password shown here doesn't work, you must reset the admin password following https://docs.gitlab.com/ee/security/reset_user_password.html#reset-your-root-password.

Password: nWuzgOFdJkyQawfwpi842UHoW93YE51QK+FK2EkBPDc=

# NOTE: This file will be automatically deleted in the first reconfigure run after 24 hours.

```

利用初始密码登录之前设置`http://173.20.14.206`登录主页IP。

![gitlab.login](.\pic\gitlab.login.png)

成功登录后你便可以选择“Edit profile” -> " Password"修改root的密码。

![gitlab.root](.\pic\gitlab.root.png)

修改成功后自动退出，重新登录。

附上GitLab常用命令：



| 常用命令                                    | 说明                                                    |
| ------------------------------------------- | ------------------------------------------------------- |
| sudo gitlab-ctl reconfigure                 | 重新加载配置，每次修改/etc/gitlab/gitlab.rb文件之后执行 |
| sudo gitlab-ctl status                      | 查看 GitLab 状态                                        |
| sudo gitlab-ctl start                       | 启动 GitLab                                             |
| sudo gitlab-ctl stop                        | 停止 GitLab                                             |
| sudo gitlab-ctl restart                     | 重启 GitLab                                             |
| sudo gitlab-ctl tail                        | 查看所有日志                                            |
| sudo gitlab-ctl tail nginx/gitlab_acces.log | 查看 nginx 访问日志                                     |
| sudo gitlab-ctl tail postgresql             | 查看 postgresql 日志                                    |

### 第4步 - gitlab重置root用户密码

切换目录：

```shell
cd /opt/gitlab/bin
```

执行 ：`sudo gitlab-rails console -e production` 命令 开始初始化密码

```shell
bspserver@bspserver-hingetech:~$ sudo gitlab-rails console -e production
[sudo] password for bspserver: 
Sorry, try again.
[sudo] password for bspserver: 
Sorry, try again.
[sudo] password for bspserver: 
--------------------------------------------------------------------------------
 Ruby:         ruby 2.7.5p203 (2021-11-24 revision f69aeb8314) [x86_64-linux]
 GitLab:       14.5.2 (76ceea558aa) FOSS
 GitLab Shell: 13.22.1
 PostgreSQL:   12.7
--------------------------------------------------------------------------------
Loading production environment (Rails 6.1.4.1)
irb(main):001:0> 
```

输入 `u=User.where(id:1).first` 来查找与切换账号（User.all 可以查看所有用户）

```shell
irb(main):001:0> u=User.where(id:1).first
=> #<User id:1 @root>
irb(main):002:0> u=User.all
=> #<ActiveRecord::Relation [#<User id:1 @root>]>
irb(main):003:0> 
```

通过`u.password='12345678'`设置密码为12345678(这里的密码看自己喜欢)：

```shell
=> #<User id:1 @root>
irb(main):005:0> u.password='12345678'
=> "12345678"
```

通过`u.password_confirmation='12345678'` 再次确认密码

```shell
irb(main):006:0> u.password_confirmation='12345678'
=> "12345678"
```

通过 `u.save!`进行保存（切记切记 后面的 !）

```shell
irb(main):007:0> u.save!
=> true
```

如果看到上面截图中的true ，恭喜你已经成功了，执行 exit 退出当前设置流程即可。

回到gitlab ,可以通过 root/12345678 这一超级管理员账号登录了



