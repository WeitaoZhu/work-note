# ARM64 的多核启动流程分析 spin-table

版本和环境信息如下：

> PC平台: ubuntu20.04
>
> Kernel版本: Linux 5.10.111
>
> UBoot版本：v2022.01
>
> GCC版本: GCC_linaro_7.5.0-2019.12
>
> 交叉编译工具链: aarch64-linux-gun-
>
> 代码阅读工具：vim+ctags+cscope

 		对称多处理器（Symmetric Multi-Processor, SMP）系统包含多个处理器，如4核Cortex-A53，并且每个处理器的地位平等。在启动过程中，处理器的地位不是平等的。core0即0号处理器为引导处理器，负责执行引导程序和初始化内核；其他处理器称为从处理器，等待引导处理器完成初始化。引导处理器完成初始化内核后，启动从处理器。



​		引导处理器启动从处理器的方法有3种。

1. 自旋表（spin-table）。
2. 电源状态协调接口（Power State Coordination Interface, PSCI）。
3. ACPI 停车协议（parking-protocol）, ACPI 是高级配置与电源接口（Advanced Configuration and Power Interface）。



​	引导处理器怎么获取从处理器的启动方法呢？大家可以参考内核代码函数**cpu_read_enable_method**，获取方法如下：

1. 不支持ACPI的情况：引导处理器从FDT设备树二进制文件中“cpu”节点的属性"enable-method"读取从处理器的启动方法，可选的方法是自旋表或者PSCI。
2. 支持ACPI的情况：如果固定ACPI描述表（Fixed ACPI Description Table, FADT）设置了允许PSCI的引导标志，那么使用PSCI，否则使用ACPI停车协议。



​	假设使用自旋表启动方法，编译U-Boot程序时需要开启配置宏:

```makefile
CONFIG_ARMV8_SPIN_TABLE
CONFIG_ARMV8_SET_SMPEN
CONFIG_ARMV8_MULTIENTRY
CONFIG_ARMV8_SWITCH_TO_EL1
```

SMP系统的引导过程如下：

![smp_spin_table](.\pic\smp_spin_table.png)

​		从bootloader说起（以uboot为例）：首先，上电后主处理器和从处理器都会启动，执行uboot，从uboot的_start的汇编代码开始执行，主处理器在uboot中欢快的执行后启动内核，进入内核执行，而从处理器会执行到spin_table_secondary_jump中。

```assembly
arch/arm/cpu/armv8/start.S
......
19 .globl  _start
20 _start:
......
120         /*
121          * Could be EL3/EL2/EL1, Initial State:
122          * Little Endian, MMU Disabled, i/dCache Disabled
123          */
124         switch_el x1, 3f, 2f, 1f
125 3:      set_vbar vbar_el3, x0
126         mrs     x0, scr_el3
127         orr     x0, x0, #0xf                    /* SCR_EL3.NS|IRQ|FIQ|EA */
128         msr     scr_el3, x0
129         msr     cptr_el3, xzr                   /* Enable FP/SIMD */
130         b       0f
131 2:      mrs     x1, hcr_el2
132         tbnz    x1, #34, 1f                     /* HCR_EL2.E2H */
133         set_vbar vbar_el2, x0
134         mov     x0, #0x33ff
135         msr     cptr_el2, x0                    /* Enable FP/SIMD */
136         b       0f
137 1:      set_vbar vbar_el1, x0
138         mov     x0, #3 << 20
139         msr     cpacr_el1, x0                   /* Enable FP/SIMD */
140 0:
```

从uboot的start汇编代码可以看出，主从处理器根据当前运行的状态进行相应的初始化配置。

```assembly
arch/arm/cpu/armv8/start.S
......
148 4:      isb
149 
150         /*
151          * Enable SMPEN bit for coherency.
152          * This register is not architectural but at the moment
153          * this bit should be set for A53/A57/A72.
154          */
155 #ifdef CONFIG_ARMV8_SET_SMPEN
156         switch_el x1, 3f, 1f, 1f
157 3:
158         mrs     x0, S3_1_c15_c2_1               /* cpuectlr_el1 */
159         orr     x0, x0, #0x40
160         msr     S3_1_c15_c2_1, x0
161         isb
162 1:
163 #endif
164 
165         /* Apply ARM core specific erratas */
166         bl      apply_core_errata
167 
168         /*
169          * Cache/BPB/TLB Invalidate
170          * i-cache is invalidated before enabled in icache_enable()
171          * tlb is invalidated before mmu is enabled in dcache_enable()
172          * d-cache is invalidated before enabled in dcache_enable()
173          */
174 
175         /* Processor specific initialization */
176         bl      lowlevel_init
177 
178 #if defined(CONFIG_ARMV8_SPIN_TABLE) && !defined(CONFIG_SPL_BUILD)
179         branch_if_master x0, x1, master_cpu  //判断是否为主cpu(core0),是跳转到master_cpu，否则往下走
180         b       spin_table_secondary_jump    //进入从处理器下一级跳转
181         /* never return */
182 #elif defined(CONFIG_ARMV8_MULTIENTRY)
183         branch_if_master x0, x1, master_cpu
184 
185         /*
186          * Slave CPUs
187          */
188 slave_cpu:
189         wfe
190         ldr     x1, =CPU_RELEASE_ADDR
191         ldr     x0, [x1]
192         cbz     x0, slave_cpu
193         br      x0                      /* branch to the given address */
194 #endif /* CONFIG_ARMV8_MULTIENTRY */
195 master_cpu:
196         bl      _main
......
```

