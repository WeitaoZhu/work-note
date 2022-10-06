# ARM64 的多核spin-table方式启动流程分析 

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
//arch/arm/cpu/armv8/spin_table.c
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



先来看一下一个使用自旋表作为启动方式的平台设备树cpu节点：

```c
//arch/arm64/boot/dts/xxx.dtsi:

   cpu@0 {
                        device_type = "cpu";
                        compatible = "arm,cortex-a53";
                        reg = <0x0 0x000>;
                        enable-method = "spin-table";
                        cpu-release-addr = <0x1 0x0000fff8>;
                };
```

可以发现启动方法为spin-table，释放地址初始化为0x10000fff8。这时候释放地址初始值没有任何意义。

那么什么时候释放地址spin_table_cpu_release_addr　的内容不是０呢？

那么我们得回到主处理器流程上来：主处理器设置好了设备树，传递给内核设备树地址之后就要启动内核，启动内核之后，执行初始化工作，执行如下路径：

```c
setup_arch   //arch/arm64/kernel/setup.c:
->smp_init_cpus  //arch/arm64/kernel/smp.c
 ->smp_cpu_setup
  ->cpu_ops[cpu]->cpu_init(cpu)
   ->smp_spin_table_ops->cpu_init  //arch/arm64/kernel/cpu_ops.c
    ->smp_spin_table_cpu_init  //arch/arm64/kernel/smp_spin_table.c
```

我们来看下smp_spin_table_cpu_init函数：

```c
//arch/arm64/kernel/smp_spin_table.c
......
25 static phys_addr_t cpu_release_addr[NR_CPUS];
......
 43 static int smp_spin_table_cpu_init(unsigned int cpu)
 44 {
 45         struct device_node *dn;
 46         int ret;
 47 
 48         dn = of_get_cpu_node(cpu, NULL);
 49         if (!dn)
 50                 return -ENODEV;
 51 
 52         /*
 53          * Determine the address from which the CPU is polling.
 54          */
 55         ret = of_property_read_u64(dn, "cpu-release-addr",
 56                                    &cpu_release_addr[cpu]);
 57         if (ret)
 58                 pr_err("CPU %d: missing or invalid cpu-release-addr property\n",
 59                        cpu);
 60 
 61         of_node_put(dn);
 62 
 63         return ret;
 64 }
```

可以发现，函数读取设备树的cpu-release-addr属性值到cpu_release_addr[cpu]中，cpu_release_addr变量是个NR_CPUS个元素的数组，每个处理器占用一个元素，其实也就是**将之前保存的spin_table_reserve_begin符号的物理地址保存到这个变量中**。

主处理器继续执行流程如下：

```c
start_kernel
->arch_call_rest_init
 ->rest_init
  ->kernel_init,
   ->kernel_init_freeable
    ->smp_prepare_cpus  //arch/arm64/kernel/smp.c
     ->cpu_ops[cpu]->cpu_prepare
      ->smp_spin_table_ops->cpu_init  //arch/arm64/kernel/cpu_ops.c
       ->smp_spin_table_cpu_prepare  //arch/arm64/kernel/smp_spin_table.c
```

我们来看这个函数：

```c
//arch/arm64/kernel/smp_spin_table.c
......
 66 static int smp_spin_table_cpu_prepare(unsigned int cpu)
 67 {
 68         __le64 __iomem *release_addr;
 69 
 70         if (!cpu_release_addr[cpu])
 71                 return -ENODEV;
 72 
 73         /*
 74          * The cpu-release-addr may or may not be inside the linear mapping.
 75          * As ioremap_cache will either give us a new mapping or reuse the
 76          * existing linear mapping, we can use it to cover both cases. In
 77          * either case the memory will be MT_NORMAL.
 78          */  //将释放地址的物理地址映射为虚拟地址
 79         release_addr = ioremap_cache(cpu_release_addr[cpu],
 80                                      sizeof(*release_addr));
 81         if (!release_addr)
 82                 return -ENOMEM;
 83 
 84         /*
 85          * We write the release address as LE regardless of the native
 86          * endianness of the kernel. Therefore, any boot-loaders that
 87          * read this address need to convert this address to the
 88          * boot-loader's endianness before jumping. This is mandated by
 89          * the boot protocol.
 90          */  //将释放地址的物理地址映射为虚拟地址
 91         writeq_relaxed(__pa_symbol(secondary_holding_pen), release_addr);
 92         __flush_dcache_area((__force void *)release_addr,
 93                             sizeof(*release_addr));  //刷数据cache
 94 
 95         /*
 96          * Send an event to wake up the secondary CPU.
 97          */
 98         sev();  //发送事件唤醒从处理器
 99 		//解除映射
100         iounmap(release_addr);
101 
102         return 0;
103 }
```

上面函数主要做两点：

