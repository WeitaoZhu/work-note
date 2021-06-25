# intel gpu tools

## Usage

 testdisplay_example.sh

## description
* The horizontal and vertical timings are defined per the following diagram.  
*  
*  
*               Active                 Front           Sync           Back  
*              Region                 Porch                          Porch  
*     <-----------------------><----------------><-------------><-------------->  
*       //////////////////////|  
*      ////////////////////// |  
*     //////////////////////  |..................               ................  
*                                                _______________
*     <----- [hv]display ----->  
*     <------------- [hv]sync_start ------------>  
*     <--------------------- [hv]sync_end --------------------->  
*     <-------------------------------- [hv]total ----------------------------->*  

testdisplay –f  <clock MHz>,<hdisp>,<hsync-start>,<hsync-end>,<htotal>, <vdisp>,<vsync-start>,<vsync-end>,<vtotal>
testdisplay -f 57.28,1280,1304,1304,1304,768,776,778,778



[Reference doc: Adding a custom display](https://www.digi.com/resources/documentation/digidocs/90001945-13/reference/yocto/r_an_adding_custom_display.htm)

Typical timing parameters include:

- **Horizontal Back Porch (HBP):** Number of PIXCLK pulses between HSYNC signal and the first valid pixel data.
- **Horizontal Front porch (HFP):** Number of PIXCLK pulses between the last valid pixel data in the line and the next HSYNC pulse.
- **Vertical Back Porch (VBP):** Number of lines (HSYNC pulses) from a VSYNC signal to the first valid line.
- **Vertical Front Porch (VFP):** Number of lines (HSYNC pulses) between the last valid line of the frame and the next VSYNC pulse.
- **VSYNC length:** Number of HSYNC pulses when a VSYNC signal is active.
- **HSYNC length:** Number of PIXCLK pulses when a HSYNC signal is active.
- **Active frame width (hactive):** Horizontal resolution.
- **Active frame height (vactive):** Vertical resolution.

![img](https://www.digi.com/resources/documentation/digidocs/90001945-13/resources/images/android/dwg_lcd_display_signals.jpg)



数字视频的基本概念源自于模拟视频。对于模拟视频我们可以这样理解：视频可以分解为若干个基本视点（像素），每个像素都有独立的色彩信息，在屏幕上依次将 这些点用电子枪按照行和列打出来，就形成了一幅完整画面，连续的打出画面，利用人眼的延迟特点就可以“显示”动态的图像了。

 

水平消隐：电子枪从左到右画出象素，它每次只能画一条扫描线，画下一条之前要先回到左边并做好画下一条扫描线的准备，这之间有一段时间叫做水平消隐（HBlank）。

垂直消隐：在画完全部256条扫描线之后它又回到屏幕左上角准备下一次画屏幕（帧），这之间的一段时间就是垂直消隐（VBlank）。电子枪就是在不断的走‘之’字形。

![img](https://images0.cnblogs.com/blog/139989/201412/041409414833641.png) 

 

![img](https://images0.cnblogs.com/blog/139989/201412/041430517018682.png)

  **行同步（HSYNC****）**：行同步就是让电子枪控制器知道下面要开始新的一行像素

  **场同步（VSYNC**： 场同步就是告诉电子枪控制器下面要开始新的画面

  **数据使能（DE****）**： 在数据使能区是有效的色彩数据，不在使能范围内的都显示黑色。

  **前肩（Front Porch****）/****后肩** **（Back Porch****）** ：行同步或场同步信号发出后，视频数据不能立即使能，要留出电子枪回扫的时间。以行扫描为例，从HSYNC结束到DE开始的区间成为行扫描的后肩（绿色区域），从DE结束到HSYNC开始称为前肩（紫色区域）。同样对于场扫面也可以由类似的定义。

 

Pixel clock：像素时脉(Pixel clock)指的是用来划分进来的影像水平线里的个别画素， Pixel clock 会将每一条水平线分成取样的样本，越高频率的 Pixel clock，每条扫瞄线会有越多的样本画素。

　 HFP: Horizon front porch

　 HBP: Horizon back porch

​     VFP: Vertical front porch

​     VBP: Vertical back porch

　 HDP： Horizon display period

​     VDP:  Vertical display period

 

HTP = HSYNC + HDP + HFP + HBP

VTR = VSYNC + VDP + VFP + VBP

f dot _ clk = pixel clock
f dot _ clk = f v * VTR * HTP
f v = vertical frequency  (这个是频率，平时见到的很多时候是60HZ)

pixel clock = ()
制式      总扫描线    图像区域扫描线    水平总象素    图像区域水平象素    采样频率
1080I/60Hz    1125    1080            2200       1920       74.25MHz
1080I/50Hz    1125    1080            2640       1920       74.25MHz
720P/60Hz    750     720            1650       1280       74.25MHz
720P/50Hz    750     720            1980       1280       74.25MHz

**带宽**：视频带宽代表显示器显示能力的一个综合指标，指每秒钟所扫描的图素个数，即单位时间内每条扫描线上显示的频点数总和，在模拟视频中以MHz为单位，图1的视频模拟带宽计算如下：，



Analog BandWidth=1650x750x60=74.25MHz 含义为每个时钟要传输74.25M个模拟视频数据。所以720p60 的pclk为74.25Mhz,同理1080p60的P CLOCK为148.5MHz



但是在数字视频中由于每个像素都是由3种不同的颜色来表示，每种颜色右由一定数量的比特来传输，因此通常会用bps来表示数字带宽，如果图1中使用了RGB传输，每种颜色用1个字节来输出，那么该视频的数字带宽为：

Digital BandWidth= 模拟带宽*8bit*3=1.782Gbps 含义为每秒要传输1.782G个比特数据