从处理器下一级跳转汇编。

```assembly
arch/arm/cpu/armv8/spin_table_v8.S
......
9 ENTRY(spin_table_secondary_jump)
 10 .globl spin_table_reserve_begin
 11 spin_table_reserve_begin:
 12 0:      wfe
 13         ldr     x0, spin_table_cpu_release_addr
 14         cbz     x0, 0b
 15         br      x0
 16 .globl spin_table_cpu_release_addr
 17         .align  3
 18 spin_table_cpu_release_addr:
 19         .quad   0
 20 .globl spin_table_reserve_end
 21 spin_table_reserve_end:
 22 ENDPROC(spin_table_secondary_jump)
```

在spin_table_secondary_jump中：首先会**执行wfe指令，使得从处理器睡眠等待**。如果被唤醒，则从处理器会判断spin_table_cpu_release_addr这个地址是否为０，为０则继续跳转到wfe处继续睡眠，否则跳转到spin_table_cpu_release_addr指定的地址处执行。

**那么spin_table_cpu_release_addr什么时候会被设置呢？**

**答案是：主处理器在uboot中读取设备树的相关节点属性获得，我们来看下如何获得。**

执行路径为：

```shell
do_bootm_linux
->boot_prep_linux
 ->image_setup_linux
  ->image_setup_libfdt
  ->arch_fixup_fdt
   ->spin_table_update_dt
```

在spin_table_update_dt函数中做了几件非常重要的事情：

```c
arch/arm/cpu/armv8/spin_table.c
......
 11 int spin_table_update_dt(void *fdt)
 12 {
 13         int cpus_offset, offset;
 14         const char *prop;
 15         int ret;
 16         unsigned long rsv_addr = (unsigned long)&spin_table_reserve_begin;
 17         unsigned long rsv_size = &spin_table_reserve_end -
 18                                                 &spin_table_reserve_begin;
 19 
 20         cpus_offset = fdt_path_offset(fdt, "/cpus");
 21         if (cpus_offset < 0)
 22                 return -ENODEV;
 23 
 24         for (offset = fdt_first_subnode(fdt, cpus_offset);
 25              offset >= 0;
 26              offset = fdt_next_subnode(fdt, offset)) {
 27                 prop = fdt_getprop(fdt, offset, "device_type", NULL);
 28                 if (!prop || strcmp(prop, "cpu"))
 29                         continue;
 30 
 31                 /*
 32                  * In the first loop, we check if every CPU node specifies
 33                  * spin-table.  Otherwise, just return successfully to not
 34                  * disturb other methods, like psci.
 35                  *////获得enable-method属性，比较属性值是否为 "spin-table"（即是使用自旋表启动方式）
 36                 prop = fdt_getprop(fdt, offset, "enable-method", NULL);
 37                 if (!prop || strcmp(prop, "spin-table"))
 38                         return 0;
 39         }
 40 
 41         for (offset = fdt_first_subnode(fdt, cpus_offset);
 42              offset >= 0;
 43              offset = fdt_next_subnode(fdt, offset)) {
 44                 prop = fdt_getprop(fdt, offset, "device_type", NULL);
 45                 if (!prop || strcmp(prop, "cpu"))
 46                         continue;
 47 				//找到ｃｐｕ节点
     				//重点：设置cpu-release-addr属性值为spin_table_cpu_release_addr的地址！
 48                 ret = fdt_setprop_u64(fdt, offset, "cpu-release-addr",
 49                                 (unsigned long)&spin_table_cpu_release_addr);
 50                 if (ret)
 51                         return -ENOSPC;
 52         }
 53 		//设置设备树的保留内存 ：添加一个内存spin_table_secondary_jump汇编代码的地址范围（这是物理地址）
 54         ret = fdt_add_mem_rsv(fdt, rsv_addr, rsv_size);
 55         if (ret)
 56                 return -ENOSPC;
 57 
 58         printf("   Reserved memory region for spin-table: addr=%lx size=%lx\n",
 59                rsv_addr, rsv_size);
 60 
 61         return 0;
 62 }
```

其实，他做的工作主要有两个：

**１．将即将供内核使用的设备树的cpu节点的cpu-release-addr属性设置为spin_table_cpu_release_addr的地址（这个地址也就是cpu的释放地址）。**

**２．将spin_table_reserve_begin到spin_table_reserve_end符号描述的地址范围添加到设备树的保留内存中。**

**实际上保留的是spin_table_secondary_jump汇编函数的指令代码段和spin_table_cpu_release_addr地址内存，当然保留是为了在内核中不被内存管理使用，这样这段物理内存的数据不会被覆盖丢失。**注意：spin_table_cpu_release_addr地址处被初始化为０（上面汇编19行）。





参考资料

[Booting AArch64 Linux — The Linux Kernel documentation (01.org)](https://01.org/linuxgraphics/gfx-docs/drm/arm64/booting.html)
