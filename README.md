# git commit logs > git@github.com:talented-rain/real-kernel.git
# 
# author: Yang Yujun
# created on: 2023.12.21
# e-mail: <yujiantianhu@163.com>

2024.04.16:
    author: Yang Yujun
    commit: change project name to "fantasystem", i.e. fantasy system

2024.04.15:
    author: Yang Yujun
    commit: 
        01) fix the issue that the first thread scheduling register was overwritten;
        02) add display generic APIs
        03) optimize code

2024.04.13:
    author: Yang Yujun
    commit: makefile optimization, supporting selective compilation of source files

2024.04.07:
    author: Yang Yujun
    commit: support --->
        01) support multithreading (time slice, priority preemption, and delayed sleep);
        02) support initcall implicit initialization;
        03) support character device driver framework;
        04) support framebuffer driver framework;
        05) support IRQ virtual interrupt numbers mapping (GIC_SPI);
        06) support timer link list (timer_list);
        07) support SD Card (porting fatfs);
        08) support Device Tree (support reading .dtb in SD Card, and parsing it to device_node and platform_device);
        09) support platform: bus - device - driver framework;
        10) provide APIs such as open/close/read/write, access hardware through “fd” and “file_operations”.

2023.12.21:
    author: Yang Yujun
    commit: create README.md first
    
# end