１．91行，cpu的释放地址处写入secondary_holding_pen的地址，由于获得的内核符号是虚拟地址所以转化为物理地址写到释放地址处。

２．98行，唤醒处于wfe状态的从处理器。

我们再次回到从处理器睡眠等待的地方：在汇编函数spin_table_secondary_jump中唤醒后执行，wfe的下几行指令，判断spin_table_cpu_release_addr地址处的内容是否为０，这个时候由于主处理器往这个地址写入了释放地址，所有会执行15行指令，跳转到secondary_holding_pen处执行，请注意：这个地址是物理地址，而且从处理器还没有开启ｍｍｕ，所以从处理器还没有进入虚拟地址的世界。

```assembly
arch/arm64/kernel/head.S：
......
 698         /*
 699          * This provides a "holding pen" for platforms to hold all secondary
 700          * cores are held until we're ready for them to initialise.
 701          */
 702 SYM_FUNC_START(secondary_holding_pen)
 703         bl      el2_setup                       // Drop to EL1, w0=cpu_boot_mode
 704         bl      set_cpu_boot_mode_flag
 705         mrs     x0, mpidr_el1
 706         mov_q   x1, MPIDR_HWID_BITMASK
 707         and     x0, x0, x1
 708         adr_l   x3, secondary_holding_pen_release
 709 pen:    ldr     x4, [x3]
 710         cmp     x4, x0
 711         b.eq    secondary_startup
 712         wfe
 713         b       pen
 714 SYM_FUNC_END(secondary_holding_pen)
 ......
```

在这个函数中又有了一层关卡：705行到708行　判断是否secondary_holding_pen_release被设置为了从处理器的编号，如果设置的不是我的编号，则我再次进入712行执行wfe睡眠等待，行吧，那就等待啥时候主处理器来将secondary_holding_pen_release设置为我的处理器编号吧。那么何时会设置呢？答案是最终要启动从处理器的时候。



我们再次回到主处理器的处理流程，上面主处理器执行到了smp_prepare_cpus之后，继续往下执行：

```c
start_kernel
->arch_call_rest_init
  ->rest_init
    ->kernel_init,
  ->kernel_init_freeable
   ->smp_prepare_cpus //arch/arm64/kernel/smp.c
    ->smp_init  //kernel/smp.c  (这是从处理器启动的函数)
    ->cpu_up
     ->do_cpu_up
      ->_cpu_up
       ->cpuhp_up_callbacks
        ->cpuhp_invoke_callback
         ->cpuhp_hp_states[CPUHP_BRINGUP_CPU]
          ->bringup_cpu
           ->__cpu_up  //arch/arm64/kernel/smp.c
            ->boot_secondary
             ->cpu_ops[cpu]->cpu_boot(cpu)
              ->smp_spin_table_ops.cpu_boot  //arch/arm64/kernel/cpu_ops.c
               ->smp_spin_table_cpu_boot //arch/arm64/kernel/smp_spin_table.c
```

我们来看smp_spin_table_cpu_boot函数：

```c
//arch/arm64/kernel/smp_spin_table.c
......
105 static int smp_spin_table_cpu_boot(unsigned int cpu)
106 {
107         /*
108          * Update the pen release flag.
109          */  //将secondary_holding_pen_release内容写为处理器id
110         write_pen_release(cpu_logical_map(cpu));
111 
112         /*
113          * Send an event, causing the secondaries to read pen_release.
114          */  //唤醒从处理器
115         sev();
116 
117         return 0;
118 }
```

