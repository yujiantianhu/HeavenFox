<h1 align="center">HeavenFox内核原理</h1>
<p align="center">作者: 杨雨俊 (Night-Rain.Yang)</p>

---------------------------------------------------------
__声明__
- HeavenFox为类Linux的操作系统内核，意在使用一种简化的方式来重写linux，在整体框架上接近于linux内核。
- 本项目多处重要代码的实现方式参考自linux内核，但又不尽相同；适合爱好者开发和探索。
- HeavenFox内核始终遵循GPL协议，不论用于哪一种场合，都是开源且自由的。

__信息 （如有需要可邮件联系）__
```
    作者:       杨雨俊 (Yang Yujun)
    创建时间:   2023.12.21
    电子邮箱:   <yujiantianhu@163.com>
    git仓库:    git@github.com:yujiantianhu/HeavenFox.git
```

---------------------------------------------------------
### 1. 目录
- [1. 目录](#1-目录)
- [2. 前言](#2-前言)
  - [2.1. 功能预览](#21-功能预览)
  - [2.2. 测试记录（持续更新）](#22-测试记录持续更新)
- [3. 终端命令](#3-终端命令)
- [4. 源码文件说明](#4-源码文件说明)
- [5. 功能配置文件](#5-功能配置文件)
  - [5.1. configs.h和mach\_configs.h](#51-configsh和mach_configsh)
  - [5.2. mach文件夹](#52-mach文件夹)
- [6. 公共接口](#6-公共接口)
  - [6.1. common](#61-common)
    - [6.1.1. 基本数据类型](#611-基本数据类型)
    - [6.1.2. 通用操作](#612-通用操作)
    - [6.1.3. 字符串通用函数](#613-字符串通用函数)
    - [6.1.4. 数据结构：链表](#614-数据结构链表)
    - [6.1.5. 数据结构：环形缓冲区](#615-数据结构环形缓冲区)
    - [6.1.6. 数据结构：环形队列](#616-数据结构环形队列)
    - [6.1.7. 数据结构：基数树](#617-数据结构基数树)
    - [6.1.8. 数据结构：红黑树](#618-数据结构红黑树)
    - [6.1.9. 内存池](#619-内存池)
  - [6.2. C++](#62-c)
- [7. 内核启动](#7-内核启动)
  - [7.1. 链接脚本](#71-链接脚本)
  - [7.2. 异常向量表](#72-异常向量表)
  - [7.3. 早期初始化](#73-早期初始化)
  - [7.4. 后期初始化](#74-后期初始化)
  - [7.5. 内核跳转](#75-内核跳转)
  - [7.6. start\_kernel](#76-start_kernel)
- [8. 平台架构](#8-平台架构)
  - [8.1. 隐式初始化](#81-隐式初始化)
  - [8.2. 设备树](#82-设备树)
    - [8.2.1. 语法](#821-语法)
    - [8.2.2. dtc编译器](#822-dtc编译器)
    - [8.2.3. dtb](#823-dtb)
    - [8.2.4. 设备节点](#824-设备节点)
    - [8.2.5. of函数](#825-of函数)
  - [8.3. 时钟](#83-时钟)
  - [8.4. 中断](#84-中断)
    - [8.4.1. 中断控制器](#841-中断控制器)
    - [8.4.2. 虚拟中断号](#842-虚拟中断号)
    - [8.4.3. 中断申请与执行](#843-中断申请与执行)
  - [8.5. 定时器](#85-定时器)
    - [8.5.1. 系统定时器](#851-系统定时器)
    - [8.5.2. 高精度定时器](#852-高精度定时器)
  - [8.6. 通知链](#86-通知链)
  - [8.7. 平台设备、驱动与总线](#87-平台设备驱动与总线)
    - [8.7.1. 设备](#871-设备)
    - [8.7.2. 总线与驱动](#872-总线与驱动)
    - [8.7.3. 设备驱动匹配](#873-设备驱动匹配)
  - [8.8. pinctrl](#88-pinctrl)
    - [8.8.1. fwk\_pinctrl\_map](#881-fwk_pinctrl_map)
    - [8.8.2. 控制器（管理单元）](#882-控制器管理单元)
    - [8.8.3. fwk\_pinctrl](#883-fwk_pinctrl)
    - [8.8.4. pinctrl绑定](#884-pinctrl绑定)
  - [8.9. gpio](#89-gpio)
    - [8.9.1. gpio组](#891-gpio组)
    - [8.9.2. gpio引脚](#892-gpio引脚)
    - [8.9.3. gpio中断](#893-gpio中断)
  - [8.10. i2c总线](#810-i2c总线)
    - [8.10.1. 控制器](#8101-控制器)
    - [8.10.2. 设备](#8102-设备)
    - [8.10.3. 驱动与总线](#8103-驱动与总线)
    - [8.10.4. 协议与数据传输](#8104-协议与数据传输)
  - [8.11. 帧缓冲](#811-帧缓冲)
    - [8.11.1. 帧缓冲信息](#8111-帧缓冲信息)
    - [8.11.2. 应用层显示接口](#8112-应用层显示接口)
  - [8.12. 网络](#812-网络)
    - [8.12.1. 网络设备](#8121-网络设备)
    - [8.12.2. 数据包](#8122-数据包)
    - [8.12.3. socket](#8123-socket)
  - [8.13. 终端](#813-终端)
- [9. 文件系统](#9-文件系统)
  - [9.1. 虚拟文件路径](#91-虚拟文件路径)
    - [9.1.1. 节点对象](#911-节点对象)
    - [9.1.2. 节点属性](#912-节点属性)
    - [9.1.3. fwk\_file与文件描述符](#913-fwk_file与文件描述符)
  - [9.2. 字符设备](#92-字符设备)
    - [9.2.1. 设备号](#921-设备号)
    - [9.2.2. fwk\_cdev](#922-fwk_cdev)
    - [9.2.3. 设备节点](#923-设备节点)
    - [9.2.4. 用例](#924-用例)
  - [9.3. 磁盘](#93-磁盘)
    - [9.3.1. 文件操作](#931-文件操作)
    - [9.3.2. 项目](#932-项目)
  - [9.4. 存储设备](#94-存储设备)
    - [9.4.1. 设备号](#941-设备号)
    - [9.4.2. fwk\_gendisk](#942-fwk_gendisk)
    - [9.4.3. 设备节点](#943-设备节点)
- [10. 线程管理](#10-线程管理)
  - [10.1. 线程状态与迁移（一）：状态定义](#101-线程状态与迁移一状态定义)
  - [10.2. 线程控制块](#102-线程控制块)
  - [10.3. 调度控制块“sgtc\_scheduler\_table”](#103-调度控制块sgtc_scheduler_table)
  - [10.4. 线程状态与迁移（二）：优先级排序](#104-线程状态与迁移二优先级排序)
  - [10.5. 线程创建与注销](#105-线程创建与注销)
  - [10.6. 线程状态与迁移（三）：schedule\_thread\_switch](#106-线程状态与迁移三schedule_thread_switch)
  - [10.7. 线程调度](#107-线程调度)
    - [10.7.1. \_\_schedule\_thread](#1071-__schedule_thread)
    - [10.7.2. context\_switch](#1072-context_switch)
    - [10.7.3. 休眠与唤醒](#1073-休眠与唤醒)
    - [10.7.4. 抢占](#1074-抢占)
  - [10.8. 同步与互斥](#108-同步与互斥)
    - [10.8.1. 关中断](#1081-关中断)
    - [10.8.2. 自旋锁](#1082-自旋锁)
    - [10.8.3. 互斥锁](#1083-互斥锁)
    - [10.8.4. 读写锁](#1084-读写锁)
    - [10.8.5. 信号量](#1085-信号量)
  - [10.9. 线程间通信：邮箱](#109-线程间通信邮箱)
  - [10.10. 等待队列](#1010-等待队列)
  - [10.11. 优先级继承](#1011-优先级继承)
  - [10.12. 内核线程](#1012-内核线程)
    - [10.12.1. idle](#10121-idle)
    - [10.12.2. kthread](#10122-kthread)
    - [10.12.3. init\_proc](#10123-init_proc)
    - [10.12.4. kworker](#10124-kworker)
    - [10.12.5. kmemp](#10125-kmemp)
    - [10.12.6. irq\_thread](#10126-irq_thread)

---------------------------------------------------------
### 2. 前言
#### 2.1. 功能预览
```
    01) 支持各级子目录及子文件自由选择是否参与编译, 由"obj-y"链接各源文件和目录;
    02) 支持程序链接脚本自动生成, 适配多种cpu配置;
    03) 支持"lib/"路径下第三方项目源码单独编译, 生成静态库文件并链接到内核镜像;
    04) 支持内核配置自由化, 可通过创建和编写"configs/mach/xxxx_defconfig", 自动生成auto.conf和autoconf.h;
    05) 支持隐式初始化, 允许灵活加入或删除组件;
    06) 提供自定义内存池, 可由kmalloc和kfree申请和释放动态内存;
    07) 支持双向链表, 环形队列/缓冲区, 基数树, 红黑树;
    08) 支持多线程调度机制, 包括线程切换、时间片轮转、优先级抢占、睡眠与唤醒;
    09) 为多线程提供临界资源互斥机制: 等待队列、互斥锁、自旋锁、信号量、读写锁;
    10) 支持线程间通信机制: 邮箱消息;
    11) 默认提供内核线程: 工作队列, 支持自由增删工作事件, 由内核线程轮询调用;
    12) 提供printk打印接口, 允许内核异步打印(由kthread线程统一读取环形缓冲区输出);
    13) 支持串口终端, 提供系统命令"help", "ts", "ttc"等;
    14) 支持高精度定时器延时, 系统定时器中断链表, 允许定时事件自由注册和脱离;
    15) 支持SD卡 (移植Fatfs), 并提供通用文件函数接口, 如: file_open/file_close, ...;
    16) 内嵌dtc编译器, 可通过"make dtbs"或"make all"直接编译设备树文件 (生成.dtb);
    17) 支持设备树, 可从SD卡读取dtb文件并解析为fwk_device_node结构体;
    18) 支持字符设备驱动架构, 如fwk_cdev结构体和fwk_file_oprts操作函数集;
    19) 支持notifier通知链机制, 驱动程序可通过注册通知链访问其他驱动;
    20) 支持总线-设备-驱动匹配机制, 设备可人为注册, 或来自于设备树(转化为fwk_platdev结构体); 通过平台总线与驱动程序匹配;
    21) 支持中断号虚拟化, 即所有硬件中断号均转换为唯一的虚拟中断号; 提供中断服务申请和释放操作, 支持线程化中断下半部;
    22) 支持时钟平台架构, 可通过fwk_clk操作时钟开关;
    23) 支持gpio平台架构, 可通过fwk_gpio_desc读写gpio, 支持gpio中断入口;
    24) 支持cpu引脚在设备树中以pinctrl形式定义, 内核提供pinctrl和gpio通用接口进行解析;
    25) 支持i2c总线架构: fwk_i2c_adapter - fwk_i2c_client - fwk_i2c_driver;
    26) 支持framebuffer驱动架构及显示相关API (支持描点、字库读取、图片解析等);
    27) 支持虚拟文件操作 (用于字符设备驱动), "打开"设备文件后将返回文件描述符fd, 由virt_open/virt_read/...等API操作字符设备;
    28) 支持网络设备驱动架构, 内嵌lwip开源ip协议栈, 并二次封装: 可直接通过sk_buff与net_device结构体传递数据与设备信息, 无需关注lwip实现;
    29) 提供网络回环驱动(loopback)和"ping"命令;
    30) 提供驱动程序: 触摸屏tsc2007驱动、环境传感器ap3216c、eeprom芯片at24c02等; 
    31) 支持lvgl开源图形库, 可直接调取lvgl函数接口设计图形化界面;
    32) 支持C++常规应用接口(如new/delete, cout/endl, string等)
```

内核支持嵌入BSP、HAL底层程序，但应仅供于驱动程序使用，切勿随意拷贝处理器SDK库，以免内核臃肿。目前已支持的CPU型号：
```c
    01) imx6ull
    02) xc7z010
```

#### 2.2. 测试记录（持续更新）
- 内核启动时间
    暂未统计，boot阶段有人为添加延时代码，用于输出上电打印信息。

- 线程切换时间
    约12us。

- ping主机耗时（经网卡）
    约0.293ms

- ping回环耗时
    约0.296ms

---------------------------------------------------------
### 3. 终端命令
    2.1, help
        显示当前支持的所有命令
    2.2, info
        显示作者信息
        info -v: 显示系统程序版本;
        info -m: 显示系统运行的机器型号;
        info -a: 显示系统信息 (版本, 机器型号等)
    2.3, ts
        显示当前的所有线程(含运行中、就绪、睡眠等状态), 显示线程栈、线程号及时间片等信息
    2.4, history
        显示历史命令(从旧到新依次显示)
    2.5, user
        修改用户名. 终端以"login@host: "作为开头
        user login xxx: 将login改为xxx
        user host xxx: 将host改为xxx
    2.6, kill
        给线程发信号
        kill -9 [tid]: 给线程号为tid(1, 2, 3, ...)的线程发"SIGKILL"(数字9), 使该线程进入睡眠(线程睡眠后将进入"死亡"状态, 将被kernel线程回收);
        kill -32 [tid]: 给线程号为tid(1, 2, 3, ...)的线程发"SIGWAKE"(数字9), 唤醒该线程(线程被回收后无法被唤醒)
    2.7, ttc
        给线程发信息. 适用于应用程序(如"example/"文件夹下)
        ttc [tid] [op]: 给线程号为tid(1, 2, 3, ...)的线程发op(操作信息), 需该线程支持该信息的解析(终端线程不负责解析该信息)
        示例 (具体线程号需通过"ts"命令确定):
        light task <线程号为128>: 
                ttc 128 on (灯亮); 
                ttc 128 off (灯灭)
        display task <线程号为130>: 
                ttc 130 --list (查看电子书列表, 同时获得对应序号); 
                ttc 130 open 11 (打开序号为11的电子书, 比如《杀人者唐斩》); 
                ttc 130 down (下一页; 需先打开电子书, 否则不做响应);
                ttc 130 up (上一页; 需先打开电子书, 否则不做响应);
                ttc 130 exit (结束电子书显示, 返回主界面)
        env_monitor_task <线程号为131>:
                ttc 131 info: 显示当前环境信息
                ttc 131 sync: 保存环境信息到EEPROM, 并回显 (读取EEPROM并打印)
    2.8, ifconfig
        查看当前已注册的网络设备, 及收发的数据包数量
    2.9, ping
        发送ICMP协议报文到对端ip地址, 并等待响应. 如: ping 127.0.0.1
    2.10, value
        读写系统程序中已注册的全局变量. 如: value -w 1 g_xxx, 可修改全局变量g_xxx的值为1. 可通过"value --list"查看可以读写的变量列表;
    2.11, mem
        查看系统内存信息
        mem info: 查看当前内存分布(起始地址和大小), 及已分配内存中最接近边界的起始地址;
    2.12, runtime
        查看从系统启动迄今所经过的时间(格式: 年-月-日 时:分:秒:毫秒), 即系统运行时间

---------------------------------------------------------
### 4. 源码文件说明
    boot/:          包含设备树、最终生成的镜像文件, 以及内核启动相关的代码;
    configs/:       可编写的功能配置文件路径;
    build/:         CMake编译生成的中间文件存放处;
    document/:      文档相关;
    scripts/:       工具或通用脚本, 如dtc编译器, jlink scripts, ..., 以及Makefile.build;
    objects/:       编译生成的中间文件, 将存放于此;

    arch/:          包括芯片启动、异常向量表、寄存器/协处理器读写等与CPU架构高度相关的文件;
    board/:         包括外设初始化、CPU通用SDK等单板相关代码;
    common/:        公共、通用的代码, 是本项目最基础的部分;
    drivers/:       驱动程序, 包括CPU自身的资源驱动、单板的资源驱动等;
    example/:       例程;
    fs/:            文件系统相关;
    include/:       头文件统一存放处;
    init/:          内核初始化相关代码, 如main.c;
    kernel/:        内核调度器及多线程相关代码;
    lib/:           第三方库源码;
    platform/:      平台架构相关, 是除了common文件夹之外的第二大基础代码, 提供更多功能的函数接口;
    term:           终端(terminal)相关

---------------------------------------------------------
### 5. 功能配置文件
configs文件夹存放功能相关配置，由两部分组成：
#### 5.1. configs.h和mach_configs.h
存放固定配置，决定整个内核的全局功能。包括：
``` c
/* 默认打印级别 */
#define CONFIG_PRINT_LEVEL                      "7"
/* 使能线程调度 */
#define CONFIG_SCHDULE                          (1)
/* 线程调度方式: 抢占 */
#define CONFIG_PREEMPT                          (1)
/* 线程调度方式: 轮询 */
#define CONFIG_ROLL_POLL                        (1)

/* 抢占启用时关闭轮询 */
#if CONFIG_PREEMPT
#undef  CONFIG_ROLL_POLL
#define CONFIG_ROLL_POLL                        (0)
#endif

/* 内核定时器频率(周期为10ms) */
#define CONFIG_HZ                               (100UL)
/* 多核生效, 用于限制单核运行 */
#define CONFIG_USE_AMP                          (0)
```

#### 5.2. mach文件夹
将根据单板特别设置功能，如架构、芯片型号、是否启用浮点、指令集选择、内存分布，以及platform/驱动启用。每个单板都可以编辑一个专有的配置，编译前通过Makefile动态生成autoconf.h和auto.conf。
以imx6ull为例，假设存在配置文件“mach/imx6ull_toppet_defconfig”，则：
``` shell
make CONFIGS=imx6ull_toppet_defconfig config
```
生成的auto.conf与imx6ull_toppet_defconfig完全一样，其会被顶层Makefile包含，并通过Makefile.build传递给各级子文件夹；
将imx6ull_toppet_defconfig中的“=”转化为宏，则生成autoconf.h，其会被config.h包含，而所有源文件则包含config.h，配置全局生效。

如下为imx6ull_toppet_defconfig：
``` makefile
# Generic
# ---------------------------------------------------------------
CONFIG_ARCH = "arm"
CONFIG_TYPE = "armv7"
CONFIG_CLASS = "cortex-a7"
CONFIG_VERDOR = "nxp"
CONFIG_CPU = "imx6ull"

CONFIG_COMPILER = "arm-none-eabi-"

CONFIG_ARCH_ARMV7 = y
CONFIG_LITTILE_ENDIAN = y

CONFIG_MANUFACTURER = "TOPEET"
CONFIG_BOARD = "TERMINATOR_v1.1"

# float with hardware
CONFIG_VFP = y
# instruction: thumb or arm
CONFIG_INSTRUCTION = arm

# 0: amp; 1: smp
CONFIG_CORE = 0

# debug or release
CONFIG_BUILD_TYPE = release

# -O0, -O1, -O2
CONFIG_OPTIMIZE = 2
# ---------------------------------------------------------------

# System Config
# Systick period = 1ms
CONFIG_HZ = 1000
CONFIG_HRTIMER_ENBALE = n

# ---------------------------------------------------------------
# Console
CONFIG_CONSOLE_DEVICE = "/dev/uart0"
# Use DMA to Receive or Send for Console ?
CONFIG_CONSOLE_RXDMA = n
CONFIG_CONSOLE_TXDMA = n

# Clock
# ---------------------------------------------------------------
# 24MHz
CONFIG_XTAL_FREQ_CLK = (24000000)
# 32.768KHz
CONFIG_RTC_FREQ_CLK = (32768)
# Cpu frequency
#CONFIG_CPU_FREQ = (528000000)
CONFIG_CPU_FREQ = (792000000)
# ---------------------------------------------------------------

# Memory
# ---------------------------------------------------------------
CONFIG_RAM_DDR_BASE = (0x80000000)
CONFIG_RAM_DDR_SIZE = (0x20000000)

# boot
# ---------------------------------------------------------------
# 120MB
CONFIG_PROGRAM_OFFSET = (0x07800000)

# RAM(->DDR) Configuration
# ---------------------------------------------------------------
# 0x80000000 ~ 0x801FFFFF: 2MB, reserved
# 0x80200000 ~ 0x805FFFFF: 4MB, early stack and params(global_data)
# 0x80600000 ~ 0x807FFFFF: 2MB, malloc len (early pool)
# 0x80800000 ~ 0x81FFFFFF: 24MB, params (boot -> kernel), extra memory reserved
# 0x82000000 ~ 0x82000FFF: 4KB, UND, stack
# 0x82001000 ~ 0x82200FFF: 2MB, FIQ, stack
# 0x82201000 ~ 0x82A00FFF: 8MB, IRQ, stack
# 0x82A01000 ~ 0x82A01FFF: 4KB, ABT, stack
# 0x82A02000 ~ 0x83201FFF: 8MB, SVC, stack
# 0x83202000 ~ 0x83401FFF: 2MB, SYS, stack
# 0x83402000 ~ 0x83C01FFF: 8MB, heap
# 0x83C02000 ~ 0x83C807FF: 7.9921875MB, reserved
# 0x84400000 ~ 0x85BFFFFF: 24MB, fixed data (such as fontlib, logo, ...)
# 0x85C00000 ~ 0x85FFFFFF: 4MB, reserved
# 0x86000000 ~ 0x863FFFFF: 4MB, device tree
# 0x86400000 ~ 0x877FEFFF: 19.99609375MB, reserved
# 0x877FF000 ~ 0x87800000: 4KB, IVT and DCD

CONFIG_PROGRAM_BASE = (CONFIG_RAM_DDR_BASE + CONFIG_PROGRAM_OFFSET)
CONFIG_PROGRAM_SIZE = (CONFIG_RAM_DDR_SIZE - CONFIG_PROGRAM_OFFSET)

CONFIG_BOARD_SP_ADDR = (0x80800000)
CONFIG_BOOT_MALLOC_LEN = (2 * 1024 * 1024)

CONFIG_B2K_PARAM_BASE = (CONFIG_BOARD_SP_ADDR)
CONFIG_B2K_PARAM_SIZE = (24 * 1024 * 1024)

CONFIG_DEVICE_TREE_BASE = (0x86000000)
CONFIG_FDT_MAX_SIZE = (4 * 1024 * 1024)

CONFIG_B2K_FIXDATA_BASE = (0x84400000)
CONFIG_B2K_FIXDATA_SIZE = (24 * 1024 * 1024)

# DMA buffer: 16MB
CONFIG_DMA_AREA_SIZE = (16 * 1024 * 1024)

# Framebuffer: 24MB
CONFIG_FB_DRAM_SIZE = (24 * 1024 * 1024)

# Socket buffer: 8MB
CONFIG_NETWORK_BUFF_SIZE = (8 * 1024 * 1024)
# ---------------------------------------------------------------

# kernel
# ---------------------------------------------------------------
# configure stack by .lds
CONFIG_STACK_WITH_LDS = y

CONFIG_SYS_STACK_SIZE = (2 * 1024 * 1024)
CONFIG_SVC_STACK_SIZE = (8 * 1024 * 1024)
CONFIG_ABT_STACK_SIZE = (4 * 1024)
CONFIG_IRQ_STACK_SIZE = (8 * 1024 * 1024)
CONFIG_FIQ_STACK_SIZE = (2 * 1024 * 1024)
CONFIG_UND_STACK_SIZE = (4 * 1024)
CONFIG_HEAP_SIZE      = (8 * 1024 * 1024)

CONFIG_STACK_BASE     = (CONFIG_BOARD_SP_ADDR   + CONFIG_B2K_PARAM_SIZE)
CONFIG_UND_STACK_BASE = (CONFIG_STACK_BASE      + CONFIG_UND_STACK_SIZE)
CONFIG_FIQ_STACK_BASE = (CONFIG_UND_STACK_BASE  + CONFIG_FIQ_STACK_SIZE)
CONFIG_IRQ_STACK_BASE = (CONFIG_FIQ_STACK_BASE  + CONFIG_IRQ_STACK_SIZE)
CONFIG_ABT_STACK_BASE = (CONFIG_IRQ_STACK_BASE  + CONFIG_ABT_STACK_SIZE)
CONFIG_SVC_STACK_BASE = (CONFIG_ABT_STACK_BASE  + CONFIG_SVC_STACK_SIZE)
CONFIG_SYS_STACK_BASE = (CONFIG_SVC_STACK_BASE  + CONFIG_SYS_STACK_SIZE)
CONFIG_HEAP_BASE      = (CONFIG_SYS_STACK_BASE)
# ---------------------------------------------------------------

# Kernel
# ---------------------------------------------------------------
CONFIG_CONTEXT_EX = n
# ---------------------------------------------------------------

# Board
# ---------------------------------------------------------------
CONFIG_LCD_PIXELBIT = (32)
# ---------------------------------------------------------------

# platform
# ---------------------------------------------------------------
CONFIG_OF = y
CONFIG_BLOCK_DEVICE = n
CONFIG_PINCTRL = y
CONFIG_MMC = y
CONFIG_UART = y
CONFIG_I2C = y
CONFIG_DMA = y
CONFIG_INPUTDEV = y
CONFIG_VIDEO = y
CONFIG_USB = y

# support multi framebuffer
CONFIG_FB_MULTI_SUPPORT = n
CONFIG_FBUFFER_NUM = 2
# ---------------------------------------------------------------

# drivers
# ---------------------------------------------------------------
CONFIG_CPU_IMX6 = y
CONFIG_CLOCK_IMX6 = y

# Gpio
CONFIG_GPIO_IMX6 = y
CONFIG_GPIO_IMX_LED = y

# Pinctrl
CONFIG_PINCTRL_IMX6 = y

# input
CONFIG_INPUT_IMX_KEY = y

# uart
CONFIG_UART_IMX6 = y

# i2c
CONFIG_I2C_IMX6 = y
CONFIG_I2C_AP3216C = y
CONFIG_I2C_AT24CXX = y
CONFIG_I2C_TSC2007 = y

# sdma
CONFIG_SDMA_IMX6 = y

# hdmi
CONFIG_HDMI_SIL9022A = y

# sd card
# for imx6ull, there are some errors with 4 bus width 
CONFIG_SDMMC_BUS_WIDTH = 1
# video
CONFIG_VIDEO_FB_IMX6 = y

# usb
CONFIG_USB_IMX6 = y
CONFIG_USB_GADGET_IMX6 = y
CONFIG_USB_IMX_MOUSE = y
# ---------------------------------------------------------------

# example
CONFIG_BUTTON_APP = y
CONFIG_DISPLAY_APP = y
CONFIG_ENV_MONITOR_APP = y
CONFIG_LIGHT_APP = y
CONFIG_TSC_APP = n
```

对于auto.conf：
> 1) 配置文件中值为"y"和"m"的参数, 如"CONFIG_XXX = y", 可在Makefile中通过obj-$(CONFIG_XXX) = aaa.o来指定编译aaa.c到内核中;
> 2) 配置文件中值为"n"的参数, 如"CONFIG_XXX = n", 可在Makefile中通过obj-$(CONFIG_XXX) = aaa.o来指定不编译aaa.c到内核中, 即aaa.c将不会参与编译
obj-xxx由scripts/Makefile.build统一处理

对于autoconf.h：
> 1) 配置文件中值为"y"和"m"的参数, 如"CONFIG_XXX = y", 最终转化为宏: #define CONFIG_XXX 1
> 2) 配置文件中值为"n"的参数, 如"CONFIG_XXX = n", 最终不会转化为宏, 内核代码可通过"#ifdef CONFIG_XXX"或"#if defined(CONFIG_XXX)"来处理它

如imx6ull_toppet_defconfig可转化为：
``` c
#define CONFIG_ARCH "arm"
#define CONFIG_TYPE "armv7"
#define CONFIG_CLASS "cortex-a7"
#define CONFIG_VERDOR "nxp"
#define CONFIG_CPU "imx6ull"
#define CONFIG_COMPILER "arm-none-eabi-"
#define CONFIG_ARCH_ARMV7 1
#define CONFIG_LITTILE_ENDIAN 1
#define CONFIG_MANUFACTURER "TOPEET"
#define CONFIG_BOARD "TERMINATOR_v1.1"
#define CONFIG_VFP 1
#define CONFIG_INSTRUCTION arm
#define CONFIG_CORE 0
#define CONFIG_BUILD_TYPE release
#define CONFIG_OPTIMIZE 2
#define CONFIG_HZ 1000
#define CONFIG_CONSOLE_DEVICE "/dev/uart0"
#define CONFIG_XTAL_FREQ_CLK (24000000)
#define CONFIG_RTC_FREQ_CLK (32768)
#define CONFIG_CPU_FREQ (792000000)
#define CONFIG_RAM_DDR_BASE (0x80000000)
#define CONFIG_RAM_DDR_SIZE (0x20000000)
#define CONFIG_PROGRAM_OFFSET (0x07800000)
#define CONFIG_PROGRAM_BASE (CONFIG_RAM_DDR_BASE + CONFIG_PROGRAM_OFFSET)
#define CONFIG_PROGRAM_SIZE (CONFIG_RAM_DDR_SIZE - CONFIG_PROGRAM_OFFSET)
#define CONFIG_BOARD_SP_ADDR (0x80800000)
#define CONFIG_BOOT_MALLOC_LEN (2 * 1024 * 1024)
#define CONFIG_B2K_PARAM_BASE (CONFIG_BOARD_SP_ADDR)
#define CONFIG_B2K_PARAM_SIZE (24 * 1024 * 1024)
#define CONFIG_DEVICE_TREE_BASE (0x86000000)
#define CONFIG_FDT_MAX_SIZE (4 * 1024 * 1024)
#define CONFIG_B2K_FIXDATA_BASE (0x84400000)
#define CONFIG_B2K_FIXDATA_SIZE (24 * 1024 * 1024)
#define CONFIG_DMA_AREA_SIZE (16 * 1024 * 1024)
#define CONFIG_FB_DRAM_SIZE (24 * 1024 * 1024)
#define CONFIG_NETWORK_BUFF_SIZE (8 * 1024 * 1024)
#define CONFIG_STACK_WITH_LDS 1
#define CONFIG_SYS_STACK_SIZE (2 * 1024 * 1024)
#define CONFIG_SVC_STACK_SIZE (8 * 1024 * 1024)
#define CONFIG_ABT_STACK_SIZE (4 * 1024)
#define CONFIG_IRQ_STACK_SIZE (8 * 1024 * 1024)
#define CONFIG_FIQ_STACK_SIZE (2 * 1024 * 1024)
#define CONFIG_UND_STACK_SIZE (4 * 1024)
#define CONFIG_HEAP_SIZE (8 * 1024 * 1024)
#define CONFIG_STACK_BASE     (CONFIG_BOARD_SP_ADDR   + CONFIG_B2K_PARAM_SIZE)
#define CONFIG_UND_STACK_BASE (CONFIG_STACK_BASE      + CONFIG_UND_STACK_SIZE)
#define CONFIG_FIQ_STACK_BASE (CONFIG_UND_STACK_BASE  + CONFIG_FIQ_STACK_SIZE)
#define CONFIG_IRQ_STACK_BASE (CONFIG_FIQ_STACK_BASE  + CONFIG_IRQ_STACK_SIZE)
#define CONFIG_ABT_STACK_BASE (CONFIG_IRQ_STACK_BASE  + CONFIG_ABT_STACK_SIZE)
#define CONFIG_SVC_STACK_BASE (CONFIG_ABT_STACK_BASE  + CONFIG_SVC_STACK_SIZE)
#define CONFIG_SYS_STACK_BASE (CONFIG_SVC_STACK_BASE  + CONFIG_SYS_STACK_SIZE)
#define CONFIG_HEAP_BASE (CONFIG_SYS_STACK_BASE)
#define CONFIG_LCD_PIXELBIT (32)
#define CONFIG_OF 1
#define CONFIG_PINCTRL 1
#define CONFIG_MMC 1
#define CONFIG_UART 1
#define CONFIG_I2C 1
#define CONFIG_DMA 1
#define CONFIG_INPUTDEV 1
#define CONFIG_VIDEO 1
#define CONFIG_USB 1
#define CONFIG_FBUFFER_NUM 2
#define CONFIG_CPU_IMX6 1
#define CONFIG_CLOCK_IMX6 1
#define CONFIG_GPIO_IMX6 1
#define CONFIG_GPIO_IMX_LED 1
#define CONFIG_PINCTRL_IMX6 1
#define CONFIG_INPUT_IMX_KEY 1
#define CONFIG_UART_IMX6 1
#define CONFIG_I2C_IMX6 1
#define CONFIG_I2C_AP3216C 1
#define CONFIG_I2C_AT24CXX 1
#define CONFIG_I2C_TSC2007 1
#define CONFIG_SDMA_IMX6 1
#define CONFIG_HDMI_SIL9022A 1
#define CONFIG_SDMMC_BUS_WIDTH 1
#define CONFIG_VIDEO_FB_IMX6 1
#define CONFIG_USB_IMX6 1
#define CONFIG_USB_GADGET_IMX6 1
#define CONFIG_USB_IMX_MOUSE 1
#define CONFIG_BUTTON_APP 1
#define CONFIG_DISPLAY_APP 1
#define CONFIG_ENV_MONITOR_APP 1
#define CONFIG_LIGHT_APP 1
```

---------------------------------------------------------
### 6. 公共接口
#### 6.1. common
common文件夹存放公共的、通用的代码，包括：

##### 6.1.1. 基本数据类型
```c
#ifdef __cplusplus
#define mr_nullptr             (nullptr)
#else
#define mr_nullptr             ((void *)0)
#endif

#if !defined(__cplusplus) && !defined(bool)
#define bool                    _Bool
#endif

typedef bool                    kbool_t;

#define mr_to_kbool(x)          ((x) ? NR_TRUE : NR_FALSE)

/* basic types */
typedef signed          char    kint8_t;
typedef unsigned        char    kuint8_t;
typedef                 short   kint16_t;
typedef unsigned        short   kuint16_t;
typedef                 int     kint32_t;
typedef unsigned        int     kuint32_t;
typedef          long   long    kint64_t;
typedef unsigned long   long    kuint64_t;
typedef                 float   kfloat_t;
typedef                 double  kdouble_t;

/* super types */
typedef                 char    kchar_t;
typedef                 char    kbyte_t;
typedef unsigned        char    kubyte_t;
typedef                 char    kbuffer_t;
typedef unsigned        char    kubuffer_t;
typedef                 int     kssize_t;
typedef unsigned        int     kusize_t;
typedef unsigned        long    kuaddr_t;
typedef unsigned        long    kutype_t;
typedef                 long    kstype_t;

#ifndef __va_list__
typedef __builtin_va_list       va_list;
#endif

#define va_start(v,l)           __builtin_va_start(v,l)
#define va_end(v)               __builtin_va_end(v)
#define va_arg(v,l)             __builtin_va_arg(v,l)

#define va_copy(d,s)            __builtin_va_copy(d,s)
```

##### 6.1.2. 通用操作
```c
/* 位操作 (移位) */
#define mr_bit_nr(val, nr)                  /* val << nr */
#define mr_bit(nr)                          /* 1 << nr */
#define mr_mask(val, mask)                  /* val & mask */
#define mr_bit_mask(val, mask, nr)          /* (val << nr) & mask */
#define mr_bit_mask_nr(val, mask, nr)       /* (val << nr) & (mask << nr) */

/* 32位地址读写(addr须保证32位对齐) */
#define mr_writel(data, addr)               /* *addr = data */
#define mr_readl(addr)                      /* *addr */
#define mr_resetl(addr)                     /* *addr = 0 */
#define mr_setbitl(bit, addr)               /* *addr |= bit; bit = 0, 1, 2, 4, 8, ..., 下同 */
#define mr_clrbitl(bit, addr)               /* *addr &= ~bit */
#define mr_getbitl(bit, addr)               /* *addr & bit */
#define mr_getbit_u32(mask, nr, addr)       /* (*addr & mask) >> nr, 一般是移动到最低位, 可用于获取多个位域的字段 */

/* 16位地址读写(addr须保证16位对齐); 操作与32位类似 */
#define mr_writew(data, addr)
#define mr_readw(addr)
#define mr_resetw(addr)
#define mr_setbitw(bit, addr)
#define mr_clrbitw(bit, addr)
#define mr_getbitw(bit, addr)
#define mr_getbit_u16(mask, nr, addr)

/* 8位地址读写(addr须保证8位对齐); 操作与32位类似 */
#define mr_writeb(data, addr)
#define mr_readb(addr)
#define mr_resetb(addr)
#define mr_setbitb(bit, addr)
#define mr_clrbitb(bit, addr)
#define mr_getbitb(bit, addr)
#define mr_getbit_u8(mask, nr, addr)

/* 位判断 */
#define mr_isBitSetl(bit, addr)             /* 32位地址判断(addr必须32位对齐), bit是否置1: (*addr & bit) == bit ? */
#define mr_isBitResetl(bit, addr)           /* 32位地址判断(addr必须32位对齐), bit是否置0: (*addr & bit) == 0 ? */
#define mr_isBitSetw(bit, addr)             /* 16位地址判断(addr必须16位对齐), bit是否置1 */
#define mr_isBitResetw(bit, addr)           /* 16位地址判断(addr必须16位对齐), bit是否置0 */
#define mr_isBitSetb(bit, addr)             /* 8位地址判断(addr必须8位对齐), bit是否置1 */
#define mr_isBitResetb(bit, addr)           /* 8位地址判断(addr必须8位对齐), bit是否置0 */

/* 对齐判断 */
#define mr_align(x, mask)                   /* 将x按"mask"向前对齐(mask = 0, 1, 2, 4, 8, ...) */
#define mr_is_aligned(x, mask)              /* 判断x是否已按"mask"(mask = 0, 1, 2, 4, 8, ...) */
#define mr_align4(x)                        /* 将x按4字节向前对齐, 如x为7, 则向前补到4字节对齐, 返回8 */
#define mr_ralign(x, mask)                  /* 将x按mask字节向后对齐, 如mask为4, x为7, 则向后补到4字节对齐, 返回4 */

/* 32位数组array[num], integer最大值为32 * num */
#define mr_word_offset(integer)             /* 返回32位字偏移, 即第几个数组: integer / 32  */
#define mr_bit_offset(integer)              /* 返回32位偏移, 即32位变量的第几位: integer % 32  */

#define ARRAY_SIZE(arr)                     /* 返回数组大小 */

/* 获取成员在结构体中的偏移 */
#define mr_member_offset(type, member)      ((kusize_t)(&((type *)0)->member))

/* 优先使用编译器内建函数 */
#ifdef __compiler_offsetof
    #define mr_offsetof(type, member)       __compiler_offsetof(type, member)
#else
    #define mr_offsetof(type, member)       mr_member_offset(type, member)
#endif

/* 根据成员地址和偏移量反推第一个成员的地址, 也就是结构体的首地址 */
#define mr_to_parent_handler(ptr, type, member)    \
({    \
    const typeof(((type *)0)->member) *ptr_member = (ptr);    \
    (type *)((char *)ptr_member - mr_offsetof(type, member));    \
})
#define mr_container_of(ptr, type, member)  mr_to_parent_handler(ptr, type, member)

/* 判断number是否为2的幂 */
kbool_t isPower2(kutype_t number);

/* 进制转换, 字符串转换 */
kutype_t dec_to_hex(kchar_t *buf, kutype_t number, kint32_t mode);
kutype_t dec_to_binary(kchar_t *buf, kutype_t number, kint32_t mode);
kint32_t ascii_to_dec(const kchar_t *str);

/* 错误检查 (返回值为NULL, 或者处于错误码内存区<内存最后一页为错误码定义区域>) */
kbool_t isValid(const void *ptr);
kbool_t IS_ERR(const void *ptr);
void *ERR_PTR(kstype_t code);
kstype_t PTR_ERR(const void *ptr);
```

##### 6.1.3. 字符串通用函数
```c
/* 获取整型位数(如1000, 位数为4) */
kusize_t get_integrater_lenth(kuint64_t value);
/* 获取字符串长度(不包括'\0') */
kusize_t get_string_lenth(const void *ptr_src);
/* 字符串拼接*/
void do_string_split(void *ptr_dst, kuint32_t offset, const void *ptr_src);
/* 字符串复制 */
kchar_t *do_string_copy(void *ptr_dst, const void *ptr_src);
/* 复制n个字符串(从第一个字符开始复制) */
kchar_t *do_string_n_copy(void *ptr_dst, const void *ptr_src, kuint32_t size);
/* 反向复制n个字符串(从最后一个字符开始复制) */
kchar_t *do_string_n_copy_rev(void *ptr_dst, const void *ptr_src, kuint32_t size);
/* 复制n个字符串(从第一个字符开始复制), 遇到'\0'停止 */
kuint32_t do_string_n_copy_safe(void *ptr_dst, const void *ptr_src, kuint32_t size);
/* 比较两个字符串 */
kbool_t do_string_compare(void *ptr_dst, const void *ptr_src);
/* 比较n个字符串 */
kbool_t do_string_n_compare(void *ptr_dst, const void *ptr_src, kuint32_t size);
/* 字符串反转 */
void do_string_reverse(void *ptr_src, kuint32_t size);
/* 无符号整型转字符串 */
kusize_t uint_to_str(void *ptr_dst, kuint64_t value);
/* 有符号整型转字符串(若为负数, 字符串含'-') */
kusize_t int_to_str(void *ptr_dst, kint64_t value);
/* 查找字符 */
kchar_t *seek_char_in_string(const void *ptr_src, kchar_t ch);
/* 获取第index个字符 */
kchar_t *seek_char_by_pos(const void *ptr_src, kuint32_t index);

/* 以下为字符串格式解析(%d, %s, ...) */
kusize_t do_fmt_convert(void *ptr_buf, kubyte_t *ptr_level, const kchar_t *ptr_fmt, va_list ptr_list, kusize_t size);
kchar_t *vasprintk_safe(const kchar_t *ptr_fmt, kusize_t *size, va_list sptr_list);
kchar_t *lv_vasprintk_safe(const kchar_t *ptr_fmt, kusize_t *size, kubyte_t *ptr_lv, va_list sptr_list);
kint32_t vasprintk(void *ptr_buf, const kchar_t *ptr_fmt, va_list sptr_list);
kint32_t sprintk(void *ptr_buf, const kchar_t *ptr_fmt, ...);
kchar_t *sprintk_safe(const kchar_t *ptr_fmt, ...);
void fmt_free(kchar_t *ptr);

/*!
 * 内核支持3种消息打印方式
 * printk: 使用内部栈保存解析后的格式数据, 无内存分配的时间损耗; 在Terminal线程初始化后, printk仅保存消息到环形缓冲区, 最后由kthread线程异步输出;
 * kprintf: 从内存池分配内存, 保存格式解析后的消息. 基本不用担心消息过长被截断的问题; 与printk一样, 由kthread线程异步输出. 申请内存时可能睡眠, 切勿用于中断上下文;
 * print_sync: 使用内部栈保存解析后的格式数据, 与printk不同的是, print_sync会立即输出消息到终端, 适用于Hard Fault发生时输出错误信息及寄存器状态;
 */
void printk(const kchar_t *ptr_fmt, ...);
void kprintf(const kchar_t *ptr_fmt, ...);
void print_sync(const kchar_t *ptr_fmt, ...);

/*!
 * 三种打印方式均支持可变格式化参数: %d, %s, %c, %u, %x, %p等, 并根据日志级别选择是否输出到终端;
 * 当配置日志级别时, printk和print_sync会获取当前时间戳, 伴随消息打印, 以便定位对应时刻; kprintf也支持日志级别, 但不会打印时间戳.
 * 支持以下日志级别(默认为7级) */
#define PRINT_LEVEL_SOH                                 "\001"
#define PRINT_LEVEL_EMERG                               PRINT_LEVEL_SOH "0"
#define PRINT_LEVEL_ALERT                               PRINT_LEVEL_SOH "1"
#define PRINT_LEVEL_CRIT                                PRINT_LEVEL_SOH "2"
#define PRINT_LEVEL_ERR                                 PRINT_LEVEL_SOH "3"
#define PRINT_LEVEL_WARNING                             PRINT_LEVEL_SOH "4"
#define PRINT_LEVEL_NOTICE                              PRINT_LEVEL_SOH "5"
#define PRINT_LEVEL_INFO                                PRINT_LEVEL_SOH "6"
#define PRINT_LEVEL_DEBUG                               PRINT_LEVEL_SOH "7"

/* 类标准库接口 */
kuint32_t kstrlen(const kchar_t *__s);
kchar_t *kstrcpy(kchar_t *__dest, const kchar_t *__src);
kchar_t *kstrncpy(kchar_t *__dest, const kchar_t *__src, kusize_t __n);
kusize_t kstrlcpy(kchar_t *__dest, const kchar_t *__src, kusize_t __n);
kchar_t *kstrncpyr(kchar_t *__dest, const kchar_t *__src, kusize_t __n);
kint32_t kstrcmp(kchar_t *__s1, const kchar_t *__s2);
kint32_t kstrncmp(kchar_t *__s1, const kchar_t *__s2, kusize_t __n);
kchar_t *kstrchr(const kchar_t *__s1, kchar_t ch);
kchar_t *kstrcat(const kchar_t *__s1, kuint32_t index);
```

在libcxplus.h中为C++定义了string类，也可通过string类来操作字符串。string类支持“=”，“+=”， “==”，“>”，“<”等运算符重载，方便字符串快速赋值和比较；或者直接调用string类的成员函数。

##### 6.1.4. 数据结构：链表
```c
/* 单向链表 */
struct list
{
    struct list *sptr_next;
};

/* 双向链表 */
struct list_head
{
    struct list_head *sptr_prev;
    struct list_head *sptr_next;
};

/* 用于定义链表时初始化为自环状态 */
#define LIST_HEAD_INIT(list)    \
{    \
    .sptr_prev = (list),    \
    .sptr_next = (list),    \
}
/* 定义链表指针并初始化为自环 */
#define DECLARE_LIST_HEAD(list)                                 struct list_head list = LIST_HEAD_INIT(&list)
/* 定义链表指针并初始化为NULL */
#define DECLARE_LIST_HEAD_PTR(ptr_list)                         struct list_head *ptr_list = mr_nullptr;
/* 获取链表项 */
#define mr_next_list_head(head, list)                           (list = ((list)->sptr_next != head) ? (list)->sptr_next : mr_nullptr)
/* 判断链表是否为空(即是否处于自环状态) */
#define mr_list_head_empty(head)                                (((ptr_list)->sptr_prev == (ptr_list)) && ((ptr_list)->sptr_next == (ptr_list)))

#define mr_list_first_entry(head, type, member)                 mr_container_of((head)->sptr_next, type, member)
#define mr_list_last_entry(head, type, member)                  mr_container_of((head)->sptr_prev, type, member)
#define mr_list_next_entry(pos, member)                         mr_container_of((pos)->member.sptr_next, typeof(*(pos)), member)
#define mr_list_prev_entry(pos, member)                         mr_container_of((pos)->member.sptr_prev, typeof(*(pos)), member)

#define mr_list_head_until(pos, head, member)                   ((pos)->member.sptr_next == (head))
#define mr_list_first_valid_entry(ptr, type, member)            (mr_list_head_empty(ptr) ? mr_nullptr : mr_list_first_entry(ptr, type, member))

/* 向后遍历 */
#define foreach_list_next_entry(pos, head, member)    \
    for (pos = mr_list_first_entry(head, typeof(*pos), member);    \
         &(pos->member) != (head);    \
         pos = mr_list_next_entry(pos, member))

#define foreach_list_prev_entry(pos, head, member)    \
    for (pos = mr_list_last_entry(head, typeof(*pos), member);    \
         &(pos->member) != (head);    \
         pos = mr_list_prev_entry(pos, member))

/* 向后遍历, 但安全 (便于在循环内释放pos, 为避免pos释放后无法获取下一个, 先用temp指向下一个) */
#define foreach_list_next_entry_safe(pos, temp, head, member)    \
    for (pos = mr_list_first_entry(head, typeof(*pos), member),    \
         temp = mr_list_next_entry(pos, member);    \
         &(pos->member) != (head);    \
         pos = temp, temp = mr_list_next_entry(temp, member))

/* get list and prev list, and then delete current list from list_head */
#define foreach_list_prev_entry_safe(pos, temp, head, member)    \
    for (pos = mr_list_last_entry(head, typeof(*pos), member),    \
         temp = mr_list_prev_entry(pos, member);    \
         &(pos->member) != (head);    \
         pos = temp, temp = mr_list_prev_entry(temp, member))

/* 初始化链表(自环: sptr_prev = sptr_next = sptr_list) */
void init_list_head(struct list_head *sptr_list);
/* 新增链表项到链表头的下一个位置(紧邻链表头的右侧) */
void list_head_add_head(struct list_head *sptr_head, struct list_head *sptr_list);
/* 新增链表项到链表头的上一个位置(紧邻链表头的左侧, 即整个链表的尾部) */
void list_head_add_tail(struct list_head *sptr_head, struct list_head *sptr_list);
/* 删除链表头右侧的第一个链表项 */
void list_head_del_head(struct list_head *sptr_head);
/* 删除链表头左侧的第一个链表项(即删除整个链表的最后一个链表项) */
void list_head_del_tail(struct list_head *sptr_head);
/* 删除链表项(左右侧的链表项连接, 本链表项自环) */
void list_head_del(struct list_head *sptr_list);
/* 移动所有的链表项到另一个链表头的下一个位置(紧邻链表头的右侧) */
void list_head_split_head(struct list_head *sptr_dst, struct list_head *sptr_src);
/* 移动所有的链表项到另一个链表头的上一个位置(紧邻链表头的左侧, 即整个链表的尾部) */
void list_head_split_tail(struct list_head *sptr_dst, struct list_head *sptr_src);
/* list_head_split_tail + init_list_head, 可用于初始化sptr_src */
void list_head_splice_init(struct list_head *sptr_dst, struct list_head *sptr_src);
```
##### 6.1.5. 数据结构：环形缓冲区
使用结构体“struct pq_buffer”来描述一个环形缓冲区：
```c
struct pq_buffer
{
    kint32_t type;                      /* 可选"NR_PQ_BUF_DROP"(有终点的缓冲区, 非环形)和"NR_PQ_BUF_RING"(环形缓冲区) */
    void *buf_start;                    /* 缓冲区的起始地址 */

    kint32_t head;                      /* 缓冲区的头部指针, 每存入一个字节, head + 1; 超出缓冲区边界时归0, 再度循环 */
    kint32_t tail;                      /* 缓冲区的尾部指针, 每取出一个字节, tail + 1; 超出缓冲区边界时归0, 再度循环 */
    kint32_t len;                       /* 已存入缓冲区的字节数 */

    kusize_t tot_len;                   /* 缓冲区的总大小. 当"len == tot_len"时, 缓冲区满 */
};
```

使用以下API可完成缓冲区的创建、写入、读出，以及销毁：
```c
/* 创建缓冲区, 大小为buf_len */
struct pq_buffer *pq_buffer_create(kint32_t type, kusize_t buf_len);
/* 销毁缓冲区 */
void pq_buffer_destroy(struct pq_buffer *sptr_pq);
/* 将大小为len的data写入到缓冲区 */
kssize_t pq_buffer_write(struct pq_buffer *sptr_pq, const void *data, kusize_t len);
/* 从缓冲区读取最大为len的data, 并返回实际读取长度 */
kssize_t pq_buffer_read(struct pq_buffer *sptr_pq, void *buffer, kusize_t len);
```

单纯使用pq_buffer_write无法对写入的每个data进行界定，比如：“12345”，“abcdef”，这明显是两组数据，但写入缓冲区后可能是“12345abcdef”；
当然，我们可以将结束符'\0'也一起写入，变成“12345\0abcdef\0”，但读出时仍然繁琐，你怎么知道读到哪算一组呢？如果每次读取都先使用strlen来获取一组数据的长度，那将带来不必要的时间损耗。
我们引入一个新的结构体，用来描述一组完整的数据：
```c
struct pq_message
{
    kusize_t len;                       /* data的大小 */
    kubyte_t data[0];                   /* data的内容 */
};
```

写入缓冲区时，先将data的长度写入，再写入data；读缓冲区时，先将data的长度读出，再读取指定长度的data。
可将缓冲区按“struct pq_message”进行解析，读完一次后偏移struct pq_message::len，再读取下一个data即可。
有两个函数专门负责此操作：
```c
kssize_t pq_message_write(struct pq_buffer *sptr_pq, const void *data, kusize_t len);
kssize_t pq_message_read(struct pq_buffer *sptr_pq, void *data, kusize_t len);
```

##### 6.1.6. 数据结构：环形队列
与环形缓冲区类似，但环形队列主要存储数据指针，而非一整个数据块，所以也并不需要确定数据块的大小。
使用结构体struct pq_queue来描述一个队列：
```c
struct pq_data
{
    /* sptr_pqd释放, 用于队列溢出或队列整体销毁时, 使用此回调释放数据 */
    void (*release)(struct pq_data *sptr_pqd);
    /* 用于检查外部缓冲区是否有足够空间承载出队的数据, 由pq_dequeue_with_chk函数调用 */
    kbool_t (*dequeue_chk)(struct pq_data *sptr_pqd, kusize_t limit);
};

typedef struct pq_queue
{
    kint32_t type;                      /* 可选"NR_PQ_DROP"(有终点的队列, 非环形)和"NR_PQ_RING"(环形队列) */

    kint32_t head;                      /* 队列的头部指针, 每存入一个字节, head + 1; 超出队列边界时归0, 再度循环 */
    kint32_t tail;                      /* 队列的尾部指针, 每取出一个字节, tail + 1; 超出队列边界时归0, 再度循环 */
    kint32_t len;                       /* 已存入队列的项数 */

    kusize_t tot_len;                   /* 队列的总大小. 当"len == tot_len"时, 队列满 */
    struct pq_data *sptr_data[];        /* 队列的本体, 总长度为tot_len, 需要从内存池手动分配 */

} srt_pq_t;
```

由于sptr_data成员为不定长数组，若要将队列定义为全局变量，需要再封装一层结构体，以保证sptr_data所占的内存不会被用于其他对象。
可定义结构体如下：
```c
struct xxx
{
    struct pq_queue sgtc_pq;
    kuint32_t *data[1024];              /* data与sgtc_pq->sptr_data为同一区域, 队列长度tot_len为1024 */
};
```

使用data[1024]隔开后，sptr_data就是一个定长数组了。
内核提供了队列相关的API：
```c
/* 创建队列, 并为sptr_data成员分配长度为data_len的内存 (type可选"NR_PQ_DROP"/"NR_PQ_RING") */
struct pq_queue *pq_queue_create(kint32_t type, kusize_t data_len);
/* 创建环形队列, 即: pq_queue_create(NR_PQ_RING, data_len) */
struct pq_queue *ring_queue_create(kusize_t data_len);
/* 销毁队列, 同时会释放sptr_pq->sptr_data成员 */
void pq_queue_destroy(struct pq_queue *sptr_pq);
/* 将参数sptr_data入队, 保存sptr_data指针到sptr_pq->sptr_data[sptr_pq->head] */
kint32_t pq_enqueue(struct pq_queue *sptr_pq, struct pq_data *sptr_data);
/* 出队, 读出sptr_pq->sptr_data[sptr_pq->tail] */
void *pq_dequeue(struct pq_queue *sptr_pq);
/* 先检查外部缓冲区的大小(limit)是否足够, 是则出队, 否则不处理 */
void *pq_dequeue_with_chk(struct pq_queue *sptr_pq, kusize_t limit);
/* 从sptr_pq->sptr_data[*base]开始往回读 (仅读取, 不出队. 比如从head开始, 不断减1), 并更新base为下一个要读的位置 */
void *pq_lookback(struct pq_queue *sptr_pq, kint32_t *base);
/* 从sptr_pq->sptr_data[*base]开始往前读 (仅读取, 不出队. 比如从tail开始, 不断加1), 并更新base为下一个要读的位置 */
void *pq_lookfront(struct pq_queue *sptr_pq, kint32_t *base);
/* base禁止小于0 (一般只有第一次读取时, 会初始化为base为-1, 以便直接从head或tail开始读) */
void *pq_lookfront_next(struct pq_queue *sptr_pq, kint32_t *base);
/* 获取当前队列的实时数量 (sptr_pq->len) */
kint32_t pq_queue_get_size(struct pq_queue *sptr_pq);
```

##### 6.1.7. 数据结构：基数树
假设我们定义一个结构体指针数组：
```c
static struct xxx *array[1024];
```
但实际我们仅需要前10个位置，那剩下的1014个位置将造成浪费。基数树可根据实际使用动态扩展，它起初不存在任何节点，当需要注册一个数字时，将为该数字建立一个节点。
一个节点的直接子节点数量通常固定，以确定分支的数量。通常用数字的二进制后缀标记分支，如果仅用最低位标记, 则0b0和0b1分别可作为一个分支，每个节点的最大分支数即为2；如果用最低两位标记，则0b00、0b01、0b10、0b11就有4种组合，最大分支数即为4。
<blockquote>
<pre>
            根节点(悬空, 无实际作用)
                |
    -------------------------
    |       |       |       |   
   0b00    0b01    0b10    0b11
   (分支1) (分支2) (分支3) (分支4)
</pre>
</blockquote>

如数字12（0x0c）最低两位为0b00，插入基数树后将挂接到分支1，而数字13会挂接到分支2。当新数字16（0x10）要插入时，就会插入到数字12的子节点分支1中，不断动态增长。
HeavenFox内核分支数量定义为8，即取低3位作为标记：0b000、0b001、0b010、0b011、0b100、0b101、0b110、0b111
```c
/* 分支数: 2 ^ 3 */
#define MAX_BRANCH                                      (3)

/* 基数树引用对象 */
typedef struct radix_link 
{
    kuint32_t depth;

} srt_radix_link_t;

/* 基数树节点 */
typedef struct radix_node 
{
    /* 父节点 */
    struct radix_node *sptr_parent;
    /* 子节点分支 */
    struct radix_node *sgtc_branches[1 << MAX_BRANCH];

    /* 引用对象 (由使用基数树的结构体定义并提供) */
    struct radix_link *sptr_link;

} srt_radix_node_t;

/* 基数树 */
typedef struct radix_tree 
{
    kuint16_t (*get) (kuint32_t);               /* 确定数字应处于哪个分支 */
    void *(*alloc) (kusize_t size);             /* 内存分配函数, 如kmalloc */
    void (*free) (void *ptr);                   /* 内存释放函数, 如kfree */

    struct radix_node sgtc_node;                /* 根节点, 占位用 */
    struct spin_lock sgtc_lock;                 /* 内置自旋锁 */

} srt_radix_tree_t;

/* 申请一个节点(调用sptr_tree->alloc并初始化). sptr_par: 父节点 */
struct radix_node *allocate_radix_node(struct radix_tree *sptr_tree, struct radix_node *sptr_par);
/* 查找节点. 需沿着分支比较 */
struct radix_node *find_radix_node(struct radix_tree *sptr_tree, kuint32_t number);
/* find_radix_node成功后, 返回引用对象sptr_link */
struct radix_link *radix_tree_look_up(struct radix_tree *sptr_tree, kuint32_t number);
/* 根据number创建一个新节点, 并注册到基数树; 保存sptr_link到新节点 */
void radix_tree_add(struct radix_tree *sptr_tree, kuint32_t number, struct radix_link *sptr_link);
/* 删除number处的节点; 若节点存在分支, 则节点保留, 清除sptr_link成员 */
void radix_tree_del(struct radix_tree *sptr_tree, kuint32_t number);
```

##### 6.1.8. 数据结构：红黑树
内核支持使用红黑树，以便排序和查找。一个红黑树节点使用结构体“struct rbt_node”来表示：
```c
/*!< 节点颜色 */
#define RBT_NODE_RED                                0x00
#define RBT_NODE_BLACK                              0x01

struct rbt_node 
{
    struct rbt_node *sptr_parent;                   /*!< 父节点 (如果一个节点没有父节点, 说明它是一个根节点; 如果一个节点的父节点是节点本身, 说明这是一个无效节点) */
    struct rbt_node *sptr_left;                     /*!< 左子节点 */
    struct rbt_node *sptr_right;                    /*!< 右子节点 */

    kuint8_t color;                                 /*!< 当前节点的颜色 (红与黑) */
};

/*!< Root node: 单独构造一个结构体, 以便快速定位和管理 */
struct rbt_root
{
    /*!< Root node must be black */
    struct rbt_node *sptr_node;
};
```

支持常见的旋转、插入、删除，以及遍历操作，使用以下API：
```c
/* 初始化一个新节点 */
void rbt_node_init(struct rbt_node *sptr_node);

/* 左旋, 常见于右子树高于左子树的情况; 此时父节点将成为其右子节点的左子节点 */
void rbt_node_left_rotate(struct rbt_root *sptr_root, struct rbt_node *sptr_node);
/* 右旋, 常见于左子树高于右子树的情况; 此时父节点将成为其左子节点的右子节点 */
void rbt_node_right_rotate(struct rbt_root *sptr_root, struct rbt_node *sptr_node);

/* 若sptr_base非NULL, 则查找以sptr_base为中心的子树的最小节点; 否则查找整棵树(以sptr_root为中心)的最小节点 */
struct rbt_node *rbt_get_first(struct rbt_root *sptr_root, struct rbt_node *sptr_base);
/* 若sptr_base非NULL, 则查找以sptr_base为中心的子树的最大节点; 否则查找整棵树(以sptr_root为中心)的最大节点 */
struct rbt_node *rbt_get_last(struct rbt_root *sptr_root, struct rbt_node *sptr_base);
/* 获取sptr_node的后继节点 */
struct rbt_node *rbt_next_node(struct rbt_node *sptr_node);
/* 获取sptr_node的前驱节点 */
struct rbt_node *rbt_prev_node(struct rbt_node *sptr_node);

/* 插入一个新节点后进行颜色调整 */
void rbt_insert_color(struct rbt_root *sptr_root, struct rbt_node *sptr_node);
/* 删除一个旧节点后进行颜色调整 */
void rbt_erase_color(struct rbt_root *sptr_root, struct rbt_node *sptr_parent, struct rbt_node *sptr_child);
/* 插入一个新节点(不涉及颜色变更); sptr_pos为插入的位置, 一般为左/右分支的地址 */
void rbt_add_node(struct rbt_node *sptr_node, struct rbt_node *sptr_parent, struct rbt_node **sptr_pos);
/* 删除一个旧节点, 同时调用rbt_erase_color完成颜色调整 */
void rbt_del_node(struct rbt_root *sptr_root, struct rbt_node *sptr_node);
/* 节点替换, 不需要变更颜色, 只要新节点颜色设置成与旧节点一样即可 */
void rbt_replace_node(struct rbt_root *sptr_root, struct rbt_node *sptr_old, struct rbt_node *sptr_new);
```

通过下述方法可以快速定义一个红黑树节点：
```c
#define INIT_RBT_NODE(node) \
    {   \
        .sptr_parent = (node),    \
        .sptr_left = mr_nullptr,    \
        .sptr_right = mr_nullptr,   \
        .color = RBT_NODE_BLACK,    \
    }

#define DECLARE_RBT_NODE(name)  \
    struct rbt_node name = INIT_RBT_NODE(&name)
```

新节点的颜色始终设置为红色，且父节点指向自身，表示未被用于任何树。
一般rbt_node会被定义具体对象的成员，如：
```c
struct xxx {
    kint32_t key;
    struct rbt_node sgtc_node;
};
```

使用mr_container_of，可通过sgtc_node的地址获得结构体xxx的首地址：
```c
#define mr_rbt_entry(pos, member)                   mr_container_of((pos)->member, typeof(*(pos)), member)
```

为此衍生出更多便利的操作方法：
```c
/*! @note 获取红黑树第一个节点的父对象 */
#define mr_rbt_first_entry(head, type, member)   \
({  \
    struct rbt_node *_node = rbt_get_first(head, mr_nullptr);   \
    _node ? mr_container_of(_node, type, member) : mr_nullptr;  \
})

/*! @note 获取红黑树最后一个节点的父对象 */
#define mr_rbt_last_entry(head, type, member)   \
({  \
    struct rbt_node *_node = rbt_get_last(head, mr_nullptr);   \
    _node ? mr_container_of(_node, type, member) : mr_nullptr;  \
})

/*! @note 获取对象pos的下一个对象, 即红黑树的下一个节点的父对象 */
#define mr_rbt_next_entry(pos, head, member)   \
({  \
    struct rbt_node *_node = rbt_next_node(&(pos)->member);   \
    _node ? mr_container_of(_node, typeof(*(pos)), member) : mr_nullptr;  \
})

/*! @note 获取对象pos的上一个对象, 即红黑树的上一个节点的父对象 */
#define mr_rbt_prev_entry(pos, head, member)   \
({  \
    struct rbt_node *_node = rbt_prev_node(&(pos)->member);   \
    _node ? mr_container_of(_node, typeof(*(pos)), member) : mr_nullptr;  \
})

/*! @note 后向遍历：获取红黑树每一个节点的父对象; 从根节点开始 */
#define foreach_next_rbt_node(pos, head, member)  \
    for (pos = mr_rbt_first_entry(head, typeof(*(pos)), member);   \
         pos;  \
         pos = mr_rbt_next_entry(pos, head, member))

/*! @note 前向遍历：获取红黑树每一个节点的父对象; 从根节点开始 */
#define foreach_prev_rbt_node(pos, head, member)  \
    for (pos = mr_rbt_last_entry(head, typeof(*(pos)), member);   \
         pos;  \
         pos = mr_rbt_prev_entry(pos, head, member))

/*!
 * @brief   插入新节点
 * @param   pos: 对象指针
 * @param   head: 根节点地址
 * @param   key_member: 对象的键值成员
 * @param   node_member: 对象的红黑树节点成员
 */
#define mr_rbt_insert_node(pos, head, key_member, node_member)  \
do {    \
    struct rbt_node **_node = &((head)->sptr_node);    \
    struct rbt_node *_parent = mr_nullptr;  \
    \
    while (_node && (*_node)) {  \
        typeof(*(pos)) *_pos = mr_container_of(*_node, typeof(*(pos)), node_member);    \
        _parent = *_node;  \
        \
        if ((pos)->key_member < _pos->key_member)  \
            _node = &((*_node)->sptr_left);   \
        else if ((pos)->key_member > _pos->key_member) \
            _node = &((*_node)->sptr_right);  \
        else    \
            _node = mr_nullptr; \
    }   \
    \
    if (_node) {    \
        rbt_add_node(&(pos)->node_member, _parent, _node); \
        rbt_insert_color(head, &(pos)->node_member);  \
    }   \
} while (0)

/*!
 * @brief   删除旧节点
 * @param   pos: 对象指针
 * @param   head: 根节点地址
 * @param   node_member: 对象的红黑树节点成员
 */
#define mr_rbt_delete_node(pos, head, node_member)  \
do {    \
    if (mr_likely(pos))    \
        rbt_del_node(head, &(pos)->node_member);    \
} while (0)

/*!
 * @brief   根据键值从红黑树中查找对应节点父对象
 * @param   key: 键值
 * @param   head: 根节点地址
 * @param   type: 父对象的变量类型
 * @param   key_member: 对象的键值成员
 * @param   node_member: 对象的红黑树节点成员
 */
#define mr_rbt_find_entry(key, head, type, key_member, node_member)   \
({    \
    struct rbt_node *_node = (head)->sptr_node;  \
    type *_found = mr_nullptr;  \
    \
    while (_node) { \
        type *_pos = mr_container_of(_node, type, node_member);    \
        \
        if ((key) < _pos->key_member)    \
            _node = _node->sptr_left; \
        else if ((key) > _pos->key_member)   \
            _node = _node->sptr_right;    \
        else {   \
            _found = _pos;    \
            break;  \
        }   \
    }   \
    \
    _found;    \
})
```

##### 6.1.9. 内存池
系统将内存中除代码段、数据段等已知的section外，将剩余的部分视为内存池。比如内存大小为256MB，编译后程序各段（包括.text、.data、.stack、.heap等）共占100MB，那剩下的156MB将全部作为内存池。
即，内存池并非代码指定的固定大小，而是由程序实际占用来决定；一般来说，内存剩余的空间远超出已使用的空间，故内存池空间也十分富裕，基本不用担心内存不足引起的申请失败问题。
Heavenfox的内存池并不会事先切割成指定大小的内存块，而是用到的时候才会将所需要的内存分割出去。
直接定义成内存块容易造成资源浪费，如事先定义32字节、128字节、256字节大小的块，当申请129字节的内存时，可能要直接分掉一个256字节的块。
Heavenfox的内存池起初是一块空白区域，内核启动时在空白区的首端添加一段信息头，结构如下：
```c
#define MEMORY_POOL_MAGIC                      (0xdfa69fe3)

typedef struct mem_block
{
    kuint32_t magic;                            /*!< 魔数, 初始化后恒等于 "MEMORY_POOL_MAGIC" */
    kuaddr_t base;                              /*!< 内存块的起始地址 = 内存区起始地址 + MEM_BLOCK_HEADER_SIZE */
    kusize_t lenth;                             /*!< 内存块的大小, 包括MEM_BLOCK_HEADER_SIZE */
    kusize_t remain;                            /*!< 内存块的剩余空间 (可分配空间). 未分配时等于lenth */

    struct mem_block *sptr_prev;                /*!< 上一个内存块的地址 */
    struct mem_block *sptr_next;                /*!< 下一个内存块的地址 */

    struct list_head sgtc_link;                 /*!< 哈希散列表 (链表项) */

} srt_mem_block_t;

#define IS_MEMORYPOOL_VALID(this)               ((this)->magic == MEMORY_POOL_MAGIC)
#define MEM_BLOCK_HEADER_SIZE                   (mr_align(sizeof(struct mem_block), ARCH_PER_SIZE))  /*!< 32bytes */
```

首次申请内存后，lenth保持不变，remain减去申请的内存大小，表示可用于下一次分配的空间；此时内存区域与未分配前基本无变化；
第二次申请内存，由于地址base已经被上一次分配，本次分配将从“base - MEM_BLOCK_HEADER_SIZE + lenth - remain”开始，即刚好跳过之前分配占用的部分；在此处插入一个新的信息头，令：
```c
struct mem_block *sptr_new;
kusize_t header_size, offset;
void *ptr_mem;

/* 信息头的大小 */
header_size = MEM_BLOCK_HEADER_SIZE;

/* 本次要分配的内存大小, 8字节对齐 */
lenth  = mr_num_align8(size);
lenth += header_size;

/* sptr_block为上一个内存块的信息头 */
offset  = (sptr_block->lenth - header_size - sptr_block->remain) + header_size;
ptr_mem = (void *)((kuint8_t *)sptr_block->base + offset);

/* 新内存块 */
sptr_new = (struct mem_block *)((kuint8_t *)ptr_mem - header_size);
sptr_new->base = (kuaddr_t)ptr_mem;
/* 新内存块的总大小, 即上一个内存块的剩余空间 */
sptr_new->lenth = sptr_block->remain;
sptr_new->remain = sptr_block->remain - lenth;
/* 连接上一个内存块 */
sptr_new->sptr_prev = sptr_block;
/* 连接下一个内存块 */
sptr_new->sptr_next = sptr_block->sptr_next;
sptr_new->magic = MEMORY_POOL_MAGIC;
```

两个内存块由插入的信息头进行分割，sptr_new此时表示除第一个内存块外，内存池剩余的总空间；而第一个内存块自我缩减，将其remain成员减为0：
```c
sptr_block->lenth = offset;
sptr_block->remain -= sptr_new->lenth;

if (sptr_block->sptr_next)
    sptr_block->sptr_next->sptr_prev = sptr_new;

sptr_block->sptr_next = sptr_new;
```

两次分配后，内存池呈现两个内存块，一个没有可用空间，另一个则代表剩余的内存池；当第三次分配时，直接将第二个内存块的remain全部交给新内存块，而它自己的remain减为0；依次类推，随着内存不断申请，原先的内存池就会出现许多内存块，而每个内存块的起始位置都有一个“struct mem_block”信息头，通过其成员sptr_prev、sptr_next进行连接。
当某个内存块用完后，需要归还到内存池，它会根据sptr_prev找到上一个内存块，然后跟它合并，即sptr_prev->remain增加，并且将当前内存块从链表中移除。当多个连续内存块归还内存池时，sptr_prev的剩余空间会逐渐增大，而当各个内存块都具有足够剩余空间时，优先从地址最低的内存块开始分配，以避免大批量连续分配后引发内存耗尽的问题。
要查找地址最低的内存块，则内存块必须构建有序链表，使低地址在前；但内存块数量较多时，需要一直遍历，颇为耗时。HeavenFox将具有剩余空间的内存块通过哈希散列表进行连接，规定每个散列表中内存块的基础大小。由枚举来定义：
```c
enum __ERT_MEM_TYPE
{
    NR_MEM_1Bytes = 0,
    NR_MEM_2Bytes,
    NR_MEM_4Bytes,
    NR_MEM_8Bytes,
    NR_MEM_16Bytes,

    NR_MEM_LowerBytes = NR_MEM_16Bytes,         /*!< [0, 31] */
    NR_MEM_32Bytes,                             /*!< [32, 63] */
    NR_MEM_64Bytes,                             /*!< [64, 127] */
    NR_MEM_128Bytes,                            /*!< [128, 255] */
    NR_MEM_256Bytes,                            /*!< [256, 511] */
    NR_MEM_512Bytes,                            /*!< [512, 1023] */
    NR_MEM_1024Bytes,                           /*!< [1024, 2047] */
    NR_MEM_2048Bytes,                           /*!< [2048, 4095] */
    NR_MEM_4096Bytes,                           /*!< [4096, 8191] */
    NR_MEM_8192Bytes,                           /*!< [8192, ...] */
    NR_MEM_HighBytes = NR_MEM_8192Bytes,

    NR_MEM_NUM,

    NR_MEM_LowerLimit = 1U << (NR_MEM_LowerBytes + 1),
    NR_MEM_HighLimit = 1U << NR_MEM_HighBytes,
};

typedef struct mem_hash
{
    struct list_head sgtc_list;

} srt_mem_hash_t;
```

一个完整的内存池使用结构体“struct mem_info”表示：
```c
typedef struct mem_info
{
    /* 内存池的起始地址和大小 */
    kuaddr_t base;                                          
    kusize_t lenth;
    
    /* 第一个内存块 */
    struct mem_block *sptr_mem;

    /* 全局哈希散列表, 连接每个内存块的sgtc_link成员 */
    struct mem_hash sgtc_hash[NR_MEM_NUM];

    void *(*alloc)(struct mem_info *sptr_info, kusize_t size, kint32_t __align_of, struct m_area *sptr_real);
    void (*free)(struct mem_info *sptr_info, void *ptr_mem);

} srt_mem_info_t;
```

比如内存块a的剩余大小为60字节，则连接到sgtc_hash[NR_MEM_32Bytes].sgtc_list中；而如果是大于8kB，则连接到sgtc_hash[NR_MEM_HighBytes].sgtc_list。如果我们希望申请一个大小为150字节的内存块（含MEM_BLOCK_HEADER_SIZE），则优先到sgtc_hash[NR_MEM_128Bytes]查看，如果非空，则逐个比较，找到第一个剩余空间大于150字节的内存块并返回；如果为空，或者没有找到，则到sgtc_hash[NR_MEM_256Bytes]查找，依次类推，找到为止。最差的情况，可在sgtc_hash[NR_MEM_HighBytes]找到。
当然，这种查找方法仍在存在弊端，后期可以考虑使用红黑树代替。
找到的内存块，便可按照前文的方法，构建一个sptr_new，即分割出一个新的内存块。

Heavenfox根据用途不同，规定了几个内存池：
```c
enum __ERT_FWK_MEMPOOL_INDEX
{
    NR_FWK_MEMPOOL_KERNEL = 0,
    NR_FWK_MEMPOOL_DMA,
    NR_FWK_MEMPOOL_SK_BUFF,
    NR_FWK_MEMPOOL_FB_DRAM,
    NR_FWK_MEMPOOL_FIXDATA,
    NR_FWK_MEMPOOL_TYPE_MAX,
};
#define FWK_AREA_OFFSET                         (24U)
#define FWK_AREA_BIT(x)                         (1U << ((x) + FWK_AREA_OFFSET))

typedef enum nrt_gfp
{
    NR_KMEM_ZERO = mr_bit(0),
    NR_KMEM_WAIT = mr_bit(1),
    NR_KMEM_NOWAIT = 0,

    NR_KMEM_NORMAL = FWK_AREA_BIT(NR_FWK_MEMPOOL_KERNEL),       /*!< memory for kernel heap */
    NR_KMEM_DMA_AREA = FWK_AREA_BIT(NR_FWK_MEMPOOL_DMA),        /*!< memory for dma */
    NR_KMEM_FBUFFER = FWK_AREA_BIT(NR_FWK_MEMPOOL_FB_DRAM),     /*!< memory for framebuffer */
    NR_KMEM_FIXDATA = FWK_AREA_BIT(NR_FWK_MEMPOOL_FIXDATA),     /*!< memory for fixed data */
    NR_KMEM_SK_BUFF = FWK_AREA_BIT(NR_FWK_MEMPOOL_SK_BUFF),     /*!< memory for sk_buff */

    NR_KMEM_KERNEL = NR_KMEM_WAIT | NR_KMEM_NORMAL,
    NR_KMEM_ATOMIC = NR_KMEM_NOWAIT | NR_KMEM_NORMAL,
    NR_KMEM_DRAM   = NR_KMEM_WAIT | NR_KMEM_FBUFFER,
    NR_KMEM_FIXED  = NR_KMEM_NOWAIT | NR_KMEM_FIXDATA,

    NR_KMEM_DMA = NR_KMEM_DMA_AREA,
    NR_KMEM_SOCK = NR_KMEM_SK_BUFF,

} nrt_gfp_t;

#define GFP_ZERO                                NR_KMEM_ZERO
#define GFP_KERNEL                              NR_KMEM_KERNEL
#define GFP_ATOMIC                              NR_KMEM_ATOMIC
#define GFP_DMA                                 NR_KMEM_DMA
#define GFP_DRAM                                NR_KMEM_DRAM
#define GFP_FIXED                               NR_KMEM_FIXED
#define GFP_SOCK                                NR_KMEM_SOCK

struct fwk_mem_info
{
    struct mem_info sgtc_info;
    struct m_area sgtc_maxrec;
};

struct fwk_mempool
{
    const kchar_t *name;
    kuint32_t mask;
    struct fwk_mem_info *sptr_mn;

    struct wait_queue_head sgtc_wqh;
    struct spin_lock sgtc_lock;
};

static struct fwk_mem_info sgtc_mempool_info[NR_FWK_MEMPOOL_TYPE_MAX] = {};

static struct fwk_mempool sgtc_kernel_mempool[NR_FWK_MEMPOOL_TYPE_MAX] =
{
    [NR_FWK_MEMPOOL_KERNEL] = {
        .name = "kernel heap",
        .mask = NR_KMEM_NORMAL,
        .sptr_mn = &sgtc_mempool_info[NR_FWK_MEMPOOL_KERNEL],
    },

    [NR_FWK_MEMPOOL_DMA] = {
        .name = "dma",
        .mask = NR_KMEM_DMA_AREA,
        .sptr_mn = &sgtc_mempool_info[NR_FWK_MEMPOOL_DMA],
    },

    [NR_FWK_MEMPOOL_SK_BUFF] = {
        .name = "network",
        .mask = NR_KMEM_SK_BUFF,
        .sptr_mn = &sgtc_mempool_info[NR_FWK_MEMPOOL_SK_BUFF],
    },

    [NR_FWK_MEMPOOL_FB_DRAM] = {
        .name = "framebuffer",
        .mask = NR_KMEM_FBUFFER,
        .sptr_mn = &sgtc_mempool_info[NR_FWK_MEMPOOL_FB_DRAM],
    },

    [NR_FWK_MEMPOOL_FIXDATA] = {
        .name = "fixed data",
        .mask = NR_KMEM_FIXDATA,
        .sptr_mn = &sgtc_mempool_info[NR_FWK_MEMPOOL_FIXDATA],
    },
};
```

除“kernel heap”大小不固定外，其他几个都是固定空间，用来特定的场合（如网络报文、帧缓冲等），它们分配内存都使用统一的接口：
```c
void *kmalloc(size_t __size, nrt_gfp_t flags);
void *kcalloc(size_t __size, size_t __n, nrt_gfp_t flags);
void *kzalloc(size_t __size, nrt_gfp_t flags);
void kfree(void *__ptr);
```

而使用哪个内存池，由参数“nrt_gfp_t flags”决定，见枚举结构“enum nrt_gfp”。

#### 6.2. C++
HeavenFox支持使用C++开发应用程序，且不依赖C++标准库。new、delate、cout、cin等常用接口均由内核重载实现。

---------------------------------------------------------
### 7. 内核启动
#### 7.1. 链接脚本
内核的链接脚本通用，由公共的链接脚本文件linkscript.lds.S定义。以下为其局部：
```c
/* 头文件, 提供宏定义 */
#include <configs/mach_configs.h>

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

/*!< Program base address (DDR) */
#if (defined(CONFIG_RAM_DDR_BASE) && defined(CONFIG_RAM_DDR_SIZE) && \
     defined(CONFIG_PROGRAM_BASE) && defined(CONFIG_PROGRAM_SIZE))

_RAM_DDR_BASE  = CONFIG_RAM_DDR_BASE;
_RAM_DDR_SIZE  = CONFIG_RAM_DDR_SIZE;
_PROGRAM_START = CONFIG_PROGRAM_BASE;
_PROGRAM_SIZE  = CONFIG_PROGRAM_SIZE;

#else
/*!<
 * Example: DDR's total size is 512MB
 * size = 512MB = 536870912(Bytes), origin = 0x80000000, end = 0xA0000000
 * 0x80000000 ~ 0x87ffffff is used as memory space, and program basic address needs to be after 0x88000000
 * lenth = (0xA0000000 - 0x88000000) = 0x18000000
 */
_RAM_DDR_BASE  = 0x80000000;
_RAM_DDR_SIZE  = 0x20000000;
_PROGRAM_START = 0x88000000;
_PROGRAM_SIZE  = 0x18000000;
#endif

/*!< Reserve Memory */
#ifdef CONFIG_RESERVE_MEMORY_SIZE
_RESERVE_MEMORY_SIZE = CONFIG_RESERVE_MEMORY_SIZE;
#else
/*!< 4MB (4K = 1page, 4MB = 1024pages) */
_RESERVE_MEMORY_SIZE = 0x400000;
#endif

/*!< 0xA0000000 - 0x400000 */
_PROGRAM_END = (_RAM_DDR_BASE + _RAM_DDR_SIZE - _RESERVE_MEMORY_SIZE);

#ifdef CONFIG_SVC_STACK_SIZE
_SVC_STACK_SIZE = CONFIG_SVC_STACK_SIZE;
#else
/*!< 2MB */
_SVC_STACK_SIZE = 2 * 1024 * 1024;
#endif

#ifdef CONFIG_IRQ_STACK_SIZE
_IRQ_STACK_SIZE = CONFIG_IRQ_STACK_SIZE;
#else
/*!< 2MB */
_IRQ_STACK_SIZE = 2 * 1024 * 1024;
#endif

/*!< DMA BUffer Size */
#ifdef CONFIG_DMA_AREA_SIZE
_DMA_BUFF_SIZE = CONFIG_DMA_AREA_SIZE;
#else
/*!< 16MB */
_DMA_BUFF_SIZE = 16 * 1024 * 1024;
#endif

/*!< framebuffer ram */
#ifdef CONFIG_FB_DRAM_SIZE
_FB_DRAM_SIZE = CONFIG_FB_DRAM_SIZE;
#else
/*!< 
 * Default: support three buffer with 1080p
 * 1080p = 1920 * 1080 * 4 ≈ 8MB, three buffer is 24MB
 * 2K = 2560 * 1440 * 4 = 14MB
 * 4K = 3840 * 2160 * 4 = 31.65MB
 * therefore, support at less 2K
 */
_FB_DRAM_SIZE = 24 * 1024 * 1024;
#endif

/*!< NetWork Tx/Rx Buffer */
#ifdef CONFIG_NETWORK_BUFF_SIZE
_NETWORK_BUFF_SIZE = CONFIG_NETWORK_BUFF_SIZE;
#else
/*!< 8MB */
_NETWORK_BUFF_SIZE = 8 * 1024 * 1024;
#endif

/*!< RAM DDR Configuration  */
MEMORY
{
#if (defined(CONFIG_PROGRAM_BASE) && defined(CONFIG_PROGRAM_SIZE))
    ram_ddr_0 : ORIGIN = CONFIG_PROGRAM_BASE, LENGTH = CONFIG_PROGRAM_SIZE
#else
    ram_ddr_0 : ORIGIN = 0x88000000, LENGTH = 0x18000000
#endif
}

/*!< Sections  */
ENTRY(_start)
SECTIONS
{
    . = _PROGRAM_START;
    . = ALIGN(16);

    __ram_ddr_start = .;

    .text :
    {
        . = ALIGN(16);

        /* 异常向量表入口 */
        __vector_table = .;

        KEEP(*(.vectors))
        KEEP(*(.start))
        KEEP(*(.head))
        *(.text)
        *(.text.*)
    } > ram_ddr_0

    __text_sec_end = .;

    .init : 
    {
        KEEP (*(.init))
    } > ram_ddr_0

    .fini : 
    {
        KEEP (*(.fini))
    } > ram_ddr_0

    /* 隐式初始化区域 */
    .dync_init : 
    {
        . = ALIGN(16);
        __dync_init_start = .;
        *(.dync_init)
        __dync_init_0_start = .;
        KEEP(*(.dync_init.0*))
        __dync_init_1_start = .;
        KEEP(*(.dync_init.1*))
        __dync_init_2_start = .;
        KEEP(*(.dync_init.2*))
        __dync_init_3_start = .;
        KEEP(*(.dync_init.3*))
        __dync_init_4_start = .;
        KEEP(*(.dync_init.4*))
        __dync_init_5_start = .;
        KEEP(*(.dync_init.5*))
        __dync_init_6_start = .;
        KEEP(*(.dync_init.6*))
        __dync_init_7_start = .;
        KEEP(*(.dync_init.7*))
        __dync_init_8_start = .;
        KEEP(*(.dync_init.8*))
        __dync_init_9_start = .;
        KEEP(*(.dync_init.9*))
        __dync_init_9_end = .;
        __dync_init_end = .;
    } > ram_ddr_0

    .dync_exit : 
    {
        . = ALIGN(16);
        __dync_exit_start = .;
        *(.dync_exit)
        __dync_exit_0_start = .;
        KEEP(*(.dync_exit.0*))
        __dync_exit_1_start = .;
        KEEP(*(.dync_exit.1*))
        __dync_exit_2_start = .;
        KEEP(*(.dync_exit.2*))
        __dync_exit_3_start = .;
        KEEP(*(.dync_exit.3*))
        __dync_exit_4_start = .;
        KEEP(*(.dync_exit.4*))
        __dync_exit_5_start = .;
        KEEP(*(.dync_exit.5*))
        __dync_exit_6_start = .;
        KEEP(*(.dync_exit.6*))
        __dync_exit_7_start = .;
        KEEP(*(.dync_exit.7*))
        __dync_exit_8_start = .;
        KEEP(*(.dync_exit.8*))
        __dync_exit_9_start = .;
        KEEP(*(.dync_exit.9*))
        __dync_exit_9_end = .;
        __dync_exit_end = .;
    } > ram_ddr_0

    __dync_sec_end = .;

    . = ALIGN(16);
    .rodata :
    {
        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        __rodata_end = .;
    } > ram_ddr_0

    . = ALIGN(16);
    .data :
    {
        __data_start = .;
        *(.data)
        *(.data.*)
        __data_end = .;
    } > ram_ddr_0

    .preinit_array : 
    {
        __preinit_array_start = .;
        KEEP (*(SORT(.preinit_array.*)))
        KEEP (*(.preinit_array))
        __preinit_array_end = .;
    } > ram_ddr_0

    /* C++全局构造函数 */
    .init_array : 
    {
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;
    } > ram_ddr_0

    .fini_array : 
    {
        __fini_array_start = .;
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array))
        __fini_array_end = .;
    } > ram_ddr_0

    .sdata :
    {
        __sdata_start = .;
        *(.sdata)
        *(.sdata.*)
        __sdata_end = .;

    } > ram_ddr_0

    .tdata :
    {
        __tdata_start = .;
        *(.tdata)
        *(.tdata.*)
        __tdata_end = .;

    } > ram_ddr_0

    . = ALIGN(16);
    .bss (NOLOAD) :
    {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    } > ram_ddr_0

    /*!< DMA内存池 */
    .dma_area (NOLOAD) :
    {
        . = ALIGN(16);
        __dma_area_start = .;
        . += _DMA_BUFF_SIZE;
        __dma_area_end = .;
    } > ram_ddr_0

    /*!< 帧缓冲内存池 */
    .fb_dram (NOLOAD) :
    {
        . = ALIGN(16);
        __fb_dram_start = .;
        . += _FB_DRAM_SIZE;
        __fb_dram_end = .;
    } > ram_ddr_0

    /*!< 网络缓冲区内存池 */
    .network_buffer (NOLOAD) :
    {
        . = ALIGN(16);
        __sk_buffer_start = .;
        . += _NETWORK_BUFF_SIZE;
        __sk_buffer_end = .;
    } > ram_ddr_0

    /*!< 内存剩余空间: 全部用于kernel内存池 */
    . = ALIGN(16);
    __mem_pool_size = _PROGRAM_END - .;

    /* kernel内存池 */
    .mem_pool (NOLOAD) :
    {
        __mem_pool_start = .;
        . += __mem_pool_size;
        __mem_pool_end = .;
    } > ram_ddr_0

    __ram_ddr_end = .;
}
```

mach_configs.h提供的宏定义主要来自autoconf.h，即与自动配置文件有关（不同CPU，配置文件不同），从而决定内存和程序入口的地址。
linkscript.lds.S使用gcc编译器转为linkscript.lds：
```Makefile
lds-y       := cpu/$(TYPE)/$(CPU)/linkscript.lds
LINK_SCRIPT := arch/$(ARCH)/lib/linkscript.lds.S

$(lds-y) : $(LINK_SCRIPT) force
    $(CC) -E -P -x c $(INCLUDE_DIRS) -o $@ $<
```

如imx6ull会得到（宏会被完全展开）：
```c
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
_RAM_DDR_BASE = (0x80000000);
_RAM_DDR_SIZE = (0x20000000);
_PROGRAM_START = ((0x80000000) + (0x07800000));
_PROGRAM_SIZE = ((0x20000000) - (0x07800000));
_RESERVE_MEMORY_SIZE = 0x400000;
_PROGRAM_END = (_RAM_DDR_BASE + _RAM_DDR_SIZE - _RESERVE_MEMORY_SIZE);
_HEAP_SIZE = (8 * 1024 * 1024);
_SVC_STACK_SIZE = (8 * 1024 * 1024);
_SYS_STACK_SIZE = (2 * 1024 * 1024);
_IRQ_STACK_SIZE = (8 * 1024 * 1024);
_FIQ_STACK_SIZE = (2 * 1024 * 1024);
_ABT_STACK_SIZE = (4 * 1024);
_UND_STACK_SIZE = (4 * 1024);
_DMA_BUFF_SIZE = (16 * 1024 * 1024);
_FB_DRAM_SIZE = (24 * 1024 * 1024);
_NETWORK_BUFF_SIZE = (8 * 1024 * 1024);
MEMORY
{
    ram_ddr_0 : ORIGIN = ((0x80000000) + (0x07800000)), LENGTH = ((0x20000000) - (0x07800000))
}
```

最后指定linkscript.lds链接各个.o文件，编译生成Heavenfox.elf。如：
```Makefile
$(TARGET_EXEC) : $(head-objs) $(lds-y) force
    $(CXX) -T$(lds-y) $(CXX_FLAGS) $(EXTRA_FLAGS) $(LINK_FLAGS) -Wl,-Map=$(TARGET_MMAP) -o $@ $(head-objs) $(OBJECT_EXEC) $(LIBS_PATH) $(LIBS)
```

#### 7.2. 异常向量表
根据链接脚本，Heavenfox的程序入口是异常向量表，它定义在“arch/$(ARCH)/lib/vectors.S”：
```nasm
    .global _start

    .global _undefined_handler
    .global _software_irq_handler
    .global _prefetch_abort_handler
    .global _data_abort_handler
    .global _unused_handler
    .global _irq_handler
    .global _fiq_handler

/* -------------------------------------------------------------------------------
 * Exception vectors table
 * -----------------------------------------------------------------------------*/
_start:
    b   _reset                                  @ 偏移地址: 0x00, 程序入口, 跳转到arch/$(ARCH)/cpu/$(TYPE)/start.S
    ldr pc, =_undefined_handler                 @ 偏移地址: 0x04, 未定义指令中止异常
    ldr pc, =_software_irq_handler              @ 偏移地址: 0x08, 软中断
    ldr pc, =_prefetch_abort_handler            @ 偏移地址: 0x0C, 预取指令中止异常
    ldr pc, =_data_abort_handler                @ 偏移地址: 0x10, 数据访问中止异常
    ldr pc, =_unused_handler                    @ 偏移地址: 0x14, 未使用
    ldr pc, =_irq_handler                       @ 偏移地址: 0x18, 通用中断
    ldr pc, =_fiq_handler                       @ 偏移地址: 0x1C, 快速中断
```

start.S负责保存异常向量表的地址到cp15协处理器，并关闭中断，禁用mmu和cache，再跳转到lowlevel_init；
lowlevel_init负责设置栈的起始地址（栈顶指针），并首次调用C语言文件board.c的s_init函数：
```nasm
    .global lowlevel_init

lowlevel_init:
    ldr sp, =CONFIG_BOARD_SP_ADDR               @ 设置栈指针, CONFIG_BOARD_SP_ADDR来自autoconf.h
    mov r12, sp
    bic r12, r12, #0x07                         @ 8字节对齐
    mov sp, r12

    push {lr}                                   @ 保存lr (sp初始化后才能使用push)

    bl s_init                                   @ jump to arch/$(ARCH)/cpu/$(TYPE)/$(CPU)/board.c

    pop {lr}                                    @ 恢复lr寄存器
    mov pc, lr                                  @ pc = lr, 返回到start.S
```

board.c负责初始化时钟和内存，以及延时周期（在定时器没有开启前，需要依赖普通延时）。主要调用（以imx6ull为例）：
```c
/* arch/$(ARCH)/cpu/$(TYPE)/$(CPU)/board.c */
void s_init(void)
{
    /*!< 延时周期设置(可使用delay/mdelay/udelay来延时) */
    arch_delay_config(false);
    
    /*!< 时钟初始化: CPU主频和外设的频率, 并开启部分外设的时钟使能开关 */
    imx6ull_clk_initial();

    /*!< 内存初始化(一般由ivt dcd完成, 此处留空) */
    imx6ull_sram_initial();
}
```

回到start.S，打开浮点运算功能VFP，跳转到_main，之后不再返回。
_main位于“arch/$(ARCH)/lib/crt0.S”，它负责完成CPU的早期、后期初始化，并初始化.bss数据段（未定义初始值或初始值为0的全局变量，统一初始化值为0）。
```nasm
    .global _main

_main:
    ldr sp, =CONFIG_BOARD_SP_ADDR               @ 重新设置栈指针(不设置也可, 程序运行到此, 栈指针应在栈顶)
    mov r12, sp
    bic r12, r12, #0x07                         @ 8字节对齐
    mov sp, r12

    mov r0, sp
    bl board_init_f_alloc_reserve               @ 栈顶向下移动, 留出空间用于全局变量global_data, 以及早期的内存池(空间不会太大)
    mov sp, r0                                  @ 重新修正sp, 使用新的栈顶

    mov r9, r0                                  @ 栈使用栈顶以下的空间, 栈顶以上首先是全局变量global_data, 地址保存到r9
    bl board_init_f_init_reserve                @ 初始化全局变量global_data, 保存内存池和栈的地址到global_data的成员
    mov r0, #0

    bl _bss_clear                               @ 擦除段.bss, 初始化未定义初始值或初始值为0的全局变量

    mov r0, #0
    bl board_init_f                             @ 早期初始化

    mov r0, #0
    ldr r10, =kernel_entry                      @ 获取内核入口到r10, 由board_init_r解析
    ldr lr, =board_init_r                       @ 后期初始化
    mov pc, lr                                  @ pc = board_init_r, 此后不再回来
```

#### 7.3. 早期初始化
board_init_f_alloc_reserve、board_init_f_init_reserve和board_init_f为早期初始化代码，由C语言编写。
前两个已在代码注释中说明了其功能，以CONFIG_BOARD_SP_ADDR为起始，向下移动CONFIG_BOOT_MALLOC_LEN，作为早期内存池；再向下移动sizeof(srt_gd_t)，留出全局变量“srt_gd_t global_data”的空间；并将此时global_data起始地址作为新的栈顶：
<blockquote>
<pre>
            --------------------    ---> CONFIG_BOARD_SP_ADDR
            |   早期内存池      |
            --------------------    ---> malloc_addr = CONFIG_BOARD_SP_ADDR - CONFIG_BOOT_MALLOC_LEN
            |   global_data    |
            --------------------    ---> iboot_sp = sp = malloc_addr - sizeof(srt_gd_t)
            |       栈         |
            --------------------
</pre>
</blockquote>

global_data是一个类型为srt_gd_t的结构体：
```c
struct global_data
{
    kuaddr_t iboot_sp;                          /* 栈顶 */
    kuaddr_t malloc_addr;                       /* 内存池起始地址: CONFIG_BOARD_SP_ADDR - CONFIG_BOOT_MALLOC_LEN */
    kuint32_t malloc_len;                       /* 内存池大小: CONFIG_BOOT_MALLOC_LEN */
    kuaddr_t os_entry;                          /* 内核入口: kernel_entry */
};
typedef struct global_data srt_gd_t;
```

board_init_f主要负责初始化内存池、串口（以便支持打印）、以及MMC（以便支持存储设备）：
```c
typedef kint32_t (*board_init_t) (void);

/*!< 早期初始化队列 */
board_init_t board_init_sequence_f[] =
{
    /*!< 建立内存池 (基于struct global_data::malloc_addr) */
    board_init_malloc_space,

    /*!< 初始化串口 */
    board_init_console,

    /*!< 初始化sdmmc外设 */
    board_init_sdmmc,
    
    mr_nullptr,
};

void board_init_f(void)
{
    /*!< 循环执行board_init_sequence_f[]的每一项函数 */
    if (board_initcall_run_list(board_init_sequence_f))
        for (;;);
}
```

board_init_console和board_init_sdmmc需由各CPU自行提供，不同CPU初始化方式不相同。

#### 7.4. 后期初始化
board_init_r用于后期初始化，负责保存内核所需的数据（如设备树、字库等），并跳转到内核。
```c
struct boot_images
{
    kuaddr_t os_entry;                          /* 内核入口: kernel_entry */

    struct global_data *sptr_gd;                /* 全局变量 */
    struct tag_params *sptr_params;             /* 启动参数 */
    struct m_area sgtc_fdt;                     /* 设备树拷贝到内存的地址, 长度信息等 */

    struct fatfs_disk *sptr_fdisk;              /* 挂载sd卡后的文件系统指针 */
};

/*!< 后期初始化队列 */
board_init_t board_init_sequence_r[] =
{
    /* 准备内核运行栈 (ABT/IRQ/SVC等模式的栈) */
    system_boot_initial,

    /* 建立专用内存池"fixed data", 准备用于存储固定数据 (为内核准备) */
    fix_data_mempool_initial,

    /* 主函数 */
    boot_main_loop,

    mr_nullptr,
};

void board_init_r(void)
{
    struct boot_images *sptr_image;
    srt_gd_t *sptr_gd;

    /* 从寄存器r9获取global_data */
    sptr_gd = board_get_gd();

    /* 保存kernel_entry入口地址到os_entry成员 */
    __asm__ __volatile__ (
        " mov %0, r10 \n"
        : "=&r"(sptr_gd->os_entry)
    );

    sptr_image = &sgtc_boot_images;
    sptr_image->sptr_gd = sptr_gd;

    /* 循环执行board_init_sequence_r[]的每一项函数, 若中途出错, 将引发断言(内核启动失败) */
    if (board_initcall_run_list(board_init_sequence_r))
        mr_assert(false);

    /* 初始化完成, 跳转到内核 */
    jump_to_kernel(sptr_image);
}
```

主要功能集中在boot_main_loop：
```c
kint32_t boot_main_loop(void)
{
    struct boot_images *sptr_image;
    kint32_t retval;

    sptr_image = &sgtc_boot_images;

    /* 挂载sd卡 */
    if (extdisk_mount(sptr_image))
        return RET_BOOT_ERR;

    /* 暂时无用, 可扩展为终端, 如串口命令解析 */
    for (;;)
    {
        break;
    }

    /* 设备树准备: 从sd卡读出.dtb文件, 拷贝到内存地址: CONFIG_DEVICE_TREE_BASE */
    retval = fdt_boot_initial(sptr_image);
    if (retval)
        goto exit;

    /* 启动数据准备 */
    retval = boot_params_prep(sptr_image);
    if (retval)
        goto exit;

exit:
    /* 卸载sd卡 */
    extdisk_unmount(sptr_image);

    /* 销毁内存池(struct global_data::malloc_addr) */
    memory_block_self_destroy(-1);

    return retval ? RET_BOOT_ERR : RET_BOOT_PASS;
}
```

启动数据（包括设备树在内）都会被连接到“struct tag_param”：
```c
typedef struct m_area
{
    void *base;
    kusize_t size;
    kuaddr_t offset;

} srt_m_area_t;

/*!< params saved for kernel */
#define TAG_PARAM_VIDEO         0
struct video_params
{
    struct m_area sgtc_hz12x12;
    struct m_area sgtc_hz16x16;                     /* 字库在内存中的位置, 如HZ16.bin */
    struct m_area sgtc_hz32x32;
};

#define TAG_PARAM_FDT           1
struct fdt_params
{
    struct m_area sgtc_fdt;                         /* base = CONFIG_DEVICE_TREE_BASE, size = 设备树实际大小, offset = 0 */
};

/* 表示当前是哪个信息 */
struct tag_header
{
    kint32_t type;                                  /* 信息类型, 如: TAG_PARAM_VIDEO, TAG_PARAM_FDT */
    kusize_t size;                                  /* 信息长度, 如: sizeof(struct video_params), sizeof(struct fdt_params) */
};

struct tag_params
{
    struct tag_header sgtc_hdr;

    /* 共用体 */
    union
    {
        struct video_params sgtc_vdp;               /* 主要是字库 */
        struct fdt_params sgtc_fdt;                 /* 设备树 */
    } u;
};

/* 获取下一个信息, 当前tag偏移size即可得到 */
#define TAG_PARAM_NEXT(tag) \
            (struct tag_params *)((void *)(tag) + (tag)->sgtc_hdr.size)
```

函数boot_params_prep首先定义一个“struct tag_params”对象，并设置它的地址为CONFIG_B2K_PARAM_BASE（来自autoconf.h），即，tag_params为一个全局对象。
首先初始化sgtc_hdr的各个成员为0，然后从sd卡读取字库，标记此刻的sgtc_hdr.type为TAG_PARAM_VIDEO，并将字库所在的内存地址保存到sgtc_vdp，而sgtc_hdr.size则设置为sizeof(struct video_params)，此时字库数据即保存到tag_params；
将tag_params偏移sizeof(struct video_params)，再保存下一个启动参数（如设备树），依次类推。

#### 7.5. 内核跳转
jump_to_kernel函数负责boot到内核的跳转，而内核入口地址已知，即sptr_image->os_entry（kernel_entry）；跳转时还要将tag_params进行传递，届时内核可将其解析。
```c
void jump_to_kernel(struct boot_images *sptr_image)
{
    kuint32_t r2;
    void (*kernel_entry)(kint32_t zero, kint32_t arch, kuint32_t params);

    r2 = (kuint32_t)sptr_image->sptr_params;
    kernel_entry = (void (*)(kint32_t, kint32_t, kuint32_t))sptr_image->os_entry;

    /* 跳转到head.S, 陷入汇编 */
    kernel_entry(0, 0, r2);
}
```

head.S需要：
> 1）设置CPU各模式下的栈顶指针，之前boot_init_f和boot_init_r均运行在SVC模式下，中断处于关闭状态，故仅设置了SVC模式的栈。而内核需要在SVC、IRQ、ABT等模式下切换。
> 2）使能D-cache和I-cache；
> 3）使能abort异常；
> 4）跳转到head-common.S

head-common.S将直接跳转到“start_kernel”函数，内核开始运行。

#### 7.6. start_kernel
首先将tag_params的地址从寄存器r2中读出（jump_to_kernel函数将tag_params保存到了r2寄存器）：
```c
static struct tag_params *sptr_tag_params;

#define mr_tag_params_get()    \
({    \
    struct tag_params *sptr_param;  \
    __asm__ __volatile__ (  \
        " str r2, [%0]  \n\t"   \
        : \
        : "r"(&sptr_param)   \
        : "cc","memory" \
    );  \
    sptr_param; \
})

void start_kernel(void)
{
    sptr_tag_params = mr_tag_params_get();

    /* 其他代码 */
}
```

然后构建内存池（包括“kernel heap”、“dma”、“framebuffer”和“network”，在《内存池》章节有简述）、解析设备树、构建时钟子系统、中断子系统、文件系统，并初始化系统定时器和platform平台。最终调用“schedule_thread”，调度器开始运转。

### 8. 平台架构
HeavenFox是一个类Linux的简化OS，这个“类”就主要体现在“平台架构”和“虚拟文件路径”两个章节，可以看到很多接口名称（结构体成员、函数名）都与Linux相似，但部分接口的具体实现则由笔者重构，代码趋向于简单化，开发者可以像写Linux一样调用这些名称相似的接口。

#### 8.1. 隐式初始化
由于不同CPU寄存器存在差异，硬件的初始化流程也有不同，如果直接调用某个CPU的初始化函数到“start_kerenl”，当CPU种类较多时，便无法适应。
内核使用链接脚本定义“.dync_init”和“.dync_exit”代码段，并指定某个初始化函数存放到该代码段，那么，只需要读取并执行该代码段的各个函数入口，便能实现初始化功能。
某个CPU的初始化函数是否能执行，则完全取自于：该初始化函数所在的.c文件，是否能被编译到内核中；而编译又取决于Makefile，Makefile又会读取auto.conf，并根据obj-y、obj-n来决定是否编译某个文件。即，最终取决于各个CPU的配置文件，如imx6ull_toppet_defconfig。选择哪个配置文件，则需要“make CONFIGS=imx6ull_toppet_defconfig config”配合。

规定如下代码段：
```c
/*!< dynamic init/exit sections*/
#define __DYNC_INIT_SEC(n)                          __section(".dync_init."#n)
#define __DYNC_EXIT_SEC(n)                          __section(".dync_exit."#n)

/*!< typedef dync_init/dync_exit */
typedef kint32_t (*dync_init_t) (void);
typedef void (*dync_exit_t) (void);

/*!< *(.dync_init.0), *(.dync_exit.0): 静态存储区 */
#define __DYNC_STC_INIT_SEC                         __DYNC_INIT_SEC(0)
#define __DYNC_STC_EXIT_SEC                         __DYNC_EXIT_SEC(0)
/*!< *(.dync_init.1), *(.dync_exit.1): 内核启动后初期代码区, 如虚拟文件路径 */
#define __DYNC_EARLY_INIT_SEC                       __DYNC_INIT_SEC(1)
#define __DYNC_EARLY_EXIT_SEC                       __DYNC_EXIT_SEC(1)
/*!< *(.dync_init.2), *(.dync_exit.2): 内核启动后期代码区, 如时钟子系统 */
#define __DYNC_LATE_INIT_SEC                        __DYNC_INIT_SEC(2)
#define __DYNC_LATE_EXIT_SEC                        __DYNC_EXIT_SEC(2)
/*!< *(.dync_init.3), *(.dync_exit.3): 内核代码区, 暂无用途 */
#define __DYNC_KERNEL_INIT_SEC                      __DYNC_INIT_SEC(3)
#define __DYNC_KERNEL_EXIT_SEC                      __DYNC_EXIT_SEC(3)
/*!< *(.dync_init.4), *(.dync_exit.4): 文件系统初始化, 如fatfs */
#define __DYNC_RTFS_INIT_SEC                        __DYNC_INIT_SEC(4)
#define __DYNC_RTFS_EXIT_SEC                        __DYNC_EXIT_SEC(4)
/*!< *(.dync_init.5), *(.dync_exit.5): 平台初始化 */
#define __DYNC_PLAT_INIT_SEC                        __DYNC_INIT_SEC(5)
#define __DYNC_PLAT_EXIT_SEC                        __DYNC_EXIT_SEC(5)
/*!< *(.dync_init.6), *(.dync_exit.6): 平台实例-驱动架构初始化 */
#define __DYNC_PATT_INIT_SEC                        __DYNC_INIT_SEC(6)
#define __DYNC_PATT_EXIT_SEC                        __DYNC_EXIT_SEC(6)
/*!< *(.dync_init.7), *(.dync_exit.7): 设备程序初始化 */
#define __DYNC_DEV_INIT_SEC                         __DYNC_INIT_SEC(7)
#define __DYNC_DEV_EXIT_SEC                         __DYNC_EXIT_SEC(7)
/*!< *(.dync_init.8), *(.dync_exit.8): 驱动程序初始化 */
#define __DYNC_DRV_INIT_SEC                         __DYNC_INIT_SEC(8)
#define __DYNC_DRV_EXIT_SEC                         __DYNC_EXIT_SEC(8)
/*!< *(.dync_init.9), *(.dync_exit.9): 临时存储区 */
#define __DYNC_PARA_INIT_SEC                        __DYNC_INIT_SEC(9)
#define __DYNC_PARA_EXIT_SEC                        __DYNC_EXIT_SEC(9)
```

使用以下接口指定：
```c
/*!
 * __attribute__((used)) const dync_init_t plat_##x __attribute__((section(".dync_init."#n))) = (dync_init_t)x
 * __attribute__((used)) const dync_exit_t plat_##x __attribute__((section(".dync_exit."#n))) = (dync_exit_t)x
 */
#define IMPORT_DYNC_INIT_ENTRY(prefix, x, sec)      __used const dync_init_t prefix##_##x sec = (dync_init_t)x
#define IMPORT_DYNC_EXIT_ENTRY(prefix, x, sec)      __used const dync_exit_t prefix##_##x sec = (dync_exit_t)x

/* init */
#define IMPORT_EARLY_INIT(x)                        IMPORT_DYNC_INIT_ENTRY(early,x, __DYNC_EARLY_INIT_SEC)
#define IMPORT_LATE_INIT(x)                         IMPORT_DYNC_INIT_ENTRY(late, x, __DYNC_LATE_INIT_SEC)
#define IMPORT_KERNEL_INIT(x)                       IMPORT_DYNC_INIT_ENTRY(os,   x, __DYNC_KERNEL_INIT_SEC)
#define IMPORT_ROOTFS_INIT(x)                       IMPORT_DYNC_INIT_ENTRY(rtfs, x, __DYNC_RTFS_INIT_SEC)
#define IMPORT_PLATFORM_INIT(x)                     IMPORT_DYNC_INIT_ENTRY(plat, x, __DYNC_PLAT_INIT_SEC)
#define IMPORT_PATTERN_INIT(x)                      IMPORT_DYNC_INIT_ENTRY(patt, x, __DYNC_PATT_INIT_SEC)
#define IMPORT_DEVICE_INIT(x)                       IMPORT_DYNC_INIT_ENTRY(bsp,  x, __DYNC_DEV_INIT_SEC)
#define IMPORT_DRIVER_INIT(x)                       IMPORT_DYNC_INIT_ENTRY(drv,  x, __DYNC_DRV_INIT_SEC)

/* exit */
#define IMPORT_EARLY_EXIT(x)                        IMPORT_DYNC_EXIT_ENTRY(early,x, __DYNC_EARLY_EXIT_SEC)
#define IMPORT_LATE_EXIT(x)                         IMPORT_DYNC_EXIT_ENTRY(late, x, __DYNC_LATE_EXIT_SEC)
#define IMPORT_KERNEL_EXIT(x)                       IMPORT_DYNC_EXIT_ENTRY(os,   x, __DYNC_KERNEL_EXIT_SEC)
#define IMPORT_ROOTFS_EXIT(x)                       IMPORT_DYNC_EXIT_ENTRY(rtfs, x, __DYNC_RTFS_EXIT_SEC)
#define IMPORT_PLATFORM_EXIT(x)                     IMPORT_DYNC_EXIT_ENTRY(plat, x, __DYNC_PLAT_EXIT_SEC)
#define IMPORT_PATTERN_EXIT(x)                      IMPORT_DYNC_EXIT_ENTRY(patt, x, __DYNC_PATT_EXIT_SEC)
#define IMPORT_DEVICE_EXIT(x)                       IMPORT_DYNC_EXIT_ENTRY(bsp,  x, __DYNC_DEV_EXIT_SEC)
#define IMPORT_DRIVER_EXIT(x)                       IMPORT_DYNC_EXIT_ENTRY(drv,  x, __DYNC_DRV_EXIT_SEC)
```

#### 8.2. 设备树
HeavenFox支持设备树。设备树文件存放于“boot/dts/$(CPU)”，而编译器位于“scripts/dtc”。设备树文件以.dts或.dtsi为后缀名，由dtc编译器将二者编译为.dtb（二进制文件），并拷贝到“boot/image”路径下，重命名为“firmware.dtb”；它被fdt_boot_initial函数从sd中读出，存储到内存地址CONFIG_DEVICE_TREE_BASE，再经tag_params传递给内核。

start_kernel将调用setup_machine函数解析tag_params：
```c
void setup_machine_fdt(void *ptr)
{
    struct fdt_params *sptr_param;
    kuint8_t *ptr_fdt_start;

    sptr_param = (struct fdt_params *)ptr;
    if (!isValid(sptr_param))
        return;
    
    /* ptr_fdt_start = CONFIG_DEVICE_TREE_BASE */
    ptr_fdt_start = sptr_param->sgtc_fdt.base;

    /* 校验是否为设备树 (设备树的起始是一个魔数: 0xd00dfeed) */
    if (!isValid(ptr_fdt_start) || !fwk_early_init_dt_verify(ptr_fdt_start))
        return;

    /*!< 初始化设备树根节点 (NULL) */
    fwk_early_init_dt_params(ptr_fdt_start);

    /*!< 解析dtb文件为struct fwk_device_node结构体 */
    fwk_unflatten_device_tree();
}

void setup_machine(struct tag_params *sptr_params)
{
    /* 取出各个参数, 其中fdt参数定位到sptr_fwk_fdt_params */
    setup_tag_params(sptr_params);

    /*!< 解析设备树 */
    setup_machine_fdt(sptr_fwk_fdt_params);
}

void start_kernel(void)
{
    sptr_tag_params = mr_tag_params_get();

    /* 省略部分函数 */

    setup_machine(sptr_tag_params);
}
```

##### 8.2.1. 语法
首先认识一下设备树。这是一个.dts文件的例程：

```c
/* 设备树注释方式与C语言一样; 语句后需以";"结尾 */
/* 可包含.h头文件 */
#include <dt-bindings/input.h>
/* .dts和.dtsi的语法完全一样, 它们之间的关系,就如同C语言的.c和.h */
#include "am4372.dtsi"

/*!
 * '/': 根节点, 和通用的树结构一样, 一个设备树只有一个根节点
 *          根节点的特点: 无父节点和兄弟节点
 *
 * 节点下可以挂接多个节点, 每个节点可能拥有自己的属性
 * 根节点属性: compatible, interrupt-parent, #address-cells, #size-cells
 * 子节点: chosen, memory@0, aliases, cpus, gic, ocp@44000000, ...
 *
 * 节点
 *      @: 后面一般接当前设备的寄存器地址. 该地址在代码中无用, 但可用于区分同名的节点
 *          (设备节点可能同名, 但设备地址一定不同)
 *      &: 引用其他节点. 被引用的节点会自动生成一个独一无二phandle属性值
 *          (未被引用过的节点也有phandle属性, 但属性值为-1)
 *      full_name: 全路径, 从根节点开始算起. 如: "/ocp@44000000/serial@48022000"
 *      name: 节点名称, 不含路径和@, 如: "serial". name一般与name属性相同, 见下文
 *          (由于不包含"@设备地址", 值可重复. 如"gpio@1", "gpio@2", name都是"gpio")
 *
 * 属性值
 *      "": 表示属性值为字符串. 一个属性值可以是多个字符串, 中间用','隔开
 *      <>: 表示属性值是32位整型. 如: reg = <0x400 0x4>
 *      []: 表示属性值是16位整型. 如: mac-address = [ 00 00 00 00 00 00 ]
 *      /bits/: 指定位数(8/16/32/64). 如: opp-hz = /bits/ 64 <800000000>;
 *      
 * name: 名称, 目前基本已不再显式地出现在dts中, 但并不是说没有这个属性 ===>
 *      内核代码手动添加, 它由节点的full_name切片得到 (最后一个'/'和'\0'之间)
 * compatible: 兼容属性, 表示节点的基本信息, 格式一般为: 厂商名,产品系列名,产品名
 * device_type: 设备类型
 * #address-cells: 表示子节点的reg属性该以几个连续的数值表示一个地址
 * #size-cells: 表示子节点的reg属性该以几个连续的数值表示一个长度
 * reg: 寄存器属性, 属性值一般为地址 + 长度. 如: 
 *      #address-cells = <2>;
 *      #size-cells = <1>;
 *      node {
 *          compatible = "template";
 *          reg = <0x200 0x400 0x4 0x600 0x800 0xC>;
 *      };
 *      
 *      因父节点属性#address-cells为2, 故reg属性需两个数表示地址, 即[0x200 0x400];
 *      因父节点属性#size-cells为1, 故reg属性需一个数表示地址, 即0x4;
 *      故 ===>
 *          地址1范围: 0x200 ~ 0x200 + 0x4; 地址2范围: 0x400 ~ 0x400 + 0x4
 *          地址3范围: 0x600 ~ 0x600 + 0xC; 地址4范围: 0x800 ~ 0x800 + 0xC
 * interrupt-controller: 空属性, bool类型; 有此属性时, 表示该设备节点是一个中断控制器
 * #interrupt-cells: 表示子节点的interrupts属性需要几个数值来描述中断信息. 如:
 *      #interrupt-cells = <2>;
 *      node {
 *          interrupts = <16 0>;
 *      };
 *
 *      父节点属性#interrupt-cells为2, 故interrupts需两个数表示中断信息, 即[16 0];
 *      一般第1个值(16)表示中断号, 第2个值表示中断触发类型(0) 
 *      (上/下边沿触发, 或高/低电平触发)
 * interrupt-parent: 本节点所属的中断控制器, 即父中断控制器; 
 *      (常引用中断控制器节点, 故一般是一个phandle值)
 * status: 表示当前节点的开关状态. 属性值为"okay"时, 节点使能; 为"disabled"则禁用
 *      (使能或禁用不会影响生成的dtb, 最终是要由内核代码自己判断status属性值来决定)
 *      (热插拔设备可以设置为"disabled", 后期可转为"okay"; 但"fail"则表示永久禁用)
 */

/ {
    /*!
     * 根节点属性1
     * 属性名: compatible
     * 属性值类型: 字符串
     * 属性值: value = "ti,am4372", "ti,am43"
     * 第一个属性值: str1 = value = "ti,am4372"
     * 第二个属性值: str2 = value + strlen(value) = "ti,am43"
     * 属性值长度: size = strlen(str1) + strlen(str2)
     */
    compatible = "ti,am4372", "ti,am43";

    /*!
     * 根节点属性2
     * 属性名: interrupt-parent
     * 属性值类型: 32位整型
     * 属性值: phandle (wakeupgen是某一个节点的名字, 被引用后生成非负数的id号)
     * 属性值长度: sizeof(unsigned int)
     * (各节点的phandle数值不重复, 可遍历整个设备树比较phandle, 获得wakeupgen)
     */
    interrupt-parent = <&wakeupgen>;

    /* 见前文 (根节点注释内容) */
    #address-cells = <1>;
    #size-cells = <1>;

    /* 不是一个真实的设备, 在HeavenFox中无意义 */
    chosen { };

    /*!
     * 根节点的子节点: memory
     * full_name (路径): "/memory@0"
     * name (名称): "memory" (name是full_name最后一个'/'起始的内容, 并剔除'@')
     */
    memory@0 {
        device_type = "memory";

        /* 父节点为根节点, #address-cells = <1>, #size-cells = <1> */
        reg = <0 0>;
    };

    /*!
     * 根节点的子节点: aliases
     * 该节点在代码中需特殊处理, 用于给其他节点起别名
     * dtc编译后, 各个属性会被展开, 比如serial0:
     *      serial0 = "/ocp@44000000/serial@48022000"
     */
    aliases {
        i2c0 = &i2c0;
        i2c1 = &i2c1;
        i2c2 = &i2c2;
        serial0 = &uart0;
        serial1 = &uart1;
        serial2 = &uart2;
        serial3 = &uart3;
    };

    cpus {
        #address-cells = <1>;
        #size-cells = <0>;

        /* cpus的子节点, 根节点的孙节点 */
        cpu: cpu@0 {
            compatible = "arm,cortex-a9";
            enable-method = "ti,am4372";
            device_type = "cpu";

            /* 父节点cpus的#size-cells = <0>, 故只有address */
            reg = <0>;

            /* dpll_mpu_ck是时钟控制器-设备树节点 */
            clocks = <&dpll_mpu_ck>;
            clock-names = "cpu";

            operating-points-v2 = <&cpu0_opp_table>;

            clock-latency = <300000>; /* From omap-cpufreq driver */
            cpu-idle-states = <&mpu_gate>;
        };
    };

    /* 中断控制器-设备树节点 */
    gic: interrupt-controller@48241000 {
        compatible = "arm,cortex-a9-gic";

        /* 空属性, 内核代码需判断是否有该属性, 有, 则注册为中断控制器 (irq_domain) */
        interrupt-controller;

        /* 引用gic的节点, interrupts属性需要3个数值描述 */
        #interrupt-cells = <3>;
        reg = <0x48241000 0x1000>,
              <0x48240100 0x0100>;
        interrupt-parent = <&gic>;
    };

    ocp@44000000 {
        compatible = "ti,am4372-l3-noc", "simple-bus";
        #address-cells = <1>;
        #size-cells = <1>;
        ranges;
        ti,hwmods = "l3_main";
        ti,no-idle;
        reg = <0x44000000 0x400000
               0x44800000 0x400000>;

        /* 中断信息: 中断组(PPI, SGI, SPI), 中断号, 中断触发类型 */
        interrupts = <GIC_SPI 9 IRQ_TYPE_LEVEL_HIGH>,
                 <GIC_SPI 10 IRQ_TYPE_LEVEL_HIGH>;

        uart1: serial@48022000 {
            compatible = "ti,am4372-uart","ti,omap2-uart";
            reg = <0x48022000 0x2000>;
            interrupts = <GIC_SPI 73 IRQ_TYPE_LEVEL_HIGH>;
            ti,hwmods = "uart2";

            /* 暂时关闭. 内核需判断status的值, 以决定是否转化为platform_device */
            status = "disabled";
        };
    
    };
};
```

基于am4372.dtsi，可根据具体单板编写.dts文件，只需包含一下am4372.dtsi即可。比如可以写一个template.dts：
```c
/* 声明设备树版本 */
/dts-v1/;

/* 包含dtsi文件, 就好像C语言包含.h文件一样 */
#include "am4372.dtsi"
#include <dt-bindings/pinctrl/am43xx.h>
#include <dt-bindings/pwm/pwm.h>
#include <dt-bindings/gpio/gpio.h>

/* 根节点内容补充; 不需要使用'&'进行引用 */
/ {
    model = "TI AM437x GP EVM";
    compatible = "ti,am437x-gp-evm","ti,am4372","ti,am43";

    aliases {
        display0 = &lcd0;
    };

    chosen {
        stdout-path = &uart0;
    };
};

/* 节点内容补充, 因为不是根节点, 需要使用'&'来引用, 表示继续填充节点usb1 */
&usb1 {
    dr_mode = "otg";

    /* status可能在dtsi是disabled, 这里改成okay, 直接开放 */
    status = "okay";

    /* pinctrl, 引脚配置 */
    pinctrl-names = "default";
    pinctrl-0 = <&usb1_pins>;
};
```

##### 8.2.2. dtc编译器
HeavenFox使用“scripts/dtc”路径下的dtc编译器来编译设备树，并将其编译规则写入到“boot/dts”路径下的Makefile文件，输入命令“make dtbs”即可一次性编译。
全文如下：
```Makefile
# 设备树所在路径: 当前位置(boot/dts)
DT_DIRS         :=  $(shell pwd)

dtb-y           :=
inc-y           :=

include $(CONF_MAKEFILE)
include $(CPU)/Makefile

# 由$(CPU)/Makefile提供, dtb-y可能叫imx6ull_topeet.dtb
TARGET          :=  $(dtb-y)

# imx6ull_topeet.dts所在路径
DT_NAME         :=  $(DT_DIRS)/$(CPU)
DT_FILE         :=  $(DT_NAME)/$(TARGET).dts

# 取头文件路径, 头文件中只能定义宏, 不能有变量类型出现
INCLUDES        :=  $(DT_DIRS)/ \
                    $(DT_DIRS)/$(CPU)/  \
                    $(DT_DIRS)/dt-bindings/
ifneq ($(inc-y),)
INCLUDES        +=  $(inc-y)
endif

# 一个用于dtc编译器(-i), 另一个用于cpp编译器(-I)
DTC_INCLUDES    :=  $(patsubst %, -i%, $(INCLUDES))
CXX_INCLUDES    :=  $(patsubst %, -I%, $(INCLUDES))

# 用法: dtc -I <源文件(输入)> -o <目标文件(输出)> -i <头文件路径> -o $@ $^
DTC             :=  $(PROJECT_DIR)/scripts/dtc/dtc
DTB             :=  $(DT_NAME)/$(TARGET).dtb
DTS             :=  $(DT_NAME)/.$(TARGET).dts
DEP             :=  $(DTS).d
TEMP            :=  $(DTS).temp

BUILD_FLAGS     :=  -nostdinc -undef -D__DTS__ -x assembler-with-cpp

all:
    # 1. 将xxx.dts生成xxx.temp
    # 2. xxx.temp生成.xxx.dts;
    # 3. .xxx.dts生成xxx.dtb
    # 4. xxx.dtb重命名为firmware.dtb
    cpp -Wp,-MD,$(DEP) $(BUILD_FLAGS) $(CXX_INCLUDES) -o $(TEMP) $(DT_FILE)
    $(DTC) -I dts -O dts $(DTC_INCLUDES) -o $(DTS) $(TEMP)
    $(DTC) -I dts -O dtb -o $(DTB) $(DTS)
    cp $(DTB) $(IMAGE_PATH)/firmware.dtb

clean:
    rm -rf $(TEMP) 
    rm -rf $(DEP) 
    rm -rf $(DTS)

distclean:
    rm -rf $(IMAGE_PATH)/firmware.dtb
    rm -rf $(DTB) 
    rm -rf $(TEMP) 
    rm -rf $(DEP) 
    rm -rf $(DTS)

check:
    # 检查.dts格式是否正确
    $(DTC) -q -I dts -O dtb $(DTC_INCLUDES) $(DT_FILE)

version:
    # 查看编译器版本
    $(DTC) -v
```

设备树文件也是可以包含.h文件的，前提是.h文件只能存放宏定义，其他语法并不能被设备树识别和支持。.h文件定义的宏可以直接用于设备树dts文件中，编译时会将宏自动展开（预处理）为具体的值。
Makefile先后使用了cpp和dtc两个编译器，xxx.dts首先由cpp编译为.xxx.dts.temp，同时由“-Mp,-MD”选项生成.xxx.dts.d；
最后由dtc编译.temp为.xxx.dts，然后再将其输出为xxx.dtb。这里的“.xxx.dts”是“xxx.dts”展开所有的宏、phandle，以及合并imx6ull.dtsi之后的设备树全貌，比如aliases节点就会被展开。
imx6ull.dtsi：
```c
/ {
    aliases {
        can0 = &flexcan1;
        can1 = &flexcan2;
        ethernet0 = &fec1;
        ethernet1 = &fec2;
    };
};
```

.xxx.dts：
```c
/ {
    aliases {
        can0 = "/soc/aips-bus@02000000/can@02090000";
        can1 = "/soc/aips-bus@02000000/can@02094000";
        ethernet0 = "/soc/aips-bus@02100000/ethernet@02188000";
        ethernet1 = "/soc/aips-bus@02000000/ethernet@020b4000";
    };
};
```

可以看到，原先的引用节点（phandle）的写法，被展开为具体节点的全路径。
还有就是合并imx6ull.dtsi，这使得设备树所有节点（dtsi和dts）全部集中到一起，最终dtb文件的内容顺序也将按照这个最终的dts文件排布。比如imx6ull.dtsi是以aliases节点开头：
```c
#include <dt-bindings/imx6ul/imx6ul-clock.h>
#include <dt-bindings/gpio.h>
#include <dt-bindings/arm-gic.h>
#include <dt-bindings/imx6ull/imx6ull-pinfunc.h>
#include <dt-bindings/imx6ull/imx6ull-pinfunc-snvs.h>
#include "../skeleton.dtsi"

/ {
    aliases {
        can0 = &flexcan1;
        can1 = &flexcan2;
        ethernet0 = &fec1;
        ethernet1 = &fec2;
    };
};
```

xxx.dts则是以model属性开头：
```c
/dts-v1/;

#include <dt-bindings/input.h>
#include "imx6ull.dtsi"

/ {
    model = "Freescale i.MX6 ULL Template Board";
    compatible = "fsl,imx6ull,template", "fsl,imx6ull";

    chosen {
        stdout-path = &uart1;
    };

    memory {
        reg = <0x80000000 0x20000000>;
    };
};
```

最终.xxx.dts呈现的结果就是二者的合并与结构重排：
```c
/dts-v1/;

/ {
    #address-cells = <0x1>;
    #size-cells = <0x1>;
    model = "Freescale i.MX6 ULL Template Board";
    compatible = "fsl,imx6ull,template", "fsl,imx6ull";

    chosen {
        stdout-path = "/soc/aips-bus@02000000/spba-bus@02000000/serial@02020000";
    };

    aliases {
        can0 = "/soc/aips-bus@02000000/can@02090000";
        can1 = "/soc/aips-bus@02000000/can@02094000";
        ethernet0 = "/soc/aips-bus@02100000/ethernet@02188000";
        ethernet1 = "/soc/aips-bus@02000000/ethernet@020b4000";
    };
};
```

这里自动补充了#address-cells和#size-cells两个属性；另外，如果节点被其他节点引用，且引用方式为<&节点名>的形式（<>内表示整数），被引用的节点也会凭空多出一个phandle属性。需注意的是，像“stdout-path = &uart1”这种写法并不会生成phandle，因为它写的并不是“stdout-path = <&uart1>”，非整型数；其实最后是解析为一个字符串，代表节点全路径：
```c
stdout-path = "/soc/aips-bus@02000000/spba-bus@02000000/serial@02020000"
```

##### 8.2.3. dtb
以HeavenFox使用的imx6ull_topeet.dts为例，使用“hexdump -C imx6ull_topeet.dtb”可查看：
```
00000000  d0 0d fe ed 00 00 86 5a  00 00 00 38 00 00 7d a0  |.......Z...8..}.|
00000010  00 00 00 28 00 00 00 11  00 00 00 10 00 00 00 00  |...(............|
00000020  00 00 08 ba 00 00 7d 68  00 00 00 00 00 00 00 00  |......}h........|
00000030  00 00 00 00 00 00 00 00  00 00 00 01 00 00 00 00  |................|
00000040  00 00 00 03 00 00 00 04  00 00 00 00 00 00 00 01  |................|
00000050  00 00 00 03 00 00 00 04  00 00 00 0f 00 00 00 01  |................|
00000060  00 00 00 03 00 00 00 20  00 00 00 1b 46 72 65 65  |....... ....Free|
00000070  73 63 61 6c 65 20 69 2e  4d 58 36 20 55 4c 4c 20  |scale i.MX6 ULL |
00000080  31 34 78 31 34 20 42 6f  61 72 64 00 00 00 00 03  |14x14 Board.....|
00000090  00 00 00 1f 00 00 00 21  66 73 6c 2c 69 6d 78 36  |.......!fsl,imx6|
000000a0  75 6c 6c 2d 74 6f 70 65  65 74 00 66 73 6c 2c 69  |ull-topeet.fsl,i|
000000b0  6d 78 36 75 6c 6c 00 00  00 00 00 01 63 68 6f 73  |mx6ull......chos|
000000c0  65 6e 00 00 00 00 00 03  00 00 00 39 00 00 00 2c  |en.........9...,|
000000d0  2f 73 6f 63 2f 61 69 70  73 2d 62 75 73 40 30 32  |/soc/aips-bus@02|
000000e0  30 30 30 30 30 30 2f 73  70 62 61 2d 62 75 73 40  |000000/spba-bus@|
000000f0  30 32 30 30 30 30 30 30  2f 73 65 72 69 61 6c 40  |02000000/serial@|
00000100  30 32 30 32 30 30 30 30  00 00 00 00 00 00 00 02  |02020000........|
00000110  00 00 00 01 61 6c 69 61  73 65 73 00 00 00 00 03  |....aliases.....|
00000120  00 00 00 24 00 00 00 38  2f 73 6f 63 2f 61 69 70  |...$...8/soc/aip|
00000130  73 2d 62 75 73 40 30 32  30 30 30 30 30 30 2f 63  |s-bus@02000000/c|
00000140  61 6e 40 30 32 30 39 30  30 30 30 00 00 00 00 03  |an@02090000.....|
```

部分字节具有特殊含义，称为dtb文件的格式特征码：

| 数据 (4字节)      | 含义                                                                                                          |
| ----------------- | ------------------------------------------------------------------------------------------------------------- |
| **`D0 0D FE ED`** | 魔数，用于dtb校验；固定数值，放在dtb文件开头；看到这个数就表示这是一个设备树文件                              |
| **`00 00 00 01`** | 1）表示所有节点的开始，与0x00000009对应；<br> 2）表示一个节点的开始，后面接的就是节点的内容；与0x00000002对应 |
| **`00 00 00 02`** | 表示一个节点的结束，前面接的就是节点的内容; 与0x00000001对应                                                  |
| **`00 00 00 03`** | 表示一个节点的属性，后面接的是该属性的值 （属性值和属性名称是分开存放的，这里仅包含值，没有名称）             |
| **`00 00 00 04`** | 表示一个节点的属性，这是一个空属性，没有属性值                                                                |
| **`00 00 00 09`** | 表示所有节点的结束；与0x00000001对应                                                                          |

下面来逐帧解析dtb的内容。
- header
前40个字节表示dtb文件的头部，每4个字节一组（即刚好一个unsigned int），分布如下：

| 名称              | 对应数据    | 含义                                 |
| ----------------- | ----------- | ------------------------------------ |
| magic             | D0 0D FE ED | 魔数, 固定为0xd00dfeed               |
| totalsize         | 00 00 86 5A | 整个设备树的大小 = 34394B = 34.394KB |
| off_dt_struct     | 00 00 00 38 | block的偏移位置 = 56字节             |
| off_dt_strings    | 00 00 7D A0 | string block的偏移位置 = 32160字节   |
| off_mem_rsvmap    | 00 00 00 28 | 保留内存区                           |
| version           | 00 00 00 11 | 设备树版本 = 17                      |
| last_comp_version | 00 00 00 10 | 向后兼容的版本 = 16                  |
| boot_cpuid_phys   | 00 00 00 00 | 用于多核系统下主CPU启动的物理ID      |
| size_dt_strings   | 00 00 08 BA | string block的大小 = 2234字节        |
| size_dt_struct    | 00 00 76 68 | block的大小 = 30312字节              |

魔数和设备树大小这些都很好理解，重点在于block（dt_struct）和string block（dt_strings）。前者用于存放节点的名字，以及属性值；后者则存放属性的名称——即属性值和属性名称存在于两个不同的区域。block处于设备树起始偏移56个字节的位置，也就是第56个字节开始，往下总共30312个字节，都是block的区域；而剩下的部分，从第（30312 + 56）个字节开始，一直到设备树末尾，全都是string block的范围，共2234个字节。
- dt_struct
从第56个字节开始，第1个数据就是00000001，从前文我们知道，这表示所有节点的起始；直接划到第（30312 + 56 - 4）字节的位置，可以看到00000009，前后呼应，即一整个dt_struct的区域：
```
00007d90  00 00 00 00 00 00 00 02  00 00 00 02 00 00 00 09  |................|
00007da0  23 61 64 64 72 65 73 73  2d 63 65 6c 6c 73 00 23  |#address-cells.#|
00007db0  73 69 7a 65 2d 63 65 6c  6c 73 00 6d 6f 64 65 6c  |size-cells.model|
00007dc0  00 63 6f 6d 70 61 74 69  62 6c 65 00 73 74 64 6f  |.compatible.stdo|
00007dd0  75 74 2d 70 61 74 68 00  63 61 6e 30 00 63 61 6e  |ut-path.can0.can|
```
以00000009作为分界，前面的是dt_struct，后面是dt_strings。
回到第56个字节处：
| 数据        | 含义                                                                                                                                                                 |
| ----------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 00 00 00 01 | 节点起始, 第一个节点是根节点                                                                                                                                         |
| 00 00 00 00 | 节点名字. 根节点没有名字, 不体现在dtb中 (内核代码可以自己指定名字, 即: '/')                                                                                          |
| 00 00 00 03 | 属性起始                                                                                                                                                             |
| 00 00 00 04 | 属性值的长度: 4字节                                                                                                                                                  |
| 00 00 00 00 | 属性名称在strings block的偏移位置: 0                                                                                                                                 |
| 00 00 00 01 | 属性值: #address-cells = <1>                                                                                                                                         |
| 00 00 00 03 | 属性起始                                                                                                                                                             |
| 00 00 00 04 | 属性值的长度: 4字节                                                                                                                                                  |
| 00 00 00 0F | 属性名称在strings block的偏移位置: 15                                                                                                                                |
| 00 00 00 01 | 属性值: #size-cells = <1>                                                                                                                                            |
| 00 00 00 03 | 属性起始                                                                                                                                                             |
| 00 00 00 20 | 属性值的长度: 32字节 (含'\0')                                                                                                                                        |
| 00 00 00 1B | 属性名称在strings block的偏移位置: 27                                                                                                                                |
| 46 72 65 65 | 属性值: model = "Freescale i.MX6 ULL Template Board" <br> 46 = 'F', 72 = 'r', ..., 64 = 'd',  倒数第二个00表示'\0', 由于还没有凑齐4个字节, 再补一个00 (即最后一个00) |
| ...         |                                                                                                                                                                      |
| 61 72 64 00 | model属性的最后3个字符 (即"Board"的最后3个字符'a', 'r', 'd')                                                                                                         |
| 00 00 00 03 | 属性起始                                                                                                                                                             |
| ...         | compatible = "fsl,imx6ull-topeet", "fsl,imx6ull"                                                                                                                     |
| 00 00 00 00 | 第1个00表示结束符，最后3个00是为了保证4字节对齐, 凑数的                                                                                                              |
| 00 00 00 01 | 新节点起始. 这里是chosen节点                                                                                                                                         |
| 63 68 6F 73 | 节点名: chos                                                                                                                                                         |
| 65 6E 00 00 | 节点名: en                                                                                                                                                           |
| 00 00 00 03 | 属性起始                                                                                                                                                             |
| 00 00 00 39 | 属性值的长度: 73字节 (含'\0')                                                                                                                                        |
| 00 00 00 2C | 属性名称在strings block的偏移位置: 44                                                                                                                                |
| 2F 73 6F 63 | 属性值: stdout-path = "/soc/aips-bus@02000000/spba-bus@02000000/serial@02020000"                                                                                     |
| ...         |                                                                                                                                                                      |
| 00 00 00 02 | 本节点结束. 这里是chosen节点                                                                                                                                         |
| 00 00 00 01 | 新节点起始. 这里是aliases节点                                                                                                                                        |
| ...         | ...                                                                                                                                                                  |
| 00 00 00 02 | 根节点的最后一个子节点结束 (chosen/aliases的兄弟节点)                                                                                                                |
| 00 00 00 02 | 根节点结束                                                                                                                                                           |
| 00 00 00 09 | 所有节点结束. dt_struct结束                                                                                                                                          |

总结一下，一个节点包括的内容为：
| 0x00000001 | 节点名称 | 属性信息 | 子节点 | 0x00000002 |
| ---------- | -------- | -------- | ------ | ---------- |

一个属性包括的内容为：
| 0x00000003 | 属性值长度 | 属性名称在dt_strings的偏移 | 属性值 |
| ---------- | ---------- | -------------------------- | ------ |

属性信息并不需要结束的表示符，因为属性值只有两种，等于0x0（“\0”），或者大于等于0x20（空格），当出现0x1/0x2/0x3时，基本就可以认为是新节点/节点结束/是新属性。另外，如果是空属性（没有属性值），则不含属性值字段。比如ranges属性：
```c
soc {
    #address-cells = <0x1>;
    #size-cells = <0x1>;
    compatible = "simple-bus";
    interrupt-parent = <0x5>;
    ranges;
};
```

它在dtb中只有0x00000003、属性长度（0x00000000）和偏移（0x000001F8），显然属性长度等于0，且没有属性值。

- dt_strings
属性信息的偏移数就是属性名称在strings block中的位置，我们知道，strings block位于整个dtb偏移32160个字节处，也就是十六进制0x7DA0。
对于根节点的几个属性，从之前分析dt_struct那里可以提取它的偏移信息：

| 所属节点   | 属性           | 名称在strings block的偏移 | 偏移 (在dtb的地址) |
| ---------- | -------------- | ------------------------- | ------------------ |
| 根节点     | #address-cells | 0                         | 0x00007DA0         |
| 根节点     | #size-cells    | 15                        | 0x00007DAF         |
| 根节点     | model          | 27                        | 0x000077BB         |
| 根节点     | compatible     | 33                        | 0x000077C1         |
| chosen节点 | stdout-path    | 44                        | 0x000077CC         |

这一区域的内容为：
```
00007d90  00 00 00 00 00 00 00 02  00 00 00 02 00 00 00 09  |................|
00007da0  23 61 64 64 72 65 73 73  2d 63 65 6c 6c 73 00 23  |#address-cells.#|
00007db0  73 69 7a 65 2d 63 65 6c  6c 73 00 6d 6f 64 65 6c  |size-cells.model|
00007dc0  00 63 6f 6d 70 61 74 69  62 6c 65 00 73 74 64 6f  |.compatible.stdo|
00007dd0  75 74 2d 70 61 74 68 00  63 61 6e 30 00 63 61 6e  |ut-path.can0.can|
```

结合右侧区域内的十六进制数据解析，确确实实就是这些属性对应的名称。
于是，根据header，我们得到了dt_struct和dt_strings的起始位置，从dt_struct中逐帧读取到各节点信息（名称和子节点）、各属性信息（属性长度和属性值，但属性名称不包含在此，而是给出一个在dt_strings的偏移地址）；各属性根据自己的偏移地址，再到dt_strings中读取自己的属性名称。
看起来似乎已比较清晰，但查找效率并不太高；最好是一次性把这些节点、属性全部查找并提取出来，保存到我们内存中，以后就不必再去检索dtb。

##### 8.2.4. 设备节点
内核将一个节点封装为fwk_device_node结构体，又将一个属性封装成fwk_of_property结构体。对于属性：
```c
typedef struct fwk_of_property
{
    /*!< 属性名,  如: "name", "compatible", "#size-cells" */
    kchar_t *name;
    /*!< 属性长度 */
    kint32_t length;
    /*!< 属性值, 如: compatible = "myBoard", 则属性值就是"myBoard", 属性长度 = kstrlen("myBoard") */
    void *value;
    /*!< 下一个属性 */
    struct fwk_of_property *sptr_next;

} srt_fwk_of_property_t;
```

刚好涵盖了dtb文件中属性的几个组成部分，只需遍历一整个dtb，将属性提取成一个个“struct fwk_of_property”结构体，然后通过sptr_next连接到所属的节点。
节点与之相似：
```c
typedef struct fwk_device_node
{
    kchar_t *name;                      /* 节点名. 如果一个节点叫"gpio@020a0000", 则name = "gpio"; 不同节点, name可能相同(但"@地址"不同) */
    kchar_t *type;                      /* 节点类型. 需要节点拥有"device_type"属性, 否则为"<null>" */
    kint32_t phandle;                   /* 节点被其他节点引用时, 本节点将自动生成一个phandle属性(32位整型), 不体现在dts/dtsi中; 若没有被引用, phandle为-1 */
    kchar_t *full_name;                 /* 节点全名, 也即节点的路径名称, 从根节点直到当前节点. 如full_name可能为: "/soc/aips-bus@02100000/ethernet@02188000". 每个节点的全名绝对不相同 */

    struct fwk_of_property *properties; /* 本节点的属性列表 (由struct fwk_of_property::sptr_next连接) */
    struct fwk_of_property *deadprops;  /* 被移除的属性列表 */

    struct fwk_device_node *parent;     /* 父节点, 即本节点的上一个路径. 根节点没有父节点, 为NULL */
    struct fwk_device_node *child;      /* 第一个子节点, 即本节点的下一个路径 */
    struct fwk_device_node *sibling;    /* 兄弟节点, 父节点的所有子节点均连接到此, 对于本节点, 只能获取下一个兄弟节点, 无法获取上一个 (因为只有allnext成员) */

    struct fwk_device_node *sptr_next;  /* 仅用在设备树-fwk_device_node构建阶段, 正常情况下必为NULL; 不可被外部使用  */
    struct fwk_device_node *allnext;    /* 每个节点通过allnext连接 */

} srt_fwk_device_node_t;
```

setup_machine函数依次调用setup_machine_fdt、fwk_unflatten_device_tree、__fwk_unflatten_device_tree、fwk_unflatten_dt_nodes解析dtb并提取每一个节点，所有的节点被链接到全局设备链表sptr_fwk_of_allNodes。解析的关键在于以下两个函数：
```c
/*!
 * @brief   提取节点, 被fwk_unflatten_dt_nodes调用
 * @param   ptr_blob:   dtb起始地址
 * @param   ptr_offset: 偏移: dtb中的第几个字节
 * @param   mem:        从内存池分配, 用于保存解析后的设备树节点
 * @param   parent:     父节点
 * @param   allNext:    下一个节点
 */
void *fwk_fdt_populate_node(struct fwk_fdt_header *ptr_blob,
                            void **ptr_offset, void **mem, void **ptr_parent, void ***allNext);

/* 提取当前节点下的属性(被fwk_fdt_populate_node调用) */
void *fwk_fdt_populate_properties(struct fwk_fdt_header *ptr_blob,
                                  void **ptr_offset, void **mem, void *node, void ***allNext, kbool_t *has_name);
```

需要注意的是，dtb文件以大端模式保存，在arm 32位处理器（小端模式）中，提取32位或16位数据时应伴随大小端转化。
```c
/*!< 魔数, dtb文件的第一个数据 */
#define FDT_MAGIC_VERIFY                        (0xd00dfeed)
/*!< 一个节点起始 */
#define FDT_NODE_START                          (0x00000001)
/*!< 一个节点结束 */
#define FDT_NODE_END                            (0x00000002)
/*!< 属性起始 */
#define FDT_NODE_PROP                           (0x00000003)
/*!< 空节点 */
#define FDT_NODE_NOP                            (0x00000004)
/*!< 所有节点的起始 */
#define FDT_ALL_NODE_START                      (FDT_NODE_START)
/*!< 所有节点的结束 */
#define FDT_ALL_NODE_END                        (0x00000009)
```

fwk_unflatten_dt_nodes函数解析过程如下（解析方法参考自Linux内核，虽由笔者重构, 但非原创，故这里不做源码展开；有兴趣可查看“platform/of/fwk_of.c”）：
```Mermaid
graph TD
    A[开始解析设备树] --> B[验证设备树有效性]
    B -->|有效| C[计算内存需求]
    C --> D[分配内存空间]
    D --> E[解析设备树节点]
    
    E --> F{当前标签类型}
    F -->|FDT_NODE_START| G[创建新节点]
    F -->|FDT_NODE_END| H[返回父节点]
    F -->|FDT_NODE_NOP| I[跳过]
    F -->|FDT_ALL_NODE_END| J[结束解析]
    
    G --> K[处理节点名称]
    K --> L[构建全路径名]
    L --> M[设置父子关系]
    M --> N[解析节点属性]
    
    N --> O{属性标签}
    O -->|FDT_NODE_PROP| P[解析属性]
    O -->|其他| Q[结束属性解析]
    
    P --> R[获取属性名]
    R --> S[获取属性值]
    S --> T[特殊属性处理]
    T -->|name| U[设置节点名]
    T -->|device_type| V[设置节点类型]
    T -->|phandle| W[设置phandle]
    
    Q --> X{缺少name属性?}
    X -->|是| Y[手动创建name属性]
    X -->|否| Z[设置默认名称]
    
    J --> AA[检查内存边界]
    AA --> AB[完成解析]
    
    classDef black fill:#000000,stroke:#FFFFFF,color:#FFFFFF;
```

##### 8.2.5. of函数
为便于获取节点和节点的前后关系，提供了一系列函数接口，可供使用：
```c
/* 获取根节点 */
struct fwk_device_node *fwk_of_node_root(void);
/* 通过full_name (path)来查找节点 (遍历全局链表sptr_fwk_of_allNodes) */
struct fwk_device_node *fwk_of_find_node_by_path(const kchar_t *ptr_path);
/* 通过name来查找节点, 如果有name相同的节点, 仅返回第一个 */
struct fwk_device_node *fwk_of_find_node_by_name(struct fwk_device_node *sptr_from, const kchar_t *ptr_name);
/* 通过device_type来查找节点 */
struct fwk_device_node *fwk_of_find_node_by_type(struct fwk_device_node *sptr_from, const kchar_t *ptr_type);
/* 通过phandle来查找节点 */
struct fwk_device_node *fwk_of_find_node_by_phandle(struct fwk_device_node *sptr_from, kuint32_t phandle);
/* 通过compatible属性值来查找节点 (若ptr_type不为NULL, 则device_type也要同时匹配) */
struct fwk_device_node *fwk_of_find_compatible_node(struct fwk_device_node *sptr_from,
                                            const kchar_t *ptr_type, const kchar_t *ptr_compat);
/* 比较sptr_matches数组中的每一项, 找到compatible属性匹配的项, 保存至sptr_match; 返回值为NULL时匹配失败 */
struct fwk_device_node *fwk_of_node_try_matches(struct fwk_device_node *sptr_node,
                                            const struct fwk_of_device_id *sptr_matches, struct fwk_of_device_id **sptr_match);
/* 从sptr_from开始遍历, 找到一个与sptr_matches某一项compatible属性匹配的节点, 保存该项到sptr_match, 并返回该节点 */
struct fwk_device_node *fwk_of_find_matching_node_and_match(struct fwk_device_node *sptr_from,
                                            const struct fwk_of_device_id *sptr_matches, struct fwk_of_device_id **sptr_match);
/* 从sptr_parent->sptr_child开始遍历, 在sptr_child->sptr_sibling中找到一个与sptr_matches某一项compatible属性匹配的节点, 保存该项到sptr_match, 并返回该节点 */ */
struct fwk_device_node *fwk_of_find_matching_child_and_match(struct fwk_device_node *sptr_parent,
                                            const struct fwk_of_device_id *sptr_matches, struct fwk_of_device_id **sptr_match);
/* fwk_of_node_try_matches改编版, 将匹配成功的sptr_match作为返回值 */
struct fwk_of_device_id *fwk_of_match_node(const struct fwk_of_device_id *sptr_matches, struct fwk_device_node *sptr_node);

/* 执行fwk_of_node_try_matches(sptr_dev->sptr_node, sptr_matches, NULL), 并返回匹配成功的sptr_match */
struct fwk_of_device_id *fwk_of_get_device_id(const struct fwk_of_device_id *sptr_matches, struct fwk_device *sptr_dev);
/* 返回sptr_node->parent */
struct fwk_device_node *fwk_of_get_parent(struct fwk_device_node *sptr_node);
/* 返回sptr_node->sptr_child, 或ptr_prev->sibling */
struct fwk_device_node *fwk_of_get_next_child(struct fwk_device_node *sptr_node, struct fwk_device_node *ptr_prev);
/* 计算子节点的数量 */
kuint32_t fwk_of_get_child_count(struct fwk_device_node *sptr_node);
/* 检测"status"属性值, 若status属性不存在或值为"ok"/"okay", 则节点有效; 若为"disabled"或其他值, 则无效 */
kbool_t fwk_of_device_is_avaliable(struct fwk_device_node *sptr_node);

/* 由本节点的"interrupt-parent"或"interrupt-extended"属性找到phandle节点, 若该节点具有"#interrupt-cells"属性, 即为父中断控制器 */
struct fwk_device_node *fwk_of_irq_parent(struct fwk_device_node *sptr_node);
/* 获取父中断控制器的"#interrupt-cells"属性值 */
kuint32_t fwk_of_n_irq_cells(struct fwk_device_node *sptr_node);
/* 当前节点有几个中断属性. 获取"interrupts"属性值个数 */
kuint32_t fwk_of_irq_count(struct fwk_device_node *sptr_node);
/* 当前节点在"alias"节点中的序号. 如gpio0 = &gpio1, 则id = 0 */
kint32_t fwk_of_get_alias_id(struct fwk_device_node *sptr_node);
/* 取本节点compatible属性第1个","后的值. 如: compatible = "xxx, yyy", 则modalias = "yyy" */
kint32_t fwk_of_modalias_node(struct fwk_device_node *sptr_node, kchar_t *modalias, kuint32_t len);

/* 遍历所有节点 */
#define foreach_fwk_of_dt_node(np, head)    \
    for (np = (isValid(head) ? head : fwk_of_node_root()); isValid(np); np = (np)->allnext)

/* 遍历parent下的所有子节点 */
#define foreach_fwk_of_child(parent, np)    \
    for (np = fwk_of_get_next_child(parent, mr_nullptr); np; np = fwk_of_get_next_child(parent, np))

/* 遍历所有父节点, 直到根节点 */
#define foreach_fwk_of_parent(np, child)    \
    for (np = child; np; np = fwk_of_get_parent(np))
```

获取本节点的属性内容，也有相关函数接口：
```c
/* 获取本节点属性名为ptr_name的属性 */
struct fwk_of_property *fwk_of_find_property(struct fwk_device_node *sptr_node,
                                        const kchar_t *ptr_name, kusize_t *ptr_lenth);
/* 根据属性名找到属性后, 返回属性值 */
void *fwk_of_get_property(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kusize_t *ptr_lenth);

/* 获取属性值(属性值类型为8位). index表示从第几个属性值开始获取, ptr_name为属性名, 而获取的属性值保存到ptr_value. 下述16位、32位同理 */
kint32_t fwk_of_property_read_u8_array_index(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, 
                                        kuint8_t *ptr_value, kuint32_t index, kusize_t size);
kint32_t fwk_of_property_read_u16_array_index(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, 
                                        kuint16_t *ptr_value, kuint32_t index, kusize_t size);
kint32_t fwk_of_property_read_u32_array_index(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, 
                                        kuint32_t *ptr_value, kuint32_t index, kusize_t size);
/* 仅获取第index个属性值 */
kint32_t fwk_of_property_read_u32_index(struct fwk_device_node *sptr_node,
                                        const kchar_t *ptr_name, kuint32_t index, kuint32_t *ptr_value);
kint32_t fwk_of_property_read_u8_array(struct fwk_device_node *sptr_node,
                                        const kchar_t *ptr_name, kuint8_t *ptr_value, kusize_t size);
kint32_t fwk_of_property_read_u16_array(struct fwk_device_node *sptr_node,
                                        const kchar_t *ptr_name, kuint16_t *ptr_value, kusize_t size);
kint32_t fwk_of_property_read_u32_array(struct fwk_device_node *sptr_node,
                                        const kchar_t *ptr_name, kuint32_t *ptr_value, kusize_t size);
/* 仅获取第0个属性值 */
kint32_t fwk_of_property_read_u8(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kuint8_t *ptr_value);
kint32_t fwk_of_property_read_u16(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kuint16_t *ptr_value);
kint32_t fwk_of_property_read_u32(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kuint32_t *ptr_value);

/* 属性值为字符串, 获取第0个字符串属性. 如: xxx = "123", "456", 则返回"123" */
kint32_t fwk_of_property_read_string(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kchar_t **ptr_string);
/* 属性值为字符串, 获取第index个字符串属性. 如: xxx = "123", "456", 若index为1, 则返回"456" */
kint32_t fwk_of_property_read_string_index(struct fwk_device_node *sptr_node,
                                        const kchar_t *ptr_name, kuint32_t index, kchar_t **ptr_string);

/* 从父节点获取属性"#size-cells"的值 */
kuint32_t fwk_of_n_size_cells(struct fwk_device_node *sptr_node);
/* 从父节点获取属性"#address-cells"的值 */
kuint32_t fwk_of_n_addr_cells(struct fwk_device_node *sptr_node);
/* 比较sptr_node的compatible属性值(字符串)是否与ptr_compat相同 */
kbool_t fwk_of_device_is_compatible(struct fwk_device_node *sptr_node, const kchar_t *ptr_compat);
/* list_name: 属性名; 比较sptr_node的list_name属性的值(字符串)是否与match_name相同 */
kint32_t fwk_of_property_match_string(struct fwk_device_node *sptr_node, const kchar_t *list_name, const kchar_t *match_name);

/* 或者可以根据phandle读出1组属性 (1组属性有args_count个属性值) */
struct fwk_of_phandle_args 
{
    /* sptr_node为phandle所对应的节点 */
    struct fwk_device_node *sptr_node;
    kuint32_t args_count;
    kuint32_t args[16];
};
/*!
 * 如节点属性为: xxx = <&aaa 1 6>, 且节点aaa的phandle为88, 则:
 *  struct fwk_of_phandle_args::sptr_node = fwk_of_find_node_by_phandle(NULL, 88);
 *  struct fwk_of_phandle_args::args_count = 2;
 *  struct fwk_of_phandle_args::args[] = { 1, 6, 0, 0, ... }
 * 
 * @param   list_name: 属性名, 如"xxx";
 * @param   cells_name: 可能为属性名"#size-cells", 一般节点aaa会具有此属性;
 * @param   cell_count: 如果节点aaa没有属性cells_name, 则cell_count被当成1组属性的属性个数; 正常情况下无用; 对于xxx = <&aaa 1 6>, cell_count应为2 (表示1和6)
 * @param   index: 要获取第几组属性, 节点属性可能是:
 *              xxx = <&aaa 1 6> <&bbb 2 3> <&ccc 9 9>
 *              若index为1, 表示第1组 <&bbb 2 3>
 * @param   sptr_args: 解析后存入此结构
 */
kint32_t fwk_of_parse_phandle_with_args(struct fwk_device_node *sptr_node, const kchar_t *list_name,
                                        const kchar_t *cells_name, kuint32_t cell_count, kint32_t index, struct fwk_of_phandle_args *sptr_args);
```

#### 8.3. 时钟
时钟在两个地方初始化，一是board.c，是CPU刚刚启动的时候，用于确定系统主频、外设时钟主频，保证时钟树的正常运行，并使能部分必需的外设时钟；二是驱动程序“drivers/clk”，在start_kernel函数中调用，补充board.c中没有初始化的时钟，如LCD像素时钟，并搭建时钟平台架构。
HeavenFox目前仅支持一种时钟平台架构，即时钟使能/失能。设备树节点会提供一个名为“clocks”的属性，比如imx6ull的gpt定时器节点：
```c
gpt1: gpt@02098000 {
    compatible = "fsl,imx6ul-gpt", "fsl,imx31-gpt";
    reg = <0x02098000 0x4000>;
    interrupts = <GIC_SPI 55 IRQ_TYPE_LEVEL_HIGH>;
    clocks = <&clks IMX6UL_CLK_GPT1_BUS>,
            <&clks IMX6UL_CLK_GPT_3M>;
    clock-names = "ipg", "osc_per";
};
```

IMX6UL_CLK_GPT1_BUS即表示gpt外设所需开启的时钟序号，在HeavenFox中，用宏来表示。
```c
#define IMX6UL_CLK_GPT1_BUS                         152
#define IMX6UL_CLK_GPT_3M                           214

/* 数量 */
#define IMX6UL_CLK_END                              236
```

数字序号往往可以表示数组下标，而内核定义了一个时钟数组，长度为IMX6UL_CLK_END。
时钟数组的类型为：
```c
struct fwk_clk_one_cell
{
    struct fwk_clk *sptr_clks;                      /* 每一个clocks属性都抽象成一个fwk_clk结构体 */
    kuint32_t clks_size;                            /* 一般等于IMX6UL_CLK_END */

};

/* 数组 */
static struct fwk_clk sgtc_imx_clks_data[IMX6UL_CLK_END];
static struct fwk_clk_one_cell sgtc_imx_clk_one_cell_data = { &sgtc_imx_clks_data[0], IMX6UL_CLK_END };
```

由设备树的时钟序号（如IMX6UL_CLK_GPT1_BUS），可找到sgtc_imx_clks_data[IMX6UL_CLK_GPT1_BUS]。内核通过结构体“struct fwk_clk”表征一个时钟设备，不过目前只支持时钟开关，它被设计为一个“struct fwk_clk_gate”结构：
```c
struct fwk_clk_gate 
{
    struct fwk_clk_hw sgtc_hw;                      /* 内含结构体"struct fwk_clk"成员 */
    void *reg;                                      /* 时钟设备的地址 */
    kuint8_t bit_idx;                               /* 开关所在的偏移位. 打开时钟: *reg |= (1 << bit_idx); 关闭时钟: *reg &= ~(1 << bit_idx) */
};
```

fwk_clk_gate是时钟设备用于操作硬件的结构（因为有reg和bit_idx）；同样的，如果以后支持分频器，它也会与gate相似。内核不会直接操作“struct fwk_clk_gate”，因为对驱动层来说，我不希望知道它的底层结构，最好是有一个通用的封装。于是规定，不管是什么时钟设备，都给它定义一个内部成员sgtc_hw，然后用mr_container_of去反推。
sgtc_hw内部含有两个重要成员：sptr_clk和sptr_core：
```c
struct fwk_clk_hw
{
    struct fwk_clk_core *sptr_core;
    struct fwk_clk *sptr_clk;
};

/* 其中 */
struct fwk_clk
{
    const kchar_t *dev_id;                          /* 设备名, 取自mr_dev_get_name() */
    const kchar_t *con_id;                          /* 条件id, 取自设备树节点属性"clock-names", 比如"ipg" */
    kuint32_t max_rate;                             /* 最大频率 */
    kuint32_t min_rate;                             /* 最小频率 */

    struct fwk_clk_core *sptr_core;                 /* 核心结构, 在时钟设备(如gate)注册时被创建 */
    struct list_head sgtc_link;                     /* 链接到"struct fwk_clk_core::sgtc_clks", 一个core可能挂接多个fwk_clk */

    kuint8_t flags;                                 /* 私有标志 */
};

struct fwk_clk_core
{
    const kchar_t *name;                            /* 时钟设备的名称 */
    const struct fwk_clk_ops *sptr_ops;             /* 时钟操作函数集, 由各时钟设备组件(如gate)自己提供 */
    struct fwk_clk_hw *sptr_hw;                     /* 指向父结构 */

    const kchar_t **parent_names;                   /* 父时钟设备集合 */
    struct fwk_clk_core **sptr_parents;             /* 保留功能 */

    kuint32_t rate;                                 /* 频率 */

    kuint32_t flags;
    kuint32_t enable_count;                         /* 使能次数, 大于0时使能, 等于0时失能 */
    kuint32_t prepare_count;                        /* 准备次数, 大于0时准备完成, 等于0时撤销准备 */

    struct list_head sgtc_clks;                     /* 可以基于fwk_clk_core申请一系列fwk_clk, 让不同驱动去使用 */
    struct mutex_lock sgtc_mutex;                   /* 内置互斥锁 */
};
```

sptr_core是时钟设备的通用结构，它同时存在于“struct fwk_clk_hw”和“struct fwk_clk”，后者是我们对外的结构，即面向驱动程序开放。
各个结构之间的关系为：

| 结构体              | 说明                                                                                                                                                                                            |
| ------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| struct fwk_clk_gate | 由CPU的时钟驱动程序创建，内含sgtc_hw                                                                                                                                                            |
| struct fwk_clk_hw   | 是fwk_clk_gate的成员，一起创建；其sptr_clk成员指向全局数组sgtc_imx_clks_data[?]                                                                                                                 |
| struct fwk_clk_core | 在fwk_clk_hw创建后申请（kmalloc），并令成员sptr_hw指向struct fwk_clk_gate::sgtc_hw，而后者的sptr_core成员指向它                                                                                 |
| struct fwk_clk      | 首次登场于全局数组sgtc_imx_clks_data，在fwk_clk_gate创建时被传递给其成员sgtc_hw.sptr_clk；<br> 后续可能登场于驱动程序，会创建一个sgtc_imx_clks_data[?]的副本，且sptr_core成员与副本源指向同一个 |

大致流程为：
```Mermaid
graph TD

    A[fwk_clk_gate_register] ---> B[创建：struct fwk_clk_gate *sptr_gate，且sptr_gate->sgtc_hw.sptr_clk = sptr_clk]
    B ---> C[fwk_clk_register]
    C ---> D[创建：struct fwk_clk_core *sptr_core]
    D ---> E[sptr_core->sptr_hw = &sptr_gate->sgtc_hw，sptr_gate->sgtc_hw.sptr_core = sptr_core]
```

即：
```shell
1) 当获得设备树节点的clock属性值xxx后, 可取得全局fwk_clk数组: struct fwk_clk *sptr_clk = sgtc_imx_clks_data[xxx]；
2) 获得sptr_clk->sptr_core->sptr_ops和sptr_clk->sptr_core->sptr_hw;
3) 在sptr_ops的任一函数中, 可反推得到: struct fwk_clk_gate *sptr_gate = mr_container_of(sptr_hw, struct fwk_clk_gate, sgtc_hw);
4) 使用sptr_gate->reg和sptr_gate->bit_idx操作寄存器
```

相关API：
```c
struct fwk_clk_ops
{
    kint32_t    (*prepare) (struct fwk_clk_hw *sptr_hw);
    void        (*unprepare) (struct fwk_clk_hw *sptr_hw);
    kint32_t    (*is_prepared) (struct fwk_clk_hw *sptr_hw);

    kint32_t    (*enable) (struct fwk_clk_hw *sptr_hw);
    void        (*disable) (struct fwk_clk_hw *sptr_hw);
    kint32_t    (*is_enabled) (struct fwk_clk_hw *sptr_hw);

    kint32_t    (*set_rate) (struct fwk_clk_hw *sptr_hw, kuint32_t rate, kuint32_t parent_rate);
    void        (*init) (struct fwk_clk_hw *sptr_hw);
};

/* 注册时钟设备: 创建fwk_clk_gate结构. 其中sptr_clk来自于sgtc_imx_clks_data[?] */
struct fwk_clk *fwk_clk_gate_register(struct fwk_clk *sptr_clk, const struct fwk_clk_ops *sptr_ops,
                                const kchar_t *name, const kchar_t *parent, void *reg, kuint8_t shift);
/* 注销 */
void fwk_clk_gate_unregister(struct fwk_clk *sptr_clk);

/* 根据设备树clock属性的值找到sgtc_imx_clks_data[sptr_args->args[0]] */
struct fwk_clk *fwk_of_clk_src_onecell_get(struct fwk_of_phandle_args *sptr_args, void *data);
/* 注册一个时钟控制器 (一般cpu只有一个时钟控制器). get回调函数即fwk_of_clk_src_onecell_get */
kint32_t fwk_clk_add_provider(struct fwk_device_node *sptr_node, 
                    struct fwk_clk *(*get)(struct fwk_of_phandle_args *, void *), void *data);
/* 注销 */
void fwk_clk_del_provider(struct fwk_device_node *sptr_node);
/* 调用fwk_of_clk_src_onecell_get */
struct fwk_clk *fwk_clk_provider_look_up(struct fwk_of_phandle_args *sptr_args);

/* 注册时钟设备(如&sptr_gate->sgtc_hw), 并创建sptr_core; 返回值一般为sptr_hw->sptr_clk */
struct fwk_clk *fwk_clk_register(struct fwk_device *sptr_dev, struct fwk_clk_hw *sptr_hw);
/* 注销 */
void fwk_clk_unregister(struct fwk_clk *sptr_clk);

/* 初始化sptr_clk */
struct fwk_clk *fwk_clk_config(struct fwk_clk *sptr_clk, 
                            struct fwk_clk_hw *sptr_hw, const kchar_t *dev_id, const kchar_t *con_id);
/* 创建并初始化sptr_clk */
struct fwk_clk *fwk_create_clk(struct fwk_clk_hw *sptr_hw, const kchar_t *dev_id, const kchar_t *con_id);

/* name对应设备树节点属性"clock-names", 通过它找到"clock"属性值, 从而找到sgtc_imx_clks_data[?]; 然后创建一个副本并返回 */
struct fwk_clk *fwk_clk_get(struct fwk_device *sptr_dev, const kchar_t *name);
/* 释放掉副本 */
void fwk_clk_put(struct fwk_clk *sptr_clk);

/* 开启时钟: sptr_clk->sptr_core->sptr_ops->enable(sptr_clk->sptr_core->sptr_hw) */
void fwk_clk_enable(struct fwk_clk *sptr_clk);
/* 开启时钟: sptr_clk->sptr_core->sptr_ops->disable(sptr_clk->sptr_core->sptr_hw) */
void fwk_clk_disable(struct fwk_clk *sptr_clk);
/* 准备时钟: sptr_clk->sptr_core->sptr_ops->prepare(sptr_clk->sptr_core->sptr_hw) */
void fwk_clk_prepare(struct fwk_clk *sptr_clk);
/* 撤销准备: sptr_clk->sptr_core->sptr_ops->unprepare(sptr_clk->sptr_core->sptr_hw) */
void fwk_clk_unprepare(struct fwk_clk *sptr_clk);
/* 时钟是否开启: sptr_clk->sptr_core->sptr_ops->is_enabled(sptr_clk->sptr_core->sptr_hw) */
kbool_t fwk_clk_is_enabled(struct fwk_clk *sptr_clk);

/* prepare + enable */
void fwk_clk_prepare_enable(struct fwk_clk *sptr_clk);
/* disable + unprepare */
void fwk_clk_disable_unprepare(struct fwk_clk *sptr_clk);
```

#### 8.4. 中断
##### 8.4.1. 中断控制器
中断可能来自CPU中断控制器，可按照中断号划分为SGI、PPI和SPI；也可能来自GPIO中断控制器，大部分CPU，每个IO引脚都能触发外部中断。
对于imx6ull，顶层CPU中断控制器下辖中断号160个，其中SGI有16个，PPI有16个，SPI有128个；
将PPI和SPI归为GPC控制器（全局共享中断控制器），中断号共128个；
GPIO是GPC的其中一个中断号，一个GPIO控制器有32个引脚，则有32个外部中断号；
将中断控制器称为域，定义一个“struct fwk_irq_domain”结构以做表示：
```c
struct fwk_irq_domain
{
    kint32_t hwirq;                                     /* 隶属于本中断控制器的起始中断号; 对于GPC, 起始中断号为32 */
    struct fwk_device_node *sptr_node;                  /* 中断控制器在设备树中的节点 */
    const kchar_t *name;                                /* 中断控制器的名称, 一般与sptr_node->name相同 */

    struct list_head sgtc_link;                         /* 所有的中断控制器组成链表, 以便管理 */
    const struct fwk_irq_domain_ops *sptr_ops;          /* 中断控制器的操作函数集 */
    void *host_data;                                    /* 中断控制器的私有数据, 暂无用途 */

    struct fwk_irq_domain *sptr_parent;                 /* 父中断控制器. 如gpc的父控制器是"interrupt-controller" */

    kuint32_t hwirq_max;                                /* 本控制器的最大中断号, 对于GPC, max = 128; 对于GPIO, max = 32 */
    kint32_t revmap[];                                  /* 线性映射, 大小为hwirq_max; 中断号会被映射为虚拟中断号, revmap保存两种中断号的关系 */
};

struct fwk_irq_domain_ops 
{
    /* 解析intspec数组, 输出out_hwirq(中断号)和out_type(中断触发类型) */
    kint32_t (*xlate)(struct fwk_irq_domain *sptr_domain, struct fwk_device_node *sptr_intc,
                const kuint32_t *intspec, kuint32_t intsize, kuint32_t *out_hwirq, kuint32_t *out_type);

    /* 为当前控制器申请一个"struct fwk_irq_chip"; free为反操作 */
    kint32_t (*alloc)(struct fwk_irq_domain *sptr_domain, kuint32_t virq, kuint32_t nr_irqs, void *arg);
    void (*free)(struct fwk_irq_domain *sptr_domain, kuint32_t virq, kuint32_t nr_irqs);       
};
```

我们从设备树的“interrupts”属性可以获得中断号和触发类型：
```c
gpt1: gpt@02098000 {
    compatible = "fsl,imx6ul-gpt", "fsl,imx31-gpt";
    reg = <0x02098000 0x4000>;
    interrupts = <GIC_SPI 55 IRQ_TYPE_LEVEL_HIGH>;
    clocks = <&clks IMX6UL_CLK_GPT1_BUS>,
            <&clks IMX6UL_CLK_GPT_3M>;
    clock-names = "ipg", "osc_per";
};
```

“interrupts”属性的3个值将被填入到数组：
```c
const kuint32_t intspec[3] = { GIC_SPI, 55, IRQ_TYPE_LEVEL_HIGH }
```

但哪一个是中断号，哪一个是中断触发类型，则要由“struct fwk_irq_domain_ops”结构体的xlate函数给出。如GPC中断控制器的xlate函数为：
```c
kint32_t gpc_irq_domain_xlate(struct fwk_irq_domain *sptr_domain, struct fwk_device_node *sptr_intc,
                    const kuint32_t *intspec, kuint32_t intsize, kuint32_t *out_hwirq, kuint32_t *out_type)
{
    if (sptr_domain->sptr_node != sptr_intc)
        return -ER_INVALID;

    /* 中断控制器归属于GPC时, intspec数组的元素个数需为3 (即: GIC_SPI, 55, IRQ_TYPE_LEVEL_HIGH) */
    if (intsize != 3)
        return -ER_INVALID;

    /*!< 只能为GIC_SPI */
    if (intspec[0] != 0)
        return -ER_INVALID;

    /* 中断号 (如: 55) */
    *out_hwirq = intspec[1];
    /* 中断触发类型 (如: IRQ_TYPE_LEVEL_HIGH, 高电平触发) */
    *out_type = intspec[2];

    return ER_NORMAL;
}
```

至于gpc_irq_domain_xlate将在什么地方被调用，以及fwk_irq_domain结构体的核心成员revmap[]的作用，见下一章节。

##### 8.4.2. 虚拟中断号
一个域的硬件中断号可能与另一个域的相同，为了统一管理，一律将硬件中断号转为虚拟中断号。比如中断控制器A，中断号0~31映射为虚拟中断号0~31；而中断控制器B的中断号0~31则紧跟着上一次映射，对应到虚拟中断号32~63，这样不管是哪个中断控制器，都能直接根据唯一的虚拟中断号识别和使用。
所谓虚拟中断号，其实是取用全局位图中的位偏移序号，如全局32位变量a的值为0b101，这里第0位和第3位均已置1，表明虚拟中断号0和3已经硬件中断号映射成功。HeavenFox维护着全局位图，每个虚拟中断号对应一个“struct fwk_irq_desc”结构体，是中断号的专属资源：
```c
struct fwk_irq_data
{
    kuint32_t irq;                          /* 虚拟中断号 */
    kutype_t hwirq;                         /* 硬件中断号 */
                 
    struct fwk_irq_chip *sptr_chip;         /* 操作函数集, 对于同一个中断控制器, sptr_chip一般为同一个 */
    struct fwk_irq_domain *sptr_domain;     /* 所在的中断控制器 */

    void *handler_data;                     /* 私有数据结构, 可以是fwk_irq_generic */
    void *chip_data;                        /* 保留功能 */

    kuint32_t mask;                         /* 位掩码, 一般配合handler_data使用, 如作为中断使能位 */
};

struct fwk_irq_desc
{
    kint32_t irq;                           /* 本desc对应的虚拟中断号 */

    kuint32_t flags;                        /* 标志, 记录中断触发类型, 可从设备树获取 */
    kchar_t irq_name[32];                   /* 中断名称 */
    struct list_head sgtc_action;           /* 关联到本中断的事件 (中断回调函数) */

    struct radix_link sgtc_radix;           /* 使用基数树相连, 当得知虚拟中断号的值后, 可在树中找到对应的"fwk_irq_desc" */
    struct spin_lock sgtc_lock;             /* 内置自旋锁 */
    
    struct fwk_irq_data sgtc_data;          /* "fwk_irq_data"内嵌于此 */
};

/* 根据虚拟中断号找到desc (通过基数树查找) */
struct fwk_irq_desc *fwk_irq_to_desc(kuint32_t virq);
/* 根据data获取desc (mr_container_of) */
struct fwk_irq_desc *fwk_irq_data_to_desc(struct fwk_irq_data *sptr_data);
/* 根据虚拟中断号得到desc后, 返回&desc->sgtc_data */
struct fwk_irq_data *fwk_irq_get_data(kuint32_t virq);

/* 已知中断控制器, 通过硬件中断号找到虚拟中断号, 再调用fwk_irq_get_data */
struct fwk_irq_data *fwk_irq_domain_get_data(struct fwk_irq_domain *sptr_domain, kuint32_t hwirq);
/* 先找到desc, 再设置desc->type成员 */
void fwk_irq_desc_set_type(kuint32_t virq, kuint32_t type);
/* 通过硬件中断号找到虚拟中断号, 获得对应的desc, 设置desc->type并返回虚拟中断号 */
kint32_t fwk_irq_domain_find_map(struct fwk_irq_domain *sptr_domain, kuint32_t hwirq, kuint32_t type);

/*!
 * 在全局位图中从irq_base开始, 找到连续nr_irqs个为0的位, 将这些位置1, 表示注册;
 * 申请nr_irqs个desc, 根据desc->irq加入到基数树;
 * 初始化每个desc的sgtc_data成员, 使虚拟中断号和硬件中断号建立联系.
 * sptr_domain->revmap[]数组存放虚拟中断号, 如sptr_domain->revmap[1] = 10, 表示硬件中断号1对应的虚拟中断号为10. 虚拟中断号采用线性映射方式
 */
kint32_t fwk_irq_domain_alloc_irqs(struct fwk_irq_domain *sptr_domain, kint32_t irq_base, kuint32_t hwirq, kuint32_t nr_irqs);
/* 解除硬件中断号的映射, 归还虚拟中断号到全局位图; 删除其在基数树的节点, 注销对应的desc */
void fwk_irq_desc_free(kint32_t irq);
/* 释放掉中断控制器下的所有虚拟中断号, 即连续调用fwk_irq_desc_free */
void fwk_irq_domain_free_irqs(struct fwk_irq_domain *sptr_domain);

/* sptr_node为中断控制器所在设备树节点, 从所有已注册的domain中找到(domain->sptr_node == sptr_node)的domain并返回 */
struct fwk_irq_domain *fwk_of_irq_host(struct fwk_device_node *sptr_node);

/*!
 * 将设备树节点的"interrupts"属性值解析到sptr_irq结构体
 * 如: interrupts = <GIC_SPI 89 IRQ_TYPE_LEVEL_HIGH>;
 *      sptr_irq->args_count = 3;
 *      sptr_irq->args[0] = GIC_SPI;
 *      sptr_irq->args[1] = 89;
 *      sptr_irq->args[2] = IRQ_TYPE_LEVEL_HIGH;
 *      sptr_irq->sptr_node = fwk_of_irq_parent(sptr_node);
 */
kint32_t fwk_of_irq_parse_one(struct fwk_device_node *sptr_node, kuint32_t index, struct fwk_of_phandle_args *sptr_irq);

/* 即fwk_irq_domain_find_map, 查找hwirq对应的虚拟中断号 (若hwirq尚未映射, 返回负值) */
kint32_t fwk_irq_find_mapping(struct fwk_irq_domain *sptr_domain, kuint32_t type, kint32_t hwirq);
/*!
 * 解析sptr_irq得到hwirq, 通过fwk_irq_domain::sptr_ops->xlate函数完成;
 * 若该hwirq已存在映射的虚拟中断号, 直接返回虚拟中断号; 
 * 否则调用fwk_irq_domain_alloc_irqs, 为其申请一个fwk_irq_desc, 注册到基数树, 然后返回对应的虚拟中断号.
 * 若fwk_irq_domain::sptr_ops->alloc有定义, 将调用alloc函数创建并初始化fwk_irq_chip, 并与domain关联
 */
kint32_t fwk_irq_create_of_mapping(struct fwk_of_phandle_args *sptr_irq);
/* 合并操作: fwk_of_irq_parse_one + fwk_irq_create_of_mapping, 节点属性保存到sptr_irq后直接解析 */
kint32_t fwk_irq_of_parse_and_map(struct fwk_device_node *sptr_node, kuint32_t index);
/* 返回: fwk_irq_of_parse_and_map(sptr_node, index) */
kint32_t fwk_of_irq_get(struct fwk_device_node *sptr_node, kuint32_t index);

/*!
 * 创建并注册一个fwk_irq_domain
 *      sptr_node: 中断控制器所在的设备树节点
 *      size: 该中断控制器下属的硬件中断号数量 (如gpio中断控制器, 中断号数量即引脚个数: 32)
 *      sptr_ops: 中断控制器解析相关的操作函数集(xlate, alloc, free)
 *      host_data: 私有数据, 一般为NULL
 * 以上参数填充给申请得到的fwk_irq_domain
 */
struct fwk_irq_domain *fwk_irq_domain_add_linear(struct fwk_device_node *sptr_node, kuint32_t size,
                                    const struct fwk_irq_domain_ops *sptr_ops, void *host_data);
/* 调用fwk_irq_domain_add_linear, 并补充fwk_irq_domain的sptr_parent和name成员 */
struct fwk_irq_domain *fwk_irq_domain_add_hierarchy(struct fwk_irq_domain *sptr_parent, struct fwk_device_node *sptr_node, 
                                    kuint32_t size, const struct fwk_irq_domain_ops *sptr_ops, void *host_data);
/* 注销中断控制器, 挂在此控制器下的所有虚拟中断号也将同时被释放 */
void fwk_irq_domain_del_hierarchy(struct fwk_irq_domain *sptr_domain);

/* 根据name从已注册的中断控制器查找到匹配者并返回 (要求: hwirq要在该匹配者的硬件中断号范围内, 即0 ~ fwk_irq_domain::hwirq_max) */
struct fwk_irq_domain *fwk_irq_get_domain_by_name(kchar_t *name, kint32_t hwirq);
/* 直接返回sptr_domain->revmap[hwirq], 获得hwirq对应的虚拟中断号 */
kint32_t fwk_irq_get_by_domain(struct fwk_irq_domain *sptr_domain, kint32_t hwirq);
/* 合并操作: fwk_irq_get_domain_by_name + fwk_irq_get_by_domain */
kint32_t fwk_irq_get_by_domain_name(kchar_t *name, kint32_t hwirq);
```

fwk_irq_domain_add_hierarchy：这是注册中断控制器的关键函数，可从内存池中申请创建一个fwk_irq_domain，并初始化，添加到全局中断控制器链表中（注册）；
fwk_irq_of_parse_and_map：这是映射虚拟中断号的关键函数，会为每个要映射的硬件中断号创建专属的fwk_irq_desc，并初始化，再返回分配到的虚拟中断号。

fwk_irq_domain代表一个中断控制器的基本信息：设备树节点、解析节点的操作方法、硬件中断号数量、线性的中断号映射表revmap；
fwk_irq_desc代表一个中断号的基本信息：虚拟中断号与基数树的纽带（用于注册和查找）、挂在该中断号上的中断回调函数集合；
fwk_irq_data则具体到该中断号的硬件信息：提供该虚拟中断号对应的硬件中断号、操作硬件（中断开关）的fwk_irq_chip函数集。
这里的fwk_irq_chip函数集不包含中断寄存器，也不包含中断号对应的位域，要操作中断控制器（硬件），最好是配合结构体fwk_irq_generic使用。

```c
struct fwk_irq_chip 
{
    /* 中断使能 */
    void (*irq_enable) (struct fwk_irq_data *sptr_data);
    /* 中断失能 */
    void (*irq_disable) (struct fwk_irq_data *sptr_data);
    /* 设置中断掩码(若位域刚好是使能位, 效果如同"irq_enable") */
    void (*irq_mask) (struct fwk_irq_data *sptr_data);
    /* 清除中断掩码(若位域刚好是使能位, 效果如同"irq_disable") */
    void (*irq_unmask) (struct fwk_irq_data *sptr_data);
    /* 获取中断状态寄存器(中断是否发生) */
    kbool_t (*irq_ack) (struct fwk_irq_data *sptr_data);
    /* 中断使能位是否置true */
    kbool_t (*irq_is_enabled) (struct fwk_irq_data *sptr_data);
    /* 设置中断触发类型 */
    kint32_t (*irq_set_type) (struct fwk_irq_data *sptr_data, kuint32_t type);
};

/* 一般赋值给fwk_irq_data::handler_data */
struct fwk_irq_generic
{
    kuaddr_t manage_reg;                    /* 中断控制寄存器的地址, 也可以不设置, 这样fwk_irq_chip完全由private_data提供硬件基本信息 */
    kuaddr_t status_reg;                    /* 中断状态寄存器的地址, 也可以不设置, 这样fwk_irq_chip完全由private_data提供硬件基本信息 */

    struct fwk_irq_chip sgtc_chip;          /* 中断控制器的操作函数集 (manage_reg和status_reg的使用者) */
    void *private_data;                     /* 私有数据, 一般是驱动程序的私有数据指针, 可通过sgtc_chip反推得到fwk_irq_generic, 再取得private_data */
};

/* 使能中断. 如: *(kuaddr_t *)manage_reg |= fwk_irq_data::mask */
void fwk_enable_irq(kint32_t irq);
/* 失能中断. 如: *(kuaddr_t *)manage_reg &= ~fwk_irq_data::mask */
void fwk_disable_irq(kint32_t irq);
/* 设置中断触发类型(上升沿/下降沿) */
void fwk_irq_set_type(kint32_t irq, kuint32_t flags);
/* 检查: *(kuaddr_t *)status_reg & fwk_irq_data::mask */
kbool_t fwk_irq_is_acked(kint32_t irq);
/* 检查: *(kuaddr_t *)manage_reg & fwk_irq_data::mask */
kbool_t fwk_irq_is_enabled(kint32_t irq);

/* 为每个中断的fwk_irq_data赋值: mask = (1 << (irq_base + i)), handler_data = sptr_gc, sptr_chip = &sptr_gc->sgtc_chip, chip_data = chip_data */
void fwk_irq_setup_generic_chip(kint32_t irq_base, kuint32_t irq_max, struct fwk_irq_generic *sptr_gc, void *chip_data);
/* 擦除fwk_irq_data的mask, handler_data, sptr_chip和chip_data成员 */
void fwk_irq_shutdown_generic_chip(kint32_t irq_base, kuint32_t irq_max);

/* 返回: mr_container_of(sptr_data->sptr_chip, struct fwk_irq_generic, sgtc_chip) */
struct fwk_irq_generic *fwk_irq_get_generic_data(struct fwk_irq_data *sptr_data);
```

manage_reg和status_reg可以不设置，完全由private_data内部实现fwk_irq_chip各个函数。在HeavenFox中，intc和gpc中断控制器的中断控制不依赖于manage_reg，由private_data指向的GIC完成；而gpio中断控制器需由fwk_irq_data::mask和fwk_irq_generic::manage_reg共同完成（寄存器地址和位掩码）。

##### 8.4.3. 中断申请与执行
每个中断号都可以挂接一个或多个回调函数，即作为该中断的中断服务程序。每个回调函数都使用一个fwk_irq_action结构体来描述：
```c
/* 中断回调函数可选的返回值 */
enum __ER_IRQ_RETURN_VAL
{
    NR_IRQ_NONE = 0,                        /* 0, 无意义 */
    NR_IRQ_HANDLED = mr_bit(0),             /* 1, 表示回调函数正常结束 */
    NR_IRQ_WAKE_THREAD = mr_bit(1),         /* 2, 表示回调函数结束后, 通知顶层中断服务程序唤醒中断线程 (中断线程化, 下半部) */
};
typedef enum __ER_IRQ_RETURN_VAL irq_return_t;

/* irq为当前中断回调函数对应的中断号(虚拟中断号), args来自于struct fwk_irq_action::ptrArgs, 是要提供给回调函数的参数 */
typedef irq_return_t (*irq_handler_t)(kint32_t irq, void *args);

struct fwk_irq_action
{
    kchar_t name[32];                       /* 中断服务程序的名字, 不允许同名 */
    irq_handler_t handler;                  /* 中断服务函数 */
    kuint32_t flags;                        /* 标志, 可表示中断触发类型 */
    void *ptrArgs;                          /* 参数指针 */

    struct list_head sgtc_link;             /* 当前中断挂接的所有回调函数, 组成链表; 链表头位于struct fwk_irq_desc::sgtc_action */
};
```

即：定义或者申请一个fwk_irq_action结构体，并提供回调函数（handler）、回调函数参数（ptrArgs），插入到该中断的链表头struct fwk_irq_desc::sgtc_action，即完成一个回调函数的注册；发生中断时，顶层中断服务程序将根据硬件中断号找到对应的虚拟中断号和fwk_irq_desc结构体，之后逐个遍历链表头sgtc_action，执行每一个回调函数。
HeavenFox提供以下函数，用于回调函数注册和释放：
```c
/* 根据irq找到fwk_irq_desc, 找到name和ptrArgs都匹配的fwk_irq_action */
void *fwk_find_irq_action(kint32_t irq, const kchar_t *name, void *args);
/*!
 * 申请(注册)一个中断线程
 * irq: 中断号
 * handler: 中断回调函数 (上半部), 赋值给fwk_irq_action::handler
 * thread_fn: 创建中断线程后, 作为中断线程的回调函数 (下半部)
 * flags: 中断标志, 赋值给fwk_irq_action::flags, 一般表示中断触发方式 (如电平触发, 边沿触发)
 * name: 中断回调的名称, 赋值给fwk_irq_action::name
 * ptrDev: 私有参数指针, 赋值给fwk_irq_action::ptrArgs, 上半部和下半部(中断线程)均将获取它
 */
kint32_t fwk_request_threaded_irq(kint32_t irq, irq_handler_t handler, irq_handler_t thread_fn, 
                                kuint32_t flags, const kchar_t *name, void *args);
/* 即: fwk_request_threaded_irq(irq, handler, mr_nullptr, flags, name, args); */
kint32_t fwk_request_irq(kint32_t irq, irq_handler_t handler, kuint32_t flags, const kchar_t *name, void *ptrDev);
/* 注销指定的回调函数 (遍历, 找到"ptrArgs == args"的fwk_irq_action, 将它从链表中脱离) */
void fwk_free_irq(kint32_t irq, void *args);
/* 销毁整个中断fwk_irq_desc::sgtc_action */
void fwk_destroy_irq_action(kint32_t irq);
```

#### 8.5. 定时器
HeavenFox需要两个时钟，一个周期性循环，作为系统时钟心跳，周期可以是10ms、5ms、或者1ms；另一个是长久性计时器，用于系统时间统计，精度一般为ns级。
系统定时器周期由宏CONFIG_HZ决定，当其设置为100时，周期 = 1 / 100 = 10ms；设置为1000时，周期为1ms。全局变量jiffies记录每一次周期，比如周期是10ms，则jiffies每递增1次，时间流过10ms。jiffies是一个32位类型，当周期为1ms时，最大只能记录49天；另一个全局变量jiffies_all则是64位类型，常用于记录系统运行时间。
长久性计时器又称为高精度定时器，1个Tick的周期一般只有几纳秒（取决于CPU选用的定时器能力），常用于记录时标，如数据包发送时刻。高精度定时器一般是64位，如xc7z010的全局定时器；也有32位的，如imx6ull的GPT定时器，需要额外记录溢出次数，则当前时刻 = 溢出次数 * 4294967296 + 当前定时器的值。
两个定时器都必须使能中断，系统定时器只需要周期中断（比较中断），而高精度定时器需要溢出中断，有时也可以使能比较中断。

##### 8.5.1. 系统定时器
对于系统定时器，中断回调函数需要：
> 1）更新jiffies和jiffies_all，即自增；
> 2）处理定时事件。

如果希望每隔一段时间处理一个相同事件，可以将事件放在系统定时器的中断处理程序中。定时器支持挂接多个事件，通过“struct timer_list”，即定时器链表来管理：
```c
struct timer_list 
{
    struct list_head sgtc_link;                     /* 每一个事件都需要挂接到系统定时器链表中 */
    kutime_t expires;                               /* 预期的超时时间(单位: jiffies), 当jiffies大于或等于expires, 事件时间到, 可以执行 */

    void (*entry)(kuint32_t args);                  /* 事件. 时间到则执行此函数 */
    kuint32_t data;                                 /* 事件函数entry的参数 */
};

/* 定义timer_list时初始化使用 */
#define TIMER_INITIALIZER(_entry, _expires, _data)  \
{   \
    .expires = _expires,    \
    .entry = _entry,    \
    .data = _data,  \
}

/* 定义1个定时器事件 */
#define DEFINE_TIMER(_name, _entry, _expires, _data)    \
    struct timer_list _name = TIMER_INITIALIZER(_entry, _expires, _data)

/* 初始化定时器事件 */
#define mr_setup_timer(timer, fn, data)    \
    do {    \
        init_list_head(&(timer)->sgtc_link);  \
        (timer)->entry = (fn); \
        (timer)->data = (data);   \
    } while (0)

/* 初始化定时器事件, 同mr_setup_timer */
void setup_timer(struct timer_list *sptr_timer, void (*entry)(kuint32_t), kuint32_t data);
/* 添加定时器到系统定时器事件链表 */
void add_timer(struct timer_list *sptr_timer);
/* 删除定时器事件 */
void del_timer(struct timer_list *sptr_timer);
/* 变更定时器事件的expires时间值, 改变事件的到期时间 */
void mod_timer(struct timer_list *sptr_timer, kutime_t expires);
/* 所有定时器事件的遍历和处理函数 */
void do_timer_event(void);

/* jiffies和时间单位互转 */
kuint64_t jiffies_to_secs(const kuint64_t j);
kuint64_t jiffies_to_msecs(const kuint64_t j);
kuint64_t jiffies_to_usecs(const kuint64_t j);
kuint64_t jiffies_to_nsecs(const kuint64_t j);
kutime_t secs_to_jiffies(const kuint32_t s);
kutime_t msecs_to_jiffies(const kuint32_t m);
kutime_t usecs_to_jiffies(const kuint32_t u);
kutime_t nsecs_to_jiffies(const kuint64_t n);
```

##### 8.5.2. 高精度定时器
当系统定时器周期低到1ms时，高精度定时器处理定时事件的功能将被弱化，只有us级定时才可能用到它。我们通常会直接获取它的时间，鲜少使用它的中断，因为它并非周期中断，事件只能依赖于比较器中断和溢出中断，而前者需频繁更改比较寄存器的值（如us级定时，两次事件相隔时间极短），可能导致寄存器不稳定，或者事件错过。
虽然如此，高精度定时器仍然支持事件链接和处理功能，并提供时间获取的相关函数接口：
```c
enum __ERT_KTIMER_STATUS
{
    NR_KTIMER_COUNTING = 0U,                        /* 当前事件的状态: 时间未到, 正在计时 */
    NR_KTIMER_PENDING,                              /* 当前事件的状态: 时间已到, 尚未执行, 先标记 */
    NR_KTIMER_INACTIVE,                             /* 当前事件的状态: 事件已执行, 且后续不再执行, 准备取消定时 */
};

struct hrtimer_list
{
    struct list_head sgtc_link;                     /* 每一个事件都需要挂接到高精度定时器链表中 */
    khrtime_t expires;                              /* 预期的超时时间(单位: tick, ns级), 当tick大于或等于expires, 事件时间到, 可以执行 */

    void (*entry)(kuint32_t args);                  /* 事件. 时间到则执行此函数 */
    kuint32_t data;                                 /* 事件函数entry的参数 */

    enum __ERT_KTIMER_STATUS status;                /* 标记本事件的状态 */
};

/* 定义hrtimer_list时初始化使用 */
#define HRTIMER_INITIALIZER(_entry, _expires, _data)    \
{   \
    .expires = _expires,    \
    .entry = _entry,    \
    .data = _data,  \
    .status = NR_KTIMER_COUNTING,   \
}

/* 定义1个定时器事件 */
#define mr_setup_hrtimer(timer, fn, data)    \
    do {    \
        init_list_head(&(timer)->sgtc_link);  \
        (timer)->entry = (fn); \
        (timer)->data = (data);   \
        (timer)->status = NR_KTIMER_COUNTING;   \
    } while (0)

/* 初始化定时器事件 */
#define DEFINE_HRTIMER(_name, _entry, _expires, _data)  \
    struct hrtimer_list _name = HRTIMER_INITIALIZER(_entry, _expires, _data)

/* 初始化定时器事件, 同mr_setup_hrtimer */
void setup_hrtimer(struct hrtimer_list *sptr_timer, void (*entry)(kuint32_t), kuint32_t data);
/* 添加定时器到系统定时器事件链表 */
void add_hrtimer(struct hrtimer_list *sptr_timer);
/* 删除定时器事件 */
void del_hrtimer(struct hrtimer_list *sptr_timer);
/* 变更定时器事件的expires时间值, 改变事件的到期时间 */
void mod_hrtimer(struct hrtimer_list *sptr_timer, khrtime_t expires);
/* 所有定时器事件的遍历和处理函数 */
void do_hrtime_event(void);

/* 获取自定时器启动以来走过的tick数; 对于64位定时器, 即当前定时器值 */
khrtime_t khrtime_ticks(void);
/* tick转time_val */
void ktime_to_spec(struct time_val *sptr_tval);
/* 如果定时器使能了比较器中断, 该函数用于装载比较值 */
void khrtime_reload_cnt(khrtime_t expires);

/* tick和时间单位互转. HRTIMER_FREQ为定时器频率, 即1s内有多少个tick */
#define SEC_TO_HRTICK(sec)                          ( (khrtime_t)(sec ) * HRTIMER_FREQ)
#define MSEC_TO_HRTICK(msec)                        (((khrtime_t)(msec) * HRTIMER_FREQ + 999ULL) / 1000ULL)
#define USEC_TO_HRTICK(usec)                        (((khrtime_t)(usec) * HRTIMER_FREQ + 999999ULL) / 1000000ULL)
#define NSEC_TO_HRTICK(nsec)                        (((khrtime_t)(nsec) * HRTIMER_FREQ + 999999999ULL) / 1000000000ULL)
#define HRTICK_TO_SEC(tick)                         ( (khrtime_t)(tick) / HRTIMER_FREQ)
#define HRTICK_TO_MSEC(tick)                        (((khrtime_t)(tick) * 1000ULL) / HRTIMER_FREQ)
#define HRTICK_TO_USEC(tick)                        (((khrtime_t)(tick) * 1000000ULL) / HRTIMER_FREQ)
#define HRTICK_TO_NSEC(tick)                        (((khrtime_t)(tick) * 1000000000ULL) / HRTIMER_FREQ)
```

#### 8.6. 通知链
线程之间、驱动程序之间可以使用通知链，即其他程序的某个条件满足后，可以“通知”另一个程序，或者说，执行另一个程序预先注册的事件。
通知链结构十分简单，通知方负责遍历通知链表，发现条件满足则对应的事件函数；而被通知方只需要创建一个通知链表，登记好事件函数入口、触发条件，然后注册到通知方的通知链表即可。
```c
/* 通知链表 */
struct fwk_notifier_chain
{
    struct list_head sgtc_nbs;                      /* 被通知方需要挂接到此 */
    struct mutex_lock sgtc_lock;                    /* 内置互斥锁 */
};

/* 定义时初始化 */
#define __NOTIFIER_CHAIN_INITIALIZE(n)   \
    {   \
        .sgtc_nbs = LIST_HEAD_INIT(&(n).sgtc_nbs),  \
        .sgtc_lock = MUTEX_LOCK_INIT(), \
    }

/* 定义一个通知链 */
#define BLOCKING_NOTIFIER_HEAD(name)    \
    struct fwk_notifier_chain name = __NOTIFIER_CHAIN_INITIALIZE(name)

/* 声明一个通知链 */
#define BLOCKING_NOTIFIER_DECLARE(name) \
    extern struct fwk_notifier_chain name

typedef kint32_t (*notifier_fn_t)(struct fwk_notifier_block *sptr_nb, kuint32_t event, void *args);

/*!< 通知块 */
struct fwk_notifier_block
{
    notifier_fn_t notifier_call;                    /* 被通知时执行此函数 */
    notifier_fn_t pengding_call;                    /* 反向通知, 通知方可以等待被通知方反馈消息 */
    kuint32_t expect_event;                         /* 条件, 预期的通知内容. 通知方通知的消息与expect_event存在相同项时, notifier_call才会被执行 */
    void *data;                                     /* 私有数据, 由被通知方自己使用 */

    struct list_head sgtc_link;                     /* 通知块需要链接到通知链 */
};

/* 注册一个通知块到通知链 (链表插入) */
kint32_t fwk_blocking_notifier_chain_register(struct fwk_notifier_chain *sptr_chain, struct fwk_notifier_block *sptr_nb);
/* 注销通知块 (链表删除) */
void fwk_blocking_notifier_chain_unregister(struct fwk_notifier_chain *sptr_chain, struct fwk_notifier_block *sptr_nb);
/* 遍历通知链, 当event与某个通知块的expect_event存在相同项时, "通知"对方 */
kint32_t fwk_blocking_notifier_call_chain(struct fwk_notifier_chain *sptr_chain, kuint32_t event, void *args);
/* 遍历通知链, 当event与某个通知块的expect_event存在相同项时, 等待对方"通知" */
kint32_t fwk_blocking_pengding_call_chain(struct fwk_notifier_chain *sptr_chain, kuint32_t event, void *args);
```

#### 8.7. 平台设备、驱动与总线
##### 8.7.1. 设备
设备可以指具体的某个元器件（硬件），也可以指某个模块（虚拟/软件）；如果是元器件，设备需要描述器件与CPU的接口方式（IO引脚、中断号）、器件自身的资源（寄存器地址）。
将所有设备组成链表，这个链表则被称为平台总线。HeavenFox有几种平台总线，根据设备接口方式划分，如虚拟总线（platform_bus），i2c总线（i2c_bus），当一个设备与CPU之间采用i2c协议通信时，那它就是一个i2c设备，将被挂接到i2c总线，而不是虚拟总线；如果设备接口方式简单，如IO设备（LED、PWM），将被挂接到虚拟总线。
为什么platform_bus被称为虚拟总线？因为对于i2c_bus，那在硬件层次上确确实实存在“总线”，有i2c控制器、i2c设备；而platform_bus用来挂接简单的设备，它在硬件上可能并不存在“总线”，如GPIO、定时器、UART，属于点对点连接，不会有一个cpu控制器挂多个设备的情况。
HeavenFox使用设备树来描述一个设备：
```c
gpt1: gpt@02098000 {
    compatible = "fsl,imx6ul-gpt", "fsl,imx31-gpt";
    reg = <0x02098000 0x4000>;
    interrupts = <GIC_SPI 55 IRQ_TYPE_LEVEL_HIGH>;
    clocks = <&clks IMX6UL_CLK_GPT1_BUS>,
            <&clks IMX6UL_CLK_GPT_3M>;
    clock-names = "ipg", "osc_per";
};

gpio1: gpio@0209c000 {
    compatible = "fsl,imx6ul-gpio", "fsl,imx35-gpio";
    reg = <0x0209c000 0x4000>;
    interrupts = <GIC_SPI 66 IRQ_TYPE_LEVEL_HIGH>,
                <GIC_SPI 67 IRQ_TYPE_LEVEL_HIGH>;
    gpio-controller;
    #gpio-cells = <2>;
    interrupt-controller;
    #interrupt-cells = <2>;
};
```

gpt1是一个定时器，这里可以获得器件自身的资源：寄存器地址0x02098000，且属于cpu内部设备，可通过中断号55（GIC_SPI，真正的中断号为32 + 55）触发中断（如时间比较、计数器溢出等），并且由时钟开关ipg和osc_per控制；gpio1是一个gpio控制器，也是cpu内部设备，寄存器地址0x0209c000。在imx6ull中，1个gpio控制器可以控制32个io引脚，每个引脚均可产生中断，且中断控制寄存器中，每两位控制1个引脚，故32个引脚共需两个中断控制寄存器，分别对应中断号66和67。
设备资源用fwk_resources来描述：
```c
enum __ERT_DEVICE_RESOURCE_TYPE
{
    NR_DEVICE_RESOURCE_IO  = 0x00000100U,       /* IO资源, 用于描述引脚 */
    NR_DEVICE_RESOURCE_MEM = 0x00000200U,       /* 内存资源, 用于描述内存/寄存器地址 */
    NR_DEVICE_RESOURCE_REG = 0x00000300U,       /* 寄存器资源, 用于描述寄存器地址 */
    NR_DEVICE_RESOURCE_IRQ = 0x00000400U,       /* 中断资源, 用于描述中断号 */
};

struct fwk_resources
{
    kchar_t *name;                              /* 资源名称, 可为NULL */
    kuaddr_t start;                             /* 起始, 对于内存/寄存器资源, 这是起始地址; 对于IO/中断资源, 这是IO引脚编号/中断号 */
    kuaddr_t end;                               /* 结束, 对于内存/寄存器资源, 这是起始地址; 对于IO/中断资源, end无意义 */
    kuint32_t type;                             /* 资源类型, 取自于"enum __ERT_DEVICE_RESOURCE_TYPE" */
};

#define RESOURCE_SIZE(sptr_res)                 (sptr_res ? (sptr_res->end - sptr_res->start + 1) : 0)

/* 同RESOURCE_SIZE */
kusize_t fwk_resource_size(struct fwk_resources *sptr_res);
/* 将地址(start)开始的区域(大小为RESOURCE_SIZE)映射为虚拟地址, 并返回 */
kuaddr_t fwk_address_map(struct fwk_resources *sptr_res);
```

比如gpio1，它的资源为：
```c
struct fwk_resources sgtc_imx6ul_gpio1_resources[] = 
{
    { "DR",     0x0209c000, 0x0209c003, NR_DEVICE_RESOURCE_REG },
    { "GDIR",   0x0209c004, 0x0209c007, NR_DEVICE_RESOURCE_REG },
    { "PSR",    0x0209c008, 0x0209c00b, NR_DEVICE_RESOURCE_REG },
    { "ICR1",   0x0209c00c, 0x0209c00f, NR_DEVICE_RESOURCE_REG },
    { "ICR2",   0x0209c010, 0x0209c013, NR_DEVICE_RESOURCE_REG },
    { "ICR3",   0x0209c014, 0x0209c017, NR_DEVICE_RESOURCE_REG },
    { "IMR",    0x0209c018, 0x0209c01b, NR_DEVICE_RESOURCE_REG },
    { "ISR",    0x0209c01c, 0x0209c01f, NR_DEVICE_RESOURCE_REG },
    { "EDGE",   0x0209c020, 0x0209c023, NR_DEVICE_RESOURCE_REG },

    { NULL,     66,         66,         NR_DEVICE_RESOURCE_IRQ },
    { NULL,     67,         67,         NR_DEVICE_RESOURCE_IRQ },
};
```

gpio1不存在真实的总线，当它作为设备时，会挂接到虚拟总线（platform_bus），此时它就是一个平台设备；使用结构体fwk_platdev来描述：
```c
struct fwk_platdev
{
    kchar_t *name;                              /* 平台设备的名称, 如果设备来自于设备树, 这里指向设备树节点的full_name */
    kint32_t id;                                /* 如果存在同组设备(如gpio1, gpio2), id表示组号(gpio1的id为0, gpio2的id为1). 若设备唯一, id为-1 */

    struct fwk_resources *sptr_resources;       /* 本设备的资源 */
    kusize_t num_resources;                     /* 资源的数量 */

    kchar_t *driver_override;                   /* 如果有一个驱动名为"xxx", 平台设备想直接和它关联, 那此处填"xxx" */
    struct fwk_device sgtc_dev;                 /* 核心结构体, 用于总线的挂接、设备树节点的连结. 属于平台设备、i2c设备等不同设备的通用数据 */
};
```

内核启动时，会调用fwk_of_platform_populate_init函数，将设备树的节点转化为fwk_platdev结构体；正如之前gpio1，将资源填充到fwk_resources，再将资源表格的首地址赋值给fwk_platdev::sptr_resources。
设备树节点能否转化为fwk_platdev，需遵循一定规则，这与Linux高度一致；即：
```
必须条件a: 自身拥有compatible属性，且status属性的值为"ok"或者"okay"(或status属性不存在);

满足以下其中一个条件, 可正常转化:
b. 设备树节点的父节点如果是根节点，且满足条件a;
c. 设备树节点的父节点如果是根节点，且自身compatible属性名为"simple-bus"、"simple-mfd"、"isa"其中一个时, 其子节点(不含孙节点)如果也满足条件a, 则子节点也能转化; 若子节点的compatible属性也是这三个之一, 则递归影响到下一级子节点, 使下一级子节点也能转化为fwk_platdev, 逐级类推;
d. i2c控制器、spi控制器若满足条件b或c, 也将转化为fwk_platdev, 但其子节点将由i2c控制器或spi控制器驱动直接转为i2c/spi设备, 不会转化为fwk_platdev
```

fwk_of_platform_populate_init最终会调用fwk_of_platform_device_create_pdata函数，申请一个fwk_resources数组，将该设备的资源填写到数组，并将首地址和数组资源个数赋值给fwk_platdev::sptr_resources和fwk_platdev::num_resources，然后注册到全局设备链表，挂接到虚拟设备总线（platform_bus）。
注册操作由函数fwk_of_register_platdevice完成，主要依赖于核心结构fwk_device：
```c
struct fwk_device
{
    kchar_t *init_name;                                         /* 设备名, 可取自设备树节点的full_name */
    struct fwk_device *sptr_parent;                             /* 父设备 */

    struct fwk_bus_type *sptr_bus;                              /* 总线核心结构, 当注册到虚拟设备总线时, 此处赋值虚拟设备总线结构指针 */
    struct fwk_device_type *sptr_type;                          /* 设备类型, 当注册到虚拟设备总线时, 此处赋值虚拟设备类型机构指针 */
    struct list_head sgtc_link;                                 /* 关键链表项, 虚拟设备总线使用此链表连接 */
    struct list_head sgtc_leaf;                                 /* 链表项, 所有的设备组成链表(不区分总线) */

    struct fwk_driver *sptr_driver;                             /* 本设备对应的驱动, 每个设备都仅有1个驱动能与之关联 */
    struct fwk_kobject sgtc_kobj;                               /* 用于构建设备列表(虚拟文件系统路径) */

    struct fwk_device_node *sptr_node;                          /* 本设备对应的设备树节点 */
    kint32_t (*release) (struct fwk_device *sptr_dev);          /* 释放本设备的回调函数, 可为NULL */

    struct fwk_pinctrl_dev_info *sptr_pctlinfo;                 /* 本设备如果含有IO (pinctrl), 则关联pinctrl资源结构 */
    void *privData;                                             /* 私有数据指针 */
};

/* 设置sptr_pdev->sgtc_dev.privData = ptrData */
void fwk_platform_set_drvdata(struct fwk_platdev *sptr_pdev, void *ptrData);
/* 返回sptr_pdev->sgtc_dev.privData */
void *fwk_platform_get_drvdata(struct fwk_platdev *sptr_pdev);

/* 添加一个新设备(注册到设备链表) */
kint32_t fwk_device_add(struct fwk_device *sptr_dev);
/* 删除一个设备(从设备链表脱离) */
kint32_t fwk_device_del(struct fwk_device *sptr_dev);
/* 申请一个虚拟平台设备*/
struct fwk_platdev *fwk_platdevice_alloc(const kchar_t *name, kint32_t id);
/* 指定总线为platform_bus, 随即调用fwk_device_add */
kint32_t fwk_platdevice_add(struct fwk_platdev *sptr_platdev);
/* 本质是调用fwk_platdevice_add */
kint32_t fwk_register_platdevice(struct fwk_platdev *sptr_platdev);
/* 调用sptr_platdev->sgtc_dev.release */
kint32_t fwk_unregister_platdevice(struct fwk_platdev *sptr_platdev);

/* 初始化sptr_dev, 如设置sgtc_link链表项自环 */
kint32_t fwk_device_initial(struct fwk_device *sptr_dev);
/* 创建并注册一个新设备, 同时添加虚拟设备文件路径, 建立inode节点(设备号为devNum, 设备类型为type, fmt为路径名称, 同时也作为设备名) */
struct fwk_device *fwk_device_create(kuint32_t type, kuint32_t devNum, kchar_t *fmt, ...);
/* 移除该虚拟设备文件, 并删除 */
kint32_t fwk_device_destroy(struct fwk_device *sptr_dev);

/* 获取设备名 */
kchar_t *fwk_dev_get_name(struct fwk_device *sptr_dev);
/* 设置设备名(支持字符格式) */
void fwk_dev_set_name(struct fwk_device *sptr_dev, kchar_t *name, ...);
/* 删除设备名(删除后应尽快更名) */
void fwk_dev_del_name(struct fwk_device *sptr_dev);

#define mr_dev_get_name(dev)                    fwk_dev_get_name(dev)
#define mr_dev_set_name(dev, fmt, ...)          fwk_dev_set_name(dev, fmt, ##__VA_ARGS__)
#define mr_dev_del_name(dev)                    fwk_dev_del_name(dev)

/*!
 * sptr_node: 当前设备所对应的设备树节点
 * sptr_platdev: 从内存池分配而来, 且已经完成设备资源的填写(如有), 准备注册
 */
kint32_t fwk_of_register_platdevice(struct fwk_device_node *sptr_node, struct fwk_platdev *sptr_platdev)
{
    if (!isValid(sptr_platdev))
        return -ER_FAULT;

    /* 初始化fwk_device(链表自环) */
    if (fwk_device_initial(&sptr_platdev->sgtc_dev))
        return -ER_FAILD;

    /* 填充name和id */
    sptr_platdev->name = sptr_node->full_name;
    sptr_platdev->id = fwk_of_get_alias_id(sptr_node);

    /* 初始化fwk_device */
    sptr_platdev->sgtc_dev.init_name = sptr_node->full_name;
    sptr_platdev->sgtc_dev.sptr_node = sptr_node;
    sptr_platdev->sgtc_dev.release = mr_nullptr;
    sptr_platdev->sgtc_dev.sptr_parent = mr_nullptr;

    /* 指向虚拟设备总线 */
    sptr_platdev->sgtc_dev.sptr_bus = &sgtc_fwk_platform_bus_type;

    /*!
     * 注册新设备:
     *  将sgtc_leaf插入到全局设备链表;
     *  将sgtc_link插入到全局虚拟设备链表(设备总线: sgtc_fwk_platform_bus_type.sptr_buspriv->sgtc_list_devices) 
     */
    return fwk_device_add(&sptr_platdev->sgtc_dev);
}
```

可通过以下API获取设备的资源：
```c
/* 将设备树节点reg属性的第index组属性值解析到sptr_res */
kint32_t fwk_of_address_to_resource(struct fwk_device_node *sptr_node, kuint32_t index, struct fwk_resources *sptr_res);
/* 将设备树节点interrupts属性的第index组属性值解析到sptr_res */
kint32_t fwk_of_irq_to_resource_table(struct fwk_device_node *sptr_node, struct fwk_resources *sptr_res, kuint32_t nr_irqs);
/* type取自"enum __ERT_DEVICE_RESOURCE_TYPE"; index表示该type类型下的第几个资源. 从sptr_pdev->sptr_resources获取指定资源 */
struct fwk_resources *fwk_platform_get_resources(struct fwk_platdev *sptr_pdev, kuint32_t index, kuint32_t type);
/* 相当于fwk_platform_get_resources(sptr_pdev, index, NR_DEVICE_RESOURCE_MEM), 返回fwk_resources::start */
kuaddr_t fwk_platform_get_address(struct fwk_platdev *sptr_pdev, kuint32_t index);
/* 相当于fwk_platform_get_resources(sptr_pdev, index, NR_DEVICE_RESOURCE_MEM), 返回RESOURCE_SIZE */
kusize_t fwk_platform_get_address_size(struct fwk_platdev *sptr_pdev, kuint32_t index);
/* 相当于fwk_platform_get_resources(sptr_pdev, index, NR_DEVICE_RESOURCE_IRQ), 返回fwk_resources::start */
kint32_t fwk_platform_get_irq(struct fwk_platdev *sptr_pdev, kuint32_t index);
/* 获取reg属性值(寄存器地址)后, 同时转化为虚拟地址 */
void *fwk_of_iomap(struct fwk_device_node *sptr_node, kuint32_t index);
```

##### 8.7.2. 总线与驱动
设备（fwk_platdev）提供器件资源（fwk_resources），而器件的操作方法由驱动实现，包括器件初始化、寄存器读写、提供上层应用的访问接口等。设备和驱动都是器件的组成部分，不同设备可能存在相同的寄存器和操作方法，如at24c02、at24c16，属于同系列的不同规格，可以使用同一套驱动程序，即：一个驱动可以适用于多种设备，但一个设备只能对应唯一的一个驱动。
驱动和设备一样，根据设备的总线类型（如platform_bus）注册到相对应的全局驱动链表；在HeavenFox中，全局设备链表头和全局驱动链表头均位于结构体fwk_bus_private：
```c
struct fwk_bus_private
{
    struct fwk_bus_type *sptr_bus;                          /* 所属的总线, 如platform_bus */

    struct list_head sgtc_list_devices;                     /* 总线设备链表, 用于链接本总线下的所有设备 */
    struct rw_lock sgtc_device_lock;                        /* 内置读写锁, 保护设备链表 */

    struct list_head sgtc_list_drivers;                     /* 总线驱动链表, 用于链接本总线下的所有驱动 */
    struct rw_lock sgtc_driver_lock;                        /* 内置读写锁, 保护驱动链表 */
};
```

设备和驱动是成套关系，当设备注册后，将遍历驱动链表，从中找到能与该设备“匹配”的驱动；反过来，驱动注册后，也会从设备链表中找到能与该驱动“匹配”的设备。匹配方法由总线结构体fwk_bus_type提供：
```c
struct fwk_bus_type
{
    /* 总线名称, 如"platform" */
    kchar_t *name;

    /* 设备-驱动匹配函数(方法) */
    kint32_t (*match) (struct fwk_device *sptr_dev, struct fwk_driver *sptr_driver);
    /* 当设备-驱动匹配成功后, 执行probe函数 */
    kint32_t (*probe) (struct fwk_device *sptr_dev);
    /* 当设备或驱动任一方注销时, 执行remove函数 */
    kint32_t (*remove) (struct fwk_device *sptr_dev);

    /* 总线所辖的设备-驱动链表 */
    struct fwk_bus_private *sptr_buspriv;
};
```

match函数由相应的总线提供，platform_bus、i2c_bus都可以自定义自己的匹配规则；这里的fwk_device和fwk_driver也将来自于对应的设备、驱动结构，如平台设备fwk_platdev；相应地，平台驱动使用fwk_platdrv表示：
```c
struct fwk_id_table
{
    kchar_t *name;                                          /* 要匹配的设备名称, 若设备由设备树转化而来, 如compatible = "fsl, xxx", 则此处的name可填入"xxx" */
    kint32_t driver_data;                                   /* 驱动私有数据, 不同设备可对应不同的驱动私有数据, 一般是器件的私有资源(如寄存器偏移地址) */
};

struct fwk_platdrv
{
    kint32_t (*probe) (struct fwk_platdev *sptr_dev);       /* 当设备与驱动匹配成功后, 此处的probe函数将被fwk_bus_type::probe调用 */
    kint32_t (*remove) (struct fwk_platdev *sptr_dev);      /* 当驱动注销时, 此处的remove函数将被fwk_bus_type::remove调用 */

    struct fwk_id_table *sptr_idTable;                      /* 希望匹配的设备列表; 设备可以来自于设备树, 也可以自编写设备程序提供 */
    kusize_t num_idTable;                                   /* 希望匹配的设备数量(即fwk_id_table列表的个数) */

    struct fwk_driver sgtc_driver;                          /* 和fwk_device一样, fwk_driver是驱动的通用结构 */
};
```

其中fwk_driver用于描述一个驱动：
```c
struct fwk_of_device_id
{
    const kchar_t *name;                                    /* 设备树节点name属性 (暂无用途, 可赋值为NULL) */
    const kchar_t *type;                                    /* 设备树节点type属性 (暂无用途, 可赋值为NULL) */
    const kchar_t *compatible;                              /* 设备树节点compatible属性, 与fwk_id_table不同, 此处名称应完整, 如"fsl, xxx" */

    const void *data;                                       /* 驱动私有数据, 不同设备可对应不同的驱动私有数据, 一般是器件的私有资源(如寄存器偏移地址) */
};

struct fwk_driver
{
    kchar_t *name;                                          /* 驱动名称 */
    kint32_t id;                                            /* 驱动编号 */

    kint32_t matches;                                       /* 驱动已匹配设备的数量, 1个驱动可以匹配多个设备 */

    const struct fwk_of_device_id *sptr_of_match_table;     /* 希望匹配的设备列表, 必须来自于设备树; 若不希望匹配设备树中的设备, 可填NULL */

    struct fwk_bus_type *sptr_bus;                          /* 本驱动要挂接的总线, 如platform_bus */
    struct list_head sgtc_link;                             /* 链接到驱动链表struct fwk_bus_private::sgtc_list_drivers */

    kint32_t (*probe) (struct fwk_device *sptr_dev);        /* 暂无用途 */
    kint32_t (*remove) (struct fwk_device *sptr_dev);       /* 暂无用途 */
};

/* 注册一个驱动(到指定总线的驱动链表) */
kint32_t fwk_driver_register(struct fwk_driver *sptr_driver);
/* 注销一个驱动(从总线的驱动链表中脱离) */
kint32_t fwk_driver_unregister(struct fwk_driver *sptr_driver);
/* 指定总线为platform_bus, 然后注册驱动 */
kint32_t fwk_register_platdriver(struct fwk_platdrv *sptr_platdrv);
/* 注销一个驱动(从platform_bus的驱动链表中脱离) */
kint32_t fwk_unregister_platdriver(struct fwk_platdrv *sptr_platdrv);
```

##### 8.7.3. 设备驱动匹配
以platform_bus总线为例，设备（fwk_device）和驱动（fwk_driver）注册后会分别挂接到链表fwk_bus_private::sgtc_list_devices和fwk_bus_private::sgtc_list_drivers，先注册成功的一方将遍历另一方的链表，依据以下规则找到匹配的对象。可见于match函数：
```c
kint32_t fwk_platform_match(struct fwk_device *sptr_dev, struct fwk_driver *sptr_driver)
{
    struct fwk_platdev *sptr_platdev;
    struct fwk_platdrv *sptr_platdrv;
    struct fwk_id_table *sptr_idTable;
    struct fwk_device_node *sptr_np;
    kuint32_t idTable_cnt;

    /* sptr_dev来自于结构体fwk_platdev, sptr_driver来自于结构体fwk_platdrv, 可反向获取二者 */
    sptr_platdev = mr_container_of(sptr_dev, struct fwk_platdev, sgtc_dev);
    sptr_platdrv = mr_container_of(sptr_driver, struct fwk_platdrv, sgtc_driver);

    /*!< 规则1: driver_override最优先, 由设备(主动方)指定驱动 */
    if (sptr_platdev->driver_override)
        return ((!kstrcmp(sptr_platdev->driver_override, sptr_driver->name)) ? ER_NORMAL : (-ER_FAILD));

    /*!< 规则2: 设备树节点匹配, 由驱动(主动方)编写of_match_table来指定可匹配的设备节点 */
    sptr_np = fwk_of_node_try_matches(sptr_dev->sptr_node, sptr_driver->sptr_of_match_table, mr_nullptr);
    if (isValid(sptr_np) && (sptr_dev->sptr_node == sptr_np))
        return ER_NORMAL;

    /*!< 规则3: 设备列表匹配, 由驱动(主动方)编写id_table来指定可匹配的设备(不一定是设备树节点, 可以是人为编写设备程序提供) */
    for (idTable_cnt = 0; idTable_cnt < sptr_platdrv->num_idTable; idTable_cnt++)
    {
        sptr_idTable = sptr_platdrv->sptr_idTable + idTable_cnt;

        if (!kstrcmp((char *)sptr_platdev->name, (char *)sptr_idTable->name))
            return ER_NORMAL;
    }

    /*!< 规则4: 优先级最低, 通过名字直接匹配 */
    if (!kstrcmp((char *)sptr_platdev->name, (char *)sptr_driver->name))
        return ER_NORMAL;

    return -ER_NOTFOUND;
}
```

匹配成功将返回ER_NORMAL（错误码：0），随即调用fwk_device_driver_probe函数；若当前设备-驱动匹配失败，则继续查找，直到匹配成功，或链表遍历完（匹配失败）。匹配失败的设备/驱动不会调用fwk_device_driver_probe。
```c
kint32_t fwk_device_driver_match(struct fwk_device *sptr_dev, struct fwk_bus_type *sptr_bus_type, void *ptr_data)
{
    struct fwk_driver *sptr_driver;
    kint32_t retval;

    sptr_driver = (struct fwk_driver *)ptr_data;

    /* 即: fwk_platform_match */
    retval = sptr_bus_type->match(sptr_dev, sptr_driver);
    if (retval < 0)
        return -ER_NOTFOUND;

    /* 每个设备均仅能匹配1个驱动, 并将匹配的驱动保存到设备数据结构体中 */
    sptr_dev->sptr_driver = sptr_driver;
    return fwk_device_driver_probe(sptr_dev);
}

/* 匹配成功后执行 */
kint32_t fwk_device_driver_probe(struct fwk_device *sptr_dev)
{
    struct fwk_bus_type *sptr_bus_type;
    kint32_t retval;

    sptr_bus_type = sptr_dev->sptr_bus;

    /*!< probe函数用于初始化驱动, 常伴随硬件的初始化; 在此之前, 先绑定IO引脚, 并初始化IO(方向, 电平, 输出能力等); 前提是pinctrl存在 */
    if (fwk_pinctrl_bind_pins(sptr_dev))
        print_warn("bind pinctrl faild, device is: %s\r\n", mr_dev_get_name(sptr_dev));

    if (sptr_bus_type->probe)
    {
        /* 即: fwk_platform_probe */
        retval = sptr_bus_type->probe(sptr_dev);
        if (retval)
        {
            /* probe返回错误, 应解除引脚绑定 */
            fwk_pinctrl_unbind_pins(sptr_dev);
            print_err("probe device and driver failed! device is: %s\r\n", sptr_dev->init_name);

            return retval;
        }

        return ER_NORMAL;
    }

    return -ER_NSUPPORT;
}

kint32_t fwk_platform_probe(struct fwk_device *sptr_dev)
{
    struct fwk_driver  *sptr_driver;
    struct fwk_platdev *sptr_platdev;
    struct fwk_platdrv *sptr_platdrv;

    sptr_driver = sptr_dev->sptr_driver;
    if (!sptr_driver)
        return -ER_NOTFOUND;

    sptr_platdev = mr_container_of(sptr_dev, struct fwk_platdev, sgtc_dev);
    sptr_platdrv = mr_container_of(sptr_driver, struct fwk_platdrv, sgtc_driver);

    /* 调用驱动自己的probe函数(由驱动程序提供, 一般是初始化硬件, 并注册到虚拟设备文件系统) */
    if ((!sptr_platdrv->probe) || (0 > sptr_platdrv->probe(sptr_platdev)))
    {
        print_warn("device driver probe anomaly, driver is: %s\r\n", sptr_platdrv->sgtc_driver.name);
        return -ER_PERMIT;
    }

    return ER_NORMAL;
}
```

#### 8.8. pinctrl
IO引脚是CPU的核心功能，1个IO引脚往往包括驱动能力、复用功能、上拉/下拉配置等，驱动开发时当然可以直接操作IO寄存器，或BSP接口进行配置，但这也要求开发者熟悉CPU的硬件资源和配置方法。我们完全可以让开发者专注于外设模块，然后省去阅读CPU芯片手册的烦扰，于是内核将IO统一管理，再提供标准接口给外设驱动的开发者；而IO则交由CPU的专业开发者，由它们来提供接口的下层。
所以：
```
内核工程师: 提供CPU IO的管理接口(数据结构与API);
CPU原厂工程师: 提供IO标准接口的底层代码;
外设驱动工程师: 使用IO标准接口, 无需理会底层
```

在Linux中，这种IO管理接口叫做pinctrl。
先来看一个imx6ull的iomux设备树节点：
```c
iomuxc: iomuxc@020e0000 {
    compatible = "fsl,imx6ul-iomuxc";
    reg = <0x020e0000 0x4000>;

    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_hog_1>;

    imx6ul-evk {
        pinctrl_hog_1: hoggrp-1 {
            fsl,pins = <
            /*  mux_reg     conf_reg    input_reg   mux_mode    input_val   conf_mode */
                0x0090      0x031C      0x0000      0x5         0x0         0x17059    /* SD1 CD */
                0x0070      0x02FC      0x0000      0x4         0x0         0x17059    /* SD1 VSELECT */
                0x005C      0x02E8      0x04B8      0x2         0x0         0x13058    /* USB_OTG1_ID */
            >;
        };
        
        pinctrl_ledgpio: ledgpiogrp {
            fsl,pins = <
            /*  mux_reg     conf_reg    input_reg   mux_mode    input_val   conf_mode */
                0x0068      0x02F4      0x0000      0x5         0x0         0x10b0
            >;
        };

        pinctrl_sil9022a_reset: pinctrl_sil9022a_resetgrp {
            fsl,pins = <
            /*  mux_reg     conf_reg    input_reg   mux_mode    input_val   conf_mode */
                0x0158      0x03E4      0x0000      0x5         0x0         0x10b0     /* Reset pin */
            >;
        };
    };
};

iomuxc_snvs: iomuxc-snvs@02290000 {
    compatible = "fsl,imx6ull-iomuxc-snvs";
    reg = <0x02290000 0x10000>;

    pinctrl-names = "default_snvs";
    pinctrl-0 = <&pinctrl_hog_2>;

    imx6ul-evk {
        pinctrl_hog_2: hoggrp-2 {
            fsl,pins = <
            /*  mux_reg     conf_reg    input_reg   mux_mode    input_val   conf_mode */
                0x0008      0x004C      0x0000      0x5         0x0         0x80000000
            >;
        };

        pinctrl_sil9022a_intr: pinctrl_sil9022a_intrgrp {
            fsl,pins = <
            /*  mux_reg     conf_reg    input_reg   mux_mode    input_val   conf_mode */
                0x0028      0x02B4      0x0000      0x5         0x0         0x10b0     /* Interrupt pin */
            >;
        };
    };
};
```

imx6ull的pinctrl节点，每6个值为1组，表示3个寄存器的偏移地址（相对于iomuxc起始地址0x020e0000）和值，3个寄存器为mux_reg、conf_reg、input_reg，而紧跟着的就是它们的寄存器值mux_mode、input_val、conf_mode，均位于“fsl,pins”属性中。1个“fsl,pins”属性可以有多组pinctrl配置，如节点pinctrl_hog_1。内核将从iomuxc出发，访问imx6ul-evk节点，再遍历其所有子节点，为子节点和“fsl,pins”属性构建pinctrl管理结构。
设备树节点通过“pinctrl-names”和“pinctrl-[数字]”来引用它们：
```c
gpio_tempate {
    compatible = "fsl,template";
    pinctrl-names = "default", "sil9022a_rst";      /* 与pinctrl-0、pinctrl-1对应 */
    pinctrl-0 = <&pinctrl_ledgpio>;                 /* 当pinctrl-name为"default"时, 引脚关联pinctrl_ledgpio */
    pinctrl-1 = <&pinctrl_sil9022a_reset>;          /* 当pinctrl-name为"sil9022a_rst"时, 引脚关联pinctrl_sil9022a_reset */

    status = "okay";
};
```

当我们使用“default”这个配置时，对应“pinctrl-0”，也就是pinctrl_ledgpio；它的“fsl,pins”的6组数组分别表示：
| 寄存器 | 寄存器值（配置） | 含义                                                                                            |
| ------ | ---------------- | ----------------------------------------------------------------------------------------------- |
| 0x0068 | 0x5              | mux，复用功能，0x5表示gpio，通用IO；0x68是相对于iomuxc地址的偏移，即mux_reg = 0x020e0000 + 0x68 |
| 0x02F4 | 0x10b0           | conf，配置功能，如电流驱动能力、上拉/下拉等；配置寄存器地址conf_reg = 0x020e0000 + 0x2F4        |
| 0x0000 | 0x0              | input_reg，不常用                                                                               |

内核将这些引脚信息整合为pinctrl，当引脚被使用时，如gpio_tempate设备树节点，当有驱动匹配它时，将调取pinctrl-names为“default”的pin节点pinctrl_ledgpio，由pinctrl完成引脚的初始化。简单来说，就是：
```c
    /* pinctrl最终操作:  */

    *((volatile unsigned long *)(0x020e0000 + 0x0068)) = 0x5;       /* mux_reg */
    *((volatile unsigned long *)(0x020e0000 + 0x02F4)) = 0x10b0;    /* conf_reg */
    *((volatile unsigned long *)(0x020e0000 + 0x0000)) = 0x0;       /* input_reg */
```

以下分别拆解pinctrl的各个模块。

##### 8.8.1. fwk_pinctrl_map
先从一个边缘的角色：fwk_pinctrl_map说起。它在pinctrl系统中属于“中转”角色，既非内核驱动IO的直接入口，也非对外使用的关键结构，作为中转，它主要完成：将设备树的pinctrl信息整合起来，组成一个个fwk_pinctrl_map结构体，再由pinctrl平台函数去解析，转存为其他关键数据结构；此后它使命终结，开发者不需要再去读取它（而是对接那些已解析好的数据结构）。
首先来看两个结构体：
```c
struct fwk_pinctrl_map_mux 
{
    /* 组的名字, 我们将每个pinctrl视为一个组, 如pinctrl_ledgpio、pinctrl_sil9022a_reset, 均代表一个组. group一般取设备树节点的名字(name), 如: "ledgpiogrp" */
    const kchar_t *group;
    /* 功能的名字, 这一版是组的父节点, 如imx6ul-evk. functions也一般取设备树节点名字(name), 如: "imx6ul-evk" */
    const kchar_t *function;
};

struct fwk_pinctrl_map_configs 
{
    /* 组或引脚的名字, 这个属性如果设备树未提供, 需pinctrl驱动程序自定义. 如: "IMX6ULL_PAD_GPIO1_IO09", 并要求驱动绑定引脚序号和名字, 以便查找 */
    const kchar_t *group_or_pin;

    /* 表示1系列配置, 配置寄存器conf_reg可以分次设置, 如: *conf_reg = configs[0] | configs[1] | ... */
    kuint32_t *configs;
    /* configs数组的个数 */
    kuint32_t num_configs;
};
```

mux和config，明显是针对IO的mux、config两个寄存器，即复用功能和配置功能。正如前文所述，fwk_pinctrl_map用于整合设备树的信息，这里的fwk_pinctrl_map_mux即负责整合mux信息，fwk_pinctrl_map_configs负责conf信息。比如这是一个组：
```c
    imx6ul-evk {
        pinctrl_hog_1: hoggrp-1 {
            fsl,pins = <
            /*  mux_reg     conf_reg    input_reg   mux_mode    input_val   conf_mode */
                0x0090      0x031C      0x0000      0x5         0x0         0x17059    /* SD1 CD */
                0x0070      0x02FC      0x0000      0x4         0x0         0x17059    /* SD1 VSELECT */
                0x005C      0x02E8      0x04B8      0x2         0x0         0x13058    /* USB_OTG1_ID */
            >;
        };
    };
```

转化为fwk_pinctrl_map_mux就是：
```c
struct fwk_pinctrl_map_mux sgtc_mux = { .group = "hoggrp-1", .function = "imx6ul-evk" };
```

转化为fwk_pinctrl_map_configs就是：
```c
struct fwk_pinctrl_map_configs sgtc_conf[] = { 
    { .group_or_pin = "IMX6ULL_PAD_UART1_RTS_B", .configs = { 0x17059 }, .num_configs = 1 },    /* pin = 0x0090 / 0x4 = 0x24 (十进制36, 即第36号引脚, 名字为"UART1_RTS_B") */
    { .group_or_pin = "IMX6ULL_PAD_GPIO1_IO05",  .configs = { 0x17059 }, .num_configs = 1 },    /* 第28号引脚, pinctrl驱动会根据group_or_pin找到数字28 */
    { .group_or_pin = "IMX6ULL_PAD_GPIO1_IO00",  .configs = { 0x13058 }, .num_configs = 1 },    /* 第23号引脚, pinctrl驱动会根据group_or_pin找到数字23 */
};
```

mux并没有表示引脚序号或名称，它是一次性读取hoggrp-1节点下的三个引脚配置，并一次性设置0x0090、0x0070、0x005C三个mux寄存器，它的属性是不能随意更改的（因为没有引脚编号，无法知道要设置哪个引脚），只能通过更新设备树进行调整；但conf寄存器是灵活的，因为我们知道引脚编号，可以修改configs的值来更新该引脚的conf寄存器。

最后将这两个结构体囊括到fwk_pinctrl_map中：
```c
enum __ERT_FWK_PINCTRL_PIN_TYPE
{
    NR_FWK_PINCTRL_PIN_MUX = 0,                         /* 类型为mux */
    NR_FWK_PINCTRL_PIN_CONF,                            /* 类型为conf */
};

struct fwk_pinctrl_map 
{
    const kchar_t *dev_name;                            /* 设备节点名称, 谁引用, 该名称就是谁. 如gpio_tempate节点引用了pinctrl_ledgpio, 那dev_name就是"gpio_tempate" */
    const kchar_t *name;                                /* 本引脚归属的pinctrl-names, 比如pinctrl_ledgpio是被pinctrl-0引用, 那name应为"default" */
    kuint32_t type;                                     /* 引脚属性类型, NR_FWK_PINCTRL_PIN_MUX或NR_FWK_PINCTRL_PIN_CONF */
    const kchar_t *ctrl_dev_name;                       /* pinctrl控制器的名称, 需要从pinctrl_ledgpio回溯, 它的控制器是"iomuxc" */

    union 
    {
        struct fwk_pinctrl_map_mux sgtc_mux;            /* 共用体1: mux, 此时type成员必须为NR_FWK_PINCTRL_PIN_MUX */
        struct fwk_pinctrl_map_configs sgtc_configs;    /* 共用体2: conf, 此时type成员必须为NR_FWK_PINCTRL_PIN_CONF */
    } ugtr_data;
};
```

pinctrl是引用了才解析，而不是一次性读取设备树所有iomuxc的子节点。比如gpio_tempate节点引用了pinctrl_ledgpio，那ledgpiogrp就会被整合成fwk_pinctrl_map：
```c
/* 全局变量 */
struct fwk_pinctrl_map sgtc_maps[2];
kuint32_t config[1] = { 0x10b0 };

/* mux */
sgtc_maps[0].dev_name = "gpio_tempate";
sgtc_maps[0].name = "default";
sgtc_maps[0].type = NR_FWK_PINCTRL_PIN_MUX;             /* 类型: mux */
sgtc_maps[0].ctrl_dev_name = "iomuxc";
sgtc_maps[0].ugtr_data.sgtc_mux.group = "ledgpiogrp";
sgtc_maps[0].ugtr_data.sgtc_mux.function = "imx6ul-evk";

/* conf */
sgtc_maps[1].dev_name = "gpio_tempate";
sgtc_maps[1].name = "default";
sgtc_maps[1].type = NR_FWK_PINCTRL_PIN_CONF;            /* 类型: conf */
sgtc_maps[1].ctrl_dev_name = "iomuxc";
sgtc_maps[1].ugtr_data.sgtc_mux.group_or_pin = "IMX6ULL_PAD_GPIO1_IO03";    /* 引脚编号26 */
sgtc_maps[1].ugtr_data.sgtc_mux.configs = &config;
sgtc_maps[1].ugtr_data.sgtc_mux.num_configs = sizeof(config) / sizeof(config[0]);
```

像pinctrl_ledgpio只有1个引脚，只需定义sgtc_maps[2]就能表示完全；但对于pinctrl_hog_1这种3引脚，需定义sgtc_maps[4]，其中sgtc_maps[0]保存mux信息，sgtc_maps[1] ~ sgtc_maps[3]保存conf信息。故为了表示sgtc_maps数组的个数，还需另一个结构体来管理：
```c
struct fwk_pinctrl_maps 
{
    struct list_head sgtc_link;                         /* 每个group的引脚信息链接到全局链表, 以便获取, 进而解析 */

    struct fwk_pinctrl_map *sptr_maps;                  /* 指向fwk_pinctrl_map数组首地址 */
    kuint32_t num_maps;                                 /* fwk_pinctrl_map数组的个数 */
};
```

获取fwk_pinctrl_maps、fwk_pinctrl_map的方法由各自CPU的芯片原厂驱动提供，与内核无关；芯片驱动需根据自身IO资源的分布和寄存器属性来创建、填写fwk_pinctrl_map，返回给内核。这一步可通过内核提供的接口（钩子函数）实现，等后续介绍完pinctrl控制器，再说明此接口。

##### 8.8.2. 控制器（管理单元）
fwk_pinctrl_map虽然内容丰富，但并不是与驱动程序打交道的接口；它只是pinctrl体系中的底层，为上层数据结构提供mux和conf的信息。
pinctrl必然有一个管理器（控制器），即引脚的管理单元。对于imx6ull而言，管理单元有两个：iomuxc和iomuxc_snvs，内核将管理单元抽象为结构体fwk_pinctrl_dev，即，imx6ull拥有两个fwk_pinctrl_dev。
它的结构为：
```c
struct fwk_pinctrl_dev 
{
    struct list_head sgtc_link;                         /* 控制器也构成全局链表. 当执行控制器"注册"时, 通过sgtc_link插入全局链表 */

    struct fwk_pinctrl_desc *sptr_desc;                 /* 控制器的核心, 用于描述本控制器的引脚信息 */
    struct fwk_device *sptr_dev;                        /* 本控制器的设备信息, 关键是sptr_dev->sptr_node成员, 表征本控制器在设备树中的位置 */

    /* 以下3个成员, 可详见"fwk_pinctrl"章节 */
    struct fwk_pinctrl_state *sptr_hog_default;         /* pinctrl-nams = "default" */
    struct fwk_pinctrl_state *sptr_hog_sleep;           /* pinctrl-nams = "sleep" */
    struct fwk_pinctrl *sptr_pctl;                      /* 控制器也有pinctrl-0、pinctrl-1属性, 所以它也是一个pinctrl的使用者, 每个使用者都被抽象为fwk_pinctrl结构体 */

    struct mutex_lock sgtc_mutex;                       /* 内置互斥锁 */
    void *driver_data;                                  /* 驱动的私有数据资源, 一般由驱动程序自定义, 并借此成员传递 */
};
```

它的核心是fwk_pinctrl_desc结构体，引脚的信息（名称、IO编号）、操作引脚的方法，均定义在此。其定义为：
```c
struct fwk_pinctrl_pin_desc 
{
    kuint32_t number;                                   /* 物理引脚序号, 如GPIO0_31, 填为31; GPIO1_0, 填为32 */
    const kchar_t *name;                                /* 引脚名字, 可自定义, 但应符合芯片手册功能描述. 如: "IMX6ULL_PAD_GPIO1_IO03" */
    void *drv_data;                                     /* 对应的私有数据. 没有则跳过 */

#define FWK_PINCTRL_PIN(a, b)  \
            { .number = a, .name = b }                  /* 便捷赋值 */
};

struct fwk_pinctrl_desc
{
    const kchar_t *name;                                /* 一般取"fwk_platdev::name", 它是在芯片pinctrl驱动的probe函数中创建的. 如"iomuxc"的驱动程序 */

    struct fwk_pinctrl_pin_desc const *sptr_pins;       /* 指向fwk_pinctrl_pin_desc数组的首地址 */
    kuint32_t npins;                                    /* fwk_pinctrl_pin_desc数组的个数 */

    const struct fwk_pinctrl_ops *sptr_pctlops;         /* pinctrl的通用操作函数集, 如解析fwk_pinctrl_map */
    const struct fwk_pinmux_ops *sptr_pmxops;           /* pinctrl关于mux的操作函数集, 如设置mux寄存器 */
    const struct fwk_pinconf_ops *sptr_confops;         /* pinctrl关于conf的操作函数集, 如设置conf寄存器 */
};
```

fwk_pinctrl_pin_desc为每个引脚定义一个名字，名字将被直接用于fwk_pinctrl_map_configs::group_or_pin成员，这在上一章中已有提及。当内核获得group_or_pin的值后，便能从fwk_pinctrl_pin_desc数组中找到它对应的引脚编号，不过这个过程十分浪费时间（需要不断地strcmp），但Linux居然也是这样做的，我们暂且保留这个奇怪的做法，以后寻隙改掉它。
它的三个函数集，其实现：
```c
/*
 * 1个控制器有多个function;
 * 1个function有多个group. group一般名为: "pinctrl_xxx", 如: pinctrl-0 = <&pinctrl_xxx>, 这里引用的就是group;
 * pinctrl驱动程序需构建function数组、group数组, 并包含mux、conf寄存器的偏移地址, iomuxc控制器的基地址, 以便读写寄存器
 */

/* pinctrl通用接口 */
struct fwk_pinctrl_ops
{
    /* 简单的一个操作, 获取当前控制器第selector个function的group首地址和group的数量.  */
    kint32_t (*get_function_groups)(struct fwk_pinctrl_dev *sptr_pctldev,
                                kuint32_t selector, kuaddr_t **groups, kuint32_t * const num_groups);
    /* 获取第func_selector个function下的groups个数 */
    kint32_t (*get_groups_count)(struct fwk_pinctrl_dev *sptr_pctldev, kuint32_t func_selector);
    /* 获取第func_selector个function的第group_selector个group的名字, 其实就是group的设备树节点名(name) */
    const kchar_t *(*get_group_name)(struct fwk_pinctrl_dev *sptr_pctldev, kuint32_t func_selector, kuint32_t group_selector);
    /* 获取本控制器所有引脚中的第number个引脚的名字 (本质是访问sptr_pctldev->sptr_desc->sptr_pins[number].name) */
    const kchar_t *(*get_pin_desc)(struct fwk_pinctrl_dev *sptr_pctldev, kuint32_t number);

    /* sptr_pctldev为IO控制器, sptr_node为group所在的设备树节点. 本函数用于将本group的各个引脚和配置保存到sptr_map. 这是设备树引脚信息转成结构体的最底层步骤 */
    kint32_t (*dt_node_to_map)(struct fwk_pinctrl_dev *sptr_pctldev, struct fwk_device_node *sptr_node, 
                                struct fwk_pinctrl_map **sptr_map, kuint32_t *num_maps);
    /* dt_node_to_map的反操作, 释放sptr_map, 将其还给内存池 */
    void (*dt_free_map)(struct fwk_pinctrl_dev *sptr_pctldev, struct fwk_pinctrl_map *sptr_map, kuint32_t num_maps);
} ;

/* pinctrl操作mux寄存器 */
struct fwk_pinmux_ops 
{
    /* 获取本控制器下的function数量 */
    kint32_t (*get_functions_count)(struct fwk_pinctrl_dev *sptr_pctldev);
    /* 获取第selector个function的名字, 本质是获取该function对应的设备树节点名(name) */
    const kchar_t *(*get_function_name)(struct fwk_pinctrl_dev *sptr_pctldev, kuint32_t selector);
    /* 设置第func_selector个function的第group_selector个group的所有引脚的mux寄存器 (一次性配置该group的所有引脚, 与conf不同) */
    kint32_t (*set_mux) (struct fwk_pinctrl_dev *sptr_pctldev, kuint32_t func_selector, kuint32_t group_selector);
};

/* pinctrl操作conf寄存器 */
struct fwk_pinconf_ops 
{
    /* 将第pin个引脚的conf寄存器值读到*config */
    kint32_t (*pin_config_get)(struct fwk_pinctrl_dev *sptr_pctldev, kuint32_t pin, kuint32_t *config);
    /* 设置*configs数组的值到第pin个引脚的conf寄存器 */
    kint32_t (*pin_config_set)(struct fwk_pinctrl_dev *sptr_pctldev, kuint32_t pin, kuint32_t *configs, kuint32_t num_configs);
};
```

dt_node_to_map是一个关键步骤，正如上一章节所述，它负责申请一个fwk_pinctrl_map数组，并把设备树某个group的引脚配置保存到数组中（mux、多个引脚的conf），为pinctrl构建提供基础。
set_mux、pin_config_set、pin_config_get涉及底层寄存器操作（mux寄存器、conf寄存器），function和group均由CPU的pinctrl驱动自定义，与pinctrl架构无关；内核只需知道，调用这3个函数，可以读写寄存器即可。

以下函数可注册/注销一个pinctrl控制器：
```c
/* 
 * 从内存池分配一个fwk_pinctrl_dev, 并赋值fwk_pinctrl_dev::sptr_desc, fwk_pinctrl_dev::sptr_dev, fwk_pinctrl_dev::driver_data;
 * 然后插入到全局pinctrl控制器链表
 */
struct fwk_pinctrl_dev *fwk_pinctrl_register(struct fwk_pinctrl_desc *sptr_desc, struct fwk_device *sptr_dev, void *driver_data);
/* 将sptr_pctldev从全局控制器链表中脱离, 并还回内存池 */
void fwk_pinctrl_unregister(struct fwk_pinctrl_dev *sptr_pctldev);
```

##### 8.8.3. fwk_pinctrl
fwk_pinctrl_dev仅表示pinctrl控制器，但它并不知道引脚被哪些设备使用。内核对引脚的使用原则是：只有引脚被设备使用时，才会进行配置。
我们已知前面的例子：
```c
gpio_tempate {
    compatible = "fsl,template";
    pinctrl-names = "default", "sil9022a_rst";          /* 与pinctrl-0、pinctrl-1对应 */
    pinctrl-0 = <&pinctrl_ledgpio>;                     /* 当pinctrl-name为"default"时, 引脚关联pinctrl_ledgpio */
    pinctrl-1 = <&pinctrl_sil9022a_reset>;              /* 当pinctrl-name为"sil9022a_rst"时, 引脚关联pinctrl_sil9022a_reset */

    status = "okay";
};
```

pinctrl_ledgpio和pinctrl_sil9022a_reset就是被设备gpio_tempate引用了，当它和驱动程序匹配成功后，设备将运转，其引用的引脚也需完成初始化配置。至于其他未被设备引用的引脚，则不会被初始化。这表示：引脚配置可能是在驱动程序的probe函数中完成的。

首先是将引脚信息解析出来，之前已经通过dt_node_to_map函数将设备树中的引脚信息保存到fwk_pinctrl_map，但fwk_pinctrl_map并不负责与驱动程序的对接，我们需要将它转化为pinctrl的上层结构。如：
```c
struct fwk_pinctrl_mux 
{
    kuint32_t group;                                    /* group编号, 表示是func中的第几个group */
    kuint32_t func;                                     /* function编号, 表示是控制器中的第几个function*/
};

struct fwk_pinctrl_configs 
{
    kuint32_t group_or_pin;                             /* 引脚编号, 第几个引脚 */
    kuint32_t *configs;                                 /* 本引脚的conf寄存器配置值(对应设备树的conf_value, 用于写到conf寄存器) */
    kuint32_t num_configs;                              /* configs数组个数, 配置可以多个, 然后: *conf_reg = configs[0] | configs[1] */
};
```

这两个结构体和fwk_pinctrl_map_mux、fwk_pinctrl_map_configs简直相像，不同的是，map系列保存的是function、group、pin的名字（字符串），而这里的两个结构体保存的是编号。同样有一个最终整合的结构体：
```c
struct fwk_pinctrl_setting 
{
    kuint32_t type;                                     /* 取自 "__ERT_FWK_PINCTRL_PIN_TYPE", 表示是mux还是conf */
    struct list_head sgtc_link;                         /* 每个引脚都对应一个fwk_pinctrl_setting, 1个group有多个fwk_pinctrl_setting, 结成链表 */
    struct fwk_pinctrl_dev *sptr_pctldev;               /* 指向所属的控制器 */
    const kchar_t *dev_name;                            /* 引用本引脚的设备树节点名, 如: "gpio_tempate" */

    union 
    {
        struct fwk_pinctrl_mux sgtc_mux;                /* 如果type是mux类型,  那这里需提供: 本组引脚位于第几个function, 第几个group */
        struct fwk_pinctrl_configs sgtc_configs;        /* 如果type是conf类型, 那这里需提供: 本引脚的编号, conf的配置值 */
    } ugtr_data;
};
```

fwk_pinctrl_setting和fwk_pinctrl_map也简直相像，不同的是，fwk_pinctrl_map保存的是设备树节点名字，而fwk_pinctrl_setting直接存储了控制器指针fwk_pinctrl_dev。事实上，fwk_pinctrl_setting正是由fwk_pinctrl_map转化得到，而fwk_pinctrl_setting也就是驱动程序对接的数据资源。

以下几个函数可将map的数据转到pinctrl（名称到编号的转换）：
```c
/* function, group, pin, 在驱动程序一般以数组形式存在, 根据名字比较可找到它们在数组中的序号 */

/* 根据名字(一般为function的设备树节点名), 找到该function在控制器中的序号 */
kint32_t fwk_pinmux_get_function_by_name(struct fwk_pinctrl_dev *sptr_pctldev, const kchar_t *name);
/* 根据名字(一般为group的设备树节点名), 找到该group在function中的序号 */
kint32_t fwk_pinmux_get_group_by_name(struct fwk_pinctrl_dev *sptr_pctldev, kuint32_t func_selector, const kchar_t *name);
/* 一次性找到function序号和group序号, 并将序号填入sptr_setting->ugtr_data.sgtc_mux的func、group成员 */
kint32_t fwk_pinmux_map_to_setting(struct fwk_pinctrl_map const *sptr_map, struct fwk_pinctrl_setting *sptr_setting);

/* 根据名字, 找到引脚在控制器中的序号 */
kint32_t fwk_pinconf_get_by_name(struct fwk_pinctrl_dev *sptr_pctldev, const kchar_t *name);
/* 根据sptr_map->ugtr_data.sgtc_conf.group_or_pin (名字) 找到引脚序号, 并填入sptr_setting->ugtr_data.sgtc_conf.group_or_pin */
kint32_t fwk_pinconf_map_to_setting(struct fwk_pinctrl_map const *sptr_map, struct fwk_pinctrl_setting *sptr_setting);
```

和fwk_pinctrl_map一样，内核将为引脚配置建立fwk_pinctrl_setting，如：
```c
/* 全局变量 */
extern struct fwk_pinctrl_map sgtc_maps[2];

for (kuint32_t i = 0; i < 2; i++) {
    struct fwk_pinctrl_setting *sptr_setting;

    sptr_setting = kzalloc(sizeof(*sptr_setting), GFP_KERNEL);

    sptr_setting->type = sgtc_maps[i].type;
    sptr_setting->dev_name = sptr_map[i].dev_name;
    init_list_head(&sptr_setting->sgtc_link);

    /* 内部函数: 根据sptr_map->ctrl_dev_name找到fwk_pinctrl_dev */
    sptr_setting->sptr_pctldev = fwk_get_pinctrl_dev_from_name(sptr_map[i].ctrl_dev_name);

    /* 转换: 根据名称找到function、group、pin在pinctrl控制器中的序号, sptr_setting保存的是序号(一般为数组下标), 而非fwk_pinctrl_map的名称形式(名字比较, 繁琐且耗时) */
    /* 是mux ? 将sptr_map->ugtr_data.sgtc_mux解析到sptr_setting->ugtr_data.sgtc_mux */
    if (sptr_setting->type == NR_FWK_PINCTRL_PIN_MUX)
    {
        fwk_pinmux_map_to_setting(sptr_map, sptr_setting);
    }
    /* 是conf ? 将sptr_map->ugtr_data.sgtc_conf解析到sptr_setting->ugtr_data.sgtc_conf */
    else if (sptr_setting->type == NR_FWK_PINCTRL_PIN_CONF)
    {
        fwk_pinconf_map_to_setting(sptr_map, sptr_setting);
    }

    /* 将sptr_setting保存到引用它的设备链表中; sptr_state, 下文再展开说明 */
    list_head_add_tail(&sptr_state->sgtc_settings, &sptr_setting->sgtc_link);
}
```

这样我们得到了一系列fwk_pinctrl_setting，接下来需要和具体设备关联起来。首先关注pinctrl-names，它和pinctrl-0、pinctrl-1是对应的，比如“default”对应pinctrl-0，“sil9022a_rst”对应pinctrl-1；所谓的pinctrl-0，其实是一组fwk_pinctrl_setting结构体（pinctrl_ledgpio为1个引脚，故只需两个fwk_pinctrl_setting结构体，分别表示mux和conf；若有两个引脚，则需3个fwk_pinctrl_setting结构体，其中1个表示mux，另外两个表示两个引脚的conf），而pinctrl-1为另一组。
内核将每个pinctrl-names抽象为fwk_pinctrl_state结构体：
```c
struct fwk_pinctrl_state 
{
    struct list_head sgtc_link;                         /* 一个设备节点可能有多个pinctrl-names, 将它们结成链表; 如gpio_tempate, 有两个name: "default", "sil9022a_rst" */

    const kchar_t *name;                                /* 指向pinctrl-names, 如: "default", 或"sil9022a_rst" */
    struct list_head sgtc_settings;                     /* 每个pinctrl-names均对应一个pinctrl, 如pinctrl_ledgpio, 对应一串fwk_pinctrl_setting, 连接到此 */
};
```

当你根据pinctrl-names找到fwk_pinctrl_state结构体（如“default”，通过比较name成员获取），可遍历sgtc_settings链表，进而提取每一个fwk_pinctrl_setting结构体，访问mux和conf寄存器。
这里gpio_tempate设备引用了引脚，它将被抽象为fwk_pinctrl结构体，以表示各个pinctrl信息：
```c
struct fwk_pinctrl
{
    struct list_head sgtc_link;                         /* 连接到全局链表 */
    struct fwk_device *sptr_dev;                        /* 本设备 */
    struct list_head sgtc_states;                       /* 连接fwk_pinctrl_state::sgtc_link, 1个设备可能有多个pinctrl-names, 表示多种pinctrl配置 */
    struct fwk_pinctrl_state *sptr_state;               /* 当前使用的state, 如"default", 此时pinctrl-0引脚配置被使用 */

    struct list_head sgtc_dt_maps;                      /* 内部结构体fwk_pinctrl_dt_map关联使用, 包含fwk_pinctrl_maps信息 */
};
```

以imx6ull为例，各个结构体分别表示：
| 结构体                     | 意义                                                                                                                        |
| -------------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| struct fwk_pinctrl         | 引脚配置的引用者，gpio_tempate就是一个引用者                                                                                |
| struct fwk_pinctrl_state   | 引脚配置分类，引用者可以有多组不同的引脚配置，不同情况使用的配置不同。如默认（default）、睡眠（sleep），体现在pinctrl-names |
| struct fwk_pinctrl_setting | 一组引脚配置，含各个引脚的mux、conf信息（引脚序号等）                                                                       |
| struct fwk_pinctrl_dev     | IO控制器，是所有引脚的父节点，也是引脚的管理者                                                                              |
| struct fwk_pinctrl_desc    | IO控制器的核心结构，含引脚的序号、名称，操作引脚的方法                                                                      |
| struct fwk_pinctrl_map     | 底层结构，不直接对驱动程序负责，只为了读取设备树，并转化为fwk_pinctrl_setting                                               |

当设备与驱动程序匹配成功时，如gpio_template，由于其具有pinctrl属性，将创建一个fwk_pinctrl结构体，并根据pinctrl-names为“default”定位到pinctrl-0，取出它的设备树phandle，然后指向pinctrl_ledgpio；以pinctrl_ledgpio为起始，反向定位其父节点，一直定位到IO控制器的位置，则可得到IO控制器的fwk_pinctrl_dev指针，从而拿到dt_node_to_map函数的地址入口。访问dt_node_to_map，将设备树节点信息提取到fwk_pinctrl_map；之后创建fwk_pinctrl_state和fwk_pinctrl_setting结构体，将fwk_pinctrl_map解析到fwk_pinctrl_setting，然后fwk_pinctrl_setting与fwk_pinctrl_state关联，赋值到fwk_pinctrl结构体中。
看起来比较混乱，我们先介绍剩余的几个接口函数，再用流程图形式重新描述以上过程。
```c
/* 获取sptr_pctldev->driver_data */
void *fwk_pinctrl_get_drvdata(struct fwk_pinctrl_dev *sptr_pctldev);

/* 获取本设备的pinctrl结构, 如尚未建立fwk_pinctrl, 则创建(同步创建fwk_pinctrl_state、fwk_pinctrl_setting), 之后注册到全局链表 */
struct fwk_pinctrl *fwk_pinctrl_get(struct fwk_device *sptr_dev);
/* 注销, 从全局链表脱离, 并释放内存 */
void fwk_pinctrl_put(struct fwk_pinctrl *sptr_pctl);

/* 依据state_name, 在sptr_pctl->sgtc_states中查找同名的state; state_name可以为"default", "idle", "sleep"等 */
struct fwk_pinctrl_state *fwk_pinctrl_lookup_state(struct fwk_pinctrl *sptr_pctl, const kchar_t *state_name);
/* 选择state, 将为sptr_pctl->sptr_state赋值为sptr_state, 并层层往下, 选择对应的引脚配置, 给mux_reg、conf_reg寄存器赋值 */
kint32_t fwk_pinctrl_select_state(struct fwk_pinctrl *sptr_pctl, struct fwk_pinctrl_state *sptr_state);
```

正常开发时，需先调用fwk_pinctrl_get申请fwk_pinctrl结构体，然后选择希望使用的引脚配置，如fwk_pinctrl_lookup_state（sptr_pctl，“default”），获取默认配置；之后选择使用即可（函数fwk_pinctrl_select_state将完成引脚mux、conf寄存器的设置）。
IO控制器也是有pinctrl引用的，所以fwk_pinctrl_dev结构体也包含了一个fwk_pinctrl。

##### 8.8.4. pinctrl绑定
之前fwk_device曾经有一个成员sptr_pctlinfo：
```c
struct fwk_device
{
    kchar_t *init_name;
    struct fwk_bus_type *sptr_bus;

    /* 省略部分成员 */

    struct fwk_pinctrl_dev_info *sptr_pctlinfo;
    void *privData;
};
```

sptr_pctlinfo是一个包含了fwk_pinctrl，以及“default”、“idle”、“sleep”三种状态的数据结构，它指向本设备的pinctrl信息，在设备驱动匹配成功时创建，并与本设备（struct fwk_device）绑定，同时调用fwk_pinctrl_select_state函数初始化引脚配置。即，驱动的probe函数甚至可以不再初始化引脚，这也是为什么很多驱动都没有引脚设置的相关入口，但引脚却确确实实配置了，因为绑定（引脚初始化）过程是在总线的probe函数完成，而不是驱动。
其定义为：
```c
struct fwk_pinctrl_dev_info
{
    struct fwk_device *sptr_dev;                                /* 本设备 */
    struct fwk_pinctrl *sptr_pctl;                              /* 本设备的pinctrl */

    struct fwk_pinctrl_state *sptr_default;                     /* 默认配置: "default" */
    struct fwk_pinctrl_state *sptr_idle;                        /* 空闲配置: "idle" */
    struct fwk_pinctrl_state *sptr_sleep;                       /* 睡眠配置: "sleep" */
};

/* 申请fwk_pinctrl, 并创建一个fwk_pinctrl_dev_info结构体(内部结构), 并赋值给sptr_dev->sptr_pctlinfo, 然后调用fwk_pinctrl_select_state函数, 启用默认配置 */
kint32_t fwk_pinctrl_bind_pins(struct fwk_device *sptr_dev);
/* 启用睡眠或空闲配置, 然后解绑, 之后释放sptr_dev->sptr_pctlinfo->sptr_pctl */
void fwk_pinctrl_unbind_pins(struct fwk_device *sptr_dev);
```

fwk_pinctrl_bind_pins函数在fwk_device_driver_probe中被调用，先于fwk_device::sptr_bus->probe。
最后总结pinctrl的原理大致为：
```Mermaid
graph TD
    A[设备驱动匹配成功，fwk_device_driver_probe函数执行] --> B[请求绑定pinctrl：<br>fwk_pinctrl_bind_pins（sptr_dev）]
    B --> C[申请fwk_pinctrl：<br>struct fwk_pinctrl *sptr_pctl = fwk_pinctrl_get（sptr_dev）]
    C --> D[从内存池分配1个sptr_pctl，且sptr_pctl->sptr_dev = sptr_dev]
    C --> E[根据pinctrl-0的值（phandle）定位到pinctrl设备节点（group）]
    E --> F[定位pinctrl设备节点的父节点，取得pinctrl控制器sptr_pctldev（类型：struct fwk_pinctrl_dev）]
    F --> G[读取设备树pinctrl的引脚信息到map：dt_node_to_map（sptr_pctldev，sptr_nd，&sptr_map，&num_maps）]
    G --> H[关键赋值：sptr_map->dev_name为引用引脚的设备，sptr_map->name为pinctrl-names，sptr_map->ctrl_dev_name为IO控制器名称]
    D --> I
    H --> I[为每个状态创建对应的fwk_pinctrl_state，并连接到sptr_pctl->sgtc_states]
    I --> J[遍历全局map链表，找到sptr_map->dev_name和mr_dev_get_name（sptr_pctl->sptr_dev）相同的group]
    J --> K[根据sptr_map->name，在sptr_pctl->sgtc_states中找到对应的fwk_pinctrl_state]
    J --> L[为每个map创建对应的fwk_pinctrl_setting，来自于内存池，并将sptr_map的type、dev_name拷贝到setting]
    K --> M
    L --> M[补充其他数据：sptr_map中的mux、conf信息也解析到fwk_pinctrl_setting]
    M --> N[各个fwk_pinctrl_setting连接到fwk_pinctrl_state::sgtc_settings]
    N --> O[fwk_pinctrl_setting、fwk_pinctrl_state创建和初始化完成，sptr_pctl添加到全局链表，fwk_pinctrl_get函数结束]
    O --> P[找到默认配置：<br>struct fwk_pinctrl_state *sptr_state = fwk_pinctrl_lookup_state（sptr_pctl，“default”）]
    P --> Q[选择该配置：<br>fwk_pinctrl_select_state（sptr_pctl，sptr_state）]
    Q --> R[将sptr_state中类型为mux的fwk_pinctrl_setting同步到mux寄存器，通过：sptr_pctldev->sptr_desc->sptr_pmxops->set_mux]
    Q --> S[将sptr_state中类型为conf的fwk_pinctrl_setting同步到conf寄存器，通过：sptr_pctldev->sptr_desc->sptr_confops->pin_config_set]
    R --> T
    S --> T[引脚的mux、conf寄存器设置完成（含复用功能、电流驱动能力、上拉下拉等）]
```

#### 8.9. gpio
gpio是CPU最常见也最常用的资源，比如xc7z010有1组GPIO（PS端），imx6ull则有5组；每组GPIO都有32个IO引脚，具有输入、输出、独立的中断，甚至复用功能。以xc7z010为例，其在设备树中可表示为：
```c
gpio0: gpio@e000a000 {
    compatible = "xlnx,zynq-gpio-1.0";
    #gpio-cells = <2>;                      /* 明确gpio-cells为2, 如: key-gpios = <&gpio0 9 0>, 除"&gpio0"外, 有效值数量为2 (9和0) */
    clocks = <&clkc 42>;                    /* 时钟gate */
    gpio-controller;                        /* 表示是gpio控制器 */
    interrupt-controller;                   /* 表示也是中断控制器 */
    #interrupt-cells = <2>;                 /* 明确interrupt-cells为2, 如: interrupts = <9 3>, 有效值数量就是2 (9和3) */
    interrupt-parent = <&intc>;             /* 隶属于intc中断控制器 */
    interrupts = <0 20 4>;                  /* gpio0在CPU中对应的中断号为20 */
    reg = <0xe000a000 0x1000>;              /* 寄存器地址 */
};

/* 引脚: gpio0_9 */
extkey {
    compatible = "xlnx,z7-lite,extkey";
    pinctrl-names = "default";              /* 名字对应属性pinctrl-0 */
    pinctrl-0 = <&pinctrl_extkey>;          /* 默认(default)的pinctrl属性 */
    key-gpios = <&gpio0 9 GPIO_ACTIVE_LOW>; /* 隶属于gpio0, 其"#gpio-cells"为2, 有效属性为<9 GPIO_ACTIVE_LOW> */
    interrupt-parent = <&gpio0>;            /* 中断控制器为gpio0 */
    interrupts = <9 IRQ_TYPE_EDGE_BOTH>;    /* 隶属于interrupt-parent, 其"#interrupt-cells"为2; 这里的9表示gpio0第9号引脚 */
    
    status = "okay";
};
```

##### 8.9.1. gpio组
将节点gpio0视为一个gpio组，并抽象为结构体fwk_gpio_chip，先看其定义：
```c
struct fwk_gpio_chip
{
    /* 本gpio组的名字 */
    const kchar_t *name;
    /* gpio组伴随驱动程序创建, 指向父设备(如: &sptr_pdev->sgtc_dev). 主要是为了存储sptr_dev中的设备树节点 */
    struct fwk_device *sptr_dev;
    /* 本组起始引脚在CPU所有引脚中的偏移, 如gpio0的base为0, gpio1的base为32 */
    kint32_t base;
    /* 本组gpio引脚数量, 一般为32 */
    kuint32_t ngpios;

    /* 这是一个数组的首地址, 而非某个单一结构体变量; 本组gpio有多少个引脚, 数组的元素个数就有多少 */
    struct fwk_gpio_desc *sptr_desc;
    /* 用于注册到全局gpio链表(关联所有gpio组) */
    struct list_head sgtc_link;
    /* 暂时没什么用 */
    struct list_head sgtc_pin_ranges;

    /* 请求一个gpio, offset为引脚编号(范围为0 ~ [ngpios - 1]). 本质是标记为"占用" */
    kint32_t (*request) (struct fwk_gpio_chip *sptr_chip, kuint32_t offset);
    /* 释放一个gpio, offset为引脚编号(范围为0 ~ [ngpios - 1]). 本质是取消"占用"标记 */
    void (*free) (struct fwk_gpio_chip *sptr_chip, kuint32_t offset);
    /* 获取gpio (序号offset)的方向. 返回值: 输入(FWK_GPIO_DIR_IN), 输出(FWK_GPIO_DIR_OUT) */
    kint32_t (*get_direction) (struct fwk_gpio_chip *sptr_chip, kuint32_t offset);
    /* 设置gpio的方向为输入 */
    kint32_t (*direction_input) (struct fwk_gpio_chip *sptr_chip, kuint32_t offset);
    /* 设置gpio的方向为输出, 并初始化值为value (0/1) */
    kint32_t (*direction_output) (struct fwk_gpio_chip *sptr_chip, kuint32_t offset, kint32_t value);
    /* 获取gpio的值 (0/1) */
    kint32_t (*get) (struct fwk_gpio_chip *sptr_chip, kuint32_t offset);
    /* 设置gpio的值为value (0/1) */
    void (*set) (struct fwk_gpio_chip *sptr_chip, kuint32_t offset, kint32_t value);
    /* 获取gpio对应的虚拟中断号 */
    kint32_t (*to_irq) (struct fwk_gpio_chip *sptr_chip, kuint32_t offset);

    /* 设备树属性"#gpio-cells"的值, 一般为2 */
    kuint32_t of_gpio_n_cells;
    /* 比较sptr_spec->sptr_node和sptr_chip->sptr_dev->sptr_node, 判断是否是同一个(引脚是否属于该gpio组). 返回值为gpio序号 */
    kint32_t (*of_xlate)(struct fwk_gpio_chip *sptr_chip,
                        const struct fwk_of_phandle_args *sptr_spec, kuint32_t *flags);
};
```

fwk_gpio_chip负责管理一个gpio组，其下辖的gpio引脚（一般为32个）则抽象为fwk_gpio_desc结构体，gpio组初始化时会一次性创建32个fwk_gpio_desc数组，保存其首地址到fwk_gpio_chip::sptr_desc成员。该操作常在CPU的gpio组驱动中完成（芯片原厂提供，在probe函数中为gpio0、gpio1、……进行初始化）。由于数组为线性结构，我们可以通过gpio引脚序号来直接得到fwk_gpio_desc结构体指针，如gpio0_9，其引脚序号为9，可直接访问fwk_gpio_chip::sptr_desc[9]即可。这一点在下一章节将详述。
同gpio组的IO引脚，均共用操作函数（request、get_direction、set等），只需传入不同的offset参数（0 ~ （ngpios - 1））即可。
以下函数可以快速初始化或注册一个gpio组：
```c
/* 初始化一个gpio组, 并提供父设备指针、起始gpio引脚序号(0, 32, 64, ...), size为引脚对数(8个为1对, 当其值为4时, 表示"ngpios = 32") */
kint32_t fwk_gpiochip_init(struct fwk_gpio_chip *sptr_chip, struct fwk_device *sptr_dev, kuint32_t base, kuint32_t size);
/* 创建fwk_gpio_desc数组, 并注册sptr_chip到全局gpio组链表 */
kint32_t fwk_gpiochip_add(struct fwk_gpio_chip *sptr_chip);
/* 从全局gpio组链表脱离(注销), 并销毁fwk_gpio_desc数组 */
void fwk_gpiochip_del(struct fwk_gpio_chip *sptr_chip);
```

##### 8.9.2. gpio引脚
接下来看fwk_gpio_desc结构体，在上一章节已解释过它的含义，每个引脚都可以用一个fwk_gpio_desc结构体表示，并通过fwk_gpio_chip进行管理。其定义为：
```c
enum __ERT_FWK_GPIODESC_FLAGS
{
    NR_FWK_GPIODESC_REQUESTED = 0,              /* 已请求. 当引脚被请求时将置位 */
    NR_FWK_GPIODESC_IS_OUT,                     /* 方向为输出. 此位置1表示输出, 否则为输入 */
    NR_FWK_GPIODESC_TRIG_FALL,                  /* 下降沿触发 */
    NR_FWK_GPIODESC_TRIG_RISE,                  /* 上升沿触发 */
    NR_FWK_GPIODESC_ACTIVE_LOW,                 /* 低电平有效. 可使gpio灵活输出高低电平 (以LED为例, 当写0时, 内核自动将其反转为1, 输出高电平, LED灭) */
    NR_FWK_GPIODESC_OPEN_DRAIN,                 /* 开漏 */
    NR_FWK_GPIODESC_OPEN_SOURCE,                /* 开源 */
    NR_FWK_GPIODESC_USED_AS_IRQ,                /* 作为中断 */
    NR_FWK_GPIODESC_IS_HOGGED                   /* 热插拔 */
};

struct fwk_gpio_desc
{
    struct fwk_gpio_chip *sptr_chip;            /* 本引脚所属的gpio组 */
    kuint32_t flags;                            /* 取自"__ERT_FWK_GPIODESC_FLAGS" */
    const kchar_t *label;                       /* 引脚标签, 取自设备树属性名. 如"xxx-gpios", 则label为"xxx" */
};
```

gpio比较简单，不需要介绍过多原理。直接看其接口函数即可：
```c
/* 一个临时的数据结构, 可存储设备树"xxx-gpios"的信息 */
struct fwk_gpio_node_prop
{
    struct fwk_device_node *sptr_par;
    kint32_t gpio;
    kuint32_t flags;
};

/* 根据sptr_desc地址推导它在数组的偏移(数组下标, 也就是gpio序号) */
kuint32_t fwk_gpiodesc_to_hwgpio(struct fwk_gpio_chip *sptr_chip, struct fwk_gpio_desc *sptr_desc);
/* 根据gpio序号(数组下标)获取数组中的sptr_desc */
struct fwk_gpio_desc *fwk_gpiochip_get_desc(struct fwk_gpio_chip *sptr_chip, kuint32_t offset);

/* 
 * 根据sptr_spec遍历全局gpio组链表, 找到匹配的fwk_gpio_chip (通过fwk_gpio_chip::of_xlate函数完成); 
 * 之后将sptr_spec信息解析到sptr_data (gpio、flags);
 * 返回fwk_gpiochip_get_desc(sptr_chip, sptr_data->gpio)
 */
struct fwk_gpio_desc *fwk_gpiochip_and_desc_find(struct fwk_gpio_node_prop *sptr_data, struct fwk_of_phandle_args *sptr_spec);

/* 
 * 从设备树获取"xxx-gpios"属性的数据: 通过fwk_of_parse_phandle_with_args函数
 * list_name应为"xxx-gpios", 属性名全称;
 * index表示第几个属性;
 * flags用于保存最后一个属性. 如: key-gpios = <&gpio0 9 GPIO_ACTIVE_LOW>, *flags = GPIO_ACTIVE_LOW
 */
struct fwk_gpio_desc *fwk_of_get_named_gpiodesc_flags(struct fwk_device_node *sptr_node, 
                                            const kchar_t *list_name, kint32_t index, kuint32_t *flags);

/* 请求一个gpio (通过sptr_desc->sptr_chip->request实现), 并赋值: sptr_desc->label = label */
kint32_t fwk_gpio_request(struct fwk_gpio_desc *sptr_desc, const kchar_t *label);
/* 释放一个gpio (通过sptr_desc->sptr_chip->request实现) */
void fwk_gpio_free(struct fwk_gpio_desc *sptr_desc);
/* 判断gpio是否已被请求 (检查sptr_desc->flag标记是否有NR_FWK_GPIODESC_REQUESTED) */
kbool_t fwk_gpio_is_requested(struct fwk_gpio_desc *sptr_desc);

/* 
 * 1. 通过fwk_of_get_named_gpiodesc_flags函数得到fwk_gpio_desc;
 * 2. 自动调用fwk_gpio_request函数占用此gpio;
 * 3. 根据flags自动设置IO方向(输入/输出)
 */
struct fwk_gpio_desc *fwk_gpio_desc_get(struct fwk_device *sptr_dev, const kchar_t *con_id, kuint32_t flags);
/* fwk_gpio_desc_get的反操作. 最后会将IO设置为输出方向, 且值为0 */
void fwk_gpio_desc_put(struct fwk_gpio_desc *sptr_desc);

/* 根据gpio获取其对应的虚拟中断号(通过sptr_desc->sptr_chip->to_irq实现) */
kint32_t fwk_gpio_desc_to_irq(struct fwk_gpio_desc *sptr_desc);
/* 设置gpio的值(通过sptr_desc->sptr_chip->set实现, 前提是方向为输出). value: 0为低电平, 1为高电平 */
void fwk_gpio_set_value(struct fwk_gpio_desc *sptr_desc, kuint32_t value);
/* 获取gpio的值(通过sptr_desc->sptr_chip->get实现. 返回值: 0为低电平, 1为高电平 */
kint32_t fwk_gpio_get_value(struct fwk_gpio_desc *sptr_desc);

/* 
 * 设置gpio的方向
 * 若dir含NR_FWK_GPIO_BIT_OUT, 则调用sptr_desc->sptr_chip->direction_output, 输出方向;
 * 若dir无NR_FWK_GPIO_BIT_OUT, 则调用sptr_desc->sptr_chip->direction_input, 输入方向;
 * 若为输出方向, 顺带设置gpio, value: 0为低电平, 1为高电平 
 */
void fwk_gpio_set_direction(struct fwk_gpio_desc *sptr_desc, kuint32_t dir, kint32_t value);
/* 设置gpio方向为输入(通过sptr_desc->sptr_chip->direction_input实现) */
void fwk_gpio_set_direction_input(struct fwk_gpio_desc *sptr_desc);
/* 设置gpio方向为输出(通过sptr_desc->sptr_chip->direction_output实现), 顺带设置gpio的值 */
void fwk_gpio_set_direction_output(struct fwk_gpio_desc *sptr_desc, kint32_t value);
/* 获取方向. 0表示输入, 非0表示输出 */
kint32_t fwk_gpio_get_direction(struct fwk_gpio_desc *sptr_desc);
/* 判断方向是否为输入. 本质为: !fwk_gpio_get_direction(sptr_desc) */
kbool_t fwk_gpio_dir_is_input(struct fwk_gpio_desc *sptr_desc);

/* 设置某个sptr_desc->flag, flag取自"__ERT_FWK_GPIODESC_FLAGS" */
void fwk_gpio_desc_set_flags(struct fwk_gpio_desc *sptr_desc, kuint32_t flag);
/* 检查sptr_desc->flag是否已置上flag, flag取自"__ERT_FWK_GPIODESC_FLAGS" */
kbool_t fwk_gpio_desc_check_flags(struct fwk_gpio_desc *sptr_desc, kuint32_t flag);
/* 清除某个sptr_desc->flag, flag取自"__ERT_FWK_GPIODESC_FLAGS" */
void fwk_gpio_desc_clr_flags(struct fwk_gpio_desc *sptr_desc, kuint32_t flag);
```

##### 8.9.3. gpio中断
gpio引脚众多，对大部分CPU来说，每个引脚都可以产生输入中断；但硬件中断号只覆盖到gpio组，具体的引脚只能根据gpio组的中断状态寄存器来判断。这意味着：
```Mermaid
graph LR
    A[gpio中断] --> B[GIC根据硬件中断号关系找到gpio中断服务程序]
    B --> C[gpio中断服务程序读取中断状态寄存器, 找到发生的io引脚]
    C --> D[gpio组自己维护各个引脚的中断服务函数, 进一步调用]
```

gpio组的驱动程序需将gpio作为一个中断控制器（fwk_irq_domain），注册到全局中断控制器链表（函数fwk_irq_domain_add_hierarchy），并将所有引脚映射为虚拟中断号（函数fwk_irq_domain_alloc_irqs）。使用某个引脚的设备，如按键、LED，它们自己的驱动程序将定义一个中断服务函数与该虚拟中断号绑定。完整的过程将是：
```Mermaid
graph TD
    A[gpio中断] --> B[GIC根据硬件中断号找到虚拟中断号]
    B --> C[根据虚拟中断号找到关联的gpio组中断服务程序, 并调用]
    C --> D[gpio组中断服务程序读取中断状态寄存器, 找到发生的io引脚]
    D --> E[根据IO序号得到引脚对应的虚拟中断号]
    E --> F[根据虚拟中断号找到关联的gpio引脚中断服务程序, 并调用]
```

用例可查看“drivers/gpio/imx-gpiochip.c”，文件较长，故不在此贴出。

#### 8.10. i2c总线
除了虚拟平台总线platform_bus外，HeavenFox还支持i2c总线（i2c_bus）。i2c在硬件上是一个名副其实的总线，它拥有一个i2c控制器（主机），多个i2c设备（从机），并通过i2c协议确定关系（如设备地址、读写方向等）。可分化为三个结构：
| 名称   | 结构体          | 含义                                                                      |
| ------ | --------------- | ------------------------------------------------------------------------- |
| 总线   | fwk_bus_type    | 总线，与虚拟平台总线为同一结构                                            |
| 控制器 | fwk_i2c_adapter | 主机，一个i2c接口对应一个adapter。如imx6ull有4个i2c接口，则对应4个adpater |
| 设备   | fwk_i2c_client  | 从机，每一个使用i2c接口的设备均是一个客户端，如eeprom，触摸屏ic等         |
| 驱动   | fwk_i2c_driver  | 针对设备（从机）的驱动（非主机），类似于fwk_platdrv                       |

有关i2c协议将不做展开，如有兴趣可阅览笔者的另一份文档《嵌入式系统设计与实现方法》。

##### 8.10.1. 控制器
控制器位于CPU，是CPU的片上外设，它对外提供SCL（时钟线）和SDA（数据线），供外部设备挂接。如下设备树的i2c1节点就是一个控制器：
```c
/ {
    soc {
        #address-cells = <1>;
        #size-cells = <1>;
        compatible = "simple-bus";
        interrupt-parent = <&gpc>;
        ranges;

        aips1: aips-bus@02000000 {
            compatible = "fsl,aips-bus", "simple-bus";
            #address-cells = <1>;
            #size-cells = <1>;
            reg = <0x02000000 0x100000>;
            ranges;

            i2c1: i2c@021a0000 {
                #address-cells = <1>;
                #size-cells = <0>;
                compatible = "fsl,imx6ul-i2c", "fsl,imx21-i2c";
                reg = <0x021a0000 0x4000>;
                interrupts = <GIC_SPI 36 IRQ_TYPE_LEVEL_HIGH>;
                clocks = <&clks IMX6UL_CLK_I2C1>;
                status = "disabled";
            };
        };
    };
};

&i2c1 {
    clock_frequency = <100000>;
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_i2c1>;
    status = "okay";

    at24c02: at24c02@50 {
        compatible = "atmel,at24cxx";
        reg = <0x50>;
        page_size = <8>;
        pages = <32>;
        status = "okay";
    };
};
```

根据fwk_platdev的转化规则，soc位于根节点下，且拥有compatible属性，可以被转化；而它的compatible值为simple-bus，且子节点aips1也拥有compatible属性，同样可以被转化；而aips1的compatible值又是simple-bus，它的子节点i2c1（具有compatible属性）也会被转化为fwk_platdev。即控制器本身也会被当成一个虚拟平台设备，对应虚拟平台驱动fwk_platdrv。
一般驱动程序fwk_platdrv的probe函数负责获取设备树资源、时钟开关，以及早期的硬件初始化，资源会保存在驱动的私有数据结构中。对于i2c控制器驱动，则在普通驱动程序基础上增加控制器结构体fwk_i2c_adapter的创建和注册，以描述一个具体的i2c控制器。其结构为：
```c
struct fwk_i2c_adapter
{
    kuint32_t id;                                       /* 由设备树节点序号获取, 如i2c1, id为0 */
    const struct fwk_i2c_algo *sptr_algo;               /* i2c协议操作函数集 */
    void *algo_data;                                    /* 传递给sptr_algo的参数 */

    kint32_t timeout;                                   /* 暂无用途 */
    struct fwk_device sgtc_dev;                         /* 控制器设备既会挂接在platform_bus (由设备树转化), 也会挂接在i2c_bus; 由sgtc_dev完成i2c_bus的挂接 */

    kint32_t nr;                                        /* 与id效果相同, 表示这是第几个控制器 */
    kchar_t name[48];                                   /* 控制器名称: i2c-[nr], 如i2c-0 */

    struct list_head sgtc_clients;                      /* 本控制器所在总线上的所有i2c设备, 挂接到此, 形成链表 */
    struct rw_lock sgtc_lock;                           /* 内置读写锁 */
};

/* 注册一个控制器设备, 挂接到i2c_bus */
kint32_t fwk_i2c_register_adapter(struct fwk_i2c_adapter *sptr_adap);
/* 初始化sptr_adap->nr, 之后调用fwk_i2c_register_adapter */
kint32_t fwk_i2c_add_adapter(struct fwk_i2c_adapter *sptr_adap);
/* 注销一个控制器设备 */
void fwk_i2c_del_adapter(struct fwk_i2c_adapter *sptr_adap);

/* 保存data到sptr_adap->sgtc_dev.privData */
void fwk_i2c_adapter_set_drvdata(struct fwk_i2c_adapter *sptr_adap, void *data);
/* 返回: sptr_adap->sgtc_dev.privData */
void *fwk_i2c_adapter_get_drvdata(struct fwk_i2c_adapter *sptr_adap);
```

##### 8.10.2. 设备
i2c设备（如at24c02）一般为i2c控制器的子节点（设备树），而i2c控制器既非根节点，可能也不包含“simple-bus”这种compatible属性值，它是不会被自动转化为fwk_platdev的。事实上，内核在注册i2c控制器时，会遍历其所有的子节点，为每个子节点（i2c设备）创建fwk_i2c_client结构体（如同虚拟平台总线下的fwk_platdev），并挂接到i2c总线上。它的结构为：
```c
struct fwk_i2c_client
{
    kuint16_t flags;                                    /* 标志, 可表示addr的类型(10bits/7bits), 如: FWK_I2C_M_TEN */
    kuint16_t addr;                                     /* 器件地址, 可以是7位, 也可以是10位; 若为后者, flags应同步置FWK_I2C_M_TEN */

    kchar_t name[32];                                   /* i2c设备的名称, 来自fwk_i2c_board_info::type */
    struct fwk_i2c_adapter *sptr_adapter;               /* 本设备挂接的i2c控制器 */
    struct fwk_i2c_driver *sptr_driver;                 /* 每个i2c设备最多有一个对应的驱动程序, 此处用于关联 */
    struct fwk_device sgtc_dev;                         /* 设备结构, 用于挂接i2c总线, 描述设备属性 */
    kint32_t irq;                                       /* 本设备的独有中断号 (不常用) */
    struct list_head sgtc_link;                         /* 挂接到sptr_adapter->sgtc_clients, 本控制器下的所有i2c设备组成链表 */
};

/* 设备信息, 可以作为中转解析到fwk_i2c_client, 或者由驱动程序自行定义和使用 */
struct fwk_i2c_board_info
{
    kchar_t type[32];                                   /* 设备名称, 如: compatible = "atmel,at24cxx", type取"at24cxx" */
    kuint16_t flags;                                    /* 标志, 赋值给fwk_i2c_client::flags */
    kuint16_t addr;                                     /* 器件地址, 赋值给fwk_i2c_client::addr */
    void *ptr_platform_data;                            /* 设备私有数据 (开发者自由使用) */

    struct fwk_device_node *sptr_node;                  /* 本设备对应的设备树节点, 赋值给fwk_i2c_client::sgtc_dev.sptr_node */
    kint32_t irq;                                       /* 设备中断号, 赋值给fwk_i2c_client::irq (不常用) */
};

/* 注册i2c设备: 挂接到i2c总线  */
kint32_t fwk_register_i2c_device(struct fwk_i2c_client *sptr_client);
/* 注销i2c设备: 从i2c总线脱离 */
kint32_t fwk_unregister_i2c_device(struct fwk_i2c_client *sptr_client);
/* 同fwk_register_i2c_device */
kint32_t fwk_i2c_add_device(struct fwk_i2c_client *sptr_client);
/* 同fwk_unregister_i2c_device */
kint32_t fwk_i2c_del_device(struct fwk_i2c_client *sptr_client);

/* 创建一个i2c设备, 挂接到i2c控制器sptr_adap下, 并挂接到i2c总线 (无需再调用fwk_register_i2c_device) */
struct fwk_i2c_client *fwk_i2c_new_device(struct fwk_i2c_adapter *sptr_adap, struct fwk_i2c_board_info const *sptr_info);
/* 注销一个i2c设备, 从i2c控制器设备链表中脱离, 并不再挂接i2c总线 */
void fwk_i2c_unregister_device(struct fwk_i2c_client *sptr_client);

/* 遍历控制器sptr_adap下的所有设备树节点(i2c设备), 为每个i2c设备创建fwk_i2c_client结构体, 并注册到i2c总线 */
void fwk_of_i2c_register_devices(struct fwk_i2c_adapter *sptr_adap);

/* 保存私有数据到sptr_client->sgtc_dev.privData */
void fwk_i2c_set_client_data(struct fwk_i2c_client *sptr_client, void *data);
/* 返回sptr_client->sgtc_dev.privData */
void *fwk_i2c_get_client_data(struct fwk_i2c_client *sptr_client);
```

##### 8.10.3. 驱动与总线
i2c驱动使用fwk_i2c_driver结构体来描述，和fwk_platdrv基本一样：
```c
struct fwk_i2c_device_id
{
    kchar_t *name;                                      /* i2c设备名称, 当与某个i2c设备fwk_i2c_client::name一致时, 匹配成功 */
    kint32_t driver_data;                               /* 私有数据地址. 不同i2c设备可拥有独立的私有数据(如单板资源信息) */
};

struct fwk_i2c_driver
{
    /* i2c驱动probe函数. i2c驱动一般使用id_table匹配设备, 较少用of_match_table; 匹配后, probe函数并执行, 对应的id_table将作为参数传入 */
    kint32_t (*probe) (struct fwk_i2c_client *sptr_client, const struct fwk_i2c_device_id *sptr_idtable);
    /* i2c驱动remove函数 */
    kint32_t (*remove) (struct fwk_i2c_client *sptr_client);

    /* 驱动本体, 用于挂接i2c总线 */
    struct fwk_driver sgtc_driver;
    /* id_table */
    const struct fwk_i2c_device_id *sptr_id_table;

    /* 本驱动匹配的设备列表 (不常用) */
    struct list_head sgtc_clients;
};

/* 注册一个i2c驱动, 挂接到i2c总线 */
kint32_t fwk_i2c_register_driver(struct fwk_i2c_driver *sptr_driver);
/* 注销一个i2c驱动, 从i2c总线脱离 */
kint32_t fwk_i2c_unregister_driver(struct fwk_i2c_driver *sptr_driver);
/* 同fwk_i2c_register_driver */
kint32_t fwk_i2c_add_driver(struct fwk_i2c_driver *sptr_driver);
/* 同fwk_i2c_unregister_driver */
kint32_t fwk_i2c_del_driver(struct fwk_i2c_driver *sptr_driver);
```

驱动挂接到i2c总线后，将与总线上的i2c设备链表逐个比较，找到匹配的设备。匹配机制与虚拟平台总线相似，但略有精简，只由id_table和of_match_table决定。规则为：
```c
kint32_t fwk_i2c_device_match(struct fwk_device *sptr_dev, struct fwk_driver *sptr_drv)
{
    struct fwk_i2c_client *sptr_client;
    struct fwk_i2c_driver *sptr_driver;
    struct fwk_device_node *sptr_np;

    /* 用于标识设备类型, 仅i2c设备类型可执行此匹配规则 */
    if (sptr_dev->sptr_type != &sgtc_fwk_i2c_client_type)
        return -ER_CHECKERR;

    sptr_client = mr_container_of(sptr_dev, struct fwk_i2c_client, sgtc_dev);
    sptr_driver = mr_container_of(sptr_drv, struct fwk_i2c_driver, sgtc_driver);

    /* 匹配规则1: 比较of_match_table的compatible成员与i2c设备节点的compatible属性是否相同 */
    sptr_np = fwk_of_node_try_matches(sptr_dev->sptr_node, sptr_drv->sptr_of_match_table, mr_nullptr);
    if (isValid(sptr_np) && (sptr_dev->sptr_node == sptr_np))
        return ER_NORMAL;

    /* 匹配规则2: 比较id_table的name成员与i2c设备的sptr_client->name(设备名称)是否相同 */
    if (sptr_driver->sptr_id_table)
        return fwk_i2c_match_id(sptr_driver->sptr_id_table, sptr_client) ? ER_NORMAL : -ER_NOTFOUND;

    return -ER_NOTFOUND;
}
```

匹配成功后即调用fwk_i2c_device_probe函数，最终fwk_i2c_driver::probe函数也将被执行。但无论是否是由id_table引起的匹配成功，fwk_i2c_device_probe函数都将重新调用fwk_i2c_match_id进行匹配，并将结果作为fwk_i2c_driver::probe函数的sptr_idtable参数传入（可能为NULL）。

##### 8.10.4. 协议与数据传输
驱动仅用于提供操作i2c设备的方法，真正传输数据还需依赖正确的i2c协议时序。协议时序应由i2c控制器驱动来提供，如imx6ull的i2c控制器驱动（imx-i2c.c）；回到最开始的fwk_i2c_adapter结构体，它有一个名为“const struct fwk_i2c_algo *sptr_algo”的成员，正是控制器的传输协议（算法）方法。其定义为：
```c
#define FWK_I2C_M_TEN                       0x0010      /* 标记i2c设备的器件地址是否为10位 */
#define FWK_I2C_M_RD                        0x0001      /* 传输方向, 是否为读 (从设备到控制器 <从机到主机>) */

struct fwk_i2c_msg
{
    kuint16_t addr;                                     /* 器件地址 */
    kuint16_t flags;                                    /* 标志, 如FWK_I2C_M_RD */

    kuint16_t len;                                      /* ptr_buf的字节数 */
    void *ptr_buf;                                      /* 数据指针 */
};

/* 对于控制器驱动, 则需实现该结构体的master_xfer函数 */
struct fwk_i2c_algo
{
    /* num表示sptr_msgs的个数 */
    kint32_t (*master_xfer)(struct fwk_i2c_adapter *sptr_adap, struct fwk_i2c_msg *sptr_msgs, kint32_t num);
};

/* 调用sptr_client->sptr_adapter->sptr_algo->master_xfer(sptr_client->sptr_adapter, sptr_msgs, num) */
kint32_t fwk_i2c_transfer(struct fwk_i2c_client *sptr_client, struct fwk_i2c_msg *sptr_msgs, kint32_t num);
/* 仅发送器件地址, 无数据, 测试i2c设备是否存在 */
kint32_t fwk_i2c_check_slave(struct fwk_i2c_client *sptr_client);

/* 发送一个字节. reg为要写入的寄存器地址 */
kint32_t fwk_i2c_write_byte_data(struct fwk_i2c_client *sptr_client, kuint8_t reg, kuint8_t value);
/* 读取一个字节. reg为要读出的寄存器地址 */
kint32_t fwk_i2c_read_byte_data(struct fwk_i2c_client *sptr_client, kuint8_t reg);
```

fwk_i2c_write_byte_data和fwk_i2c_read_byte_data是读写i2c设备的一个简洁接口，当然也可以自己定义fwk_i2c_msg，并使用fwk_i2c_transfer传输数据。比如fwk_i2c_read_byte_data的内部实现为：
```c
kint32_t fwk_i2c_read_byte_data(struct fwk_i2c_client *sptr_client, kuint8_t reg)
{
    const struct fwk_i2c_algo *sptr_algo = sptr_client->sptr_adapter->sptr_algo;
    struct fwk_i2c_msg sgtc_msgs[2];
    kuint8_t value = 0;
    kint32_t retval;

    if (!sptr_algo ||
        !sptr_algo->master_xfer)
        return -ER_NSUPPORT;

    sgtc_msgs[0].addr = sptr_client->addr;
    sgtc_msgs[0].flags = 0;
    sgtc_msgs[0].ptr_buf = &reg;
    sgtc_msgs[0].len = 1;

    sgtc_msgs[1].addr = sptr_client->addr;
    sgtc_msgs[1].flags = FWK_I2C_M_RD;
    sgtc_msgs[1].ptr_buf = &value;
    sgtc_msgs[1].len = sizeof(value);

    retval = sptr_algo->master_xfer(sptr_client->sptr_adapter, &sgtc_msgs[0], ARRAY_SIZE(sgtc_msgs));
    return retval ? retval : value;
}
```

#### 8.11. 帧缓冲
HeavenFox为显示器提供了帧缓冲平台框架、显示接口API（如描点、画线、图像等），并支持第三方库（LVGL）。
帧缓冲设备和gpio、i2c控制器一样，其设备树节点一般会满足fwk_platdev的转化条件，即，帧缓冲设备是挂接在虚拟平台总线（platform_bus）上的。帧缓冲驱动需要定义fwk_platdrv，并注册到虚拟平台总线，以匹配帧缓冲设备。

- 参数

显示设备，如LCD，一般包含如下参数：
```
PCLK: Pixel Clock, 像素时钟, 下降沿采样
P_DATA: 像素数据
ENABEL: V_DEN, 数据使能信号, 高电平有效
HSYNC: Horizontal Sync, 水平同步信号, 当产生此信号(脉冲)时表示开始显示新的一行, 低电平有效
VSYNC: Vertical Sync, 垂直同步信号, 当产生此信号(脉冲)时表示要显示一屏图像, 低电平有效
HSPW: Horizontal Sync Pulse Width, 水平同步信号脉宽
VSPW: Vertical Sync Pulse Width, 垂直同步信号脉宽
HFP: Horizontal front porch timing, 水平前沿, ENABLE下降沿与HSYNC上升沿的时间间隔, 即产生水平信号前的延时时间, 一般用CLK表示单位, 即多少个PCLK时钟
HBP: Horizontal back porch timing, 水平后沿, ENABLE上升沿与HSYNC下降沿的时间间隔, 即产生水平信号后的延时时间, 一般用CLK表示单位, 即多少个PCLK时钟
VFP: Vertical front porch timing, 垂直前沿, ENABLE下降沿与VSYNC上升沿的时间间隔, 即产生垂直信号前的延时时间, 一般用行表示单位, 即多少个HSYNC周期
VBP: Vertical back porch timing, 垂直后沿, ENABLE上升沿与VSYNC下降沿的时间间隔, 即产生垂直信号后的延时时间, 一般用行表示单位, 即多少个HSYNC周期
```

设备树中display-timings节点用于描述具体参数，如：
```c
display-timings {
    native-mode = <&timing_1366x768>;       /* 可以支持多种显示屏配置, 然后通过native-mode来选择 */

    timing_1366x768: timing-768p {
        clock-frequency = <70000000>;       /* 时钟频率PCLK: 70000000Hz */
        hactive = <1366>;                   /* 水平分辨率: 1366个像素点 */
        vactive = <768>;                    /* 垂直分辨率: 768个像素点 */
        
        hback-porch = <20>;                 /* 水平前沿HFP: 20个clk */
        hsync-len = <32>;                   /* 水平后沿HBP: 32个clk */
        hfront-porch = <48>;                /* 水平同步信号脉宽HSPW: 48个clk */
        vback-porch = <13>;                 /* 垂直后沿VBP: 13行 */
        vsync-len = <6>;                    /* 垂直前沿VFP: 6行 */
        vfront-porch = <3>;                 /* 垂直同步信号脉宽VSPW: 3行, 即包含3个HSYNC周期 */
        
        hsync-active = <0>;                 /* 水平同步HSYNC极性: 0, 低电平有效 */
        vsync-active = <0>;                 /* 垂直同步VSYNC极性: 0, 低电平有效 */
        de-active = <1>;                    /* 数据线ENABLE极性: 1, 高电平有效 */
        pixelclk-active = <1>;              /* 时钟线PCLK极性: 1, 高电平有效 ===> 上升沿采样; 也可以设为0, 下降沿采样 */
    };
};
```

- 时序
```
a. PCLK不断产生时钟脉冲, 当产生VSYNC脉冲时, 表示要显示一帧数据; 随即VSYNC置低电平, 等待HSYNC和ENABLE信号; 
b. 当产生HSYNC脉冲时, 表示要显示一行数据; 随即HSYNC置低电平(HSYNC的脉宽一般比VSYNC短), 等待ENABLE信号;
c. 经过一段时间(水平后沿HBP)后, ENABLE置1, 此时P_DATA的数据才会被认为有效; 在ENABLE的脉宽范围内, 每过一个PCLK时钟下降沿, 采集一次数据; 直到一行数据显示完成;
d. ENABLE置0后, 经过一段时间(水平前沿HFP), HSYNC再次产生脉冲, 重复b ~ d的工作;
e. 当各行数据显示完毕, 一帧图像完成; ENABLE置0后, 经过一段时间(垂直前沿VFP)后, VSYNC再次产生脉冲, 准备下一帧图像的显示, 重复a ~ e的工作
    
    水平方向:
         HSPW   HBP                       行[有效]数据显示                      HFP
        | <---> | <------> | <-------------------------------------------------> | <------> |
        | ----- | -------- | --------------------------------------------------- | -------- |
        |       |          |                                                     |          |
    HSYNC=1 HSYNC=0 ENABLE=1                                           ENABLE=0   HSYNC=1

    垂直方向:
         VSPW    VBP               水平方向各行[有效]数据显示                     VFP
        | <---> | <------> | <----------........................---------------> | <------> |
        | ----- | -------- | --------------------------------------------------- | -------- |
        |       |          |                                                     |          |
    VSYNC=1 VSYNC=0 ENABLE=1                                           ENABLE=0   VSYNC=1      

在[VSPW + VBP]之间可能会包含多个HSYNC, 由于每个HSYNC信号显示一行数据(不过此时ENABLE = 0, 数据无效), 故相当于表示: VBP为跳过的行数; VFP同理
```

- 频率
```
每个PCLK时钟脉冲到来后显示一个像素点, 假设LCD分辨率为HOZVAL * LINE, 那显示一行有效数据需要的时间为HOZVAL个PCLK, 加上水平同步前肩HFP, 后肩HBP, 水平同步脉宽HSPW, 每行共需使用: (HSPW + HBP + HOZVAL + HFP)个PCLK;
算上垂直方向, 显示一帧图像需要花费的时间为(行数 * 每行的时钟数): (VSPW + VBP + LINE + VFP) * (HSPW + HBP + HOZVAL + HFP)个PCLK;
同样是上面那个panel-timing的例子, 已知LCD刷新率为60Hz(即每秒钟可刷新60帧图像):
LCD显示1帧图像所需时钟数 = (VSPW + VBP + LINE + VFP) * (HSPW + HBP + HOZVAL + HFP) = (22 + 1 + 480 + 22) * (43 + 1 + 800 + 210) = 553350个PCLK
每帧图像需要的时间为: (1 / 60)s
故PCLK的宽度 = ((1 / 60) / 553350)s = (1 / (60 * 553350))s
故PCLK的频率 = 1 / (1 / (60 * 553350)) = 60 * 553350 = 33201000Hz
故LCD每个像素点时钟 = PCLK的频率 = 553350 * 60Hz = (VSPW + VBP + LINE + VFP) * (HSPW + HBP + HOZVAL + HFP) * 刷新率 = 33201000Hz
```

- 显存
```
显存大小 = 水平分辨率(水平方向的像素点个数) * 垂直分辨率(垂直方向的像素点个数) * 每个像素点所需的字节数 = HOZVAL * LINE * BPP(Bytes-Per-Pixel)
比如分辨率为1024 * 600, 采用ARGB888格式(透明度 + R + G + B, 32位, 4字节), 则显存 = 1024 * 600 * 4 = 2457600Bytes = 2.46MBytes
ARM Cortex-A系列一般有MMU内存管理单元, 操作ARM的虚拟映射地址相当于操作片外RAM(如DDR3)的物理地址, 将LCD寄存器CUR_BUF赋值为DDR的某个虚拟地址, 即可将其作为显存
```

##### 8.11.1. 帧缓冲信息
结构体fwk_fb_info用于描述一个帧缓冲设备的信息，它包含：显示设备的参数、显存地址和大小、显示设备的读写接口等。HeavenFox定义了fwk_fb_info指针数组，长度为32，即允许注册32个帧缓冲驱动。
其定义为：
```c
struct fwk_fb_info
{
    kint32_t node;                                      /* 帧缓冲驱动的序号, 也用于表示次设备号. 一般是数组sgtc_fwk_registered_fb的下标 */

    struct fwk_fb_fix_screen_info sgtc_fix;             /* 屏幕固有参数 */
    struct fwk_fb_var_screen_info sgtc_var;             /* 屏幕可变参数 */

    const struct fwk_fb_oprts *sptr_fbops;              /* 操作函数集合, 用于读写显示设备 */
    struct fwk_device *sptr_dev;                        /* 父设备指针, 一般来自于fwk_platdev::sgtc_dev, 即匹配的帧缓冲设备 */
    struct fwk_device *sptr_idev;                       /* 字符设备指针, 注册帧缓冲驱动时自动创建, 生成"/dev/fb"虚拟设备文件路径 */

    kuint8_t *screen_base;                              /* 显存起始地址, 对于双倍/三倍缓冲区, screen_base始终指向正在显示的缓冲区地址, 即: screen_base是可变的 */
    kuint32_t screen_size;                              /* 显存大小 (单位: 字节). 如果是双倍/三倍缓冲区, screen_size = 双倍/三倍缓冲区的总大小 */

    void *ptr_par;                                      /* 驱动私有数据 */
};

struct fwk_fb_info *sgtc_fwk_registered_fb[32];         /* 全局指针数组 */

/* 创建一个fwk_fb_info. size: ptr_par的大小; sptr_dev: 设备指针, 一般来自fwk_platdev::sgtc_dev */
struct fwk_fb_info *fwk_framebuffer_alloc(kusize_t size, struct fwk_device *sptr_dev);
/* 释放一个fwk_fb_info (kfree) */
void fwk_framebuffer_release(struct fwk_fb_info *sptr_fb_info);

/* 注册fwk_fb_info到全局数组sgtc_fwk_registered_fb[], 并创建虚拟设备文件路径 */
kint32_t fwk_register_framebuffer(struct fwk_fb_info *sptr_fb_info);
/* 从sgtc_fwk_registered_fb[sptr_fb_info->node]注销 */
void fwk_unregister_framebuffer(struct fwk_fb_info *sptr_fb_info);

/* 返回: sgtc_fwk_registered_fb[idx] */
struct fwk_fb_info *fwk_get_fb_info(kuint32_t idx);
```

帧缓冲信息结构fwk_fb_info常在设备-驱动匹配成功后，在帧缓冲驱动的probe函数中创建和注册，并创建虚拟设备文件路径，如“/dev/fb0”，应用层访问该虚拟路径将得到主次设备号，驱动层根据次设备号（数组下标）定位sgtc_fwk_registered_fb[]，就可获得帧缓冲设备的信息（fwk_fb_info）。

- 固有参数
```c
struct fwk_fb_fix_screen_info
{
    kchar_t id[16];                                     /* 显示设备标识 (不常用) */
    kuaddr_t smem_start;                                /* 帧缓冲区/显存起始地址, 对于多倍缓冲区, 显存应一次性申请, 并保存地址到此. smem_start不可变, 除非释放显存 */
    kuint32_t smem_len;                                 /* 始终为一块(单倍)缓冲区的大小 */
};
```

- 可变参数
```c
struct fwk_fb_var_screen_info
{
    kuint32_t xres;                                     /* 可见屏幕宽度. 即单倍缓冲区的宽度 */
    kuint32_t yres;                                     /* 可见屏幕高度. 即单倍缓冲区的高度 */
    kuint32_t xres_virtual;                             /* 虚拟屏幕宽度, 含不可见区域. 即全部缓冲区的宽度 (双倍/三倍缓冲区总宽度) */
    kuint32_t yres_virtual;                             /* 虚拟屏幕高度, 含不可见区域. 即全部缓冲区的高度 (双倍/三倍缓冲区总高度) */
    kuint32_t xoffset;                                  /* 基于0的宽度偏移, 即表示x方向的目标位置. 最大不能超过xres_virtual */
    kuint32_t yoffset;                                  /* 基于0的高度偏移, 即表示y方向的目标位置. 最大不能超过yres_virtual */

    kuint32_t bits_per_pixel;                           /* bpp, 每个像素点所占的位数. 如16, 24, 32, ... */

    kuint32_t height;                                   /* 屏幕高度 (单位: ms; 不常用) */
    kuint32_t width;                                    /* 屏幕宽度 (单位: ms; 不常用) */

    kuint32_t pixclock;                                 /* 像素时钟, 单位: ps(皮秒). 设备树一般提供的是Hz, 需使用FB_KHZ_2_PICOS进行转化 */

    kuint32_t left_margin;                              /* 左侧边沿, 即HBP (水平同步后沿), 单位为pixclock */
    kuint32_t right_margin;                             /* 右侧边沿, 即HFP (水平同步前沿), 单位为pixclock */
    kuint32_t upper_margin;                             /* 上侧边沿, 即VBP (垂直同步后沿), 单位为row, 即多少行水平像素时钟 */
    kuint32_t lower_margin;                             /* 下侧边沿, 即VFP (垂直同步前沿), 单位为row, 即多少行水平像素时钟 */
    kuint32_t hsync_len;                                /* 水平同步信号脉宽, 即HSPW, 位于HBP左侧 */
    kuint32_t vsync_len;                                /* 垂直同步信号脉宽, 即VSPW, 位于VBP上侧 */
};
```

可变参数大部分来自于设备树节点display-timings，如pixclock、xres、yres，以及各种margin。一般在帧缓冲驱动的probe函数中解析设备树节点属性来获取。

- 操作函数集
提供给应用层的访问接口，如当应用层访问“/dev/fb0”时，最终将调用fb_open函数，启用显示设备。定义如下：
```c
enum __ERT_FB_IOCTL_CMD
{
    NR_FB_IOGET_VARINFO = FWK_IOR('F', 0, struct fwk_fb_var_screen_info),   /* 获取可变参数 */
    NR_FB_IOSET_VARINFO = FWK_IOW('F', 1, struct fwk_fb_var_screen_info),   /* 设置可变参数 */
    NR_FB_IOGET_FIXINFO = FWK_IOR('F', 2, struct fwk_fb_fix_screen_info),   /* 获取固定参数. 固定参数不允许应用层设置 */
};

struct fwk_fb_oprts
{
    /* 打开显示设备, 屏幕启用显示; user: 1表示由应用程序调用, 0表示由驱动程序调用 */
    kint32_t (*fb_open) (struct fwk_fb_info *sptr_info, kint32_t user);
    /* 关闭显示设备, 屏幕停用显示; user含义同fb_open */
    kint32_t (*fb_release) (struct fwk_fb_info *sptr_info, kint32_t user);

    /* 读帧缓冲数据. 慎用, 涉及帧缓冲数据拷贝, 数据量较大, 比较缓慢 */
    kssize_t (*fb_read) (struct fwk_fb_info *sptr_info, kbuffer_t *ptr_buf, kusize_t count, kuint8_t *ptr_offset);
    /* 写帧缓冲数据. 慎用, 涉及帧缓冲数据拷贝, 数据量较大, 比较缓慢 */
    kssize_t (*fb_write) (struct fwk_fb_info *sptr_info, const kbuffer_t *ptr_buf, kusize_t count, kuint8_t *ptr_offset);

    /* 给帧缓冲驱动下发"命令". arg: 参数, 应与cmd配套. cmd: 取自"enum __ERT_FB_IOCTL_CMD" */
    kint32_t (*fb_ioctl) (struct fwk_fb_info *sptr_info, kuint32_t cmd, kuaddr_t arg);
    /* 将显存起始起始和大小赋值给vma, 应用层可直接访问显存地址, 无需通过fb_read和fb_write, 省去中间商 */
    kint32_t (*fb_mmap) (struct fwk_fb_info *sptr_info, struct fwk_vm_area *vma);
};
```

比较典型的是双倍/三倍缓冲区，一般为了防止出现“拉窗帘”的现象，会先给备用缓冲区写入数据，然后再更改LCD的显存地址为备用缓冲区，实现“立即显示”的效果。可以先将fwk_fb_var_screen_info::yoffset偏移一个缓冲区的高度（如：yoffset += yres），再调用fb_ioctl生效。命令cmd应选择：NR_FB_IOSET_VARINFO。
如imx6ull的LCD驱动为：
```c
kint32_t imx_fbdev_ioctl(struct fwk_fb_info *sptr_info, kuint32_t cmd, kuaddr_t arg)
{
    struct fwk_fb_var_screen_info *sptr_var;
    struct imx_fbdev_drv *sptr_drv;
    srt_imx_lcdif_t *sptr_lcdif;
    kuaddr_t new_smem;

    sptr_drv = fwk_fb_get_drvdata(sptr_info);
    sptr_lcdif = (srt_imx_lcdif_t *)sptr_drv->base;

    switch (cmd)
    {
        case NR_FB_IOSET_VARINFO:
            /* 整片(所有)缓冲区的起始地址 */
            new_smem = sptr_info->sgtc_fix.smem_start;
            /* 参数应为fwk_fb_var_screen_info */
            sptr_var = (struct fwk_fb_var_screen_info *)arg;

            /* yres_virtual表示整片(所有)缓冲区的总高度, yoffset不应超出此范围 */
            if ((sptr_var->xoffset >= sptr_info->sgtc_var.xres_virtual) ||
                (sptr_var->yoffset >= sptr_info->sgtc_var.yres_virtual))
                return -ER_MORE;

            /* yofffset若小于yres, 表示落在缓冲区1; 否则落在缓冲区2 */
            if (sptr_var->yoffset >= sptr_info->sgtc_var.yres)
            {
                /* 偏移到下一个缓冲区 */
                new_smem += sptr_info->sgtc_fix.smem_len;
                new_smem  = mr_align(new_smem, 8);
            }

            /* 切换显存地址(当前要显示的缓冲区) */
            sptr_info->screen_base = (kuint8_t *)new_smem;

            /* 先关闭LCD */
            mr_writel(0U, &sptr_lcdif->CTRL_SET);

            /* CUR_BUF: 写入当前缓冲区寄存器, 更新显存地址 */
            mr_writel(new_smem, &sptr_lcdif->CUR_BUF);
            /* NEXT_BUF: 不使用"下一个缓冲区寄存器", 可直接与CUR_BUF相同 */
            mr_writel(new_smem, &sptr_lcdif->NEXT_BUF);

            /* 重新打开LCD */
            mr_writel(mr_bit(17) | mr_bit(0), &sptr_lcdif->CTRL_SET);

            break;

        default:
            break;
    }

    return ER_NORMAL;
}
```

##### 8.11.2. 应用层显示接口
HeavenFox提供了通用的显示接口，如描点、画线、字符串显示、bmp图像打印等。主要依赖两个结构体传递参数：
```c
/* 描述帧缓冲区的固定信息 */
struct fwk_disp_info
{
    void *buffer;                                           /* 当前缓冲区: 正在显示的区域 */
    void *buffer_bak;                                       /* 备用缓冲区: 中转区域, 大小应与buffer相同 */
    kusize_t buf_size;                                      /* 当前缓冲区(buffer/buffer_bak)的大小 */
    kuint32_t width;                                        /* 屏幕的宽度 (buffer/buffer_bak的宽度) */
    kuint32_t height;                                       /* 屏幕的高度 (buffer/buffer_bak的高度) */
    kuint8_t bpp;                                           /* 每个像素点所占的位数, 如16, 24, 32, ... */

    struct mutex_lock sgtc_lock;                            /* 内置互斥锁, 防止缓冲区被多个线程同时操作, 导致显示内容错乱 */
};

/* 绘制区域控制 */
struct fwk_disp_ctrl
{
    struct fwk_disp_info *sptr_di;                          /* 缓冲区信息公用, 均指向同一片缓冲区 */
    struct fwk_font_setting sgtc_set;                       /* 字体属性 */

    kuint32_t x_start;                                      /* 绘制区域的x方向起始位置 */
    kuint32_t y_start;                                      /* 绘制区域的y方向起始位置 */
    kuint32_t x_end;                                        /* 绘制区域的x方向终点位置 */
    kuint32_t y_end;                                        /* 绘制区域的y方向终点位置 */

    kuint32_t x_next;                                       /* 绘制区域x方向的实时位置, 当其到达x_end时, 表示一行绘制结束 */
    kuint32_t y_next;                                       /* 绘制区域y方向的实时位置, 当其到达y_end, 且x_next到达x_start时, 表示一帧图像绘制结束 */
};
```

fwk_disp_ctrl表示一次绘制过程，含本次绘制的起止位置、绘制的实时位置、绘制所用的字体、颜色、间距等。不同的绘制单元可以使用不同的字体属性，由结构体fwk_font_setting控制。其定义为：
```c
enum __ERT_FWK_FONT_TYPE
{
    NR_FWK_FONT_SONG        = 0,                            /* 字体: 宋体 */
    NR_FWK_FONT_XINGKAI,                                    /* 字体: 行楷 */
};

struct fwk_font_setting
{
    void *ptr_ascii;                                        /* ascii字库首地址, 如g_font_ascii_song12、g_font_ascii_song16等 */
    void *ptr_hz;                                           /* 汉字库首地址, 可能来自于HZK16.bin (GB2312), 如: fwk_font_hz_song16_get()->base */

    kuint32_t font;                                         /* 字体选择: 取自"enum __ERT_FWK_FONT_TYPE" */
    kuint32_t color;                                        /* 前景字体颜色: 可选RGB_RED、RGB_BLUE、RGB_GREEN等, 取32位 */
    kuint32_t background;                                   /* 背景颜色 */
    kuint32_t size;                                         /* 字体大小, 可选16、24等 */
    kuint32_t line_spacing;                                 /* 行间距, 即上下两行相隔的像素点数 */
    kuint32_t word_spacing;                                 /* 字间距, 即左右相邻字符相隔的像素点数 */

    kuint32_t left_spacing;                                 /* 左侧页边距, 即段落距离屏幕左边界缩进的像素点数 */
    kuint32_t right_spacing;                                /* 右侧页边距, 即段落距离屏幕右边界保留的像素点数 */
    kuint32_t upper_spacing;                                /* 上侧页边距, 即段落距离屏幕上边界缩进的像素点数 */
    kuint32_t down_spacing;                                 /* 下侧页边距, 即段落距离屏幕下边界保留的像素点数 */
};

/*!< 8 * 12 ASCII字符点阵库 */
extern const kuint8_t g_font_ascii_song12[];

/*!< 16 * 16 ASCII字符点阵库 */
extern const kuint8_t g_font_ascii_song16[];

/*!< 24 * 12 ASICII字符点阵库 */
extern const kuint8_t g_font_ascii_song24[];

/* 获取汉字库(宋体16 * 16)的首地址 */
struct m_area *fwk_font_hz_song16_get(void);
```

比如以下用例：
```c
/* 配置字体属性 */
void display_task_settings_init(struct fwk_font_setting *sptr_set)
{
    sptr_set->color = RGB_BLACK;                            /* 前景色: 黑色 */
    sptr_set->background = RGB_WHITE;                       /* 背景色: 白色 */
    sptr_set->font = NR_FWK_FONT_SONG;                      /* 字体: 宋体 */
    sptr_set->line_spacing = 8;                             /* 行间距: 8个像素点 */
    sptr_set->word_spacing = 2;                             /* 字间距: 2个像素点 */
    sptr_set->ptr_ascii = (void *)g_font_ascii_song16;      /* ascii字库: 8 * 16 */
    sptr_set->ptr_hz = fwk_font_hz_song16_get()->base;      /* 汉字库: 取自HZK16.bin */
    sptr_set->size = FWK_FONT_16;                           /* 字体大小: 16 */

    sptr_set->left_spacing  = 16;                           /* 左侧页边距: 16个像素点 */
    sptr_set->right_spacing = 8;                            /* 右侧页边距: 8个像素点 */
    sptr_set->upper_spacing = 8;                            /* 上侧页边距: 8个像素点 */
    sptr_set->down_spacing  = 16;                           /* 下侧页边距: 16个像素点 */
}
```

各显示接口API：
```c
/* RGB格式转换(RGB888 <---> RGB565). srctype: 16/24/32, destype: 16/24/32 */
kuint32_t fwk_display_convert_rgbbit(kuint8_t srctype, kuint8_t destype, kuint32_t data);

/* 根据x、y坐标计算偏移量(相对于第1个像素点), 得到当前像素点在帧缓冲区的偏移点 */
kuint32_t fwk_display_advance_position(kuint32_t xpos, kuint32_t ypos, kuint32_t x_max);
/* 在偏移点offset处写一个像素点. 以32位为例: (bpp == 32) ? (*((kuint32_t *)buffer + offset) = data) : 其他  */
void fwk_display_write_pixel(void *buffer, kuint32_t offset, kuint8_t bpp, kuint32_t data);
/* 在偏移点offset处写一个像素点, 同时根据LCD像素格式自动转换data的值 (RGB565、RGB888) */
void fwk_display_write_frame_data(void *buffer, kuint32_t offset, kuint8_t bpp, kuint32_t data);

/* 在(xpos, ypos)这个位置写一个像素点 */
void fwk_display_write_point(struct fwk_disp_info *sptr_disp, kuint32_t xpos, kuint32_t ypos, kuint32_t data);
/* 以(x_start, y_start) ~ (x_end, y_end)两点画直线 */
void fwk_display_write_straight_line(struct fwk_disp_info *sptr_disp, kuint32_t x_start, kuint32_t y_start, 
                            kuint32_t x_end, kuint32_t y_end, kuint32_t data);
/* 以(x_start, y_start) ~ (x_end, y_end)两点画矩形 */
void fwk_display_write_rectangle(struct fwk_disp_info *sptr_disp, kuint32_t x_start, kuint32_t y_start, 
                            kuint32_t x_end, kuint32_t y_end, kuint32_t data);
/* 以(x_start, y_start) ~ (x_end, y_end)两点画矩形, 并填充矩形内部 */
void fwk_display_fill_rectangle(struct fwk_disp_info *sptr_disp, kuint32_t x_start, kuint32_t y_start, 
                            kuint32_t x_end, kuint32_t y_end, kuint32_t data);
/* 擦除整个屏幕(为当前缓冲区写入某个颜色) */
void fwk_display_clear(struct fwk_disp_info *sptr_disp, kuint32_t data);

/* 初始化sptr_disp各个成员 */
void fwk_display_ctrl_init(struct fwk_disp_info *sptr_disp, void *fbuffer, 
                          void *fbuffer2, kusize_t size, kuint32_t width, kuint32_t height, kuint32_t bpp);
/* 设置sptr_dctrl: 起始、终止坐标 */
void fwk_display_set_cursor(struct fwk_disp_ctrl *sptr_dctrl, 
                            kuint32_t x_start, kuint32_t y_start, kuint32_t x_end, kuint32_t y_end);
/* 交换sptr_disp->buffer和sptr_disp->buffer_bak, 即备用缓冲区上台, 而当前缓冲区退居后方 */
void fwk_display_frame_exchange(struct fwk_disp_info *sptr_disp);
/* 刷新: 将备用缓冲区的数据拷贝到当前缓冲区 */
void fwk_display_frame_flush(struct fwk_disp_info *sptr_disp, kusize_t size);
/* 同步: 将当前缓冲区的数据拷贝到备用缓冲区 */
void fwk_display_frame_sync(struct fwk_disp_info *sptr_disp, kusize_t size);

/* 显示字符串, 支持ascii、汉字; 起止坐标来自于sptr_dctrl, 使用前应设置sptr_dctrl的各个成员 (先调用fwk_display_set_cursor) */
kusize_t fwk_display_word(struct fwk_disp_ctrl *sptr_dctrl, const kchar_t *fmt, ...);
```

bmp图像使用另一个结构体“struct fwk_bmp_ctrl”来控制绘制区域，它除了包含起始、终止绘制坐标外，还有bmp图像的信息头（bmp图像偏移14个字节的位置，大小为40字节）。如下：
```c
struct fwk_bmp_ctrl
{
    struct fwk_disp_info *sptr_disp;                        /* 缓冲区信息公用, 均指向同一片缓冲区 */
    struct fwk_bmp_info_header sgtc_bi;                     /* bmp文件有两个头部, 前14字节为文件头(标识是否为BMP), 紧随其后的40个字节为信息头(表征图像宽高, 每个像素的位数等) */

    kuint32_t x_start;                                      /* 绘制区域的x方向起始位置 */                                 
    kuint32_t y_start;                                      /* 绘制区域的y方向起始位置 */
    kuint32_t x_next;                                       /* 绘制区域x方向的实时位置 */
    kuint32_t y_next;                                       /* 绘制区域y方向的实时位置 */
};

/* 其中: */
/* 图像文件头: 14字节 */
struct fwk_bmp_file_header
{
    kuint16_t picType;                                      /* 内容固定为0x42, 0x4D, 表示ascii为: 'B', 'M', 即BMP文件 */
    kuint32_t bmpSize;                                      /* BMP大小 (单位: byte) */

    kuint16_t reserved1;                                    /* 保留 */
    kuint16_t reserved2;                                    /* 保留 */
    kuint32_t offsetbit;                                    /* BMP file header + BMP info header + color palette */

} __attribute__((packed));

/* 图像信息头: 理论大小为40字节. 但真实大小取决于infoSize成员 */
struct fwk_bmp_info_header
{
    kuint32_t infoSize;                                     /* 整个信息头的长度, 并不是sizeof(struct fwk_bmp_info_header), 由具体图像决定. 跳过文件头大小(14字节)和infoSize, 才是图像数据 */
    kint32_t width;                                         /* 图像宽度 (单位: 像素点) */
    kint32_t height;                                        /* 图像高度 (单位: 像素点). 正数表示正方形 (从左下角开始), 负数表示反方向 (从左上角开始)  */
    kuint16_t plane;                                        /* color flat book, generally fixed at 1 */
    kuint16_t pixelbit;                                     /* 像素点颜色位数: 1、4、8、16、24 */
    kuint32_t compression;                                  /* 数据压缩属性 */
    kuint32_t imageSize;                                    /* 图像数据的字节数 */
    kuint32_t X_pixelPerM;                                  /* 水平方向单元长度, 即相邻像素点的距离 (单位: pixels/meter) */
    kuint32_t Y_pixelPerM;                                  /* 垂直方向单元长度, 即相邻像素点的距离 (单位: pixels/meter) */
    kuint32_t colorIndex;                                   /* the number of color indexes in the palette. 16-bit and 24-bit true color without color palette. This is 0 */
    kuint32_t importanIndex;                                /* the number of indexes that have an important impact on the image, 0 indicates that all are important */

} __attribute__((packed));
```

使用以下接口可快速绘制一张BMP图像：
```c
/* 初始化sptr_bctl */
void fwk_bitmap_ctrl_init(struct fwk_bmp_ctrl *sptr_bctl, 
                          struct fwk_disp_info *sptr_disp, kuint32_t x_start, kuint32_t y_start);

/* 检查bmp文件头(确定为BMP文件), 并保存信息头到sptr_bctl->sgtc_bi, 然后返回整个头部的大小 (14 + infoSize) */
kint32_t fwk_bitmap_get_and_check(struct fwk_bmp_ctrl *sptr_bctl, const kuint8_t *image);
/* 从image偏移"14 + infoSize"处开始读取图像数据, 并将数据输出到显存(缓冲区), 显示图像(仅显示前size个字节) */
kint32_t fwk_display_bitmap(struct fwk_bmp_ctrl *sptr_bctl, const kuint8_t *image, kusize_t size);

/* 一步到位: fwk_bitmap_get_and_check + 整幅图像显示 */
kssize_t fwk_display_whole_bitmap(struct fwk_bmp_ctrl *sptr_bctl, const kuint8_t *image);
```

#### 8.12. 网络
HeavenFox内置第三方库Lwip，网络协议层由Lwip全权处理。内核需要提供Lwip对下的底层代码，及对上的应用接口封装。
Lwip网络数据包均通过pbuf结构体传递，发送时将创建一个pbuf，并拷贝数据到其中；接收时同样创建一个pbuf，将接收的数据保存，然后由Lwip协议层传递给应用程序。HeavenFox为能与之解耦，参考Linux增加一个fwk_sk_buff结构体，专门负责底层报文收发。于是：
```Mermaid
graph TD

    A[应用程序发送数据包buffer] ---> B[socket调用lwip协议层，封装为pbuf]
    B ---> C[lwip经协议层添加报文头部]
    C ---> D[lwip_lowlevel_output：读取pbuf并创建fwk_sk_buff，拷贝pbuf数据段（含头部）到fwk_sk_buff]
    D ---> E[将fwk_sk_buff添加到发送队列] 
    E ---> F[网络发送线程被唤醒，取出发送队列的每一个fwk_sk_buff]
    F ---> G[进入驱动层，取出fwk_sk_buff的数据段（含头部），发送到网络设备寄存器]

    H[网络设备寄存器接收数据] ---> I[创建fwk_sk_buff并拷贝数据到其数据段]
    I ---> J[添加到接收队列]
    J ---> K[网络接收线程被唤醒，取出接收队列的每一个fwk_sk_buff]
    K ---> L[创建pbuf，拷贝fwk_sk_buff到其数据段] 
    L ---> M[进入lwip协议层，添加报文头部]
    M ---> N[进入应用层，触发接收回调函数，socket可读取数据（拷贝pbuf数据段到应用层接收缓冲区）]
```

看起来中间涉及三次拷贝，但网络寄存器的数据可以启用DMA辅助接收，可事先申请一大块内存用于fwk_sk_buff，并使DMA缓冲区地址指向fwk_sk_buff，则省去了CPU搬运寄存器数据到fwk_sk_buff的时间，只剩下fwk_sk_buff到pbuf，以及pbuf到应用层缓冲区的拷贝工作。

##### 8.12.1. 网络设备
每个网络设备都可以被抽象成一个“struct fwk_net_device”结构体，用于描述设备的物理地址、收发的数据包总数等，并提供网络设备的收、发及初始化函数接口，系统将通过fwk_net_device进行全局管理。
```c
struct fwk_net_device
{
    kchar_t name[32];                                       /*!< 网络设备名称, 可通过终端"ifconfig"命令查看 */

    kint32_t ifindex;                                       /*!< 可用于同名设备的序号区分, 如: eth0, eth1 */

    struct list_head sgtc_link;                             /*!< 所有的网络设备使用链表连接到全局 */
    struct fwk_netdev_stats sgtc_stats;                     /*!< 数据收发统计 */
    const struct fwk_netdev_ops *sptr_netdev_oprts;         /*!< 网络设备函数接口集合 */

    kuint32_t mtu;                                          /*!< 单次数据包的最大字节数限制(如1500) */
    kuint16_t hard_header_len;                              /*!< 物理层报文头部大小(如EtherNetII, 大小为14字节) */

    kutype_t last_rx;                                       /*!< 最后一次接收的时间 */
    kuint8_t dev_addr[6];                                   /*!< MAC地址 */

    struct fwk_device sgtc_dev;
    void *private_data;                                     /*!< 用于保存驱动私有数据 */
};

/* 创建一个网络设备. sizeof_priv表示额外的数据, 一般保存到private_data成员 */
struct fwk_net_device *fwk_alloc_netdev_mq(kint32_t sizeof_priv, const kchar_t *name,
                                        void (*setup) (struct fwk_net_device *sptr_ndev), kuint32_t queue_count);
/* 释放内存 */
void fwk_free_netdev(struct fwk_net_device *sptr_ndev);
/* 根据name从全局网络设备链表中查找对应的网络设备 */
struct fwk_net_device *fwk_ifname_to_ndev(const kchar_t *name);
/* 注册到全局网络设备链表 */
kint32_t fwk_register_netdevice(struct fwk_net_device *sptr_ndev);
/* 注销 */
kint32_t fwk_unregister_netdevice(struct fwk_net_device *sptr_ndev);
/* 下一个网络设备 */
struct fwk_net_device *next_netdevice(struct fwk_net_device *sptr_ndev);

#define fwk_alloc_netdev(sizeof_priv, name, setup)  \
                fwk_alloc_netdev_mq(sizeof_priv, name, setup, 1)
#define fwk_netdev_priv(sptr_ndev)                ((sptr_ndev)->private_data)
```

sptr_netdev_oprts是网络设备的操作函数集，由驱动程序提供，平台在获取到fwk_net_device后会直接访问函数集，可根据需要定义对应的入口。常用函数如：
```c
struct fwk_netdev_ops
{
    kint32_t (*ndo_init) (struct fwk_net_device *sptr_ndev);
    void (*ndo_uninit) (struct fwk_net_device *sptr_ndev);
    kint32_t (*ndo_open) (struct fwk_net_device *sptr_ndev);
    kint32_t (*ndo_stop) (struct fwk_net_device *sptr_ndev);
    netdev_tx_t (*ndo_start_xmit) (struct fwk_sk_buff *sptr_skb, struct fwk_net_device *sptr_ndev);
    kint32_t (*ndo_set_mac_address) (struct fwk_net_device *sptr_ndev, void *ptr_addr);
    kint32_t (*ndo_do_ioctl) (struct fwk_net_device *sptr_ndev, struct fwk_ifreq *sptr_ifr, kint32_t cmd);
    void (*ndo_tx_timeout) (struct fwk_net_device *sptr_ndev);
};
```

ndo_start_xmit就是发送线程取出队列后要调用的发送函数，而接收队列通常在中断中提交，是驱动程序主动发起，不需要平台去调用。收发函数分别是：
```c
/* 读取发送队列, 调用ndo_start_xmit */
kint32_t fwk_dev_queue_xmit(struct fwk_sk_buff *sptr_skb);
/* 提交数据到接收队列 */
kint32_t fwk_netif_rx(struct fwk_sk_buff *sptr_skb);
```

##### 8.12.2. 数据包
结构体“struct fwk_sk_buff”拥有完整的数据段（包括头部、数据部分、尾部），和Linux的“struct sk_buff”基本相同。
```c
/* 数据队列, 连接当前网络设备的所有数据包 */
struct fwk_sk_buff_head
{
    /*!< 前两个成员与"struct fwk_sk_buff"相同, 也必须相同, 以便二者可以互相强制类型转换 */
    struct fwk_sk_buff *sptr_next;
    struct fwk_sk_buff *sptr_prev;

    /* 数据包的个数 */
    kint32_t qlen;
};

struct fwk_sk_buff
{
    /*!< 用于连接前后数据包. 必须与"struct fwk_sk_buff_head"前两个成员相同 */
    struct fwk_sk_buff *sptr_next;
    struct fwk_sk_buff *sptr_prev;

    struct fwk_net_device *sptr_ndev;       /*!< 数据包绑定的网络设备 */

    kuint32_t len;                          /*!< 数据长度. 包括头部 (tail - data) */
    kuint32_t data_len;                     /*!< 数据长度, 不含头部 (len - 数据链路层 - 传输层数据) */

    kuint16_t protocol;                     /*!< 传输层协议, 如IP, ARP */
    kuint16_t transport_header;             /*!< 传输层所在的偏移(相对于head) */
    kuint16_t network_header;               /*!< 网络层所在的偏移(相对于head) */
    kuint16_t mac_header;                   /*!< 物理层所在的偏移(相对于head) */

    /*!< head ---> data ---> tail ---> end */
    sk_buff_data_t tail;                    /*!< 数据段结尾, 与data对应 */
    sk_buff_data_t end;                     /*!< 数据结尾, 与head对应 */
    kuint8_t *head;                         /*!< 数据段起始 */
    kuint8_t *data;                         /*!< 实际数据的起始地址 */
};
```

“struct fwk_sk_buff_head”是数据包连接的链表头，它的前两个成员与“struct fwk_sk_buff”相同，方便类型转换，如：
```c
struct fwk_sk_buff_head sgtc_head;
struct fwk_sk_buff sgtr_skb, *sptr_skb;

sgtc_head.sptr_next = &sgtr_skb;
sgtc_head.sptr_prev = &sgtr_skb;

/* 将链表头视为普通skb */
sptr_skb = (struct fwk_sk_buff *)sgtc_head;
printk("%u\n", sptr_skb->sptr_next->data_len);
```

在“struct fwk_sk_buff”结构体中有4个特殊的成员：tail、end、head和data，它们的关系和Linux中sk_buff一样。head表示数据段的起始，它一般是4字节对齐，而EtherNetII的数据头长度一般为14字节，假设head的地址为0，EtherNetII取head的地址，也为0，那紧随其后的网络层数据起始地址就是14（非4字节对齐），而网络层数据长度为20字节，故之后的传输层数据的起始地址也同样不是4字节对齐了。如果传输层和网络层涉及到unsigned int类型变量的读写，可能引发未对齐异常（触发abort）。实际上，EtherNetII数据头由3部分组成：
```c
struct fwk_eth_hdr
{
    kuint8_t h_dest[6];                      /*!< 目标物理地址 */
    kuint8_t h_source[6];                    /*!< 源物理地址 */

    kuint16_t h_proto;                       /*!< 网络层协议类型, 如IP, ARP */

} __packed;
```

根本不存在kuint32_t或kint32_t这种类型，即，EtherNetII完全不需要4字节对齐的起始地址。
一般我们将head往右偏两个字节，得到struct fwk_sk_buff::data成员。这样网络层的起始地址就是16，而传输层的起始地址就是36，满足4字节要求。如果为了保证数据段的“对称”，我们给尾部也留出两个字节，分别得到tail和end，其中end = tail + 2。
HeavenFox提供了对应的操作接口：
```c
/* head和end不变, data和tail同时偏移len个字节 (比如len = 2) */
void fwk_skb_reserve(struct fwk_sk_buff *sptr_skb, kuint32_t len);
/* head和data不变, tail向右偏移len个字节, 留出数据段的空间 */
void *fwk_skb_put(struct fwk_sk_buff *sptr_skb, kuint32_t len);
/* data左移len个字节, 数据段向左扩张 */
void *fwk_skb_push(struct fwk_sk_buff *sptr_skb, kuint32_t len);
/* data右移len个字节, 数据段缩小 */
void *fwk_skb_pull(struct fwk_sk_buff *sptr_skb, kuint32_t len);
```

“struct fwk_sk_buff”的transport_header、network_header和mac_header分别表示传输层、网络层、物理层数据相对于head的偏移，以便快速定位到指定数据位置。比如初始化时，mac_header被赋值为“data - head”，则物理层数据的起始地址 = head + mac_header = data。
可以使用函数接口进行操作：
```c
/* 初始化: sptr_skb->mac_header = data - head */
void fwk_skb_reset_mac_header(struct fwk_sk_buff *sptr_skb);
/* sptr_skb->mac_header = data - head + offset */
void fwk_skb_set_mac_header(struct fwk_sk_buff *sptr_skb, const kint32_t offset);
/* return head + sptr_skb->mac_header */
kuint8_t *fwk_skb_mac_header(struct fwk_sk_buff *sptr_skb);
/* return head + sptr_skb->mac_header - data */
kint32_t fwk_skb_mac_offset(struct fwk_sk_buff *sptr_skb);

/* 同mac_header */
void fwk_skb_reset_network_header(struct fwk_sk_buff *sptr_skb);
void fwk_skb_set_network_header(struct fwk_sk_buff *sptr_skb, const kint32_t offset);
kuint8_t *fwk_skb_network_header(struct fwk_sk_buff *sptr_skb);
kint32_t fwk_skb_network_offset(struct fwk_sk_buff *sptr_skb);

/* 同mac_header */
void fwk_skb_reset_transport_header(struct fwk_sk_buff *sptr_skb);
void fwk_skb_set_transport_header(struct fwk_sk_buff *sptr_skb, const kint32_t offset);
kuint8_t *fwk_skb_transport_header(struct fwk_sk_buff *sptr_skb);
kint32_t fwk_skb_transport_offset(struct fwk_sk_buff *sptr_skb);
```

以上是“struct fwk_sk_buff”重要成员的含义及赋值方法，最终整个结构将通过sptr_prev和sptr_next串接到全局队列“struct fwk_sk_buff_head”，即入队和出队；一个数据包至少需要经历1次入队和出队：发送时需要将lwip的pbuf转化为skb，然后入队，当队列被读取时出队，交给发送寄存器或DMA；接收时需要将新收到的skb入队，当队列被接收线程读取时出队，转化为lwip的pbuf。
```c
/* 创建一个skb, 数据部分(head ~ end)长度为data_size */
struct fwk_sk_buff *fwk_alloc_skb(kuint32_t data_size, nrt_gfp_t flags);
/* 释放一个skb */
void fwk_free_skb(struct fwk_sk_buff *sptr_skb);
/* 添加skb到全局队列 */
kint32_t fwk_skb_enqueue(struct fwk_sk_buff_head *sptr_head, struct fwk_sk_buff *sptr_skb);
/* 从全局队列读出skb */
struct fwk_sk_buff *fwk_skb_dequeue(struct fwk_sk_buff_head *sptr_head);
```

##### 8.12.3. socket
任何一个网络设备链路开启后（link up），都将被分配一个结构体“struct fwk_network_if”，存储设备的ip、子网掩码、网关信息，以及操作函数集。
而HeavenFox又为每个网络设备的访问者分配一个“套接字”，每个“套接字”对应一个“struct fwk_network_object”结构，并使用基数树保存它们。当要获取套接字xxx对应的fwk_network_object结构时，可从基数树快速找到。
```c
struct fwk_network_com
{
    kint32_t domain;                                /* 域, 可选NET_AF_INET */
    kint32_t type;                                  /* 类型, 可选NR_SOCK_STREAM/NR_SOCK_DGRAM/NR_SOCK_RAW */
    kint32_t protocol;                              /* 协议, 可选NET_IP_PROTO_UDP/NET_IP_PROTO_ICMP */

    struct fwk_sockaddr_in sgtc_sin;                /* 绑定的ip地址 */
    void *private_data;                             /* 中间件的私有数据, 如lwip返回的结构指针(udp_pcb, raw_pcb) */
};

struct fwk_network_object
{
    struct fwk_network_com sgtc_socket;             /* 访问者的核心结构 */
    struct fwk_network_if *sptr_if;                 /* 绑定ip地址后, 将根据ip得到网络设备对应的sptr_if, 赋值于此 */

    struct radix_link sgtc_radix;                   /* 基数树链接点, 套接字即基数树对应的"数字"节点 */
};
```

“struct fwk_network_object”只是一个简单结构，它的真正核心是sgtc_socket和sptr_if。前者是其固有成员，用来存储申请套接字时传入的参数信息；后者是指针，根据“struct fwk_network_com::sgtc_sin”中的ip地址找到对应的网络节点后，保存该节点的“struct fwk_network_if”到此，由此建立联系；如果找不到节点，说明网络设备可能不存在，或者链路处于关闭状态，即尚未注册到lwip。
```c
struct fwk_sockaddr_in 
{
    fwk_sa_family_t sin_family;                         /* 如NET_AF_INET */
    kuint16_t       sin_port;                           /* 端口号 */
    struct fwk_in_addr sin_addr;                        /* ip地址 */

    /*!< zero[8], 无意义 */
    kuint8_t zero[16 - sizeof(fwk_sa_family_t) - sizeof(kuint16_t) - sizeof(struct fwk_in_addr)];
};

struct fwk_network_if
{
    kchar_t ifname[32];                                 /* 与网络设备节点名称相同 */

    struct fwk_sockaddr_in sgtc_ip;                     /* ip地址 */
    struct fwk_sockaddr_in sgtc_gw;                     /* 网关 */
    struct fwk_sockaddr_in sgtc_netmask;                /* 子网掩码 */

    struct fwk_network_if_ops *sptr_oprts;              /* 由协议层定义的函数集, 一般是lwip提供 */

    struct list_head sgtc_link;                         /* 已激活的网络设备, 链接到全局链表 */
    void *private_data;                                 /* 用于协议层的私有数据, 如自定义的lwip_data */
};

/* 启用链路: 根据name找到网络设备节点, 并创建和初始化"struct fwk_network_if", 注册到lwip协议层(netif_add) */
kint32_t net_link_up(const kchar_t *name, struct fwk_sockaddr_in *sptr_ip, 
                    struct fwk_sockaddr_in *sptr_gw, struct fwk_sockaddr_in *sptr_mask);
/* 关闭链路, 销毁"struct fwk_network_if" */
kint32_t net_link_down(const kchar_t *name);
```

套接字（“struct fwk_network_object”）由以下API创建和使用：
```c
/* 创建"struct fwk_network_object"并返回基数树对应的数字作为套接字. 目前仅支持UDP和ICMP */
kint32_t net_socket(kint32_t domain, kint32_t type, kint32_t protocol);

/* 绑定ip. 根据ip找到匹配的"struct fwk_network_if", 并执行sptr_oprts->init() */
kint32_t socket_bind(kint32_t sockfd, const struct fwk_sockaddr *sptr_addr, fwk_socklen_t addrlen);
/* 调用sptr_oprts->sendto, 发送数据包 */
kssize_t socket_sendto(kint32_t sockfd, const void *buf, kssize_t len, 
                            kint32_t flags, const struct fwk_sockaddr *sptr_dest, fwk_socklen_t addrlen);
/* 调用sptr_oprts->recvfrom, 接收数据包 */
kssize_t socket_recvfrom(kint32_t sockfd, void *buf, size_t len, 
                            kint32_t flags, struct fwk_sockaddr *sptr_src, fwk_socklen_t *addrlen);

/* 关闭套接字: 销毁"struct fwk_network_object"并取消其在基数树的存在 */
void virt_close(kint32_t fd);
```

大致流程为：
```Mermaid
graph TD
    A[驱动程序创建并注册“struct fwk_net_device”网络设备节点] ---> B[应用程序启动链路：net_link_up]
    B ---> C[创建网络接口“struct fwk_network_if”，进入平台中间层]
    C ---> D[根据name找到网络设备节点，与fwk_network_if绑定]
    D ---> E[调用lwip协议层netif_add函数，注册接口和ip，进入lwip协议层]
    E ---> F[执行回调lwip_enet_init]
    F ---> G[执行函数钩子“struct fwk_net_device::sptr_netdev_oprts->ndo_open”，进入驱动层]
    G ---> H[初始化网络设备、网卡硬件（寄存器）]
    H ---> I[返回平台中间层，创建并初始化发送线程、发送队列]

    O[应用层调用net_socket，申请套接字] ---> P[创建“struct fwk_network_object”结构，保存到全局基数树]
    P ---> Q[返回基数树节点值作为“套接字”]
    Q ---> R[应用层调用socket_bind，根据ip地址找到匹配的fwk_network_if]
    R ---> S[执行“struct fwk_network_if::sptr_oprts->init”函数，创建和初始化lwip数据资源（如udp_pcb）]
    S ---> T[应用层调用socket_sendto，实际执行“struct fwk_network_if::sptr_oprts->sendto”函数]
    T ---> U[创建pbuf，并拷贝应用程序缓冲区数据到pbuf，进入lwip协议层，发出数据]
    U ---> V[触发lwip发送回调函数，进入lwip_lowlevel_output]
    V ---> W[读出每一个pbuf，创建fwk_sk_buff结构，拷贝数据，并添加到发送队列，唤醒发送线程]
    W ---> X[发送线程被唤醒，读出每一个fwk_sk_buff，调用fwk_dev_queue_xmit]
    X ---> Y[执行函数钩子“struct fwk_net_device::sptr_netdev_oprts->ndo_start_xmit”，进入驱动层]
    Y ---> Z[保存fwk_sk_buff指针到DMA缓冲区，使能硬件发送]
```

#### 8.13. 终端
内核支持串口终端（波特率115200），源码位于“term”文件夹下。
远程主机使用终端时，键盘每输入一个字符，都会被立即发送到设备从机，HeavenFox则根据传来的字符判断主机的行为。支持以下特殊字符的解析：
```shell
    空格键: 自动忽略重复的空格、首尾的空格;
    退格键: 输入退格键时, 删除前一个字符;
    方向上键: 显示上一个输入的命令;
    方向下键: 显示下一个输入的命令;
    Ctrl+C: 取消/中止输入;
    回车键: 确认键, 内核将解析之前输入的字符串, 处理对应的功能;
```

终端是HeavenFox的一个内核线程，名称为“terminal”，但优先级并不高，主要靠轮询（查询串口是否有数据接收）分辨是否有输入。
终端支持动态插入和删除命令，使用如下接口实现：
```c
/* 每一个命令都使用该结构体表示 */
struct term_cmd
{
    /* 命令名称 */
    kchar_t name[32];

    /* 命令的执行函数, argc表示命令参数, argv为命令内容. 如"ttc 188 66", argc = 3, argv[] = { "ttc", "188", "66" } */
    kint32_t (*do_excute)(struct term_cmd *, kint32_t argc, kchar_t **argv);
    /* 命令的帮助函数 */
    void (*help)(void);

    /* 所有命令连接到全局链表 */
    struct list_head sgtc_link;
};

/* 创建一个命令 (名字为name) */
struct term_cmd *term_cmd_allocate(const kchar_t *name, nrt_gfp_t gfp_mask);
/* 销毁一个命令 */
void term_cmd_free(struct term_cmd *sptr_cmd);
/* 注册新命令 */
kint32_t term_cmd_add(struct term_cmd *sptr_cmd);
/* 注销命令 */
void term_cmd_del(struct term_cmd *sptr_cmd);
```

回车键按下后，内核将遍历全局命令链表sgtc_term_cmd_lists（即命令分发），找到与之对应的命令对象“struct term_cmd”，执行其do_excute函数。

---------------------------------------------------------
### 9. 文件系统
#### 9.1. 虚拟文件路径
##### 9.1.1. 节点对象
HeavenFox有两种“文件系统”：虚拟文件系统和真实文件系统，后者来自于磁盘（如SD卡），文件路径由磁盘的文件系统提供，如Fatfs32；前者由内核构建，采用“拟文件系统”的方式，虚构出一个文件路径，以假乱真，称之为“虚拟文件路径”（VFP）。
虚拟文件路径的每个节点由结构体fwk_kobject描述，比如存在路径“/sys/xxx”，“文件xxx”就会被表示为一个fwk_kobject。
虚拟文件路径的每个目录由结构体fwk_kset描述，比如存在路径“/sys/xxx”，“目录sys”就会被表示为一个fwk_kset。目录也是文件路径上的一个节点，fwk_kset最终也要通过fwk_kobject描述，即：fwk_kobject也是fwk_kset的成员之一。
真实文件系统会被“挂载”到虚拟文件系统中，外表看起来与虚拟文件系统无异；挂载后就成为了虚拟文件系统的一个目录（节点），将为其分配fwk_kset结构体。但是它的子目录和文件当前不会被分配fwk_kobject或fwk_kset结构体，它本质上还不是虚拟文件路径。如Fatfs32，将被挂载到“/mnt/FatFs_32”路径下，成为一个fwk_kobject节点对象。

虽然fwk_kobject可以表示文件系统的节点对象，但它也能用于其他场合，因其具有name、引用计数等成员，可配合其他需要此类成员的数据结构使用，此时则不一定是文件系统节点对象。
fwk_kobject的定义为：
```c
struct fwk_kobject
{
    kchar_t *name;                                      /* 当前节点对象的名称, 代表了它在虚拟文件路径上的名字 */
    struct atomic sgtc_ref;                             /* 引用计数 */

    struct list_head sgtc_link;                         /* 链表头位于kset, 表示当前目录下的所有节点对象 */
    struct fwk_kobject *sptr_parent;                    /* 父对象, 如果对象是文件或目录, 父对象应为: &fwk_kset::sgtc_kobj */
    struct fwk_kset *sptr_kset;                         /* 父路径, 如果对象是文件或目录, 则必然归属于某个目录. 除根目录节点外, 若sptr_kset为NULL, 则节点对象处于游离状态 */
    struct fwk_inode *sptr_inode;                       /* 当前文件更深层次的属性, 描述一个节点的信息 */

    kbool_t is_dir;                                     /* 当前节点对象的性质1: 是否为目录 */
    kbool_t is_disk;                                    /* 当前节点对象的性质2: 是否为磁盘 (即归属于"真实文件系统") */

    struct spin_lock sgtc_lock;                         /* 内置自旋锁 */
};

/* 初始化引用计数sptr_kref->sgtc_ref为0 */
void fwk_kref_init(struct atomic *sptr_kref);
/* 引用计数 + 1 */
void fwk_kref_get(struct atomic *sptr_kref);
/* 引用计数 - 1 */
void fwk_kref_put(struct atomic *sptr_kref);
/* 判断: 引用计数是否为0; 为0则返回true */
kbool_t fwk_kref_is_zero(struct atomic *sptr_kref);

/* 初始化一个对象 */
void fwk_kobject_init(struct fwk_kobject *sptr_kobj);
/* 创建(从内存池)一个对象并初始化 */
struct fwk_kobject *fwk_kobject_create(void);
/* 
 * 添加对象到虚拟文件路径, 使用前应指定要挂载的目录(sptr_kobj->sptr_kset), 否则将挂接到根目录. 
 * 本函数将同步创建inode文件节点
 * sptr_parent为父对象, 但并一定是父目录 (sptr_parent != &sptr_kobj->sptr_kset->sgtc_kobj). 可以为NULL
 * fmt为本对象的名称, 用于命名. 
 */
kint32_t fwk_kobject_add(struct fwk_kobject *sptr_kobj, struct fwk_kobject *sptr_parent, const kchar_t *fmt, ...);
/* fwk_kobject_add的变形, 允许由上层接口提供可变参数 */
kint32_t fwk_kobject_add_vargs(struct fwk_kobject *sptr_kobj, struct fwk_kobject *sptr_parent, const kchar_t *fmt, va_list sptr_list);
/* fwk_kobject_add的反操作, 删除文件节点, 并取消对象命名 */
void fwk_kobject_del(struct fwk_kobject *sptr_kobj);
/* fwk_kobject_del的进一步操作, 释放sptr_kobj (kfree) */
void fwk_kobject_destroy(struct fwk_kobject *sptr_kobj);
/* 以sptr_head为起始, 构建节点名为name的全路径. 如name为"/dev/xxx/yyy/zzz", 若xxx和yyy不存在, 本函数将同时创建这两个目录 */
struct fwk_kobject *fwk_kobject_populate(struct fwk_kobject *sptr_head, const kchar_t *name);
/* 以sptr_head为起始, 找到路径为name的节点对象. name应为路径全名, 如"/dev/xxx"表示文件xxx, "/dev/xxx/"表示目录xxx */
struct fwk_kobject *fwk_find_kobject_by_path(struct fwk_kobject *sptr_head, const kchar_t *name);
/* 设置对象的名字, 即赋值sptr_kobj->name */
kint32_t fwk_kobject_set_name(struct fwk_kobject *sptr_kobj, const kchar_t *fmt, ...);
/* fwk_kobject_set_name的变形, 允许由上层接口提供可变参数 */
kint32_t fwk_kobject_set_name_args(struct fwk_kobject *sptr_kobj, const kchar_t *fmt, va_list sptr_list);
/* 重命名sptr_kobj->name, 其实和fwk_kobject_set_name完全一样 */
kint32_t fwk_kobject_rename(struct fwk_kobject *sptr_kobj, const kchar_t *fmt, ...);
/* 删除名字; 当对象是文件路径的一个节点时, 慎用此函数 */
void fwk_kobject_del_name(struct fwk_kobject *sptr_kobj);
/* 获取名字 */
kchar_t *fwk_kobject_get_name(struct fwk_kobject *sptr_kobj);

/* 引用计数 + 1 */
struct fwk_kobject *fwk_kobject_get(struct fwk_kobject *sptr_kobj);
/* 引用计数 - 1 */
void fwk_kobject_put(struct fwk_kobject *sptr_kobj);
/* 判断引用计数是否非0 (表示被引用, 返回true) */
kbool_t fwk_kobject_is_refered(struct fwk_kobject *sptr_kobj);
```

fwk_kset则十分简单，它只有两个成员，sgtc_kobj表示“目录也是一个对象”，并提供节点路径的通用属性；sgtc_list则用于联结本目录下的所有的节点（包括子目录、文件等）。
其定义为：
```c
struct fwk_kset
{
    struct fwk_kobject sgtc_kobj;                       /* 节点对象 */
    struct list_head sgtc_list;                         /* 链表头 */
};

/* 反推导fwk_kset */
#define mr_fwk_kset_get(sptr_kobj)  \
    (((sptr_kobj) && (sptr_kobj)->is_dir) ? mr_container_of(sptr_kobj, struct fwk_kset, sgtc_kobj) : mr_nullptr)

/* 初始化一个fwk_kset */
void fwk_kset_init(struct fwk_kset *sptr_kset);
/* 创建并初始化一个fwk_kset, 并设置fwk_kset::sgtc_kobj.name; sptr_parent则赋值给fwk_kset::sgtc_kobj.sptr_parent */
struct fwk_kset *fwk_kset_create(const kchar_t *name, struct fwk_kobject *sptr_parent);
/* 注册一个fwk_kset, 即根据name添加到虚拟文件路径 */
kint32_t fwk_kset_register(struct fwk_kset *sptr_kset);
/* fwk_kset_create + fwk_kset_register */
struct fwk_kset *fwk_kset_create_and_register(const kchar_t *name, struct fwk_kobject *sptr_parent);
/* 注销, 从路径中删除该目录 */
void fwk_kset_unregister(struct fwk_kset *sptr_kset);
/* 通过sptr_kobj来注销fwk_kset (需要反推导fwk_kset的地址) */
void fwk_kset_kobject_remove(struct fwk_kobject *sptr_kobj);
/* 获取根目录"/"的地址 */
struct fwk_kset *fwk_kset_get_root(void);
```

HeavenFox至少存在两个“虚拟目录”，分别是“/sys/”、“/dev/”，前者用于描述系统信息（如驱动详细信息、设备树节点等），后者描述设备驱动程序，每一个注册的驱动程序都将挂载到此路径下。
整个虚拟文件系统呈现如下结构：
```Mermaid
graph TD
    A[根目录“/”] --> C[dev：fwk_kset]
    A --> B[sys：fwk_kset]
    A --> D[mnt：fwk_kset]
    A --> E[其他]
    C --> L[ledgpio：fwk_kobject]
    C --> M[extkey：fwk_kobject]
    C --> N[fb0：fwk_kobject]
    D --> O[FAT32_2：fwk_kset]
    O --> P[boot]
    O --> Q[home]
```

FAT32_2是磁盘（SD卡）Fat32文件系统，是真实的文件系统，其子文件夹“boot”和“home”均是SD卡的目录。ledgpio、extkey、fb0均是HeavenFox的驱动程序，其本身并非文件，只是虚拟为文件节点对象；因其并非文件夹，只用fwk_kobject便可表示。

##### 9.1.2. 节点属性
fwk_inode是节点对象的属性，同时也提供了应用层操作节点对象的方法。如“/dev/ledgpio”，这是一个LED驱动程序“文件”，我们自然希望通过操作该“文件”控制LED设备；但这个所谓的“文件”并不真实存在，不可能真的打开它获取设备信息，因此，操作“虚拟文件”，本质上只能访问它的属性，而非其内容。
当打开“文件”时，内核将根据文件路径找到fwk_kobject结构体（如调用函数fwk_find_kobject_by_path），然后访问fwk_kobject::sptr_inode，获得节点对象的属性，再通过sptr_inode进一步调用底层硬件接口函数。fwk_inode的定义为：
```c
#define INODE_TYPE_FILE                     (0)     /* 表示是一个文件 */
#define INODE_TYPE_DIR                      (1)     /* 表示是一个目录 */

struct fwk_inode
{
    kchar_t *name;                                  /* 节点名称, 一般取sptr_kobj->name */
    kuint8_t type;                                  /* 节点类型: 文件(INODE_TYPE_FILE) 或 目录(INODE_TYPE_DIR) */
    kuint32_t r_dev;                                /* 设备号. 只要"/dev/"路径下的文件拥有设备号 */

    struct fwk_file_oprts *sptr_foprts;             /* 文件操作函数接口 */
    struct fwk_kobject *sptr_kobj;                  /* 对应的节点对象指针 */

    union
    {
        struct fwk_cdev *sptr_cdev;                 /* char device: 字符设备, 当设备文件为字节流时, 需底层驱动注册为字符设备 */
        struct fwk_block_device *sptr_blkdev;       /* block device: 块设备, 当设备为磁盘时, 需底层驱动注册为块设备 */
    };
};

/* 为sptr_inode->sptr_foprts赋值, type不同, 操作函数接口也不同 */
kint32_t fwk_inode_set_ops(struct fwk_inode *sptr_inode, kuint32_t type, kint32_t devNum);
/* 创建一个节点属性, 并调用fwk_inode_set_ops. 一般在fwk_kobject建立时就会调用, 与fwk_kobject强联系 */
struct fwk_inode *fwk_mk_inode(struct fwk_kobject *sptr_kobj, kuint32_t type, kint32_t devNum);
/* 删除节点属性 */
void fwk_rm_inode(struct fwk_inode *sptr_inode);
/* 根据节点的名称(完整绝对路径)获取节点属性 (先找到fwk_kobject, 再返回其关联的fwk_inode), 本质是调用fwk_find_kobject_by_path */
struct fwk_inode *fwk_inode_find(kchar_t *name);
/* 根据节点的名称(完整绝对路径)获取节点属性 (先找到fwk_kobject, 并检查fwk_kobject::is_dir是否为true, 是则返回其关联的fwk_inode) */
struct fwk_inode *fwk_inode_find_disk(const kchar_t *name);
```

sptr_foprts为文件操作方法，是节点的中间层函数集，根据设备类型分流，并调用更底层的硬件操作函数。流程大致为：
```Mermaid
graph LR
    A[文件路径“/dev/xxx”] --> B[对象查找：sptr_kobj = fwk_find_kobject_by_path]
    B --> C[获得对象节点：sptr_kobj->sptr_inode]
    C --> D[sptr_kobj->sptr_inode->sptr_foprts]
```

有关sptr_foprts的内容，下一章节再表。

##### 9.1.3. fwk_file与文件描述符
当一个文件被“打开”时，需要使用fwk_file结构体来描述这个被打开的文件。其定义为：
```c
#define O_RDONLY                        00000001    /* 文件以只读模式打开 */
#define O_WRONLY                        00000002    /* 文件以只写模式打开 */
#define O_RDWR                          00000003    /* 文件以可读可写模式打开 */

struct fwk_file
{
    kuint32_t mode;                                 /* 打开文件的模式. 只有O_RDONLY(bit 0)置1时才允许读, 只有O_WRONLY(bit 1)置1时才允许写 */

    struct fwk_inode *sptr_inode;                   /* 该文件的节点属性(文件被打开后, 临时将inode赋值到此) */
    struct fwk_file_oprts *sptr_foprts;             /* 该文件的底层硬件操作函数集, 文件被打开后, 由inode获取硬件信息, 并将操作方法赋值到此 */

    void *private_data;                             /* 提供给底层驱动程序使用, 可保存驱动程序的私有数据 */
};

/* 其中: */
struct fwk_file_oprts
{
    /* 打开文件, sptr_inode表示该文件的属性, sptr_file描述文件被打开的状态(以什么方式打开). 返回值为错误码 */
    kint32_t (*open) (struct fwk_inode *sptr_inode, struct fwk_file *sptr_file);
    /* 关闭文件. 返回值为错误码 */
    kint32_t (*close) (struct fwk_inode *sptr_inode, struct fwk_file *sptr_file);
    /* 将大小为size的buffer写给文件. 返回值为写入的字节数 */
    kssize_t (*write) (struct fwk_file *sptr_file, const kbuffer_t *buffer, kssize_t size);
    /* 从文件中读出大小为size的数据, 保存到buffrr. 返回值为读出的字节数 */
    kssize_t (*read) (struct fwk_file *sptr_file, kbuffer_t *buffer, kssize_t size);
    /* 以某种方式操作文件, 方式取决于命令cmd, 由文件自行定义cmd的含义; arg为变量地址, 可能是一个数组地址, 也可能是结构体地址 */
    kint32_t (*unlocked_ioctl) (struct fwk_file *sptr_file, kuint32_t cmd, kuaddr_t arg);
    /* 效果同unlocked_ioctl, 不常用 */
    kint32_t (*compat_ioctl) (struct fwk_file *sptr_file, kuint32_t cmd, kuaddr_t arg);
    /* 内存映射, 直接将驱动程序某个内存的地址返回, 应用层可直接访问 */
    kint32_t (*mmap) (struct fwk_file *sptr_file, struct fwk_vm_area *sptr_vma);
};
```

在HeavenFox中维护着一个全局文件数组，即：
```c
struct fwk_file *fd_array[32];
```

当一个文件被“打开”后，内核将为其申请一个fwk_file结构体，并保存地址到全局数组fd_array的空位置。数组的下标则被称为“文件描述符”（file descriptor，简称fd）。假设数组当前状态为：
```c
fd_array[] = { 0x1, 0x2, 0x3, 0, 0, 0, 0x4 };
```

新申请的fwk_file将被插入到空位置fd_array[3]，并将下标“3”作为文件描述符，即“fd = 3”，此后访问fd_array[fd]，即可使用fwk_file。
文件操作可通过以下API完成：
```c
/* virt全称为virtual, 一般用于访问"虚拟文件路径"; 磁盘文件系统不可使用此接口 */

/* 打开设备文件, dev表示文件名称(带绝对路径), mode为文件打开的方式 */
kint32_t virt_open(const kchar_t *dev, kuint32_t mode);
/* 关闭设备文件 */
void virt_close(kint32_t fd);
/* 将大小为size的buffer写给文件. 返回值为写入的字节数 */
kssize_t virt_write(kint32_t fd, const void *buf, kusize_t size);
/* 从文件中读出大小为size的数据, 保存到buffrr. 返回值为读出的字节数 */
kssize_t virt_read(kint32_t fd, void *buf, kusize_t size);
/* 以某种方式操作文件, 方式取决于命令request, 由文件自行定义request的含义; 可变参数可以是变量地址, 比如数组地址、结构体地址 */
kssize_t virt_ioctl(kint32_t fd, kuint32_t request, ...);
/* 内存映射, length、prot、flags均无意义, 填0即可. addr填NULL即可, 暂无用途. offset为内存偏移量 */
void *virt_mmap(void *addr, kusize_t length, kint32_t prot, kint32_t flags, kint32_t fd, kuint32_t offset);
/* 与virt_mmap成对使用, 解除内存映射. length填0即可 */
kint32_t virt_munmap(void *addr, kusize_t length);
```

这些函数本质都是调用fd_array[fd]的sptr_foprts函数集，如virt_open，最终会执行到sptr_foprts->open函数。
打开一个文件的流程大概如下：
```Mermaid
graph TD
    A[设备文件“/dev/xxx”] --> B[打开，可读可写：<br>fd = virt_open（“/dev/xxx”，O_RDWR）]
    B --> C[对象查找：sptr_kobj = fwk_find_kobject_by_path（NULL，“/dev/xxx”）]
    C --> D[获得对象节点：sptr_inode = sptr_kobj->sptr_inode]
    D --> E[获得fwk_inode的操作函数：sptr_foprts = sptr_inode->sptr_foprts]
    B --> F[从内存池创建文件结构：sptr_file（struct fwk_file）] 
    E --> H[文件与节点属性关联：<br>sptr_file->sptr_inode = sptr_inode;<br> sptr_file->sptr_foprts = sptr_foprts]
    F --> H
    H --> I[fwk_inode自身open函数：<br>sptr_foprts->open（sptr_inode，sptr_file）]
    I --> J[继续调用，并调整sptr_foprts指向底层驱动的操作函数集]
    J --> K[底层驱动open函数：<br>sptr_foprts->open（sptr_inode，sptr_file）]
```

现在有了文件描述符fd，可得“sptr_file = fd_array[fd]”，且sptr_file->sptr_foprts已关联驱动层的操作函数集，要读写该驱动（“设备文件”），可直接访问write函数：
```Mermaid
graph LR
    A[写文件，1个字节：virt_write（fd，buf，1）] --> B[sptr_file = fd_array（fd）]
    B --> C[sptr_foprts = sptr_file->sptr_foprts]
    C --> D[sptr_foprts->write（sptr_file，buf，1）]
```

所以，只要驱动程序提供fwk_file_oprts函数集，并与fwk_inode关联，就能通过上层函数（“virt_”系列）访问到底层；关键正在于关联关系的建立。下一章节将描述此过程。

#### 9.2. 字符设备
HeavenFox与Linux一样，将硬件设备分为三类：字符设备、存储设备、网络设备。网络设备在前面的章节中已经介绍（fwk_net_device），它本身并不参与虚拟文件路径的建立，即网络设备不会在虚拟文件路径中创建节点；但字符设备相反，内核注册的字符设备会挂接到“/dev/”路径下，生成一个节点对象，故字符设备需在“节点对象与文件系统”章节之后才能展开介绍。
如果你对字符设备这个概念不是很清楚，那完全可以用排除法，一个设备如果既非网络设备（以太网卡、wifi、蓝牙、zigbee等），也非存储设备（emmc、sd卡、flash），那它就是一个字符设备。值得注意的是，EEPROM虽然是存储设备，但它读写时需要按字节操作，不符合存储设备的大批量读写特点，它是归在字符设备里面的。液晶显存framebuffer虽然看起来像是大批量读写，但实际也是一个个像素点绘制，本身也是字节操作，它自然属于字符设备。

##### 9.2.1. 设备号
常见的字符设备包括LED（简单的IO）、按键（简单的IO）、AD、framebuffer、串口、各种传感器等，读写时可能是IO直驱（如LED、PWM、按键），也可能是借助简单通讯协议（如SPI、I2C等）。每个字符设备都有一个唯一的设备号，也叫设备ID，它是一个32位的整型数值，由主设备号（高12位）和次设备号组成（低20位），比如使用以下几个宏来拆分/合并它：
```c
/* 次设备号位域的掩码 */
#define DEV_NUM_BITMASK(bits)       ((1u << (bits)) - 1)
/* 次设备号位数 */
#define DEV_MINOR_BITS              (20)
/* 主设备号位数 */
#define DEV_MAJOR_BITS              (32 - (DEV_MINOR_BITS))
/* 合并主次设备号 */
#define MKE_DEV_NUM(major, minor)   ((kuint32_t)(((major) << (DEV_MINOR_BITS)) | (minor)))
/* 单独提取主设备号 */
#define GET_DEV_MAJOR(devNum)       ((kuint32_t)((devNum) >> (DEV_MINOR_BITS)))
/* 单独提取次设备号 */
#define GET_DEV_MINOR(devNum)       ((kuint32_t)((devNum) & (DEV_NUM_BITMASK(DEV_MINOR_BITS))))
```

这意味着，最多支持“2^12”个不同的主设备号，和“2^20”个不同的次设备号。一些常用字符设备的主设备号已经固定，它们是：
```c
enum __ERT_CHRDEV_MAJOR
{
    NR_CHRDEV_DUMMY_MAJOR = 0,      /* 保留, 不可使用 */

    NR_STDIN_MAJOR,                 /* 输入流 */
    NR_STDOUT_MAJOR,                /* 输出流 */
    NR_STDERR_MAJOR,                /* 错误码 */
    NR_DEBUG_MAJOR,                 /* 调试使用 */

    NR_LED_MAJOR,                   /* LED设备 */
    NR_KEY_MAJOR,                   /* 按键设备 */
    NR_INPUT_MAJOR,                 /* 输入设备 (若按键或触摸屏被开发者定义为输入设备, 也应使用此设备号) */
    NR_UART_MAJOR,                  /* 串口设备 */
    NR_RTC_MAJOR,                   /* 实时时钟设备 */
    NR_FBDEV_MAJOR,                 /* 帧缓冲设备 */
    NR_MISC_MAJOR,                  /* 混杂设备 */
    NR_TSC_MAJOR,                   /* 触摸设备 */
    NR_USB_MAJOR,                   /* USB设备 (这是根hub的主设备号) */

    NR_CHRDEV_MAJOR_MAX,
};
```

虽然主设备号理论上最大可支持“2^12”，实际受限于字符设备自身定义；内核按主设备号定义了一个全局数组，每个字符设备的主设备号被视为数组下标，注册到该全局数组中，统一管理。但该数组的长度仅有255，意味着实际只能注册255个设备。
其定义为：
```c
struct fwk_char_device
{
    kuint32_t major;                /* 主设备号 */
    kuint32_t baseminor;            /* 起始次设备号 */
    kuint32_t count;                /* 本设备次设备号的数目 */
    kchar_t name[32];               /* 本设备的名称 */

    /* 主设备号相同的设备(次设备号不同), 连接成链表 */
    struct fwk_char_device *sptr_next;
};

/* 数组 ? 不, 这是个哈希散列表, 每个数组下标对应一个主设备号, 同一主设备号的设备组成链表 */
struct fwk_char_device *sgtc_fwk_chrdevs[255];
```

1个fwk_char_device可以表示多个设备，数量由count决定，而baseminor表示这些设备的起始次设备号：意味着，这些设备必须次设备号连续。次设备号不连续时，应拆分成多个fwk_char_device，并用sptr_next连接。
fwk_char_device内部并无有意义的成员，它的核心作用就是控制主、次设备号的分配，保证不重复。即，驱动程序的设备号分配需由fwk_char_device间接完成。有以下接口可使用：
```c
/* 
 * 申请一个字符设备(本质是申请主设备号). 
 * baseminor: 指定起始次设备号; 
 * count: 设备数量(次设备号的数量); 
 * name: 设备名, 赋值给fwk_char_device::name
 * devNum: 若成功申请主设备号, 则主设备号与次设备号baseminor合并后赋值到此.
 */
kint32_t fwk_alloc_chrdev(kuint32_t *devNum, kuint32_t baseminor, kuint32_t count, const kchar_t *name);
/*
 * 指定主次设备号(devNum), 并注册. 若该设备号已经被其他设备占用, 则返回错误码, 注册失败
 * 支持主设备号越级, 即"GET_DEV_MAJOR(devNum + count) != GET_DEV_MAJOR(devNum)"
 */
kint32_t fwk_register_chrdev(kuint32_t devNum, kuint32_t count, const kchar_t *name);
/* 注销设备号, 将devNum返还sgtc_fwk_chrdevs[], 以便其他设备未来使用 */
void fwk_unregister_chrdev(kuint32_t devNum, kuint32_t count);
```

##### 9.2.2. fwk_cdev
字符设备用结构体fwk_cdev描述，其定义为：
```c
struct fwk_cdev
{
    kuint32_t devNum;                           /* 本设备的设备号 */
    kuint32_t count;                            /* 设备号数量 */

    struct fwk_file_oprts *sptr_oprts;          /* 设备操作函数集 (被应用层"virt_"系列函数调用) */
    struct fwk_cdev *sptr_next;                 /* 连接"主设备相同, 次设备号不连续"的各个fwk_cdev, 当前未使用 */

    void *privData;                             /* 私有数据, 预留给驱动程序使用 */
};
```

sptr_oprts是驱动程序可自定义的操作函数，是virt_open、virt_write触及底层硬件设备的重要接口。fwk_cdev同时也是fwk_inode结构体的成员（在其union共用体中），它会在fwk_inode::sptr_foprts->open函数中被赋值给fwk_inode::sptr_cdev，并由fwk_inode结构体带给驱动程序的open函数。
似乎看起来有点混乱，我们先来拆解各个步骤。当virt_open函数调用后，fwk_inode::sptr_foprts->open会被调用，这里需要找到设备文件对应的驱动程序fwk_cdev，查找依据则是fwk_inode->rdev（设备号）。fwk_cdev在内核中和fwk_char_device一样，由一个按设备号存储的全局数组管理，各个字符设备需要将fwk_cdev注册到该数组中。与fwk_char_device不同的是，该全局数组并非fwk_cdev类型，其定义为：
```c
struct fwk_probes
{
    kuint32_t devNum;                           /* 设备号 */
    kuint32_t range;                            /* 设备号数量 */

    void *data;                                 /* 私有数据. 用来保存字符设备fwk_cdev地址 */
    struct fwk_probes *sptr_next;               /* 同设备号、不同次设备号且不连续的fwk_probes组成链表 */
};

struct fwk_kobj_map
{
    struct mutex_lock sgtc_mutex;               /* 内置互斥锁 */
    struct fwk_probes *sptr_probes[255];        /* 数组 ? 不, 这是个哈希散列表, 每个数组下标对应一个主设备号, 同一主设备号的设备组成链表 */
};

/* 内核启动后, 从内存池分配; chrdev_map是字符设备专用 */
struct fwk_kobj_map *sptr_fwk_chrdev_map;
```

fwk_probes::range成员和fwk_char_device::count一样，表示设备号范围，fwk_register_chrdev函数内部会校验“devNum ~ （devNum + count）”的有效性（即设备号未被使用过），并支持以下情况：
```
devNum和(devNum + range)的主设备号一样, 则只需创建一个fwk_probes/fwk_char_device;
devNum和(devNum + range)的主设备号不一样, 则主设备号发生了越级, 需创建多个fwk_probes/fwk_char_device, 注册到全局数组
```

fwk_probes注册、注销由以下几个函数完成：
```c
/*
 * 注册新的设备号
 * sptr_domain: 字符设备可取sptr_fwk_chrdev_map
 * devNum: 起始设备号
 * range: 设备号的范围 (devNum ~ (devNum + range))
 * data: 传入fwk_cdev结构体地址, 将赋值到fwk_probes::data
 * 根据devNum和range寻找散列表的插入位置, 创建一个或多个(取决于主设备号是否越级)fwk_probes结构体, 加入到散列表中
 */
kint32_t fwk_kobj_map(struct fwk_kobj_map *sptr_domain, kuint32_t devNum, kuint32_t range, void *data);
/* 注销设备号(带范围), 将这些设备号还给内核 */
kint32_t fwk_kobj_unmap(struct fwk_kobj_map *sptr_domain, kuint32_t devNum, kuint32_t range);
/* 根据设备号查找, 返回主次设备号符合范围的data成员 (fwk_cdev) */
void *fwk_kobjmap_lookup(struct fwk_kobj_map *sptr_domain, kuint32_t devNum);
```

如此，注册fwk_cdev，其实就是调用fwk_kobj_map函数，并将fwk_cdev作为data参数传入。内核还提供了更上层的API，可以省去操作fwk_kobj_map的过程：
```c
/* 初始化一个sptr_cdev, sptr_oprts将被赋值到sptr_cdev->sptr_oprts */
kint32_t fwk_cdev_init(struct fwk_cdev *sptr_cdev, const struct fwk_file_oprts *sptr_oprts);
/* 从内存池分配, 并初始化一个sptr_cdev */
struct fwk_cdev *fwk_cdev_alloc(const struct fwk_file_oprts *sptr_oprts);
/* 注册一个sptr_cdev (本质是调用fwk_kobj_map) */
kint32_t fwk_cdev_add(struct fwk_cdev *sptr_cdev, kuint32_t devNum, kuint32_t count);
/* 注销一个sptr_cdev (本质是调用fwk_kobj_unmap) */
kint32_t fwk_cdev_del(struct fwk_cdev *sptr_cdev);
```

让我们完善之前“打开文件”的流程：
```Mermaid
graph TD
    A[设备文件“/dev/xxx”] --> B[打开，可读可写：<br>fd = virt_open（“/dev/xxx”，O_RDWR）]
    B --> C[对象查找：sptr_kobj = fwk_find_kobject_by_path（NULL，“/dev/xxx”）]
    C --> D[获得对象节点：sptr_inode = sptr_kobj->sptr_inode]
    D --> E[获得fwk_inode的操作函数：sptr_foprts = sptr_inode->sptr_foprts]
    B --> F[从内存池创建文件结构：sptr_file（struct fwk_file）] 
    E --> H[文件与节点属性关联：<br>sptr_file->sptr_inode = sptr_inode;<br> sptr_file->sptr_foprts = sptr_foprts]
    F --> H
    H --> I[fwk_inode自身open函数：<br>sptr_foprts->open（sptr_inode，sptr_file）]
    I --> J[假设这是个字符设备，则进一步：<br>sptr_cdev = fwk_kobjmap_lookup（sptr_fwk_chrdev_map，sptr_inode->r_dev）]
    J --> K[暂存sptr_cdev：sptr_inode->sptr_cdev = sptr_cdev，sptr_foprts = sptr_cdev->sptr_oprts]
    K --> L[底层驱动open函数：<br>sptr_foprts->open（sptr_inode，sptr_file）]
```

这样整个字符设备的操作过程就完整了。

##### 9.2.3. 设备节点
还有一步没有实现，就是如何将驱动程序加入到虚拟文件路径，产生一个“/dev/xxx”的设备节点对象。这一操作由函数fwk_device_create完成。其定义为：
```c
enum __ERT_DEVICE_TYPE
{
    NR_TYPE_NONE = 0,                           /* 无意义的设备类型, 常见于普通文件 (非设备驱动文件) */

    NR_TYPE_CHRDEV,                             /* 字符设备类型 */
    NR_TYPE_BLKDEV,                             /* 存储设备类型 */
    NR_TYPE_NETDEV,                             /* 网络设备类型 */
};

/* type: 设备类型, 取自"enum __ERT_DEVICE_TYPE"; devNum: 设备号; fmt: 虚拟文件路径节点对象的名字, 如fmt = "xxx", ===> "/dev/xxx" */
struct fwk_device *fwk_device_create(kuint32_t type, kuint32_t devNum, kchar_t *fmt, ...);
```

该函数的内部构造为（简化版）：
```c
struct fwk_device *fwk_device_create(kuint32_t type, kuint32_t devNum, kchar_t *fmt, ...)
{
    struct fwk_kobject *sptr_kobj;
    struct fwk_kobject *sptr_parent;
    struct fwk_device *sptr_dev;
    kchar_t *name;
    va_list sptr_valist;

    /* 获取父目录的节点对象指针 */
    sptr_parent = fwk_find_kobject_by_path(NULL, "/dev/");

    /* 处理可变参数 */
    va_start(sptr_valist, fmt);
    name = vasprintk_safe(fmt, NULL, sptr_valist);
    va_end(sptr_valist);

    /* 在"/dev"目录下创建节点对象(名称为name) */
    sptr_kobj = fwk_kobject_populate(sptr_parent, name);

    /* 设置中间层的操作函数集, 为sptr_inode->sptr_foprts赋值(通用接口, 非驱动层的操作函数; 驱动层函数集为sptr_cdev->sptr_oprts) */
    if (sptr_kobj->sptr_inode->type == INODE_TYPE_FILE)
        fwk_inode_set_ops(sptr_kobj->sptr_inode, type, devNum);

    /* 创建并初始化一个fwk_device */
    sptr_dev = kzalloc(sizeof(*sptr_dev), GFP_KERNEL);
    fwk_device_initial(sptr_dev);

    /* 设置名称: sptr_dev->sgtc_kobj.name */
    mr_dev_set_name(sptr_dev, "%s", name);
    /* 作为一个新设备, 挂接到全局设备链表 */
    fwk_device_add(sptr_dev);

    /* 可变参数用完即弃 */
    fmt_free(name);
    return sptr_dev;
}

/* 其中: */
kint32_t fwk_inode_set_ops(struct fwk_inode *sptr_inode, kuint32_t type, kint32_t devNum)
{
    if (!sptr_inode)
        return -ER_NOMEM;

    /* 保存设备号, 以便sptr_inode->sptr_foprts->open能找到fwk_cdev */
    sptr_inode->r_dev = devNum;

    switch(type)
    {
        /* 字符设备 */
        case NR_TYPE_CHRDEV:
            /* 提供中间层的open、close函数, 负责调用fwk_kobjmap_lookup找到设备号匹配的fwk_cdev, 获取其fwk_cdev::sptr_oprts */
            sptr_inode->sptr_foprts = &sgtc_fwk_inode_def_chrfoprts;
            break;

        /* 存储设备 */
        case NR_TYPE_BLKDEV:
            /* 与sgtc_fwk_inode_def_chrfoprts类似 */
            sptr_inode->sptr_foprts = &sgtc_fwk_inode_def_blkfoprts;
            break;

        /* 网络设备 */
        case NR_TYPE_NETDEV:
            /* 网络设备不需要用open函数, 此函数集为空 */
            sptr_inode->sptr_foprts = &sgtc_fwk_inode_def_netfoprts;
            break;

        default:
            break;
    };

    return ER_NORMAL;
}
```

##### 9.2.4. 用例
一个简单的驱动例程（含之前的虚拟平台总线、gpio架构）：
```c
/*
 * Template of Character Device : LED
 * CPU: IMX6ULL
 *
 * File Name:   led_gpio.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.13
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/base/fwk_cdev.h>
#include <platform/base/fwk_chrdev.h>
#include <platform/base/fwk_inode.h>
#include <platform/base/fwk_fs.h>
#include <platform/of/fwk_of.h>
#include <platform/base/fwk_platdrv.h>
#include <platform/base/fwk_uaccess.h>
#include <platform/gpio/fwk_gpiodesc.h>

/*!< The defines */
struct led_drv_data
{
    kchar_t *ptrName;
    kuint32_t major;
    kuint32_t minor;
    struct fwk_gpio_desc *sptr_gdesc;

    struct fwk_cdev *sptr_cdev;
    struct fwk_device *sptr_idev;

    void *ptrData;
};

#define LED_DRIVER_NAME                             "ledgpio"
#define LED_DRIVER_MAJOR                            (NR_LED_MAJOR)

/*!< API function */
/*!
 * @brief   驱动程序open函数
 * @param   sptr_inode, sptr_file
 * @retval  错误码
 * @note    virt_open终极调用函数
 */
static kint32_t led_driver_open(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    struct led_drv_data *sptr_data;

    /*!
     * @note
     * sptr_cdev已经在sptr_inode->sptr_foprts->open函数中根据设备号获取, 并保存到sptr_inode->sptr_cdev;
     * sptr_cdev->privData在led_driver_probe函数中赋值为sptr_data (参数传递), 这里可以取出
     * 可以转存到sptr_file->private_data, 更方便led_driver_write函数取用
     */
    sptr_data = sptr_inode->sptr_cdev->privData;
    sptr_file->private_data = sptr_data;

    return 0;
}

/*!
 * @brief   驱动程序退出函数
 * @param   sptr_inode, sptr_file
 * @retval  错误码
 * @note    virt_close终极调用函数
 */
static kint32_t led_driver_close(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    /* 不需要了 */
    sptr_file->private_data = mr_nullptr;

    return 0;
}

/*!
 * @brief   驱动程序写函数
 * @param   sptr_file: 文件指针
 * @param   ptrBuffer: 写缓冲 (ptrBuffer[0]为LED状态值, 0或1)
 * @param   size: 写缓冲的大小, 对于LED, 只需1个字节即可表示
 * @retval  错误码
 * @note    将ptrBuffer[0]的值作为LED的状态值
 */
static kssize_t led_driver_write(struct fwk_file *sptr_file, const kbuffer_t *ptrBuffer, kssize_t size)
{
    struct led_drv_data *sptr_data;
    kuint8_t value;

    /* led_driver_open函数赋值, 这里读出 */
    sptr_data = (struct led_drv_data *)sptr_file->private_data;

    /* 谨慎行为, 将ptrBuffer先拷贝到value变量 */
    fwk_copy_from_user(&value, ptrBuffer, 1);
    /* gpio设置值: LED状态 = value (0或1, 灭或亮) */
    fwk_gpio_set_value(sptr_data->sptr_gdesc, !!value);

    return 0;
}

/*!
 * @brief   驱动程序读函数
 * @param   sptr_file: 文件指针
 * @param   ptrBuffer: 读缓冲 (ptrBuffer[0]为LED状态值, 0或1)
 * @param   size: 读缓冲的大小, 对于LED, 只需1个字节即可表示
 * @retval  错误码
 * @note    将LED的状态值保存到ptrBuffer[0]. 这里不需要读它的状态, 留空
 */
static kssize_t led_driver_read(struct fwk_file *sptr_file, kbuffer_t *ptrBuffer, kssize_t size)
{
    return 0;
}

/*!< 关键: 配置LED驱动操作函数集 */
const struct fwk_file_oprts sgtc_led_driver_oprts =
{
    .open   = led_driver_open,
    .close  = led_driver_close,
    .write  = led_driver_write,
    .read   = led_driver_read,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   虚拟平台总线probe函数 (当设备-驱动匹配成功后被调用)
 * @param   sptr_pdev: LED设备 (设备树节点转化得到)
 * @retval  错误码
 * @note    负责LED驱动的初始化工作
 */
static kint32_t led_driver_probe(struct fwk_platdev *sptr_pdev)
{
    struct led_drv_data *sptr_data;
    struct fwk_gpio_desc *sptr_gdesc;
    struct fwk_cdev *sptr_cdev;
    struct fwk_device *sptr_idev;
    kuint32_t devnum;
    kint32_t retval;

    /* led1为LED设备在设备树节点中的属性名(全称应为"led1-gpios"), 这里获取gpio描述信息 */
    sptr_gdesc = fwk_gpio_desc_get(&sptr_pdev->sgtc_dev, "led1", 0);
    if (!isValid(sptr_gdesc))
        return -ER_NODEV;

    /* 设置方向为输出, 且初始化为0 (熄灭状态) */
    fwk_gpio_set_direction_output(sptr_gdesc, 0);
    fwk_gpio_set_value(sptr_gdesc, 0);

    /* 指定主设备号为LED_DRIVER_MAJOR, 次设备号为0, 数量1. 注册设备号 (其实是检查该设备号是否可用, 若可用, 则占有它) */
    devnum = MKE_DEV_NUM(LED_DRIVER_MAJOR, 0);
    retval = fwk_register_chrdev(devnum, 1, LED_DRIVER_NAME);
    if (retval < 0)
        goto fail1;

    /* 从内存池分配一个sptr_cdev, 并赋值sptr_cdev->sptr_oprts = &sgtc_led_driver_oprts */
    sptr_cdev = fwk_cdev_alloc(&sgtc_led_driver_oprts);
    if (!isValid(sptr_cdev))
        goto fail2;

    /* 根据设备号注册sptr_cdev */
    retval = fwk_cdev_add(sptr_cdev, devnum, 1);
    if (retval < 0)
        goto fail3;

    /* 定义为字符设备类型(NR_TYPE_CHRDEV), 构建"/dev/ledgpio"节点对象 */
    sptr_idev = fwk_device_create(NR_TYPE_CHRDEV, devnum, LED_DRIVER_NAME);
    if (!isValid(sptr_idev))
        goto fail4;
    
    /* 创建驱动私有数据, 并保存各种重要参数 */
    sptr_data = (struct led_drv_data *)kzalloc(sizeof(struct led_drv_data), GFP_KERNEL);
    if (!isValid(sptr_data))
        goto fail5;

    sptr_data->ptrName = LED_DRIVER_NAME;
    sptr_data->major = GET_DEV_MAJOR(devnum);
    sptr_data->minor = GET_DEV_MINOR(devnum);
    sptr_data->sptr_cdev = sptr_cdev;
    sptr_data->sptr_gdesc = sptr_gdesc;
    sptr_data->sptr_idev = sptr_idev;

    /* 传递给sptr_cdev, 在led_driver_open中读出后赋值给文件指针 */
    sptr_cdev->privData = sptr_data;
    /* 保存到sptr_pdev->sgtc_dev.privData, 方便led_driver_remove函数获取 */
    fwk_platform_set_drvdata(sptr_pdev, sptr_data);

    return ER_NORMAL;

/* 失败后依次退出 */
fail5:
    fwk_device_destroy(sptr_idev);
fail4:
    fwk_cdev_del(sptr_cdev);
fail3:
    kfree(sptr_cdev);
fail2:
    fwk_unregister_chrdev(devnum, 1);
fail1:
    fwk_gpio_desc_put(sptr_gdesc);

    return -ER_FAILD;
}

/*!
 * @brief   虚拟平台总线remove函数 (当设备或驱动注销时被调用)
 * @param   sptr_pdev: LED设备 (设备树节点转化得到)
 * @retval  错误码
 * @note    负责LED驱动的退出工作
 */
static kint32_t led_driver_remove(struct fwk_platdev *sptr_pdev)
{
    struct led_drv_data *sptr_data;
    kuint32_t devnum;

    /* 读取sptr_pdev->sgtc_dev.privData */
    sptr_data = (struct led_drv_data *)fwk_platform_get_drvdata(sptr_pdev);
    if (!isValid(sptr_data))
        return -ER_NULLPTR;

    devnum = MKE_DEV_NUM(sptr_data->major, sptr_data->minor);

    /* 删除虚拟文件路径节点对象、注销字符设备、归还设备号 */
    fwk_device_destroy(sptr_data->sptr_idev);
    fwk_cdev_del(sptr_data->sptr_cdev);
    kfree(sptr_data->sptr_cdev);
    fwk_unregister_chrdev(devnum, 1);

    /* 释放gpio */
    fwk_gpio_desc_put(sptr_data->sptr_gdesc);

    /* 记得将sptr_pdev->sgtc_dev.privData置NULL, 以免其他驱动误用 */
    kfree(sptr_data);
    fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);

    return ER_NORMAL;
}

/*!< 设备树匹配列表, 当compatible成员的值与设备树节点compatible属性值相同时, 匹配成功, 之后led_driver_probe函数被调用 */
static const struct fwk_of_device_id sgtc_led_driver_id[] =
{
    { .compatible = "fsl,led-gpio", },
    {},
};

/*!< 驱动定义: 需提供probe、remove函数 */
static struct fwk_platdrv sgtc_led_platdriver =
{
    .probe  = led_driver_probe,
    .remove = led_driver_remove,
    
    .sgtc_driver =
    {
        .name   = LED_DRIVER_NAME,
        .id     = -1,
        .sptr_of_match_table = sgtc_led_driver_id,
    },
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   驱动初始化函数
 * @param   无
 * @retval  错误码
 * @note    内核线程初始化时调用, 将驱动挂接到虚拟平台总线, 然后匹配总线上的各个设备
 */
kint32_t __fwk_init led_driver_init(void)
{
    return fwk_register_platdriver(&sgtc_led_platdriver);
}

/*!
 * @brief   驱动退出函数
 * @param   无
 * @retval  无
 * @note    内核退出时调用, 将驱动从虚拟平台总线脱离(注销)
 */
void __fwk_exit led_driver_exit(void)
{
    fwk_unregister_platdriver(&sgtc_led_platdriver);
}

/* 指定隐式初始化, 直接定义到".dync.init"段, 由内核线程遍历".dync.init"的所有地址, 最终调用led_driver_init() */
IMPORT_DRIVER_INIT(led_driver_init);
/* 指定定义到".dync.exit"段 */
IMPORT_DRIVER_EXIT(led_driver_exit);
```

#### 9.3. 磁盘
目前HeavenFox支持的存储设备即SD卡，它会初始化两次，一是在boot阶段，用于读取卡内的文件（如设备树）；二是在内核初始化阶段，注册fatfs文件系统，并挂载sd卡。
sd卡默认挂载到虚拟文件路径“/media/FAT32_2”，如需要访问“home/fox/123.txt”，需引用完整路径“/media/FAT32_2/home/fox/123.txt”。

##### 9.3.1. 文件操作
“FAT32_2”是虚拟文件与真实文件的枢纽节点，它本身是一个虚拟文件路径，但内部却关系着SD卡的Fatfs文件系统，它的子目录、子文件都是货真价实。
在之前fwk_kobject一章中，我们知道fwk_kobject有一个成员“kbool_t is_disk”，它在虚拟文件路径中恒为false，但对于“/mnt/FAT32_2”，它的值为true，表示“FAT32_2”是一个真实的文件系统，是一个磁盘路径。
真实文件系统不再使用“virt_”系列的文件操作函数，且不返回文件描述符，而是直接使用文件流结构体fs_stream，其定义为：
```c
/* 文件操作模式 */
#define O_RDONLY                        00000001    /* 只读 */
#define O_WRONLY                        00000002    /* 只写 */
#define O_RDWR                          00000003    /* 可读可写 */
#define O_CREAT                         00000100    /* 新建 */
#define O_APPEND                        00002000    /* 追加 */

struct fs_stream
{
    kchar_t *full_name;                             /* 文件的完整路径, 如: "/media/FAT32_2/home/fox/123.txt" */
    kuint32_t mode;                                 /* 打开文件的方式, 如只读(O_RDONLY)、只写(O_WRONLY)、可读可写(O_RDWR) */
    struct fwk_inode *sptr_dnode;                   /* 文件的节点属性, 与字符设备用法一样. 不过, 只有"FAT32_2"存在节点对象, 它的所有子目录、子文件都共用此节点 */
    struct fwk_block_device_oprts *sptr_bops;       /* 与字符设备fwk_file_oprts一样, 这是存储设备的操作函数集(含文件读写操作) */

    void *private_data;                             /* 私有数据指针, 便于传递重要数据 */
};
```

fwk_block_device_oprts结构体我们后续再展开，但观察fs_stream，会发现它与“struct fwk_file”结构体内容极为相似，都有“文件打开方式”、“节点属性”、“操作函数集”；它同样表示一个被打开的文件，只不过这个文件是真实的。真实的文件由文件系统（如fatfs）自行管理，内核不会对真实文件建立fwk_kobject、fwk_kset节点对象，因为这需要遍历所有文件，是一个毫无意义的工作。同样地也不会构建fwk_inode，内核要获取真实文件的属性，只能通过fatfs文件系统。“/mnt/FAT32_2”是“最后一个”虚拟文件路径，它对下是真实文件系统；它自己是虚拟的，自然是一个fwk_kobject节点对象，也自然具有fwk_inode节点属性，所有真实文件都将共享这一节点属性，因为fwk_inode有一个“struct fwk_file_oprts”操作函数集，是应用层对接字符设备、存储设备的中间门户，内核访问真实文件，也需要先经过它，获取存储设备的信息，尤其是“struct fwk_block_device_oprts”操作函数集的资源地址。

真实文件可通过以下API操作：
```c
/* 打开一个名为name的文件(需含完整绝对路径), 打开方式为mode (新建、末尾追加、只读、只写、可读可写, ...) */
struct fs_stream *file_open(const kchar_t *name, kuint32_t mode);
/* 关闭文件 */
void file_close(struct fs_stream *sptr_fs);
/* 类似virt_write, 将buf的内容写入到文件中 */
kssize_t file_write(struct fs_stream *sptr_fs, const void *buf, kusize_t size);
/* 类似virt_read, 将文件的内容读出到buf中 */
kssize_t file_read(struct fs_stream *sptr_fs, void *buf, kusize_t size);
/* 获取文件的总大小 */
kssize_t file_size(struct fs_stream *sptr_fs);
/* 文件指针偏移, 定位到要读/写的位置 */
kint32_t file_lseek(struct fs_stream *sptr_fs, kuint32_t offset);
/* 获取当前文件指针的偏移值 */
kssize_t file_tell(struct fs_stream *sptr_fs);
```

当访问文件“/media/FAT32_2/home/fox/123.txt”时，需要先经过虚拟文件路径“/media/FAT32_2”，再进入fatfs文件系统。整体流程为：
```Mermaid
graph TD
    A[打开文件：<br>struct fs_stream *sptr_fstream = file_open（“/media/FAT32_2/home/fox/123.txt”，O_RDWR）]
    B[找到虚拟文件路径和磁盘之间的交汇点：<br>struct fwk_inode *sptr_inode = fwk_inode_find_disk（“/media/FAT32_2/home/fox/123.txt”）]
    C[为即将打开的文件创建文件指针：<br>sptr_fs（struct fs_stream），从内存池分配]
    D[访问中间层open函数：<br>sptr_inode->sptr_foprts->open，获取存储设备的数据]
    E[访问存储设备，进而访问fatfs文件系统]
    F[路径不含磁盘设备，不存在文件系统，直接退出]

    A --> B
    B --> |sptr_inode有效|C
    B --> |sptr_inode无效|F
    C --> D
    D --> E
```

##### 9.3.2. 项目
fs_stream结构体表示一个被打开的文件，但没有描述文件的结构关系，如：文件系统中哪些文件、隶属于哪个目录等。内核用fs_item表示一个项目（文件或目录）的静态信息：
```c
enum __ERT_FS_ITEM_TYPE
{
    NR_FS_ITEM_DIR = 0,                             /* 项目的类型: 是一个目录 */
    NR_FS_ITEM_FILE,                                /* 项目的类型: 是一个文件 */
};

struct fs_item
{
    kchar_t *name;                                  /* 项目的名字 */
    kuint32_t type;                                 /* 项目的类型: 取自"enum __ERT_FS_ITEM_TYPE" */
    kusize_t size;                                  /* 项目的大小 */
    struct list_head sgtc_link;                     /* 链表项, 同一目录下的子文件和子目录连接成链 */

    void *private_data;                             /* 私有数据, 一般为文件系统的核心结构(如fatfs的FILINFO数据) */
};
```

项目通过sgtc_link连接，链表头位于结构体fs_list，它用于表示一个被打开的目录结构，与fs_stream结构体类似：
```c
struct fs_list
{
    kchar_t *path;                                  /* 本目录的完整路径名称 */
    kuint32_t mode;                                 /* 打开本目录的方式 */

    kuint32_t file_num;                             /* 本目录子文件的数量 */
    struct list_head sgtc_files;                    /* 每个子文件都是一个fs_item项目, 将它们连接成链 */

    kuint32_t dir_num;                              /* 本目录子目录的数量 */
    struct list_head sgtc_dirs;                     /* 每个子目录都是一个fs_item项目, 将它们连接成链 */

    struct fwk_inode *sptr_dnode;                   /* 磁盘节点属性, 磁盘的所有项目共享一个节点属性 (虚拟文件路径与磁盘的交汇点) */

    /* 读取目录的方法(内核调取fatfs的接口). pattern为格式筛选 */
    kint32_t (*readdir)(struct fs_list *sptr_list, const kchar_t *pattern);
    void *private_data;                             /* 私有数据, 一般为文件系统的核心结构(如fatfs的DIR数据) */
};

/* 本目录下子目录和子文件的总和 */
#define mr_dir_items_num(sptr_list)                 ((sptr_list)->dir_num + (sptr_list)->file_num)
```

目录被打开时，将创建一个fs_list结构体，并根据目录路径找到对应的磁盘（如SD卡），取得该磁盘文件系统（如fatfs）的readdir入口地址，随后调用readdir函数访问磁盘文件系统，并读取本目录下的所有项目（子文件和子目录），为每个项目创建独立的fs_item结构体，保存到fs_list链表中。访问fs_list结构体，即可取出所有的项目。
可以使用一些接口进行操作：
```c
/* 打开目录, 并将符合格式pattern的项目读取到fs_list */
struct fs_list *dir_open(const kchar_t *path, const kchar_t *pattern, kuint32_t mode);
/* 关闭目录, 顺便销毁fs_list和各个fs_item */
void dir_close(struct fs_list *sptr_fs);
/* 刷新项目列表, 重新读取磁盘文件系统, 更新fs_list的项目 */
kint32_t dir_flush(struct fs_list *sptr_fs, const kchar_t *pattern);
/* 读取fs_list中的每一个项目, 先读出所有目录项, 最后读出所有文件项 */
struct fs_item *dir_read_item(struct fs_list *sptr_fs, struct fs_item *sptr_prev);

/* 遍历 */
#define foreach_dir_item(_item, _fs)    \
    for ((_item) = dir_read_item(_fs, mr_nullptr); (_item); (_item) = dir_read_item(_fs, _item))
```

#### 9.4. 存储设备
##### 9.4.1. 设备号
磁盘（如SD卡）是名副其实的存储设备，文件系统（fatfs）要在存储设备上运行，首先要初始化存储设备，并搭建虚拟文件路径和磁盘之间的接口。
存储设备和字符设备一样，也是通过设备号进行管理。字符设备用“struct fwk_char_device *sgtc_fwk_chrdevs[255]”全局数组标记每一个设备号，并通过fwk_register_chrdev函数注册；存储设备同样也有类似结构，即：
```c
struct fwk_block_major_name
{
    kchar_t name[32];                               /* 存储设备的名称 */
    kint32_t major;                                 /* 主设备号 */
};

/* 全局数组, 主设备号即数组下标; 非哈希散列表 */
struct fwk_block_major_name *sgtc_block_major_name[255];
```

内核可通过检查数组sgtc_block_major_name[255]来确定哪些设备号可用，并将可用的某个设备号占有（创建fwk_block_major_name结构并填充到对应位置），即注册过程。注册、注销可由以下结构完成：
```c
/* 注册一个存储设备, 主要是操作数组sgtc_block_major_name */
kint32_t fwk_register_blkdev(kuint32_t major, const kchar_t *name);
/* 注销存储设备, 将设备号还回数组 */
void fwk_unregister_blkdev(kuint32_t major, const kchar_t *name);
```

只有fwk_register_blkdev函数执行成功（返回0），主设备号major才是可用状态。

##### 9.4.2. fwk_gendisk
内核简单地将存储设备抽象为“struct fwk_gendisk”结构体，包含对设备的读写、挂载/卸载等通用操作；存储设备的设备号也能直接保存到fwk_gendisk结构体中。这与字符设备的fwk_cdev结构体类似。
其定义为：
```c
struct fwk_gendisk
{
    /* 主设备号 */
    kint32_t major;
    /* 次设备号起始 */
    kint32_t first_minor;
    /* 次设备号个数 */
    kint32_t minors;
    /* 磁盘名称 */
    kchar_t disk_name[32];

    /* 函数入口, 需由文件系统提供具体内容 */
    /* 挂载, 伴随系统初始化和磁盘初始化 */
    kint32_t (*mount)(struct fwk_gendisk *sptr_disk);
    /* 卸载, 磁盘恢复非激活状态 */
    kint32_t (*unmount)(struct fwk_gendisk *sptr_disk);
    /* 格式化 */
    kint32_t (*mkfs)(struct fwk_gendisk *sptr_disk);
    /* 创建文件夹 */
    kint32_t (*mkdir)(struct fwk_gendisk *sptr_disk, const kchar_t *dir_name);
    /* 删除文件夹 */
    kint32_t (*rmdir)(struct fwk_gendisk *sptr_disk, const kchar_t *dir_name);
    /* 打开文件夹, dir_open函数最终将调用此入口 */
    kint32_t (*opendir)(struct fwk_gendisk *sptr_gdisk, struct fs_list *sptr_list);
    /* 关闭文件夹, dir_close函数最终将调用此入口 */
    kint32_t (*closedir)(struct fwk_gendisk *sptr_gdisk, struct fs_list *sptr_list);

    /* 磁盘设备, 伴随设备号申请操作 */
    struct fwk_device *sptr_devfs;
    /* 更多的操作函数 (如文件读写) */
    struct fwk_block_device_oprts *sptr_bops;

    /* 简单表示磁盘设备类型, 对标字符设备 */
    struct fwk_block_device *sptr_blkdev;
};

/* 其中: */
struct fwk_block_device
{
    kchar_t name[32];                           /* 存储设备名称 */
    kuint32_t major;                            /* 主设备号 */

    struct fwk_inode *sptr_inode;               /* 文件的节点属性, 暂时无意义 */
    struct fwk_gendisk *sptr_gdisk;             /* 含有磁盘信息 */
};
```

对于Linux，fwk_gendisk代表磁盘本身，而fwk_block_device表示磁盘在文件系统中的抽象；在HeavenFox中，fwk_block_device主要用于存储设备的管理（基于设备号），并提供fwk_gendisk磁盘信息。
存储设备和字符设备都是由“struct fwk_probes”管理，只是隶属于不同的全局哈希散列表。这个在之前“fwk_cdev”一章中已有描述，这里再重提一嘴：
```c
struct fwk_probes
{
    kuint32_t devNum;                           /* 设备号 */
    kuint32_t range;                            /* 设备号数量 */

    void *data;                                 /* 私有数据. 用来保存存储设备fwk_block_device地址 */
    struct fwk_probes *sptr_next;               /* 同设备号、不同次设备号且不连续的fwk_probes组成链表 */
};

struct fwk_kobj_map
{
    struct mutex_lock sgtc_mutex;               /* 内置互斥锁 */
    struct fwk_probes *sptr_probes[255];        /* 数组 ? 不, 这是个哈希散列表, 每个数组下标对应一个主设备号, 同一主设备号的设备组成链表 */
};

/* 内核启动后, 从内存池分配; blkdev_map是字符设备专用 */
struct fwk_kobj_map *sptr_fwk_blkdev_map;
```

存储设备将以主设备号作为sptr_fwk_blkdev_map->sptr_probes[255]的数组下标，并将主设备号相同的设备结成链表，用fwk_probes::sptr_next连接。注册fwk_gendisk，本质就是创建一个fwk_probes结构体，并将fwk_block_device结构地址交给fwk_probes::data成员（注意，是fwk_block_device，而非fwk_gendisk）。此后可根据设备号找到sptr_fwk_blkdev_map->sptr_probes[主设备号]，并结合次设备号返回符合的fwk_probes数据结构，从而得到data成员。
详见以下API：
```c
/* 初始化sptr_gdisk, 并令sptr_gdisk->sptr_bops = sptr_oprts */
kint32_t fwk_gendisk_init(struct fwk_gendisk *sptr_gdisk, 
                            const struct fwk_block_device_oprts *sptr_oprts);
/* 申请一个新的fwk_gendisk, 并调用fwk_gendisk_init进行初始化, 且令sptr_gdisk->minors = minors */
struct fwk_gendisk *fwk_alloc_gendisk(kint32_t minors, 
                            const struct fwk_block_device_oprts *sptr_oprts);
/* 自动分配一个sptr_blkdev, 并注册sptr_gdisk到sptr_probes数组; 本函数将同时执行sptr_gdisk->mount, 挂载磁盘 */
kint32_t fwk_add_gendisk(struct fwk_gendisk *sptr_gdisk);
/* 执行sptr_gdisk->unmount, 卸载磁盘, 并释放sptr_blkdev, 将sptr_gdisk与sptr_probes数组脱离 */
kint32_t fwk_del_gendisk(struct fwk_gendisk *sptr_gdisk);
```

fwk_gendisk虽然提供了mount（挂载）、unmount（卸载）等接口，但更多聚焦于磁盘本身，表示磁盘的基本功能；而磁盘读写涉及文件输入输出流，聚焦于文件操作，这些功能比较集中，统一归于结构体“struct fwk_block_device_oprts”。其定义为：
```c
struct fwk_block_device_oprts
{
    /* 打开文件. 被file_open函数终极调用 */
    kint32_t (*open) (struct fwk_block_device *sptr_blkdev, struct fs_stream *sptr_file);
    /* 关闭文件 */
    kint32_t (*close) (struct fwk_block_device *sptr_blkdev, struct fs_stream *sptr_file);

    /* 将buffer的数组写入到文件流. 被file_write函数终极调用 */
    kssize_t (*write) (struct fs_stream *sptr_file, const void *buffer, kuint32_t size, kuint32_t offset);
    /* 将文件数据读出到buffer. 被file_read函数终极调用 */
    kssize_t (*read) (struct fs_stream *sptr_file, void *buffer, kuint32_t size, kuint32_t offset);
    /* 文件指针偏移, 改变将要读写的起始位置. 被file_lseek函数终极调用 */
    kint32_t (*lseek) (struct fs_stream *sptr_file, kuint32_t offset);
    /* 返回文件大小. 被file_size函数终极调用 */
    kssize_t (*fsize) (struct fs_stream *sptr_file);
    /* 返回文件指针偏移量. 被file_tell函数终极调用 */
    kssize_t (*fpos) (struct fs_stream *sptr_file);
};
```

fwk_block_device_oprts函数集并非由存储设备驱动程序提供，而是作为对接文件系统（如fatfs）的接口，在文件系统已有API的基础上二次封装得到，应来自于“fs”文件夹；相当于：
```Mermaid
graph TD
    A[存储设备（如SD卡）提供读写扇区的接口] --> B[填充文件系统预留的访问接口，比如fatfs的disk_read函数]
    B --> C[文件系统提供统一的读写API，比如fatfs的f_read函数]
    C --> D[将f_read函数二次封装，作为fwk_block_device_oprts的read函数]
    D --> E[file_read函数调用时，将层层深入，访问读扇区的底层接口]
```

打开文件的整个过程就是：
```Mermaid
graph TD
    A[打开文件：<br>file_open（“/media/FAT32_2/home/fox/123.txt”，O_RDWR）]
    B[找到虚拟文件路径和磁盘之间的交汇点：<br>struct fwk_inode *sptr_inode = fwk_inode_find_disk（“/media/FAT32_2/home/fox/123.txt”）]
    C[为即将打开的文件创建文件指针：<br>sptr_fs（struct fs_stream），从内存池分配]
    D[访问中间层open函数：<br>sptr_inode->sptr_foprts->open]
    E[根据设备号获取fwk_block_device：sptr_blkdev = fwk_kobjmap_lookup（sptr_fwk_blkdev_map，sptr_inode->r_dev）]
    G[获取文件操作函数：sptr_fs->sptr_bops = sptr_blkdev->sptr_gdisk->sptr_bops]
    H[访问文件层open函数：sptr_fs->sptr_bops->open（sptr_blkdev, sptr_fs）]
    I[进入fatfs的open函数：f_open]
    J[执行存储设备（SD卡）初始化函数：SD卡检测卡插入、激活等]
    F[路径不含磁盘设备，不存在文件系统，直接退出]

    A --> B
    B --> |sptr_inode有效|C
    B --> |sptr_inode无效|F
    C --> D
    D --> E
    E --> G
    G --> H
    H --> I
    I --> J
```

打开文件时已经获得了fs_stream文件指针，直接使用其读取文件：
```Mermaid
graph LR
    A[读文件：file_read（sptr_fs，buf，size）] --> B[文件层的read函数：sptr_fs->sptr_bops->read（sptr_fs，buf，size，0）]
    B --> C[fatfs的read函数：f_read]
    C --> D[执行存储设备（SD卡）的读扇区函数：发送读扇区命令，接收SD卡数据]
```

##### 9.4.3. 设备节点
存储设备同样会在“/dev/”路径下创建节点对象，但对于内核来说意义不大，因为都是借助“file_”系列函数调用文件系统接口，而非直接访问存储设备的扇区……。创建节点依然是使用fwk_device_create函数，我们只需将设备类型改成存储设备（NR_TYPE_BLKDEV）即可。节点的名称则取决于fwk_gendisk::disk_name。

---------------------------------------------------------
### 10. 线程管理

#### 10.1. 线程状态与迁移（一）：状态定义
在HeavenFox中，线程就是一个任务，当任务在执行时，称它处于运行态（running）；当它准备执行时，称它处于就绪态（ready）；一个无法被执行的任务，称它处于睡眠态（sleep），这种状态的任务，如果不能及时唤醒，将会被系统回收（被杀死）；一个暂时不能执行的任务，但是有机会变成就绪态，称它处于挂起态（suspend），处于该状态的任务，系统不会回收。
在单核CPU中，任何时刻只有一个任务处于运行态，当它结束时，会自动从处于就绪态的各个任务中取一个来运行。内核始终至少保留一个任务处于就绪态，如空闲线程。一个运行中的任务结束时，可能变为就绪、挂起、睡眠中的任一个状态；当变为就绪态时，可以在其他任务执行结束后重新恢复运行；当变为挂起态时，需要由中断或其他任务“唤醒”，并置为就绪态。挂起态不能直接恢复为运行态，必须经由就绪态，以便调度器裁决。

#### 10.2. 线程控制块
每个任务（线程）都使用结构体“struct thread”来描述：
```c
struct thread
{
    /*!< 线程名字: 每个线程都有独一无二的名字 */
    kchar_t name[32];

    /*!< 线程id: 每个线程都有独一无二的id */
    kuint32_t tid;

    /*!< state为当前状态(运行, 就绪, 挂起, 睡眠); to_state为目标状态, 当它非0时, 表示即将进行状态迁移 */
    kuint32_t state;
    kuint32_t to_state;

    /*!< 线程入口, 即任务的主体; ptr_args为start_routine的参数 */
    void *(*start_routine) (void *);
    void *ptr_args;

    /*!< 线程属性, 包括优先级, 时间片, 线程栈等信息 */
    struct thread_attr *sptr_attr;

    /*!< 所有的线程组成一个链表 */
    struct list_head sgtc_link;

    /*!< 所有的线程, 根据其优先级划分, 组成哈希散列表 */
    struct list_head sgtc_hash;

    /*!< 当前时间片 (初始值 = sptr_attr->sgtc_param.init_budget), 随着任务运行而递减 */
    kutime_t expires;

    /*!< 线程信号 */
    kuint32_t flags;

    /*!< 内置自旋锁 */
    struct spin_lock sgtc_lock;

    /*!< 每个任务的专属邮箱, 非必须 */
    struct mailbox *sptr_mb;

    /*!< 表示哪些锁被本线程持有, 一个线程可能持有多个不同的锁 */
    struct lock_owners sgtc_owners;     
    /*!< 线程正在等待哪个锁, 一个线程只能等待一把锁; 当线程被一把锁阻塞时, 是不可能再去请求另一把锁的 */
    struct lock_waiter sgtc_wait; 

    /*!< 私有参数, 一般保存sleep时的定时器事件指针 */
    void *time_event;
};
```

sptr_attr表示线程的属性：
```c
struct scheduler_param
{
    kint32_t priority;                          /*!< 初始优先级; 如果希望更改一个线程的优先级, 修改该成员即可 */
    kint32_t cur_priority;                      /*!< 当前优先级; 线程在更换状态时会读取priority的值作为当前优先级 */

    struct time_spec init_budget;               /*!< 初始时间片; 线程在迁移到运行态前会读取init_budget作为时间片 */
};

struct thread_attr
{
    kint32_t detachstate;                       /*!< 结合/分离, 源自于pthread的设定, HeavenFox默认线程独立, 该成员无作用 */
    kint32_t schedpolicy;                       /*!< 调度策略(抢占/轮转), 源自于pthread的设定, HeavenFox默认全都支持, 该成员暂无作用 */
    kint32_t inheritsched;                      /*!< 继承策略(是否继承父进程), 源自于pthread的设定, HeavenFox默认不继承, 该成员暂无作用 */

    struct scheduler_param sgtc_param;          /*!< 调度参数: 优先级, 时间片 */

    void *ptr_stack_start;                      /*!< 线程栈起始地址 (只有线程栈来自于内存池才会置该成员) */
    kuaddr_t stack_addr;                        /*!< 线程栈顶, 8字节对齐 */
    kusize_t stacksize;                         /*!< 线程栈大小 */

    struct mem_info sgtc_pool;                  /*!< 线程池 (即专为线程服务的内存池, 一般不使用) */
};
```

#### 10.3. 调度控制块“sgtc_scheduler_table”
所有线程都需要注册到调度控制块sgtc_scheduler_table，以便管理和调度。它是一个“struct scheduler_table”结构体：
```c
/*!< 哈希散列表 */
struct thread_hash
{
    struct list_head sgtc_list;                 /*!< 同一个hash的各个线程使用链表连接 */
    struct thread *sptr_tail;                   /*!< 链表的最后一项, 为NULL时表示hash为空 */
};

struct thread_list
{
    kuint64_t ffs_l;                            /*!< 每一位表示一个优先级, 可表示0 ~ 63 */
    kuint64_t ffs_h;                            /*!< 每一位表示一个优先级, 可表示64 ~ 99 */

    struct thread_hash sgtc_hash[100];          /*!< 0 ~ 99, 每一个hash表示一个优先级; 同优先级的线程放在同一个hash中 */
    struct spin_lock sgtc_lock;                 /*!< 内置自旋锁 */

/*!< sgtc_hash[0-99]均为空 */
#define __THREAD_HASH_EMPTY(hash)               (!(hash)->ffs_l && !(hash->ffs_h))           
};

struct scheduler_core
{
    struct thread_list sgtc_ready;              /*!< 就绪哈希散列表 */
    struct thread_list sgtc_suspend;            /*!< 挂起哈希散列表 */
    struct thread_list sgtc_sleep;              /*!< 睡眠哈希散列表 */
};

/*!< 线程管理器 */
struct scheduler_table
{
    kint32_t max_tidarr;                        /*!< 保留功能. 表示sptr_tid_array的长度 (1024) */
    kint32_t max_tids;                          /*!< 保留功能. 表示sptr_tid_array和sptr_tids的长度 */
    kint32_t max_tidset;                        /*!< 保留功能. tid最大值(包括sptr_tid_array和sptr_tids) */
    kint32_t ref_tidarr;                        /*!< 保留功能. 实时表示sptr_tid_array分配掉的tid数量 */

    struct {
        kutype_t cnt_out;                       /*!< sched_cnt每溢出一次, cnt_out++ */
        kutype_t sched_cnt;                     /*!< 调度次数, 每发生一次调度, sched_cnt++ */
    } sgtc_cnt;

    struct scheduler_core sgtc_core;            /*!< 哈希列表, 链表排序太慢, 借助数组快速排序(依据: 优先级) */

    struct list_head sgtc_ready;                /*!< 就绪列表, 按优先级从高到低排序 */
    struct list_head sgtc_suspend;              /*!< 挂起列表, 按优先级从高到低排序 */
    struct list_head sgtc_sleep;                /*!< 睡眠列表, 按优先级从高到低排序 */

    struct thread *sptr_work;                   /*!< 处于运行态的线程, 即当前正在运行的线程 */

    struct thread **sptr_tids;                  /*!< 保留功能 (当sptr_tid_array满时, 从内存池开辟) */
    struct thread *sptr_tid_array[1024];        /*!< 线程tid即数组sptr_tid_array的下标, 可表示tid: 0 ~ 1023 */

    struct spin_lock sgtc_lock;                 /*!< 调度锁 */
}
```

有关优先级和散列表，下一章节再做简述。这里“struct thread *sptr_tid_array[1024]”其实就是保存各个线程控制块（thread结构体）的指针，数组的下标就是线程的tid。
线程在迁移时，就是在sptr_work、sgtc_ready、sgtc_suspend和sgtc_sleep之间跑来跑去，读取sptr_work就能知道当前运行的是哪个线程，而其他三个都是链表形式，在插入时严格按照优先级从高到低的顺序排列，读取链表的第一项，就能获取该状态下优先级最高的线程。
调度器实例定义在“sched.c”：
```c
struct scheduler_table sgtc_scheduler_table =
{
    .max_tidarr     = 0,
    .max_tids       = THREAD_MAX_NUM,
    .max_tidset     = 0,
    .ref_tidarr     = 0,
    .sgtc_cnt       = {},

    /*!< 初始化自环 */
    .sgtc_ready     = LIST_HEAD_INIT(&sgtc_scheduler_table.sgtc_ready),
    .sgtc_suspend   = LIST_HEAD_INIT(&sgtc_scheduler_table.sgtc_suspend),
    .sgtc_sleep     = LIST_HEAD_INIT(&sgtc_scheduler_table.sgtc_sleep),

    .sptr_work      = mr_nullptr,
    .sptr_tids      = mr_nullptr,
    /*!< 初始化无任何线程 */
    .sptr_tid_array = { mr_nullptr },
    .sgtc_lock      = SPIN_LOCK_INIT(),
};
```

#### 10.4. 线程状态与迁移（二）：优先级排序
链表不同于数组，想要快速实现排序，一般要借助复杂的结构，比如红黑树；但内核对优先级做了限制（0 ~ 99），所以我们可以将每个同优先级的线程都归到一个链表中，组成100个链表，再用长度为100的指针数组存放每个链表的首地址，即为哈希散列表。
内核规定数值越低，优先级越高，故优先级为0的线程，优先级最高，而99最低。
当已知一个线程的优先级为88时，可直接找到数组的第88个位置，插入到该处的链表末尾，即完成一次插入；不过，这里使用的是“struct thread”结构体的sgtc_hash成员（插入到“struct scheduler_table”结构体的sgtc_core），并不是sgtc_link，我们还需找到当前数组的上一个非空位置。
假设即将插入的线程优先级为88，要插入的哈希数组为xxx_hash[100]， 则：
> 1）若xxx_hash[88]非空（即sptr_tail非NULL），直接使sgtc_hash插入到链表尾部，sgtc_link可直接链接sptr_tail->sgtc_link，并更新sptr_tail为新的thread；
>
> 2）若xxx_hash[88]为空（即sptr_tail为NULL），直接使sgtc_hash插入到链表尾部，此时新线程为xxx_hash[88]的唯一链表项，使sptr_tail直接等于新线程；之后需要向后查找，xxx_hash[0] ~ xxx_hash[87]，获取离xxx_hash[88]最近的一个散列表。需要借助“struct thread_list”结构体的ffs_l和ffs_h成员，它们的每一个bit表示一个优先级，前者可表示0 ~ 63，后者表示64 ~ 99；当新线程插入xxx_hash[88]后，需同步置ffs_h的第24位（88 - 64）为1，然后检查ffs_h的第0 ~ 23位是否存在置位，若有（假设为第a位），获取该位（优先级为“a + 64”，是比新线程优先级高且最近的位置）对应的xxx_hash[a + 64]，将新线程的sgtc_link成员链接到xxx_hash[a + 64].sptr_tail->sgtc_link即可；若在ffs_h没有找到，则退到ffs_l查找，若也没有，则表示新线程的优先级为最高。

内核由函数__thread_hash_add负责该过程：
```c
static struct list_head *__thread_hash_add(struct thread_list *sptr_list, struct thread *sptr_thread)
{
    /*!< 当前优先级 */
    kint32_t prio = thread_get_priority(sptr_thread->sptr_attr);
    struct thread_hash *sptr_hash = sptr_list->sgtc_hash + prio;
    struct thread *sptr_tail = sptr_hash->sptr_tail;
    struct list_head *sptr_prev = mr_nullptr;

    /*!< hash不为空, 表示已经有其他同优先级的线程存在, 直接加入 */
    if (sptr_tail)
    {
        list_head_add_head(&sptr_tail->sgtc_hash, &sptr_thread->sgtc_hash);

        /*!< sptr_prev标记上一个线程, 以便sptr_thread->sgtc_link插入 */
        sptr_prev = &sptr_tail->sgtc_link;
        sptr_hash->sptr_tail = sptr_thread;
    }
    /*!< hash为空, sptr_thread为该优先级的首个线程 */
    else
    {
        kint32_t prev_prio = THREAD_PROTY_NUM;
        
        /*!< sptr_thread为唯一线程 */
        list_head_add_head(&sptr_hash->sgtc_list, &sptr_thread->sgtc_hash);
        sptr_hash->sptr_tail = sptr_thread;

        /*!< 优先级在0 ~ 63之间, 仅需查询ffs_l */
        if (prio < 64)
        {
            kuint64_t ffs_l = sptr_list->ffs_l;
            kuint64_t bit_nr = (1ULL << prio);

            /*!< 抹掉比prio更高(包括prio)的位, 假设ffs_l为0b1111, 而prio为2, 则mask等于0b0011, 以便仅查询比prio低的置位 */
            kuint64_t mask = (bit_nr - 1) & ffs_l;

            sptr_list->ffs_l |= bit_nr;

            /*!< 找到比prio更低的置位, 即优先级比prio更高 */
            if (mask)
                prev_prio = mr_flsll(mask) - 1;
        }
        /*!< 优先级在63 ~ 99之间, 需先查询ffs_h, 若未找到比prio更低的置位, 则再查询ffs_l */
        else
        {
            kuint64_t ffs_h = sptr_list->ffs_h;
            kuint64_t ffs_l = sptr_list->ffs_l;
            kuint64_t bit_nr = (1ULL << (prio - 64U));
            kuint64_t mask = (bit_nr - 1) & ffs_h;

            sptr_list->ffs_h |= bit_nr;

            /*!< 如果抹掉比prio更高(包括prio)的位后, mask仍不为0, 表示ffs_h存在比prio更低的置位; 否则只能从ffs_l查询 */
            if (mask)
                prev_prio = 64U + mr_flsll(mask) - 1;
            else if (ffs_l)
                prev_prio = mr_flsll(ffs_l) - 1;
        }

        /*!< 若成功比prio更低的置位, 则获取该位(优先级)相应的hash, sptr_thread->sgtc_link将插入到sptr_prev后方 */
        if (prev_prio < THREAD_PROTY_NUM)
        {
            struct thread_hash *sptr_prevhash = sptr_list->sgtc_hash + prev_prio;
            sptr_prev = &sptr_prevhash->sptr_tail->sgtc_link;
        }
    }

    /*!< sptr_prev也可能为NULL, 说明sptr_thread为最高优先级 */
    return sptr_prev;
}

/*!< sptr_thread: 要插入的线程; sptr_head: 链表头(如&sgtc_ready); sptr_hash: 以优先级划分的哈希散列表(如就绪态所在的哈希表头) */
static kint32_t __schedule_add_status_list(struct thread *sptr_thread, 
                            struct list_head *sptr_head, struct thread_list *sptr_hash)
{
    struct list_head *sptr_last;

    /*!< 如果sptr_thread不是孤独的线程, 它正在其他链表中: 禁止插入 */
    if (mr_unlikely(!mr_list_empty(&sptr_thread->sgtc_link)))
        return -ER_EXISTED;

    /*!< 加载struct scheduler_param::priority到struct scheduler_param::cur_priority */
    thread_sync_priority(sptr_thread->sptr_attr);

    /*!< 根据优先级插入hash的对应位置, 并找到比sptr_thread优先级高且优先级数值最近的线程 */
    sptr_last = __thread_hash_add(sptr_hash, sptr_thread);

    /*!< 没有找到这样的线程? 说明sptr_thread为最高优先级, 直接插入到链表头的邻方! */
    if (!sptr_last)
        sptr_last = sptr_head;

    /*!< sptr_head实现有序排列 */
    list_head_add_head(sptr_last, &sptr_thread->sgtc_link);

    return ER_NORMAL;
}
```

线程状态发生迁移时，需要从当前态的散列表中删除，再插入到新状态对应的散列表。删除操作由__thread_hash_remove负责：
```c
static void __thread_hash_remove(struct thread_list *sptr_list, struct thread *sptr_thread)
{
    kint32_t prio = thread_get_priority(sptr_thread->sptr_attr);
    struct thread_hash *sptr_hash = sptr_list->sgtc_hash + prio;
    struct thread *sptr_tail = sptr_hash->sptr_tail;

    /*!< 很不幸, sptr_thread是hash链表的最后一项 */
    if (sptr_tail == sptr_thread)
    {
        /*!< sptr_thread是唯一的线程 */
        if (mr_list_reach_head(&sptr_hash->sgtc_list, &sptr_thread->sgtc_hash))
        {
            /*!< 初始化sptr_tail */
            sptr_hash->sptr_tail = mr_nullptr;

            /*!< 身为唯一的线程, 优先级也随之消散 */
            if (prio < 64)
                sptr_list->ffs_l &= ~(1ULL << prio);
            else
                sptr_list->ffs_h &= ~(1ULL << (prio - 64));
        }
        /*!< 还好, 不是唯一 */
        else
        {
            /*!< 取sptr_thread的前一个线程作为尾部 (sptr_thread是最后一个线程, 但不是唯一, 则它的上一个线程也肯定在同一个hash) */
            sptr_hash->sptr_tail = mr_list_prev_entry(sptr_thread, sgtc_hash);
        }
    }

    /*!< 删除线程, 并且将前后两个线程牵手 */
    list_head_del(&sptr_thread->sgtc_hash);
}

/*!< sptr_thread: 要删除的线程; sptr_head: 链表头(如&sgtc_ready); sptr_hash: 以优先级划分的哈希散列表(如就绪态所在的哈希表头) */
static void __schedule_del_status_list(struct thread *sptr_thread, 
                            struct list_head *sptr_head, struct thread_list *sptr_hash)
{
    /*!< 先从hash中移除 */
    __thread_hash_remove(sptr_hash, sptr_thread);

    /*!< 从状态链表中删去 */
    list_head_del(&sptr_thread->sgtc_link);
}
```

#### 10.5. 线程创建与注销
HeavenFox的线程都需要从内存池申请，不允许全局定义。新线程首先要确定tid， 需要在数组sgtc_scheduler_table.sptr_tid_array[1024]中找到一个未被使用的位置（即数组为NULL），取其下标作为tid。可通过函数get_unused_tid_from_scheduler实现：
```c
tid_t get_unused_tid_from_scheduler(kuint32_t i_start, kuint32_t count)
{
    kuint32_t i;
    kutype_t flags;

    spin_lock_irqsave(&__SCHED_LOCK, &flags);
    for (i = i_start; i < (i_start + count); i++)
    {
        if (!SCHED_THREAD_HANDLER(i))
        {
            spin_unlock_irqrestore(&__SCHED_LOCK, flags);
            return i;
        }
    }

    spin_unlock_irqrestore(&__SCHED_LOCK, flags);

    return -ER_MORE;
}
```

新创建的线程指针最终需要保存到sgtc_scheduler_table.sptr_tid_array[tid]中，首次注册默认为就绪态，即插入到就绪链表中。
```c
kint32_t register_new_thread(struct thread *sptr_thread, tid_t tid)
{
    struct thread_attr *sptr_it_attr;
    kutype_t flags;
    kint32_t retval;

    sptr_it_attr = sptr_thread->sptr_attr;

    if (SCHED_THREAD_HANDLER(tid))
        return -ER_INVALID;

    /*!< 检查线程栈是否存在 */
    if (!sptr_it_attr->stack_addr)
        return -ER_NOMEM;

    spin_lock_irqsave(&__SCHED_LOCK, &flags);
    /*!< 保存线程指针到数组, 占有该tid */
    SCHED_THREAD_HANDLER(tid) = sptr_thread;

    /*!< 初始化线程链表和优先级链表 */
    init_list_head(&sptr_thread->sgtc_link);
    init_list_head(&sptr_thread->sgtc_hash);

    /*!< initial spinlock */
    spin_lock_init(&sptr_thread->sgtc_lock);

    /*!< 默认名字: thread-[tid] */
    sprintk(sptr_thread->name, "thread-%d", tid);

    /*!< 默认为就绪态, 添加到就绪链表 */
    retval = schedule_add_ready_list(sptr_thread);
    if (retval < 0)
    {
        /*!< 添加失败 */
        SCHED_THREAD_HANDLER(tid) = mr_nullptr;
        spin_unlock_irqrestore(&__SCHED_LOCK, flags);
        return retval;
    }

    /*!< 同步状态 */
    __SYNC_THREAD_STATE(sptr_thread, NR_THREAD_READY);
    spin_unlock_irqrestore(&__SCHED_LOCK, flags);

    return ER_NORMAL;
}
```

处于就绪链表的线程，如果优先级靠前，那么在下一次调度时就有机会转变为运行态。
一般注册线程不会去直接调用register_new_thread，因为线程的属性还没有设置。考虑使用更全面的注册函数thread_create或kernel_thread_create。
kernel_thread_create主要用于内核线程的创建和注册，内核线程的tid一般要求在0 ~ 127之间；
thread_create主要用于用户线程的创建和注册，用户线程的tid一般要求在128 ~ 1023之间。
内核线程不可或缺，用于维持内核的基本运作，一般是HeavenFox自己提供，也不能轻易注销；而用户线程由用户根据实际应用自由创建和注销。

内核线程
```c
/*!
 * @param   ptr_id: __thread_create注册完成后返回的tid;
 * @param   base: 指定tid, 小于0时由内核自动分配; 否则以base作为tid: 若tid已经被占用, 则线程创建失败
 * @param   sptr_attr: 属性, 可以由外部传入全局变量的地址, 也可以由__thread_create自行创建
 * @param   pfunc_start_routine: 线程入口
 * @param   ptr_args: pfunc_start_routine的参数
 */
kint32_t __kernel_thread_create(tid_t *ptr_id, kint32_t base, 
                        struct thread_attr *sptr_attr, void *(*pfunc_start_routine) (void *), void *ptr_args)
{
    return __thread_create(ptr_id, base, 
                                sptr_attr, pfunc_start_routine, ptr_args, 0);
}

tid_t kernel_thread_create(tid_t tid, struct thread_attr *sptr_attr, 
                        void *(*pfunc_start_routine) (void *), void *ptr_args)
{
    kint32_t retval;

    /*!< tid小于0, 表示由内核自动分配tid; */
    if (tid < 0)
    {
        tid_t new_tid;

        retval = __kernel_thread_create(&new_tid, -1, 
                                    sptr_attr, pfunc_start_routine, ptr_args);
        return retval ? -1 : new_tid;
    }

    /*!< tid >= 0, 以tid作为新线程的id */
    retval = __kernel_thread_create(mr_nullptr, tid, 
                                sptr_attr, pfunc_start_routine, ptr_args);
    return retval ? -1 : tid;
}
```

用户线程
```c
/*!< 参数同__kernel_thread_create */
kint32_t __real_user_thread_create(tid_t *ptr_id, kint32_t base, 
                        struct thread_attr *sptr_attr, void *(*pfunc_start_routine) (void *), void *ptr_args)
{
    /*!< 与内核线程创建不同, __thread_create最后一个参数为THREAD_USER, 表示这是用户线程 */
    return __thread_create(ptr_id, base, 
                                sptr_attr, pfunc_start_routine, ptr_args, THREAD_USER);
}

kint32_t thread_create(tid_t *ptr_id, struct thread_attr *sptr_attr, 
                        void *(*pfunc_start_routine) (void *), void *ptr_args)
{
    /*!< 用户线程的tid都是由内核自动分配 */
    return __real_user_thread_create(ptr_id, -1, 
                                sptr_attr, pfunc_start_routine, ptr_args);
}
```

用户线程和内核线程都是通过函数__thread_create创建线程，只是通过其最后一个参数区分类型。
```c
static kint32_t __thread_create(tid_t *ptr_id, kint32_t base, struct thread_attr *sptr_attr,
                                void *(*pfunc_start_routine) (void *), void *ptr_args, kuint32_t flags)
{
    tid_t tid;
    struct thread *sptr_thread;
    struct thread_attr *sptr_it_attr;
    kuint32_t i_start = THREAD_TID_DYNC;
    kuint32_t count = THREAD_TID_USER - THREAD_TID_DYNC;
    kint32_t retval;

    /*!< 关闭抢占, 防止此时发生调度 */
    mr_preempt_disable();

    /*!< 如果是用户线程, tid从THREAD_TID_USER(128)开始分配; count表示可能要查找的最大次数, 顾名思义, 当其为0时, 禁止查找; 为1时, 只允许tid为128 */
    if ((THREAD_USER & flags) == THREAD_USER)
    {
        i_start = THREAD_TID_USER;
        count   = THREAD_MAX_NUM - THREAD_TID_USER;
    }

    /*!< base有效, 只允许分配tid为base (count = 1) */
    if ((base >= 0) && (base < THREAD_TID_DYNC))
    {
        i_start = base;
        count   = 1;
    }

    /*!< 获取未被使用的tid */
    tid = get_unused_tid_from_scheduler(i_start, count);
    if (tid < 0)
        goto fail;

    /*!< 若外部未提供属性结构, 自己创建一个 */
    sptr_it_attr = sptr_attr;
    if (!sptr_it_attr)
    {
        sptr_it_attr = (struct thread_attr *)kmalloc(sizeof(struct thread_attr), GFP_KERNEL);
        if (!isValid(sptr_it_attr))
            goto fail;

        /*!< 自动创建的属性, 使用默认参数 */
        if (!thread_attr_init(sptr_it_attr))
            goto fail2;
    }

    /*!< 再度检查 */
    if (!thread_attr_revise(sptr_it_attr))
        goto fail3;

    /*!< 创建线程 */
    sptr_thread = (struct thread *)kzalloc(sizeof(struct thread), GFP_KERNEL);
    if (!isValid(sptr_thread))
        goto fail3;

    sptr_thread->tid            = tid;
    sptr_thread->sptr_attr      = sptr_it_attr;
    sptr_thread->start_routine  = pfunc_start_routine;
    sptr_thread->ptr_args       = ptr_args;

    /*!< 注册, 添加到就绪列表 */
    retval = register_new_thread(sptr_thread, tid);
    if (retval < 0)
        goto fail4;

    if (ptr_id)
        *ptr_id = tid;
    
    mr_preempt_enable();
    return ER_NORMAL;

fail4:
    kfree(sptr_thread);
fail3:
    thread_attr_destroy(sptr_it_attr);
fail2:
    if (!isValid(sptr_attr))
        kfree(sptr_it_attr);
fail:
    mr_preempt_enable();
    return -ER_FAULT;
}
```

注销操作相对简单，使用thread_destory函数即可。但需注意，只有处于睡眠态的线程才允许注销，故注销线程前应切换线程为睡眠态。
```c
struct thread *unregister_thread(tid_t tid)
{
    struct thread *sptr_thread;
    kutype_t flags;

    spin_lock_irqsave(&__SCHED_LOCK, &flags);

    /*!< 运行中的线程不准注销 */
    if ((tid < 0) || (tid == mr_current->tid))
    {
        sptr_thread = ERR_PTR(-ER_LOCKED);
        goto END;
    }

    sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (!sptr_thread)
        goto END;

    /*!< 只有处于睡眠态的线程才能注销 */
    if (sptr_thread->state != NR_THREAD_SLEEP)
    {
        sptr_thread = ERR_PTR(-ER_BUSY);
        goto END;
    }

    /*!< 从睡眠链表中分离 */
    schedule_detach_sleep_list(sptr_thread);
    SCHED_THREAD_HANDLER(tid) = mr_nullptr;

END:
    spin_unlock_irqrestore(&__SCHED_LOCK, flags);
    return sptr_thread;
}

kint32_t thread_destory(tid_t tid)
{
    struct thread *sptr_thread;

    /*!< 从当前链表中注销, 并退还占用的tid */
    sptr_thread = unregister_thread(tid);
    if (IS_ERR(sptr_thread))
        return PTR_ERR(sptr_thread);

    if (mr_nullptr == sptr_thread)
        return ER_NORMAL;

    /*!< 若当前线程是因为调用了sleep引发的睡眠, 在它注销前需要销毁定时事件 */
    if (sptr_thread->time_event)
        thread_sleep_quit(sptr_thread->time_event);
    
    sptr_thread->time_event = mr_nullptr;

    /*!< 释放内存 */
    kfree(sptr_thread->sptr_attr);
    kfree(sptr_thread);

    return ER_NORMAL;
}
```

#### 10.6. 线程状态与迁移（三）：schedule_thread_switch
切换线程状态要用到函数schedule_thread_switch，但在此之前需要先设置线程的目标状态（“struct thread”结构体的to_state成员），schedule_thread_switch会根据state成员获知当前线程状态，并从对应链表中删除；再根据to_state成员获知线程希望迁移的状态，然后将当前线程插入到对应的链表。
线程状态切换，其实就是链表的转换。

线程切换规则：
    1）不论何时，都必须有一个线程处于运行态；
    2）处于运行态的线程，可以切换为就绪/挂起/睡眠；
    3）处于就绪态的线程，可以切换为运行/挂起/睡眠；
    4）处于挂起态的线程，可以切换为就绪/睡眠；
    5）处于睡眠态的线程，可以切换为就绪/挂起；

```c
kint32_t schedule_thread_switch(struct thread *sptr_thread)
{
//  struct thread *sptr_thread;
    tid_t tid;
    kuint32_t src, dst;
    kint32_t retval;
    
    /*!< Protected by caller, do not disable again */
//  mr_preempt_disable();

//  sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (mr_unlikely(!sptr_thread))
        return -ER_NODEV;

    tid = sptr_thread->tid;
    src = sptr_thread->state;
    dst = sptr_thread->to_state;

    /*!<
     * thread switch:
     * (At all times, it is necessary to ensure that at least one thread (including idle threads) is running)
     * running ---> ready/suspend/sleep
     * ready ---> running/suspend/sleep
     * suspend ---> ready/sleep
     * sleep ---> ready/suspend
     *
     * (only running and ready state can be switched to any state)
     */
    if (mr_unlikely(((src == NR_THREAD_RUNNING) && (dst == NR_THREAD_RUNNING))) || 
        mr_unlikely(dst >= NR_THREAD_STATUS_MAX))
        goto fail;

    /*!< for idle thread, only ready and running state can be chosen */
    if ((tid == THREAD_TID_IDLE) && 
        mr_unlikely((dst != NR_THREAD_RUNNING) && (dst != NR_THREAD_READY)))
        goto fail;

    /*!< do not suspend self in interrupt */
    if (mr_unlikely(dst == NR_THREAD_RUNNING) && 
        mr_unlikely(IS_IN_INTERRUPT()))
        goto fail;

    /*!< 先从旧的状态分离 */
    switch (src)
    {
        case NR_THREAD_RUNNING:
            /*!< 对于运行线程, 需要从就绪列表找出一个接替者; 如果没有接替成为运行态的线程, 则分离失败, 禁止切换 */
            retval = schedule_reinstall_work_role();
            if (mr_unlikely(retval))
                goto fail;
            
            break;

        case NR_THREAD_READY:
            schedule_detach_ready_list(sptr_thread);
            break;

        case NR_THREAD_SUSPEND:
            schedule_detach_suspend_list(sptr_thread);
            break;

        case NR_THREAD_SLEEP:
            schedule_detach_sleep_list(sptr_thread);
            break;

        default:
            break;
    }

    /*!< 先分离后加入 */
    mr_barrier();

    /*!< 切换为目标状态 */
    switch (dst)
    {
        case NR_THREAD_RUNNING:
            /*!< 其他线程希望成为运行态, 需要剥夺当前运行线程的CPU */
            retval = schedule_despoil_work_role(sptr_thread);
            break;

        case NR_THREAD_READY:
            retval = schedule_add_ready_list(sptr_thread);
            break;

        case NR_THREAD_SUSPEND:
            retval = schedule_add_suspend_list(sptr_thread);
            break;

        case NR_THREAD_SLEEP:
            retval = schedule_add_sleep_list(sptr_thread);
            break;

        default:
            retval = -ER_ERROR;
            break;
    }

    /*!< 操作失败 */
    if (mr_unlikely(retval < 0))
        goto fail;
    /*!< 运行线程丢失 */
    if (mr_unlikely(!SCHED_RUNNING_THREAD))
        goto fail;

    /*!< 同步状态: sptr_thread->state = dst; sptr_thread->to_state = 0 */
    __SYNC_THREAD_STATE(sptr_thread, dst);
    return ER_NORMAL;
    
fail:
    __SYNC_THREAD_STATE(sptr_thread, src);
    return -ER_INVALID;
}
```

#### 10.7. 线程调度
##### 10.7.1. __schedule_thread
线程调度需要：
1）找出下一个可以运行的线程a，即就绪列表中优先级最高者；
2）将当前线程改为就绪态，即将接替运行的线程a改为运行态，中间涉及列表转换，即调用schedule_thread_switch；
2）保存当前线程的上下文；
3）恢复线程a的上下文；
4）切换到线程a运行（更新pc寄存器）。

内核使用结构体“struct scheduler_context”来记录发生调度时的当前线程和下一个线程：
```c
struct scheduler_context
{
    kuaddr_t entry;                         /*!< 线程入口, 函数指针 */
    kuaddr_t args;                          /*!< entry的参数 */
    kuaddr_t first;                         /*!< 标记新线程是否为首次执行 */

    kuaddr_t next_sp;                       /*!< 新线程的栈 */
    kuaddr_t prev_sp;                       /*!< 当前线程的栈 */
};
```

1）start_kernel函数初始化完毕后，开启首次调度；主动调度（主动让出CPU）可使用schedule_thread函数：
```c
void schedule_thread(void)
{
    struct scheduler_context *sptr_context;

    /*!< 关闭抢占 */
    mr_preempt_disable();

    /*!< 保存cpsr寄存器到spsr寄存器, 即存储当前状态; 并关闭中断, 线程调度在中断关闭状态下执行 */
    __push_psr();
    mr_local_irq_disable();
    mr_preempt_enable();

    /*!< 禁止在中断回调函数中调用schedule_thread, 否则将使中断终止, 引发不可预料的错误 */
    if (mr_unlikely(IS_IN_INTERRUPT()))
    {
        /*!< 立即还原为运行态, 并清除to_state的值 */
        __SYNC_THREAD_STATE(mr_current, NR_THREAD_RUNNING);
        mr_warn(false);

        goto END;
    }
   
    /*!< 核心1: 挑选下一个可运行的线程 */
    sptr_context = __schedule_thread();
    if (!sptr_context)
        goto END;

    /*!< 核心2: 线程上下文切换; 期间spsr寄存器会重新赋值给cpsr, 还原schedule_thread调用前的状态 */
    context_switch(sptr_context);
    return;

END:
    /*!< 出师未捷, 直接还原cpsr寄存器并返回(中断会被重新打开) */
    __pop_psr();
}
```

2）主要核心代码在于__schedule_thread和context_switch，前者负责从就绪列表中挑选一个优先级最高的线程，作为下一个运行线程；后者将保存当前线程的上下文，并恢复接替线程的上下文，从而切换到新线程运行。
__schedule_thread本质是通过调用schedule_thread_switch完成状态迁移，同时初始化sptr_context各个成员，为context_switch做好准备。
```c
struct scheduler_context *__schedule_thread(void)
{
    struct thread *sptr_thread;
    struct thread *sptr_prev;
    struct thread_list *sptr_hash;
    struct scheduler_context *sptr_context;
    kint32_t retval;

    /*!< 暂存就绪hash */
    sptr_hash = SCHED_READY_HASH;

    /*!< 通过判断ffs_l和ffs_h是否为0, 可获知hash是否为空; 为空时, 没有任何线程处于就绪, 无法调度, 直接返回 */
    if (__THREAD_HASH_EMPTY(sptr_hash))
        goto fail;

    /*!< 获取当前运行线程; 若sptr_prev为NULL, 说明可能是第一次调度 */
    sptr_prev = SCHED_RUNNING_THREAD;
    if (mr_unlikely(!sptr_prev))
    {
        /*!< 全局标志, 为1时表示之前已经调度过; 若为1, 则与prev == NULL矛盾 */
        if (mr_unlikely(thread_schedule_ref))
            goto fail;
        else
        {
            /*!< 由start_kernel函数发起的首次调度, 此时没有任何线程处于运行态, 需要从就绪列表中获取 */
            sptr_prev = mr_list_first_entry(SCHED_READY_LIST, struct thread, sgtc_link);           
            __SET_THREAD_TARGET_STATE(sptr_prev, NR_THREAD_RUNNING);
        }
    }
    
    /*!< 可能调用了schedule_thread但没有设置目标态, 默认切换到就绪态 */
    if (NR_THREAD_NONE == sptr_prev->to_state)
        __SET_THREAD_TARGET_STATE(sptr_prev, NR_THREAD_READY);

    /*!< 第一次调度时, 就绪态的线程迁移到运行态; 否则, 当前运行的线程迁移到目标态, 并从就绪列表获取可运行的线程. 该操作有可能失败 */
    retval = schedule_thread_switch(sptr_prev);
    sptr_thread = SCHED_RUNNING_THREAD;
    if (mr_unlikely(retval < 0) || 
        mr_unlikely(!sptr_thread))
        goto fail;
    
    sptr_context = &sgtc_context;

    /*!< 线程上下文准备 */
    /*!< 首次调度标志, 仅在第一次调度时thread_schedule_ref才会为0 */
    sptr_context->first = (kuaddr_t)&thread_schedule_ref;
    /*!< 线程入口, 如果新线程是第一次准备运行, 调度后直接从此入口进入; 否则entry不会被使用 */
    sptr_context->entry = (kuaddr_t)&sptr_thread->start_routine;
    /*!< 提供给entry的参数, 同样只有在线程第一次准备运行时有用 */
    sptr_context->args = (kuaddr_t)&sptr_thread->ptr_args;
    /*!< 当前运行的线程栈 */
    sptr_context->prev_sp = 0;
    /*!< 即将运行的线程栈 */
    sptr_context->next_sp = thread_get_stack(sptr_thread->sptr_attr);

    /*!< 第一次调度时不存在正在运行的线程, 自然也就没有线程栈 */
    if (mr_likely(thread_schedule_ref))
        sptr_context->prev_sp = thread_get_stack(sptr_prev->sptr_attr);

    /*!< 用于记录调度次数 */
    scheduler_record();

    /*!< address of sgtc_context ===> r0 */
    return sptr_context;

fail:
    return mr_nullptr;
}
```

##### 10.7.2. context_switch
__schedule_thread填充好sgtc_context的各个成员后，返回地址给context_switch，而：
```c
void context_switch(struct scheduler_context *sptr_info)
{
    __switch_to((kuaddr_t)sptr_info);
}
```

__switch_to是汇编的入口，sgtc_context的地址将保存到r0寄存器，然后跳转到__switch_to。
另外，每个线程栈在线程创建时都会在栈顶留出一段空间，用于存放“struct context_regs”结构体。这是由函数thread_set_stack完成的。
```c
struct context_regs
{
    kuaddr_t r0;
    kuaddr_t r1;
    kuaddr_t r2;
    kuaddr_t r3;
    kuaddr_t r4;
    kuaddr_t r5;
    kuaddr_t r6;
    kuaddr_t r7;
    kuaddr_t r8;
    kuaddr_t r9;
    kuaddr_t r10;
    kuaddr_t r11;
    kuaddr_t r12;
    kuaddr_t sp;                            /*!< 重要, 保存线程上下文后, 当前sp的值会保存到这里 */
    kuaddr_t lr;
    kuaddr_t pc;
    kuaddr_t psr;

    /*!< 标志位. bit0: 调度原因(0: 通过调用schedule_thread; 1: 通过中断抢占引起) */
    kuaddr_t flags;
};
```

next_sp就是栈顶，故得到了next_sp，自然也就得到了context_regs。

__switch_to的大致流程为：
```Mermaid
graph TD
    A[__switch_to] --> B[保存寄存器状态]
    B --> C{首次调度?}
    C -->|是| D[_context_save_first]
    C -->|否| E[非首次调度处理]
    
    D --> F[设置首次调度标志]
    F --> G[调用 __thread_init_before]
    G --> H[_sched_first_ready]
    
    E --> I[设置调度来源]
    I --> J[_context_save]
    
    J --> K[保存SP到当前线程栈顶的特定位置]
    K --> L[调用 __thread_init_before]
    L --> M[_context_restore]
    
    H --> N[设置新线程栈]
    N --> O[准备线程入口和参数]
    O --> P[_sched_init_before]
    
    M --> Q{新线程是第一次被调度?}
    Q -->|否| R[恢复线程上下文]
    Q -->|是| H
    R --> P
    
    P --> S{调度来源是中断?}
    S -->|是| T[_switch_to_irq]
    S -->|否| U[_switch_to_next]
    U --> V[弹出PC完成切换]
    T --> W[返回中断上下文, 回收中断]
```

调度需覆盖的几种情况：
```
提供返回路径函数: ret_with_first_schedule

情况1: 首次调度, 加锁, 新线程为首次调度, 指定从ret_with_first_schedule返回, 解锁, 然后跳转到新线程入口;
情况2: 旧线程主动发起调度schedule_thread, 加锁, 新线程为首次调度, 指定从ret_with_first_schedule返回, 解锁, 然后跳转到新线程入口;
情况3: 旧线程主动发起调度schedule_thread, 加锁, 新线程此前调度过(也是通过schedule_thread), 恢复新线程上下文, 新线程从它之前运行过的schedule_thread返回, 解锁;
情况4: 旧线程被抢占__schedule_thread_irq, 加锁, 新线程为首次调度, 指定从ret_with_first_schedule返回, 解锁, 然后跳转到新线程入口;
情况5: 旧线程被抢占__schedule_thread_irq, 加锁, 新线程此前调度过(通过schedule_thread主动发起), 恢复新线程上下文, 新线程从它之前运行过的schedule_thread返回, 解锁;
情况6: 旧线程被抢占__schedule_thread_irq, 加锁, 新线程此前调度过(也是被抢占的: __schedule_thread_irq), 恢复新线程上下文, 新线程从它之前运行过的__schedule_thread_irq返回, 解锁, 最后由rfeia回到线程上下文(之前被打断的地方);
情况7: 旧线程主动发起调度schedule_thread, 加锁, 新线程此前调度过(是被抢占的: __schedule_thread_irq), 恢复新线程上下文, 新线程从它之前运行过的__schedule_thread_irq返回, 解锁, 最后由rfeia回到线程上下文(之前被打断的地方);
```

对应代码如下：
```nasm
#include <common/linkage.h>
#include <configs/mach_configs.h>
#include <kernel/asm_text.h>

    .text
    .arm

ENTRY(__switch_to)
__switch_to:
    sub sp, #8
    stmdb sp!, { r0 - r12, lr }                     @ 保存r0 ~ r12、lr (r0 = &sgtc_context, 含旧线程、新线程的信息)

    mrs r12, cpsr
    add r8, sp, #ARCH_OFFSET_LR
    stmia r8, { r12, lr }                           @ 保存lr和cpsr (lr未来会作为pc弹出)

    /* sp从低到高分别存储: r0, r1, ..., r12, lr, cpsr, lr(未来的pc) */

    add r2, r0, #CONTEXT_FIRST_OFFSET               @ 地址: r2 = &sgtc_context.first
    ldr r1, [r2]                                    @ 读值: r1 = *(&sgtc_context.first)
    cmp r1, #0                                      @ 如果first为0, 那就是本核心的整个调度器首次调度
    beq _context_save_first                         @ 首次调度特别处理

    b _context_save

_context_save_first:
    mov r12, #1
    str r12, [r2]                                   @ 将1写给*(&sgtc_context.first), first = true, 永久标记

    b _context_restore

_context_save:
    ldr r2, [r0, #CONTEXT_PREV_SP]                  @ r2 = &sgtc_context.prev_stack_addr, stack_addr的值是当前线程的栈顶地址
    ldr r1, [r2]                                    @ r1 = 当前线程的栈顶地址

    add r8, r1, #ARCH_OFFSET_LR                     @ 栈顶的上方有一块特别区域, 同样按r0 ~ r15的方式定义, 这里偏移到r14的位置
    str sp, [r8, #-4]                               @ 将当前线程的sp保存到栈上方偏移r13的位置, 这样sp就可在下次轻易获取: 系统知道它就在栈上方偏移r13的位置

    b _context_restore                              @ r0 ~ r15已经全都保存完, 当前线程可以终结了; 下面是新线程的恢复

_context_restore:
    ldr r2, [r0, #CONTEXT_NEXT_SP]                  @ r2 = &sgtc_context.next_stack_addr, stack_addr的值是新线程的栈顶地址
    ldr r1, [r2]                                    @ r1 = 新线程的栈顶地址

    ldr sp, [r1, #ARCH_OFFSET_SP]                   @ 我们知道, 栈顶上方有一块预留区域, 其偏移r13的地址就保存着sp的值; 取出更新为当前sp, 此即新线程的栈
    cmp sp, #0                                      @ 这个位置为空值 ? 说明新线程此前还没有被调度 (还没有执行过_context_save), 这是首次要运行
    beq _sched_first                                @ 首次运行的新线程特殊处理, 因为, 它无需恢复上下文   

    ldr r1, =__thread_init_before                   @ 对新线程的时间片进行初始化, 趁现在新线程的上下文还没复原, 寄存器正是随意使用的时期
    blx r1                                          @ 建议sp更新为新线程后再调用__thread_init_before, 这样用的栈就是新线程的

    ldmia sp!, { r0 - r12, lr }                     @ 新线程是之前运行过的, 它有上下文要恢复: 复原r0 ~ r12、lr, 但是cpsr和pc先留着, 还没到结束的时候
    b _sched_init_before                            @ 执行启动前的最后一次准备工作

_sched_init_before:
    push { r12 }                                    @ r12准备用来暂存cpsr
    ldr r12, [sp, #4]                               @ sp现在指向r12, 往上偏4字节, 就是上次存的cpsr
    msr cpsr, r12                                   @ 更新到cpsr, 注意: 这个cpsr也是处于关中断状态的, 不用担心会立即发生中断, 因为进调度器前都会关中断
    pop { r12 }                                     @ sp += 4, 弹出r12
    add sp, #4                                      @ sp += 4, 现在指向lr的位置

    b _switch_to_next                               @ 一切就绪

_switch_to_next:
    pop { pc }                                      @ 返回到schedule_thread或schedule_thread_irq

/*!< ---------------------------------------------------------------------------------------------------- */
/*!< ---------------------------------------------------------------------------------------------------- */
_sched_first:
    ldr r1, [r0, #CONTEXT_NEXT_SP]                  @ r1 = &sgtc_context.next_stack_addr, stack_addr的值是新线程的栈顶地址
    ldr r8, [r1]                                    @ r8 = 新线程的栈顶地址
    mov sp, r8                                      @ 作为首次运行的线程, r8就可以直接当成sp: *((unsigned int *)sgtc_context.next_stack_addr)

    ldr r1, [r0, #CONTEXT_ENTRY_OFFSET]             @ r1 = &sgtc_context.next_entry, next_entry的值是新线程的入口地址
    ldr r8, [r1]                                    @ r8 = 线程入口的地址
    push { r8 }                                     @ 等价于: push { lr }, 这是下次作为pc的值

    ldr r1, [r0, #CONTEXT_ARGS_OFFSET]              @ r1 = &sgtc_context.next_args, args的值是新线程的入口参数
    ldr r8, [r1]                                    @ r8 = 线程入口的参数值, 将作为函数的参数
    push { r8 }                                     @ r8保存着入口参数args, 需要先暂存

    ldr r1, =__thread_init_before                   @ 对新线程的时间片进行初始化, 新线程是首次调度, 寄存器随意使用
    blx r1                                          

    mrs r0, cpsr
    ldr r1, =ret_with_first_schedule                @ 做一些准备工作, 包括自旋锁解锁、cpsr预备
    blx r1
    mov r1, r0                                      @ 返回值: cpsr默认值
    
    pop { r0 }                                      @ 还原args给r0, 作为线程入口参数
    msr cpsr, r1                                    @ 更新cpsr

    pop { pc }                                      @ 弹出线程入口的地址作为pc, 新线程运行

ENDPROC(__switch_to)

/*!< ---------------------------------------------------------------------------------------------------- */
/*!< ---------------------------------------------------------------------------------------------------- */
/*!< 抢占调度, 从中断触发 >
ENTRY(__schedule_from_irq)
__schedule_from_irq:
    stmdb sp!, { r0 - r12, lr }                     @ 先压栈，这里的lr是svc模式的lr，而irq模式的lr已经保存在栈里

    /*  当前栈区状态(从sp开始, 低到高): r0, r1, ..., r12, lr_svc, spsr, lr_irq(未来的pc) */

    ldr r1, =schedule_thread_irq                    @ 跳到C语言, 旧线程加锁, 切换新线程, 然后由新线程解锁
    blx r1                                          @ lr_svc会被更新

    ldmia sp!, { r0 - r12, lr }                     @ 还原之前的栈
    rfeia sp!                                       @ 从这里返回: 将spsr给cpsr, lr_irq赋值给pc

ENDPROC(__schedule_from_irq)
```

##### 10.7.3. 休眠与唤醒
切换线程其实只需两步：一是设置线程的目标态（即设置to_state）；二是调用schedule_thread函数，它会根据目标态完全切换。
比如让线程挂起：
```c
/*!< 挂起自身 */
void schedule_self_suspend(void)
{
    struct thread *sptr_cur = SCHED_RUNNING_THREAD;
    kutype_t flags;

    spin_lock_irqsave(&sptr_cur->sgtc_lock, &flags);
    
    /*!< 防止错误地挂起别人家的线程 */
    if (mr_likely(__GET_THREAD_STATE(sptr_cur) == NR_THREAD_RUNNING))
        __SET_THREAD_TARGET_STATE(sptr_cur, NR_THREAD_SUSPEND);

    spin_unlock_irqrestore(&sptr_cur->sgtc_lock, flags);
    schedule_thread();
}

/*!< 挂起其他线程 */
kint32_t schedule_thread_suspend(tid_t tid)
{
    struct thread *sptr_thread;
    kutype_t flags;
    kint32_t retval;

    /*!< 根据tid获取线程 */
    sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (mr_unlikely(!sptr_thread))
        return -ER_NODEV;

    spin_lock_irqsave(&sptr_thread->sgtc_lock, &flags);

    /*!< 设置线程目标态为挂起态 */
    __SET_THREAD_TARGET_STATE(sptr_thread, NR_THREAD_SUSPEND);

    /*!< 竟然就是自己? */
    if (mr_unlikely(__GET_THREAD_STATE(sptr_thread) == NR_THREAD_RUNNING))
    {
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

        /*!< Self suspend */
        schedule_thread();
        return ER_NORMAL;
    }

    spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

    /*!< 切换状态 */
    spin_lock_irqsave(&__SCHED_LOCK, &flags);
    retval = schedule_thread_switch(sptr_thread);
    spin_unlock_irqrestore(&__SCHED_LOCK, flags);
    
    return retval;
}
```

要让线程变成睡眠态，也形同此法。有挂起、睡眠，自然就有唤醒：
```c
kint32_t schedule_thread_wakeup(tid_t tid)
{
    struct thread *sptr_thread;
    kuint32_t state;
    kutype_t flags;
    kint32_t retval;

    /*!< 根据tid获取线程 */
    sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (mr_unlikely(!sptr_thread))
        return -ER_NODEV;

    spin_lock_irqsave(&sptr_thread->sgtc_lock, &flags);

    /*!< 已经在运行, 无需唤醒 */
    if (sptr_thread == SCHED_RUNNING_THREAD)
    {
        __SYNC_THREAD_STATE(sptr_thread, NR_THREAD_RUNNING);
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

        return -ER_FORBID;
    }

    /*!< 只有处于睡眠态和挂起态, 才有唤醒这个操作 */
    state = __GET_THREAD_STATE(sptr_thread);
    if ((state != NR_THREAD_SUSPEND) &&
        (state != NR_THREAD_SLEEP))
    {
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);
        return -ER_INVALID;
    }

    /*!< 唤醒, 即线程从挂起/睡眠态转变为就绪态 */
    __SET_THREAD_TARGET_STATE(sptr_thread, NR_THREAD_READY);
    spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

    /*!< 切换状态 */
    spin_lock_irqsave(&__SCHED_LOCK, &flags);
    retval = schedule_thread_switch(sptr_thread);
    spin_unlock_irqrestore(&__SCHED_LOCK, flags);

    return retval;
}
```

有这些接口还不够，有时候我们希望挂起线程后，过一段时间它能自动醒来，这里涉及到定时机制，需要定时器的参与。
内核提供了sleep（秒级睡眠）、msleep（毫秒级睡眠）和usleep（微秒级睡眠）三种方式，不过这里的睡眠并不是指切换为睡眠态，而是挂起态。
使用sleep进行睡眠时，当前线程会切换到挂起态，在睡眠时间结束后，通过定时器中断的回调函数，调用schedule_thread_wakeup将线程唤醒，使其变成就绪态。内核封装了schedule_timeout函数，它以jiffies作为单位睡眠：
```c
void schedule_timeout(kutime_t count)
{
    struct ktime_event sgtc_event;
    struct timer_list *sptr_tm = &sgtc_event.u.sgtc_tm;
    struct thread *sptr_cur = mr_current;
    kutype_t flags;

    /*!< count为0, 允许切换到就绪态 */
    if (!count) {
        schedule_thread();
        return;
    }

    sgtc_event.type = KTIME_EVENT_JIFFIES;
    sgtc_event.sptr_cur = sptr_cur;

    /*!< 初始化定时器链表 */
    setup_timer(sptr_tm, thread_sleep_timeout, (kuint32_t)&sgtc_event);

    /*!< 设置当前线程目标态为挂起态 */
    spin_lock_irqsave(&sptr_cur->sgtc_lock, &flags);
    sptr_cur->time_event = (void *)&sgtc_event;
    __SET_THREAD_TARGET_STATE(sptr_cur, NR_THREAD_SUSPEND);
    mr_preempt_disable();
    spin_unlock_irqrestore(&sptr_cur->sgtc_lock, flags);

    /*!< 启动定时器, 定时时长为count (单位: jiffies); 中间需关闭抢占, 否则抢占可能在mod_timer之前发生, 从而使线程提前挂起, 而无法被唤醒 */
    mod_timer(sptr_tm, jiffies + count);
    mr_preempt_enable();

    /*!< 再次判断, 若抢占真的发生过, to_state肯定会因为调度成功而清0 */
    if (mr_likely(__GET_THREAD_TARGET_STATE(sptr_cur) == NR_THREAD_SUSPEND))
        schedule_thread();

    sptr_cur->time_event = mr_nullptr;
    del_timer(sptr_tm);
}
```

thread_sleep_timeout是一个定时唤醒函数，它在定时时间到之后被定时器回调函数调用，用于唤醒线程：
```c
static void thread_sleep_timeout(kuint32_t args)
{
    struct ktime_event *sptr_event = (struct ktime_event *)args;
    struct thread *sptr_thread = sptr_event->sptr_cur;
    kuint32_t status, to_status;
    kutype_t flags;

    if (mr_unlikely(!sptr_thread))
        return;

    spin_lock_irqsave(&sptr_thread->sgtc_lock, &flags);
    status = __GET_THREAD_STATE(sptr_thread);
    to_status = __GET_THREAD_TARGET_STATE(sptr_thread);

    /*!< 确认线程是否在挂起态, 若是, 表明schedule_timeout已经执行完成, 可执行唤醒操作 */
    if (status == NR_THREAD_SUSPEND)
    {
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

        /*!< 唤醒, 可能失败; 若返回-ER_NODEV, 说明线程不需要唤醒 */
        if ((-ER_NODEV) == schedule_thread_wakeup(sptr_thread->tid))
            return;

        spin_lock_irqsave(&sptr_thread->sgtc_lock, &flags);

        /*! 再次获取状态 */
        status = __GET_THREAD_STATE(sptr_thread);
    }
    /*!< 如果本函数在schedule_timeout未执行完就发生, 此时state也没有处于挂起态, 则无需再挂起(因为时间已到) */
    else if (to_status == NR_THREAD_SUSPEND)
    {
        __SET_THREAD_TARGET_STATE(sptr_thread, NR_THREAD_NONE);
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

        return;
    }

    spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

    /*!< state确实曾经在挂起态, 但唤醒失败. 若status还处于挂起/睡眠, 继续定时, 下次再唤醒 */ 
    if ((status != NR_THREAD_READY) &&
        (status != NR_THREAD_RUNNING))
    {
        if (sptr_event->type == KTIME_EVENT_JIFFIES)
            mod_timer(&sptr_event->u.sgtc_tm, jiffies + 1);
        else
            mod_hrtimer(&sptr_event->u.sgtc_hrtm, khrtime_ticks() + MSEC_TO_HRTICK(1));
    }
}
```

sleep的本质就是秒转为jiffies，然后调用schedule_timeout；msleep、usleep则是将毫秒、微秒转为jiffies。

##### 10.7.4. 抢占
内核支持抢占，即，不使用schedule_thread，也不使用sleep，而是强制将当前线程挂起（剥夺线程持有的CPU），并切换到下一个线程。
该操作往往由中断发起，并根据两条标准决定是否抢占：
1）就绪列表是否存在比当前运行线程优先级更高的线程，若有，立即切换到该线程；
2）如果没有更高的优先级，则检查当前线程的时间片，如已耗尽，则检查就绪列表是否有优先级相同的线程，若有，立即切换到该线程。

内核中定义了一个抢占计数器，当它为0时，允许抢占；非0时，表示禁止抢占。
有一组接口可以使用它：
```c
/*!< 定义为原子变量 */
struct atomic sgtc_sched_preempt_cnt;

#define mr_preempt_cnt_dec()                        atomic_dec(&sgtc_sched_preempt_cnt)
#define mr_preempt_cnt_inc()                        atomic_inc(&sgtc_sched_preempt_cnt)
#define mr_preempt_cnt()                            ATOMIC_READ(&sgtc_sched_preempt_cnt)
#define mr_preempt_is_locked()                      (!!mr_preempt_cnt())

/*!< 使能抢占: 减计数, 减为0时才真正开启抢占 */
#define mr_preempt_enable() \
    do {    \
        mr_barrier();   \
        mr_preempt_cnt_dec();   \
    } while (0)

/*!< 失能抢占: 加计数, 大于0时抢占被禁止 */
#define mr_preempt_disable()    \
    do {    \
        mr_preempt_cnt_inc();   \
        mr_barrier();   \
    } while (0)
```

时间片是一个线程可以运行的时间，在没有更高优先级就绪线程的情况下，当前线程至少可以运行一个时间片的时间（一个时间片可能是40ms，或者其他时间），如果没有更高优先级、也没有相同的优先级线程处于就绪，则当前线程可以一直执行下去。
内核系统节拍可能是1000Hz，则每1ms检查一次时间片，如果时间片是40ms，则理论上可以检查40次，才能确定时间片耗尽。则：检查时间片、以及是否可以抢占的代码就是一个定时器回调函数。
```c
static void kthread_schedule_timeout(kuint32_t args)
{
    struct timer_list *sptr_tim = (struct timer_list *)args;
    struct thread *sptr_work, *sptr_ready;
    kuint32_t work_prio, next_prio;

    /*!< 获取当前线程 */
    sptr_work = mr_current;

    /*!< 自旋锁, 内部有抢占计数器加1操作 */
    spin_lock(&sptr_work->sgtc_lock);
    
    /*!< --------------------------------------------------------- */
    /*!< 时间片减1 (expires的单位是jiffies) */
    if (sptr_work->expires)
        sptr_work->expires--;

    /*!< 算上spin_lock那一次的抢占计数器+1, 如果计数器大于1, 说明之前还有别的禁抢占代码 */
    if (mr_preempt_cnt() > 1)
        goto END;
    
    /*!< --------------------------------------------------------- */
    /*!< 执行到此, 说明抢占允许. 获取就绪列表中的第一个线程(优先级最高) */
    sptr_ready = get_first_ready_thread();

    /*!< 就绪列表为空, 无法抢占 */
    if (!sptr_ready)
        goto END;

    /*!< 获取两个线程的优先级 */
    work_prio = thread_get_priority(sptr_work->sptr_attr);
    next_prio = thread_get_priority(sptr_ready->sptr_attr);
   
/*!< 激进的抢占方式, 默认开启 */
#if CONFIG_PREEMPT
    /*!< "就绪线程优先级更高", 或者"时间片已耗尽, 且优先级相等" */
    if (__THREAD_IS_LOW_PRIO(work_prio, next_prio) ||
        (!sptr_work->expires && (work_prio == next_prio)))
        g_sched_flag = true;

/*!< 低调的抢占方式 */
#else
    /*!< 禁止优先级抢占, 只有时间片耗尽时, 才允许更高或相同优先级的就绪线程抢占CPU */
    if (!sptr_work->expires &&
        (__THREAD_IS_LOW_PRIO(work_prio, next_prio) ||
        (work_prio == next_prio)))
        g_sched_flag = true;
#endif
    
END:
    spin_unlock(&sptr_work->sgtc_lock);

    /*!< 每一个jiffies检查一次 */
    mod_timer(sptr_tim, jiffies + 1);
}
```

kthread_schedule_timeout对应的定时器链表由内核线程kthread定义和激活, 它并未直接执行线程切换，而是将抢占标志保存到全局变量g_sched_flag。在中断回调函数退出后，会再次检查是否允许抢占，若是，则将全局变量g_sched_flag传递给另一个全局变量g_asm_sched_flag；当返回到irq_handler时，将根据g_asm_sched_flag的值决定是否发起线程切换：
```nasm
.global g_asm_sched_flag
g_asm_sched_flag:
    .word 0x00000000

_irq_handler:
    sub lr, lr, #0x04
    _exception_save_params                          @ 保存中断上下文

    bl exec_irq_handler                             @ 执行中断回调函数, 并执行"g_asm_sched_flag = g_sched_flag"
    _exception_restore_params                       @ 恢复中断上下文

    push { r0 }
    ldr r0, g_asm_sched_flag
    cmp r0, #0                                      @ 检查g_asm_sched_flag是否为0
    bne 1f                                          @ 不为0, 需要进行线程切换

    pop { r0 }                                      @ 为0, 中断结束, 返回线程上下文
    movs pc, lr

1:                                                  @ 处理抢占
    mov r0, #0
    str r0, g_asm_sched_flag                        @ g_asm_sched_flag清0
    pop { r0 }

    srsdb sp!, #ARCH_SVC_MODE                       @ 保存lr_irq和spsr到sp_svc
    cpsid i, #ARCH_SVC_MODE                         @ 切换到svc模式的同时关闭中断 (spsr已经保存, 此时关闭中断不会影响上下文)
    
    b __schedule_before                             @ 跳转到"context.S"
```

context.S是我们的老朋友，之前仅介绍了__switch_to，其实它还有另一部分代码（即调度来源为：SCHED_FROM_IRQ），包括了之前出现过的“_switch_to_irq”：
```nasm
ENTRY(__schedule_before)
__schedule_before:
    stmdb sp!, { r0 - r12, lr }                     @ 保存r0 ~ r12和当前线程上下文的lr(中断发生时, 线程上下文的lr, 并非中断上下文的lr)

    ldr r1, =__schedule_thread
    blx r1                                          @ 核心依然是执行__schedule_thread, 并返回r0 = &sgtc_context

    cmp r0, #0                                      @ __schedule_thread返回NULL, 无法切换新线程
    beq _switch_fail                                @ 应原路返回

    add r8, sp, #ARCH_OFFSET_PC                     @ 取出spsr (在irq_handler中使用srsdb保存的)
    ldr r12, [r8]                                   @ r12 = spsr_irq                    
    cmp r12, #0
    orreq r12, #ARCH_SVC_MODE
    str r12, [r8]                                   @ 如果spsr为0, 首先设置为svc模式

    str r0, g_asm_context_info                      @ 暂存&sgtc_context到全局变量, 以便空出r0

1:
    ldr r2, [r0, #CONTEXT_PREV_SP]                  @ r1 = prev_sp
    ldr r1, [r2]                                    @ r1 = *prev_sp, 取得当前线程的栈顶

    add r2, r1, #ARCH_FRAME_SIZE                    @ 偏移到struct context_regs::flags的位置
    ldr r3, [r2]                                    @ r2 = &flags, r3 = flags
    orr r3, #SCHED_FROM_IRQ                         @ 标记调度来源: 中断

2:
    b _context_save                                 @ 跳转到_context_save, 之后的操作与__switch_to相同

/* -------------------------------------------------------------------------------
 * 调度失败
 * -----------------------------------------------------------------------------*/
_switch_fail:
    stmia sp!, { r0 - r12, lr }                     @ 恢复当前线程的上下文, 原路返回

/* -------------------------------------------------------------------------------
 * schedule new thread (by IRQ)
 * -----------------------------------------------------------------------------*/
_switch_to_irq:
    rfeia sp!                                       @ 如果调度来源是中断, 则返回到中断上下文, 结束之前的中断, 经中断上下文回到线程上下文

ENDPROC(__schedule_before)
```

#### 10.8. 同步与互斥
##### 10.8.1. 关中断
多线程运行时，会存在多个线程同时读写一个全局变量的情况，比如线程a将全局变量xxx置为1，而线程b将它值为2，最后线程a再读取的时候就是2而不是1；如果这是一个全局的指针变量，线程a将指针赋值为某个全局变量的地址，而线程b将指针置为NULL，那么线程a再对指针操作的时候，就会触发异常；如果这是一个全局的链表，线程a正在加入新链表项，还没加完就被线程b抢占，而它也要加入新链表项，则链表可能因为错乱的指针导致链表项前后关系错误。
这里的全局变量或全局函数，被称为临界资源。临界资源必须使用互斥，以防止多个线程“同时”操作。
一种暴力的方式是关中断，中断关闭时kthread_schedule_timeout无法被执行，自然就不能发生抢占。
关中断有几种方式：
```c
/*!< 暴力开启中断; 如果关闭中断前就是关中断的状态, 这里将导致中断错误打开 */
#define mr_local_irq_enable()                               mr_enable_cpu_irq()
/*!< 暴力关闭中断 */
#define mr_local_irq_disable()                              mr_disable_cpu_irq()

/*!< 关中断前先保存中断状态 */
#define mr_local_irq_save(flags)   \
    do {    \
        flags = __get_cpsr();   \
        mr_disable_cpu_irq();   \
    } while (0)

/*!< 如果关中断前是开中断的状态, 才允许重新打开中断 */
#define mr_local_irq_restore(flags)   \
    do {    \
        if (!(flags & CPSR_BIT_I)) \
            mr_enable_cpu_irq();    \
    } while (0)

/*!< 同mr_local_irq_enable */
void local_irq_enable(void);
/*!< 同mr_local_irq_disable */
void local_irq_disable(void);
/*!< 同mr_local_irq_save */
void local_irq_save(kutype_t *flags);
/*!< 同mr_local_irq_restore */
void local_irq_restore(kutype_t *flags);
```

在多核CPU且SMP模式中，关中断仅对某个核生效，无法禁止其他核读写全局变量，故关中断只能用于单核CPU。

##### 10.8.2. 自旋锁
为解决多核SMP的问题，引入自旋锁。使用全局计数器（各个核可见），当计数器为0时，允许访问临界资源；当计数器大于0时，表示已有其他线程正在访问，当前线程将原地等待计数器变为0（自旋）。等待期间，线程将空耗CPU，直到更高优先级或时间片耗尽，才有可能让出CPU，待其他线程访问临界资源结束，计数器减为0。
当前核也需要处理线程和中断的关系，因为中断是无条件打断线程，二者对临界资源同样存在竞争关系；此时线程就可以采用关中断的方式。

自旋锁的定义为：
```c
typedef struct spin_lock
{
    struct atomic sgtc_atc;                             /*!< 计数器使用原子变量 */

} srt_spin_lock_t;

#define DECLARE_SPIN_LOCK(lock) \
    struct spin_lock lock = { .sgtc_atc = ATOMIC_INIT() }

#define SPIN_LOCK_INIT()    \
    { .sgtc_atc = ATOMIC_INIT() }
```

使用如下API就可以调用它：
```c
/* 检查是否已上锁(计数器不为0) */
kbool_t spin_is_locked(struct spin_lock *sptr_lock);
/* 初始化自旋锁 */
void spin_lock_init(struct spin_lock *sptr_lock);
/* 加锁, 如已经上锁, 将原地等待 */
void spin_lock(struct spin_lock *sptr_lock);
/* 解锁 */
void spin_unlock(struct spin_lock *sptr_lock);
/* 尝试加锁, 如已经上锁, 返回错误码 */
kint32_t spin_try_lock(struct spin_lock *sptr_lock);
/* 加锁, 成功后暴力关闭中断(慎用) */
void spin_lock_irq(struct spin_lock *sptr_lock);
/* 尝试加锁, 成功后暴力关闭中断(慎用) */
kint32_t spin_try_lock_irq(struct spin_lock *sptr_lock);
/* 解锁, 并暴力开启中断(慎用) */
void spin_unlock_irq(struct spin_lock *sptr_lock);
/* 加锁, 成功后先获取当前中断状态, 然后再关闭中断 */
void spin_lock_irqsave(struct spin_lock *sptr_lock, kutype_t *flags);
/* 尝试加锁, 成功后先获取当前中断状态, 然后再关闭中断 */
kint32_t spin_try_lock_irqsave(struct spin_lock *sptr_lock, kutype_t *flags);
/* 解锁, 并恢复加锁前的中断状态 */
void spin_unlock_irqrestore(struct spin_lock *sptr_lock, kutype_t flags);
/* 软中断使用. 先禁用软中断, 再加锁(不关闭中断) */
void spin_lock_bh(struct spin_lock *sptr_lock);
/* 软中断使用. 先解锁, 再恢复软中断 */
void spin_unlock_bh(struct spin_lock *sptr_lock);
```

注：不论使用哪一种API进行加锁，加锁时都会同时使抢占计数器自增，即抢占被关闭；解锁时抢占计数器自减。

##### 10.8.3. 互斥锁
自旋锁要么关中断，要么就是原地等待，很容易就拖慢内核的运行速度。互斥锁不需要等待，当加锁失败时，直接调用schedule_thread切换到其他线程，直到锁可用时，才为当前线程加锁。
加锁和解锁期间，临界资源不需要强制在关中断状态下运行，抢占也能处于开启状态。只有一点，如果在中断中加互斥锁，schedule_thread会检测到当前处于中断回调函数，将原路返回，不会发生切换（在中断回调函数中切换线程是危险操作，中断不应该随意跳出）；但中断的优先级最高，如果中断获取不到锁，又无法让持有锁的线程释放，则内核将陷入死锁，只能一直空耗在加锁失败和schedule_thread失败之间。故，中断禁止使用互斥锁。

互斥锁是比较简单的锁，它的定义和相关API如下：
```c
typedef struct mutex_lock
{
    struct atomic sgtc_atc;                 /* 和自旋锁一样, 互斥锁也是一个计数器, 为0时锁空闲, 大于0时锁被持有 */
    struct lock_owner sgtc_owner;           /* 标记锁的持有者, 以及被本锁阻塞的线程链表(pending list) */

} srt_mutex_lock_t;

/* 判断是否已经上锁 */
kbool_t mutex_is_locked(struct mutex_lock *sptr_lock);
/* 初始化互斥锁 */
void mutex_init(struct mutex_lock *sptr_lock);
/* 加锁; 如加锁失败, 将切换线程 */
void mutex_lock(struct mutex_lock *sptr_lock);
/* 尝试加锁; 如加锁失败, 立即返回错误码; 如中断希望使用互斥锁, 可以用此函数 */
kint32_t mutex_try_lock(struct mutex_lock *sptr_lock);
/* 解锁 */
void mutex_unlock(struct mutex_lock *sptr_lock);
```

##### 10.8.4. 读写锁
读写锁也是互斥锁。有时候写操作应该互斥，但读操作却可以同时，就可以借助读写锁。它的规则为：
1）一个线程在读临界资源时，另一个线程也可以读，但不可以写。
2）一个线程在写临界资源时，另一个线程不可以读，也不可以写。

其定义和API如下：
```c
typedef struct rw_lock
{
    struct atomic sgtc_read;            /* 读锁计数器 */
    struct atomic sgtc_write;           /* 写锁计数器 */

} srt_rw_lock_t;

#define RW_LOCK_INIT()          { .sgtc_read = ATOMIC_INIT(), .sgtc_write = ATOMIC_INIT() }

/* 检查读锁是否已上锁 */
kbool_t rd_is_locked(struct rw_lock *sptr_lock);
/* 加读锁; 若写锁被占, 则加读锁失败, 切换其他线程运行 */
void rd_lock(struct rw_lock *sptr_lock);
/* 尝试加读锁; 若写锁被占, 返回错误码 */
kint32_t rd_try_lock(struct rw_lock *sptr_lock);
/* 解读锁 */
void rd_unlock(struct rw_lock *sptr_lock);

/*! 检查写锁是否已上锁 */
kbool_t wr_is_locked(struct rw_lock *sptr_lock);
/* 加写锁; 若写锁或读锁任一被占, 则加写锁失败, 切换其他线程运行 */
void wr_lock(struct rw_lock *sptr_lock);
/* 尝试加写锁; 若写锁或读锁任一被占, 返回错误码 */
kint32_t wr_try_lock(struct rw_lock *sptr_lock);
/* 解写锁 */
void wr_unlock(struct rw_lock *sptr_lock);
```

##### 10.8.5. 信号量
信号量是一个扩展版的互斥锁，它允许多次加锁，当加锁次数达到指定的上限时，再加锁则失败，切换其他线程运行。
结构也比较简单，如下：
```c
typedef struct semaphore
{
    struct atomic sgtc_atc;                 /* 不同于互斥锁, 信号量是减计数; 即计数器初值非0, 加锁时自减; 当计数器为0时, 则锁分配完毕 */

} srt_semaphore_t;

#define SEMAPHORE_INITIALIZE()  \
{   \
    .sgtc_atc = ATOMIC_INIT()   \
}

/* 判断计数器是否为0, 为0表示信号量已上锁 */
kbool_t sema_is_locked(struct semaphore *sptr_sem);
/* 给计数器赋初值 */
void sema_init(struct semaphore *sptr_sem, kuint32_t val);
/* 申请信号量, 若计数器为0, 申请失败, 切换其他线程运行; 否则申请成功, 计数器自减 */
void sema_down(struct semaphore *sptr_sem);
/* 尝试申请信号量, 若计数器为0, 申请失败, 返回错误码 */
kint32_t sema_down_try_lock(struct semaphore *sptr_sem);
/* 释放信号量. 计数器自增 */
void sema_up(struct semaphore *sptr_sem);
```

#### 10.9. 线程间通信：邮箱
Heavenfox的线程可以通过全局变量直接传递消息，也可以通过邮箱的方式。所谓邮箱，由发送方申请一个缓冲区存放“邮件”，邮件包含：发送方的邮箱名称、接收方的邮箱名称、邮件内容（消息数据）。一封邮件可以包含多条消息。接收方需要检查自己的邮箱，非空时读出邮件，并释放掉已读的邮件（发送方申请了内存作为缓冲区，需要由接收方释放）。
对于邮箱而言，邮件本质上就是链表，发送方需要根据接收方的邮箱地址找到目的邮箱，并将邮件插入到目的邮箱的邮件链表中；而接收方所谓的读取邮件，就是读取自身邮箱的链表。
每一个线程最多只能有一个邮箱，且邮箱名称唯一、不能重复。也可以没有邮箱，视具体应用自由定义。
之前在“线程控制块”章节中简述了“struct thread”结构体，它有一个指针成员就是邮箱；实际上，邮箱是由各线程自行定义，并赋值给thread结构体。
邮箱结构为：
```c
struct mailbox
{
    kchar_t name[32];                               /* 邮箱名称, 不允许有同名的邮箱 */
    tid_t tid;                                      /* 邮箱所属的线程 */

    kuint32_t num_mails;                            /* 当前邮箱存有的接收邮件数量 */
    struct list_head sgtc_mail;                     /* 收到的邮件链表 */

    struct list_head sgtc_link;                     /* 每个线程的邮箱都要统一管理, 通过sgtc_link链接到全局 */
    struct mutex_lock sgtc_lock;                    /* 内置互斥锁; 不允许在中断中使用邮箱 */
};

static DECLARE_LIST_HEAD(sgtc_kernel_mailboxs);     /* 全局链表头, 管理所有邮箱 */

/* 根据邮箱名称找到对应的邮箱(遍历全局链表头) */
struct mailbox *mailbox_find(const kchar_t *name);
/* 插入新邮箱到全局链表头 */
void mailbox_insert(struct mailbox *sptr_mb);

/* 初始化邮箱 */
kint32_t mailbox_init(struct mailbox *sptr_mb, tid_t tid, const kchar_t *name);
/* 注销邮箱 (从全局链表头删除) */
void mailbox_deinit(struct mailbox *sptr_mb);
/* 创建邮箱. name为邮箱的名字 */
struct mailbox *mailbox_create(tid_t tid, const kchar_t *name);
/* 销毁邮箱. 调用mailbox_deinit注销, 并释放现存的所有邮件 */
void mailbox_destroy(struct mailbox *sptr_mb);
```

再来看邮件：
```c
enum __ERT_KERNEL_MAIL_TYPE
{
    NR_MAIL_TYPE_SYN = 0,
    NR_MAIL_TYPE_KEY,
    NR_MAIL_TYPE_ABS,
    NR_MAIL_TYPE_REL,
    NR_MAIL_TYPE_SERIAL,                        /* 邮件来源: 串口 */
};

/* 一条消息 */
struct mail_msg
{
    kuint32_t type;                             /* 消息的类型, 如按键/坐标, 取自"__ERT_KERNEL_MAIL_TYPE" */
    kuint32_t code;                             /* 消息的子类型, 如按键1/按键2/.../x坐标/y坐标 */

    kuint8_t *buffer;                           /* 消息的内容 */
    kuint32_t size;                             /* 消息的大小(单位: 字节) */
};

struct mail
{
    const kchar_t *src_name;                    /* 邮件发送方(邮箱名字) */

    struct mail_msg *sptr_msg;                  /* 消息 */
    kuint32_t num_msgs;                         /* 消息的数量, 支持多条消息 */

    kuint32_t status;                           /* 邮件状态 */
    struct list_head sgtc_link;                 /* 邮件需插入到接收方的邮件链表中 */

    struct mutex_lock sgtc_lock;                /* 内置互斥锁 */
};
```

线程一般不需要创建邮件，可以将其定义在栈中；因为发送邮件时会从内存池申请缓冲区，并将邮件拷贝，再用缓冲区去发送。
```c
/* 邮件初始化, 关联发送方邮件 */
void mail_init(struct mailbox *sptr_mb, struct mail *sptr_mail);
/* 创建一封邮件, 并调用mail_init */
struct mail *mail_create(struct mailbox *sptr_mb);
/* 销毁邮件, sptr_mail必须为发送方的邮件(比如通过mail_create创建, 然后mail_send发送, 之后就可以使用mail_destroy销毁) */
void mail_destroy(struct mailbox *sptr_mb, struct mail *sptr_mail);
/* 发送邮件. sptr_mail会被复制一份, 并将复制品发送. mb_name为接收方的邮箱名 */
kint32_t mail_send(const kchar_t *mb_name, struct mail *sptr_mail);
/* 接收邮件. 读接收邮件链表 */
struct mail *mail_recv(struct mailbox *sptr_mb, kutime_t timeout);
/* 读邮件完成, 可调用此函数释放掉已读的邮件 */
void mail_recv_finish(struct mail *sptr_mail);
```

#### 10.10. 等待队列
当资源不可用（如数据不可读、数据未准备）时，可能需要先等资源可用才能继续执行，就可以先切换其他线程运行。等待队列其实就是封装了这一过程：判断资源是否可用，如不可用，切换线程。
当资源一直不可用时，可能会一直切换线程，即当前线程持续“睡眠”；有两种方式可以中止等待：1）资源可用，即等待条件满足；2）被信号打断，可用给线程发信，如唤醒信号。
等待队列被封装为一个宏：
```c
/* 方式1: condition为false时, 持续调用schedule_thread() */
#define wait_event(sptr_wqh, condition) \
    do {    \
        if (condition)  \
            break;  \
        (void)__wait_event(sptr_wqh, condition, 0, schedule_thread());    \
    } while (0)

/* 方式2: condition为false时, 每隔一段时间(timeout)检查一次condition */
#define wait_event_timeout(sptr_wqh, condition, timeout) \
    do {    \
        if (condition)  \
            break;  \
        (void)__wait_event(sptr_wqh, condition, 0, schedule_timeout(timeout));    \
    } while (0)

/* 方式3: 在方式1的基础上, 允许给线程发信号中止等待 */
#define wait_event_interruptible(sptr_wqh, condition) \
    do {    \
        if (condition)  \
            break;  \
        (void)__wait_event(sptr_wqh, condition, 1, schedule_thread());    \
    } while (0)

/* 方式4: 在方式2的基础上, 允许给线程发信号中止等待 */
#define wait_event_interruptible_timeout(sptr_wqh, condition, timeout) \
    do {    \
        if (condition)  \
            break;  \
        (void)__wait_event(sptr_wqh, condition, 1, schedule_timeout(timeout));    \
    } while (0)
```

可见核心是__wait_event。在介绍它之前，先来看等待队列的数据结构：
```c
/* 等待队列头 */
typedef struct wait_queue_head
{
    struct spin_lock sgtc_lock;
    struct list_head sgtc_task;                     /* 每个等待队列都链接到链表头 */

} srt_wait_queue_head_t;

/* 初始化链表头 */
#define __WAITQUEUE_HEAD_INITIALIZER(name)  \
{   \
    .sgtc_lock = SPIN_LOCK_INIT(),  \
    .sgtc_task = LIST_HEAD_INIT(&(name)->sgtc_task) \
}

#define DECALRE_WAITQUEUE_HEAD(name)    \
    srt_wait_queue_head_t name = __WAITQUEUE_HEAD_INITIALIZER(name)

/* 等待队列项 */
typedef struct wait_queue
{
    struct thread *sptr_task;                       /* 当前线程 */
    struct list_head sgtc_link;                     /* 链表项, 需插入到链表头struct wait_queue_head::sgtc_task */

} srt_wait_queue_t;

#define __WAITQUEUE_INITIALIZER(tsk)  \
{    \
    .sptr_task = tsk,  \
    .sgtc_link = { mr_nullptr, mr_nullptr }    \
}

#define DECLARE_WAITQUEUE(name, tsk)    \
    srt_wait_queue_t name = __WAITQUEUE_INITIALIZER(tsk)

/* 初始化等待队列头. sptr_wqh一般是全局变量 */
void init_waitqueue_head(struct wait_queue_head *sptr_wqh);
/* 等待队列项sptr_wq一般是局部变量, 需要插入到队列头的sgtc_task成员尾部 */
void add_wait_queue(struct wait_queue_head *sptr_wqh, struct wait_queue *sptr_wq);
/* 移除等待队列项 */
void remove_wait_queue(struct wait_queue_head *sptr_wqh, struct wait_queue *sptr_wq);
/* 给每个等待队列所在的线程发唤醒信号 */
void wake_up_common(struct wait_queue_head *sptr_wqh, kuint32_t state);
```

__wait_event则负责定义一个等待队列项（局部变量），并插入到等待队列头的尾部，并持续检查condition是否为true，是则结束，否则切换线程；如果允许信号唤醒，当线程接收到唤醒信号时，等待队列也结束。
它也是一个宏：
```c
#define __wait_is_interruptible(sptr_wq)                (mr_thread_is_flags(NR_THREAD_SIG_INTR, (sptr_wq)->sptr_task))

#define __wait_event(sptr_wqh, condition, sig_enable, func) \
({  \
    DECLARE_WAITQUEUE(sgtc_wq, mr_current);  \
    const kuint32_t state = (sig_enable) ? NR_THREAD_SIG_INTR : NR_THREAD_SIG_NORMAL;  \
    \
    if (!sgtc_wq.sptr_task)   \
        while (!(condition)); \
    else {  \
        init_list_head(&sgtc_wq.sgtc_link); \
        add_wait_queue(sptr_wqh, &sgtc_wq); \
        thread_state_signal(sgtc_wq.sptr_task, state, true);   \
        \
        for (;;) {   \
            \
            /*!< 条件满足, 退出循环 */  \
            if (condition)  \
                break;  \
            \
            /*!< 信号功能启用且收到唤醒信号, 退出循环 */   \
            if (__wait_is_interruptible(&sgtc_wq) && thread_state_pending(sgtc_wq.sptr_task)) \
                break;  \
            \
            /*!< 执行func, 如schedule_thread(), schedule_timeout */    \
            func; \
            \
        }   \
        \
        thread_state_signal(sgtc_wq.sptr_task, state, false);   \
        remove_wait_queue(sptr_wqh, &sgtc_wq);  \
    }   \
})
```

这里的NR_THREAD_SIG_INTR表示线程允许接收中断信号，它操作的是“struct thread”结构体的flags成员。只有允许接收中断信号，才能被信号打断。
唤醒则正是依赖这条规则：
```c
/* 当wake_up_common的state成员为NR_THREAD_SIG_NORMAL时, 函数直接返回, 无效果 */
#define wake_up(sptr_wqh)   \
    do {    \
        wake_up_common(sptr_wqh, NR_THREAD_SIG_NORMAL);    \
    } while (0)

/* 当wake_up_common的state成员为NR_THREAD_SIG_INTR时, 置thread结构体的flags成员NR_THREAD_SIG_WAKEUP位, 表示唤醒 */
#define wake_up_interruptible(sptr_wqh)   \
    do {    \
        wake_up_common(sptr_wqh, NR_THREAD_SIG_INTR);    \
    } while (0)
```

#### 10.11. 优先级继承
线程在运行时可能会发生优先级反转：低优先级线程持有锁，且被阻塞，导致高优先级线程无法获得锁，最终高优先级线程也阻塞，只能等待低优先级线程解决阻塞后释放锁。在此情况下，高优先级线程的“优先级被迫拉低”，甚至不如低优先级线程，即“优先级反转”。
HeavenFox内核针对此现象，引入优先级继承机制，即：高优先级线程无法获得锁时，将调整持有锁的线程优先级，使之与高优先级线程的优先级相同；持有锁的低优先级线程“继承”了高优先级线程的优先级，使其暂时可以优先执行。优先级继承由结构体lock_owner、lock_owners、lock_waiter共同完成，我们在“互斥锁”一章中，互斥锁结构体“mutex_lock”的成员之一，即结构体lock_owner；而另外两个成员，则位于线程控制块“struct thread”结构体中。
```c
struct thread
{
    /*!< 线程名字: 每个线程都有独一无二的名字 */
    kchar_t name[32];

    /*!< 线程id: 每个线程都有独一无二的id */
    kuint32_t tid;

    /*!< state为当前状态(运行, 就绪, 挂起, 睡眠); to_state为目标状态, 当它非0时, 表示即将进行状态迁移 */
    kuint32_t state;
    kuint32_t to_state;

    /*!< 省略部分内容 ... */

    /*!< 表示哪些锁被本线程持有, 一个线程可能持有多个不同的锁 */
    struct lock_owners sgtc_owners;     
    /*!< 线程正在等待哪个锁, 一个线程只能等待一把锁; 当线程被一把锁阻塞时, 是不可能再去请求另一把锁的 */
    struct lock_waiter sgtc_wait; 

    /*!< 私有参数, 一般保存sleep时的定时器事件指针 */
    void *time_event;
};

typedef struct mutex_lock
{
    struct atomic sgtc_atc;                 /* 和自旋锁一样, 互斥锁也是一个计数器, 为0时锁空闲, 大于0时锁被持有 */
    struct lock_owner sgtc_owner;           /* 标记锁的持有者, 以及被本锁阻塞的线程链表(pending list) */

} srt_mutex_lock_t;
```

它们的定义为：
```c
/*!< 锁的持有者; 一把锁只能被一个线程持有 */
struct lock_owner 
{
    struct thread *sptr_self;
    /*!<  关联到线程; 1个线程可以持有多个锁, 使用本成员进行连接, 链表头位于: struct lock_owners::sgtc_gets */
    struct list_head sgtc_link;         

    /*!< 保护sgtc_pendings */
    struct spin_lock sgtc_lock;         
    /*!< 1把锁只能被1个线程持有, 其他请求本锁的线程视为等待者, 形成pending链表; 这里是链表头 */
    struct list_head sgtc_pendings;     
};

/*!< 线程持有锁的信息 */
struct lock_owners 
{
    /*!< 保护sgtc_gets */
    struct spin_lock sgtc_lock;         
    /*!< 1个线程可以持有多个锁, 形成gets链表; 这里是链表头 */
    struct list_head sgtc_gets;         
};

/*!< 锁请求和等待; 1把锁可以被多个线程请求, 每个求而不得的线程都视为1个锁等待者 */
struct lock_waiter 
{
    /*!< 正在等待的锁 (线程正在被阻塞的地方) */
    struct lock_owner *sptr_wait;       
    /*!< 关联到锁的pending链表 (struct lock_owner::sgtc_pendings) */
    struct list_head sgtc_link;         
};
```

内核的加锁流程为：
```Mermaid
graph TD
    A[线程请求锁：mutex_lock] --> B[锁空闲]
    B --> |是| C
    B --> |否| D
    C[锁计数器自增，并将锁添加到线程的持有锁链表：sgtc_gets] --> E[加锁成功]
    D[将本线程按优先级高低添加到锁的等待链表：sgtc_pendings] --> F[从等待链表中取出第一个等待者，即优先级最高者]
    F --> G[持有锁的线程优先级比等待者低]
    G --> |是| I
    G --> |否| H
    H[锁持有者优先级更高，无需调整，直接返回]
    I[锁持有者优先级更低，需变更持有者线程的优先级，与优先级最高的等待者一致] --> J[重新调整持有者线程在调度链表中的位置，等待下一次调度]
    H --> K[将本线程（等待者）挂起，等待锁释放后唤醒]
    J --> K
```

解锁时需要将等待者唤醒，重新竞争锁。流程为：
```Mermaid
graph TD
    A[锁持有者释放锁：mutex_unlock] --> B[锁空闲，或请求解锁的线程（本线程）并非锁的持有者]
    B --> |是| C
    B --> |否| D
    C[无需解锁，直接返回]
    D[锁计数器自减，并将锁从本线程的持有锁链表（sgtc_gets）移除] --> E[重新计算本线程的优先级，需考虑本线程是否还持有其他锁，仍然要根据其他锁的等待者链表取最高优先级]
    E[设置本线程的当前优先级，但无需调整调度链表，因为本线程正在运行] --> F[将本锁的等待者链表（sgtc_pendings）中的第一个线程唤醒（优先级最高者）]
    F --> G[结束]
```

锁竞争（添加等待者链表、计算最高优先级）和锁释放（脱离等待者链表、计算解锁后的最高优先级），由以下几个重要函数完成，互斥锁可直接调用，实现优先级继承功能（可选）。
```c
/* 将pending链表上的等待者唤醒, 同时从pending链表删除; wake_all可选: true(唤醒全部线程), false(只唤醒第一个, 即优先级最高者) */
void unlock_pending_wakeup(struct lock_owner *sptr_owner, kbool_t wake_all);
/* 从线程所有锁的pending链表中选出优先级最高者, 并返回其优先级值 */
kuint32_t lock_find_max_priority(struct lock_owners *sptr_owners);
/* 初始化锁 */
void lock_context_init(struct lock_owner *sptr_owner);
/* 本线程获得锁, 将锁添加到线程的持有锁链表(sgtc_gets), 并视本线程为锁的持有者 */
void lock_context_save(struct lock_owner *sptr_owner);
/* lock_context_save的反操作, 将锁从sgtc_gets链表中剔除 */
void unlock_context_restore(struct lock_owner *sptr_owner);

/* 锁竞争; 只由求而不得的等待者可以调用, 用于: 将等待者添加到锁的pending链表, 并选出最高优先级, 设置锁持有者线程的优先级, 调整持有者在调度链表中的顺序(按优先级排序) */
/* sgtc_atc: 用于内部二次判别, 只有锁计数器确实非空闲时, 才需要竞争; inherit_enable: true (允许优先级继承), false (禁用优先级继承, 该选项不会计算最高优先级和调整调度链表) */
kint32_t lock_compete(struct lock_owner *sptr_owner, struct atomic *sgtc_atc, kbool_t inherit_enable);
/* 仅在inherit_enable为true时有效, 用于重新挑选并设置本线程优先级 */
kint32_t unlock_release(struct thread *sptr_self, kbool_t inherit_enable);
```

详见内核代码“kernel/mutex.c”互斥锁的用法。

#### 10.12. 内核线程
##### 10.12.1. idle
idle（空闲）线程是内核的兜底线程，只要调度开启、CPU在运转，空闲线程就会一直在运行态和就绪态之间切换，不会挂起，也无法睡眠，更不可能被杀死。
这意味着：即使用户线程和除空闲线程以外的所有内核线程都处于挂起或睡眠态，内核也能通过执行空闲线程而不会被终止。
空闲线程的id为0，但优先级最低，除非其他线程都歇菜，否则不可能轮得到空闲线程运行。而它的功能也很简单，就是努力让自己切换成就绪态：
```c
static void *rest_entry(void *args)
{
    /* 设置线程名称 */
    thread_set_self_name("idle");

    for (;;)
    {   
        /* 切换线程: 如果没有任何线程处于就绪态, 则立即返回并继续运行 */
        schedule_thread();
    }

    return args;
}
```

##### 10.12.2. kthread
空闲线程的id虽然为0，但它却不是内核创建的第一个线程。内核中最重要的线程是kthread，也是第一个被创建的任务，它负责：
> 1）创建线程定时监视任务，每过一个系统节拍检查一次当前线程的时间片，及是否满足抢占条件；
> 2）处理平台、设备和驱动程序的隐式初始化，注册平台设备和驱动组件；
> 3）创建其他内核线程，如终端、工作队列等；
> 4）记录系统运行时间；
> 5）管理打印缓冲区，统一打印输出；
> 6）对处于睡眠态的线程，将作为僵死线程进行清理

这里的线程定时监视任务，即“抢占”一章中提及的kthread_schedule_timeout函数；而几乎所有的驱动程序，都要由kthread调用驱动程序初始化入口，从而完成总线-设备-驱动的probe机制。
此外，kthread负责销毁无用线程，而在HeavenFox中，线程如果处于睡眠态，将被当成无用线程，将面临被清理的结局。

##### 10.12.3. init_proc
线程init_proc由kthread创建，但kthread仅管理已知的内核线程，而用户线程交由init_proc负责。
init_proc的工作目前较为简单：
1）执行所有C++的全局构造函数；
2）创建网络回环设备节点；
3）创建用户线程（自定义，可创建如显示、环境传感器、触摸屏、按键等任务）

##### 10.12.4. kworker
内核中有一个特殊的线程，名为工作者线程，其他线程可以通过注册工作队列，再由工作者线程提取，异步执行。
工作者线程拥有极高的优先级，当其他线程希望某个任务可以尽快被执行，或者中断回调函数觉得某段代码过于复杂，希望由线程上下文来处理，就可以通过工作队列，安排给工作者线程。

- 工作队列
工作队列本身是一个链表（类似定时器链表），其他线程可以定义工作项，再插入到工作队列链表头，之后再遍历链表处理其中的回调函数即可。
```c
/* 工作队列头 */
typedef struct workqueue_head
{
    struct list_head sgtc_work;                     /* 每个工作项组成链表, 插入到sgtc_work尾部 */
    struct spin_lock sgtc_lock;                     /* 内置自旋锁 */

} srt_workqueue_head_t;

/* 初始化工作队列 */
#define INIT_WORKQUEUE_HEAD(sptr_wqh)   \
    do {    \
        init_list_head(&(sptr_wqh)->sgtc_link); \
        spin_lock_init(&(sptr_wqh)->sgtc_lock); \
    } while (0)

/* 定义工作队列 */
#define DECLARE_WORKQUEUE(name) \
    struct workqueue_head name = {  \
        .sgtc_work = LIST_HEAD_INIT(&(name).sgtc_work),  \
        .sgtc_lock = SPIN_LOCK_INIT(),  \
    }

/* 遍历工作队列 */
#define foreach_workqueue_safe(sptr_wq, sptr_temp, sptr_wqh)    \
    foreach_list_next_entry_safe(sptr_wq, sptr_temp, &(sptr_wqh)->sgtc_work, sgtc_link)
```

工作项也是一个结构体：
```c
typedef void (*func_work_t) (struct workqueue *);

/* 工作项 */
typedef struct workqueue
{
    func_work_t func;                               /* 回调函数, 参数为工作项自己 */
    kutype_t data;                                  /* func函数内部使用, 可传入私有变量指针 */

    struct list_head sgtc_link;                     /* 链表项, 注册时将插入到链表头struct workqueue_head::sgtc_work */

} srt_workqueue_t;

/* 初始化一个工作项 */
#define INIT_WORK(sptr_wq, _func)  \
    do {    \
        (sptr_wq)->func = _func; \
        (sptr_wq)->data = 0; \
        init_list_head(&(sptr_wq)->sgtc_link);  \
    } while (0)

/* 定义一个工作项 */
#define DECLARE_WORK(name, _func)  \
    struct workqueue name = {   \
        .func = _func,   \
        .data = 0,   \
        .sgtc_link = LIST_HEAD_INIT(&(name).sgtc_link), \
    }
```

工作项可以使用如下API与工作队列建立关系：
```c
/* 添加工作项到工作队列 */
void queue_work(struct workqueue_head *sptr_wqh, struct workqueue *sptr_wq);
/* 将工作项从工作队列分离 (使用自旋锁保护) */
void detach_work(struct workqueue_head *sptr_wqh, struct workqueue *sptr_wq);
/* 将工作项从工作队列分离 (无锁, 确认外部安全时分离) */
void detach_work_safe(struct workqueue_head *sptr_wqh, struct workqueue *sptr_wq);
/* 将工作队列的所有工作项拷贝到另一个工作队列, 并初始化自身 */
void work_splice_and_init(struct workqueue_head *sptr_src, struct workqueue_head *sptr_dst);
/* 判断工作队列是否为空 */
kbool_t is_workqueue_empty(struct workqueue_head *sptr_wqh);
```

- 工作者线程
线程定义了一个专属工作队列sgtc_kworker_wqh，可以通过queue_work函数添加工作项到队列中：
```c
void schedule_work(struct workqueue *sptr_wq)
{
    queue_work(&sgtc_kworker_wqh, sptr_wq);
}
```

线程将从队列中读出每个工作项，并一一执行其回调函数。

##### 10.12.5. kmemp
kmemp线程主要负责内存管理工作。内核允许申请的内存可以异步释放，即：线程a从内存池申请内存，用完后调用释放函数，但并不是立即释放，而是进行标记；待kmemp线程恢复运行，再统一释放的这些被标记的内存块。这样线程a无需再耗费时间去归还内存块。
要使用kmemp线程，要求内存申请和释放使用特定函数接口：
```c
/* 申请内存, 本质是调用kmalloc */
void *fwk_malloc(kusize_t __size, nrt_gfp_t gfp_mask);
/* 申请内存, 并初始化为0 */
void *fwk_zalloc(kusize_t __size, nrt_gfp_t gfp_mask);
/* 申请(__size * __n)大小的内存块 */
void *fwk_calloc(kusize_t __size, size_t __n, nrt_gfp_t gfp_mask);
/* 释放内存(不会立即释放) */
void fwk_free(void *__ptr);
```

调用fwk_free后，内存块会被插入到释放链表中，即标记为即将释放的内存；这需要在内存块首部添加一个管理信息头：
```c
struct fwk_memp_list
{
    kuint32_t magic;                                /* 魔数, 固定为0x66dfa3c2, 用于识别 */
    nrt_gfp_t gfp_mask;                             /* 分配方式, 如GFP_KERNEL */

    /* 真正的释放函数, 归还内存块 */
    void (*release)(struct fwk_memp_list *sptr_memp);

    struct fwk_memp_list *sptr_next;                /* 下一个要被释放的内存块 */
    void *ptr;                                      /* fwk_malloc的返回值: struct fwk_memp_list::ptr, 即去掉信息头后的真正内存地址 */
};
```

要释放的内存块通过sptr_next进行连接，最终链到kmemp定义的全局链表sgtc_kmemp_list_head中，kmemp线程只需不断读取sgtc_kmemp_list_head的sptr_next，便可获得需要释放的内存块，再调用struct fwk_memp_list::release将其真正地释放。
Heavenfox支持C++创建用户线程，而内存分配接口new和free则分别被重载为fwk_malloc和fwk_free。

##### 10.12.6. irq_thread
除工作者线程外，中断中复杂的代码也可以交由中断线程irq_thread来处理。不同于工作者线程，中断线程只为中断回调函数服务，是名副其实的中断下半部。
申请中断可以使用特定的接口fwk_request_threaded_irq：
```c
/*!
 * @param   irq: 中断号
 * @param   handler: 中断回调函数, 由irq_handler异常向量表入口处理;
 * @param   thread_fn: 中断线程回调函数, 由中断线程irq_thread调用;
 * @param   args: 将赋值给中断事件结构体“struct fwk_irq_action”的ptrArgs成员, 而ptrArgs将作为handler和thread_fn的函数参数
 */
kint32_t fwk_request_threaded_irq(kint32_t irq, irq_handler_t handler, irq_handler_t thread_fn, 
                                kuint32_t flags, const kchar_t *name, void *args);
```

中断回调函数handler返回值如果为NR_IRQ_WAKE_THREAD，将在中断回调函数的退出路径唤醒中断线程irq_thread；这里的irq_thread并非只有一个，当fwk_request_threaded_irq被调用时，也会同时创建一个irq_thread，线程名为“threadirq-中断号”，若内核中有多个驱动使用该接口申请中断，则将存在多个irq_thread中断线程，只是线程号和线程名不同。
irq_thread根据传入的args参数（struct fwk_irq_action::ptrArgs）处理对应的中断事件（调用thread_fn回调函数），完成后若没有新的唤醒信息，则将自身挂起，等待下一次唤醒。
中断线程的优先级极高（与工作者线程相同），故具有优先执行的特权。
