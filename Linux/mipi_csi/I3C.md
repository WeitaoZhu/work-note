## I3C 总线协议详细解析

 

目前随着手机等移动设备包含的sensor越来越多，传统应用在sensor上的I2C/SPI接口的局限性也越来越明显，典型的缺陷如下：

   1、sensor等设备的增加，对控制总线的速度和功耗提出了更加严苛的要求；

   2、虽然I2C是一种2线接口，但是往往此类device需要额外增加一条中断INT信号线；

   处于解决上述问题的原因，推出了I3C的接口总线和协议，下面一起来看下I3C总线的特性。

一、I3C的应用场景

![img](https://pianshen.com/images/328/17c168d0dc15c76ae3e1f85618facb38.png)

从上图中我们可以看到：

   1、I3C总线可以应用在各种sensor中；

   2、可以使用在任何传统的I2C/SPI/UART等接口的设备中。

二、什么是I3C

![img](https://pianshen.com/images/429/137810e97d81d02fbf70c1065fa9e6dd.png)

I3C吸纳了I2C和SPI的关键特性，并将其统一起来，同时在I2C的基础上，保留了2线的串行接口结构，这样工程师就可以在单个设备中连接大量的传感器。

​    从上图中我们可以将特性具体一下：

​    1、I3C总线可以支持multi-master即多主设备

​    2、I3C总线与传统的I2C设备仍然是兼容的

​    3、可以支持软中断

   4、相比较于I2C总线的功耗更低

   5、速度更快，可以支持到12.5MHZ

   从下图中可以看到在传统的I2C接口设备中包含了太多的I/0口了(碎片式的接口），将之（I2C/SPI)替换成I3C之后可以节省很大部分的信号线（省去了中断信号的一根线EINT，若取代SPI，可以省的更多）的开销，在布局布线时也更方便.

![img](https://pianshen.com/images/335/066bea7f9f1bc60a04214d244306c39f.png)

按照目前MIPI联盟的规划，I3C总线在将来除了应用sensor之外，还有如下的应用领域：camera、TP等

![img](https://pianshen.com/images/445/c2bad7527641be971c16c7ad296975bd.png) 三、I3C接口协议

   ![img](https://pianshen.com/images/106/57e79aa600d433a504a9ed921475dc2a.png)

从上图就可以很清楚的看到I3C总线的应用了，I3C总线中支持多主设备，同时兼容I2C。

其中支持设备的具体类型有：

1、I3C主设备

   -----SDR-only master

2、I3C secondary MASTER

   -----SDR-only secondary master（注意是slave of main master,即相比较主设备而言仍然时从设备）

3、I3C 从设备

  -----SDR only slave

 4、I2C slave

 ![img](https://pianshen.com/images/675/cbaa025439cedbf114b6f1976205cfb3.png)

![img](https://pianshen.com/images/863/c628a3bcef387400c767f948bb469437.png)

 上图为I3C的串行clk和data传输的波形，注意下方的标注：SDA的接口为开漏结构，而SCL的接口为推挽结构！

四、I3C特性详细介绍

1、SDR动态地址分配

---I3C可以为所有的I3C从设备动态的分配7-bit address（注：在I3C从设备中会有两个standardized characteristics register和内部的48-bit的临时ID去协助此过程，具体咋协助俺还不知道）

---仍然支持I2C的静态地址

2、SDR的带内中断

---在“bus available(总线空闲）”的状态下，从设备可以发出“START"请求信号；

---当主设备接收到请求信号后，主设备发出时钟信号并将分配的地址驱动到总线上，然后从设备响应地址（为防止理解问题，英文描述如下）

---如果此时有多个从设备响应中断，那么分配的地址中最低的一个设备将会赢得仲裁

---数据载荷（即强制数据位）可以和带内中断一起使用（？？？未明白，后面填坑）

![img](https://pianshen.com/images/901/668e4219ded912a689d3a37a0b1397bd.png)

3、error detection and recovery methodology（错误检测和恢复方法）

 ---主要针对master 和slave产生的错误（9种错误类型：奇偶性、循环冗余校验CRC5）

4、common command codes(公共命令码）

![img](https://pianshen.com/images/284/02e9f2c61998c698ada987a8ef1836b4.png)

 ![img](https://pianshen.com/images/38/b15c7a847c9d45624bdc94fd06f1cc1e.png)

5、对I2C总线的支持

---支持I2C的fast mode/fast mode+（note:fast speed 1Mbit/s,high speed 3.4Mbit/s)

---对于the velocity of i3c clk 12.5MHZ需要50ns的spike filters(tsp)尖峰滤波器

---I3C不支持clock stretching（时钟拉伸，不了解的童鞋可以查查I2C的协议）

---不使用open-drain驱动器

---不支持10-bit的I2C扩展地址

![img](https://pianshen.com/images/561/8ec02f4ae398cd6c347d1f81dc4a0b89.png)

6、I3C的HDR-DDR 模式

![img](https://pianshen.com/images/544/c51623d4cf0789ab1d3cef2e8f97c060.png)

 ![img](https://pianshen.com/images/378/f3e212c41a3a379a139a98f162997b02.png)

7、I3C的拓扑

![img](https://pianshen.com/images/960/ff242c00a19edcdeda160f660c017840.png)

 

![img](https://pianshen.com/images/230/d502ca77b134d9cd0c7c9e9c570cbc2e.png)

 附：相关知识点

1、clk stretching(时钟拉伸/延展）

​    照例附上I2C协议中关于I2C stretching的介绍，其主要的作用就是在I2C总线进行通信是，其速度是由主设备决定的，与RS232不同，I2C总线会根据预先设定好的波特率在主设备和从设备之间提供一个严格精确的时钟信号，然而在某些时候，I2C的从设备并没有准备好进行通信（翻译很简略，详细见下文），需要慢一点儿，那么实现这一功能的机制就是clock stretching，当slave需要进行clock stetcing的时候，就需要将clk信号拉低以减小总线的速度，同时，作为master而言，需要等待clk被拉高才能继续传输(主设备需要不断的回读）；

   

![img](https://pianshen.com/images/688/bf35468101e8b8558eedb59bee98fad0.png)

 ![img](https://pianshen.com/images/34/7460e06e46e6c8aac6baca7ae99d8e62.png)

版权声明：本文为博主原创文章，遵循[ CC 4.0 BY-SA ](https://creativecommons.org/licenses/by-sa/4.0/)版权协议，转载请附上原文出处链接和本声明。本文链接：https://blog.csdn.net/lxm920714/article/details/103426440