```c

......
726 SYM_FUNC_START_LOCAL(secondary_startup)
 727         /*
 728          * Common entry point for secondary CPUs.
 729          */
 730         bl      __cpu_secondary_check52bitva
 731         bl      __cpu_setup                     // initialise processor
 732         adrp    x1, swapper_pg_dir
 733         bl      __enable_mmu    //加载直接映射页表与swapper页表，打开mmu
 734         ldr     x8, =__secondary_switched
 735         br      x8    //跳转至下一级切换
 736 SYM_FUNC_END(secondary_startup)
 737 
 738 SYM_FUNC_START_LOCAL(__secondary_switched)
 739         adr_l   x5, vectors   //设置从处理器的kernel异常向量表 
 740         msr     vbar_el1, x5
 741         isb
 742 
 743         adr_l   x0, secondary_data
 744         ldr     x1, [x0, #CPU_BOOT_STACK]       // get secondary_data.stack
 745         cbz     x1, __secondary_too_slow
 746         mov     sp, x1
 747         ldr     x2, [x0, #CPU_BOOT_TASK]
 748         cbz     x2, __secondary_too_slow
 749         msr     sp_el0, x2
 750         scs_load x2, x3
 751         mov     x29, #0
 752         mov     x30, #0
 753 
 754 #ifdef CONFIG_ARM64_PTR_AUTH
 755         ptrauth_keys_init_cpu x2, x3, x4, x5
 756 #endif
 757 
 758         b       secondary_start_kernel
 759 SYM_FUNC_END(__secondary_switched)
......
 785 /*
 786  * Enable the MMU.
 787  *
 788  *  x0  = SCTLR_EL1 value for turning on the MMU.
 789  *  x1  = TTBR1_EL1 value
 790  *
 791  * Returns to the caller via x30/lr. This requires the caller to be covered
 792  * by the .idmap.text section.
 793  *
 794  * Checks if the selected granule size is supported by the CPU.
 795  * If it isn't, park the CPU
 796  */
 797 SYM_FUNC_START(__enable_mmu)
 798         mrs     x2, ID_AA64MMFR0_EL1
 799         ubfx    x2, x2, #ID_AA64MMFR0_TGRAN_SHIFT, 4
 800         cmp     x2, #ID_AA64MMFR0_TGRAN_SUPPORTED
 801         b.ne    __no_granule_support
 802         update_early_cpu_boot_status 0, x2, x3
 803         adrp    x2, idmap_pg_dir
 804         phys_to_ttbr x1, x1
 805         phys_to_ttbr x2, x2
 806         msr     ttbr0_el1, x2                   // load TTBR0
 807         offset_ttbr1 x1, x3
 808         msr     ttbr1_el1, x1                   // load TTBR1
 809         isb
 810         msr     sctlr_el1, x0
 811         isb
 812         /*
 813          * Invalidate the local I-cache so that any instructions fetched
 814          * speculatively from the PoC are discarded, since they may have
 815          * been dynamically patched at the PoU.
 816          */
 817         ic      iallu
 818         dsb     nsh
 819         isb
 820         ret
 821 SYM_FUNC_END(__enable_mmu)
```



可以看到这里将从处理器编号写到了secondary_holding_pen_release中，然后唤醒从处理器，从处理器再次欢快的执行，最后执行到secondary_startup，来做从处理器的初始化工作（如设置mmu，异常向量表等），最终从处理器还是处于wfi状态，但是这个时候从处理器已经具备了执行进程的能力，可以用来调度进程，触发中断等，和主处理器有着相同的地位。

咱们总结概述一下spin-table方式的多核启动流程：

1. 从处理器的第一个关卡U-Boot程序中的函数spin_table_secondary_jump，从处理器睡眠等待，被唤醒后，检查全局变量spin_table_cpu_release_addr的值是不是0，如果是0，继续等待睡眠。引导处理器将会把全局变量spin_table_cpu_release_addr的值设置为一个函数的地址。

2. U-Boot程序：引导处理器执行函数boot_prep_linux，为执行内核做准备工作，其中一项准备工作是调用函数spin_table_update_dt，修改FDT设备树二进制文件。

   a. 为每个处理器的“cpu”节点插入一个属性“cpu-release-addr”，把属性值设置为全局变量spin_table_cpu_release_addr的地址，称为处理器放行地址。

   b. 在内存保留区（memory reserve map，对应FDT设备树源文件的字段“/memreserve/”）添加全局变量spin_table_cpu_release_addr的地址。

3. 引导处理器在内核函数smp_cpu_setup中，首先调用函数cpu_read_enable_method以获取从处理器的启动方法，然后调用函数smp_spin_table_cpu_init，从FDT设备树二进制文件中“cpu”节点的属性“cpu-release-addr”得到从处理器的放行地址。

4. 引导处理器执行内核函数smp_spin_table_cpu_prepare，针对每个从处理器，把放行地址设置为函数secondary_holding_pen，然后唤醒从处理器。

5. 从处理器被唤醒，执行secondary_holding_pen，这个函数设置了第二个关卡，当引导处理器把全局变量secondary_holding_pen_release设置为从处理器的编号时，才会放行。

6. 引导处理器完成内核的初始化，启动所有从处理器，针对每个从处理器，调用函数smp_spin_table_cpu_boot，把全局变量secondary_holding_pen_release设置为从处理器的编号。

7. 从处理器发现引导处理器把全局变量secondary_holding_pen_release设置为自己的编号，通过第二个关卡，执行函数secondary_startup。

8. 从处理器执行函数__secondary_switched，把向量基准地址寄存器（VBAR_EL1）设置为异常向量表的起始地址，设置栈指针寄存器，调用C程序的入口函数secondary_start_kernel。

9. 从处理器最终在idle线程中执行wfi睡眠，主处理器继续往下进行内核初始化，直到启动init进程，后面多个处理器都被启动起来，都可以调度进程，多进程还会被均衡到多核。



参考资料

[Booting AArch64 Linux — The Linux Kernel documentation (01.org)](https://01.org/linuxgraphics/gfx-docs/drm/arm64/booting.html)

Linux内核深度解析 作者:*余华兵* 出版社:人民邮电出版社 1.3.3 SMP系统的引导

