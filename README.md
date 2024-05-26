# Project Name: Gardens System
# 
# Note:
    Gardens System aims to simplify the Linux kernel by refering to the kernel mechanism,
    which is convenient for the developers to research and apply, and the project follows
    the GPL license and is open source for life.

# Information:
    git commit logs > git@github.com:yujiantianhu/gardens.git
    
    author: Yang Yujun
    created on: 2023.12.21
    e-mail: <yujiantianhu@163.com>

# Function:
    support multithreading (time slice, priority preemption, and delayed sleep);
    support initcall implicit initialization;
    support character device driver framework;
    support framebuffer driver framework;
    support IRQ virtual interrupt numbers mapping (GIC_SPI);
    support timer link list (timer_list);
    support SD Card (porting fatfs);
    support Device Tree (support reading .dtb in SD Card, and parsing it to device_node and platform_device);
    support platform: bus - device - driver framework;
    provide APIs such as open/close/read/write, access hardware through “fd” and “file_operations”;
    support flexible selection of source files to be complied through Makefile;
    support pinctrl, gpio (and gpio-irq), clock subsystem, and i2c bus framework;
    support workqueue, mutex lock, spin lock and mailbox (inter-thread communication, e.g, ITC)
    
# Methods:
    make clean:     delete all the .o and .d;
    make distclean: delete the ./objects/* and .elf, .dis, .img;
    
    or using cmake:
    mkdir -p ./build
    cd ./build
    make distclean
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    make
    
    Image path: ./boot/gardens.img

# Readme:
# -----------------------------------------------------------------------

# 2024.05.26
    author: Yang Yujun
    commit:
        01) add mailbox (inter-thread communication);
        02) optimize mutex lock and spin lock;
        03) add workqueue (for the low half interrupt);
        04) standardize the naming of variable types (such as kint32_t, structure, and so on)

# 2024.05.13
    author: Yang Yujun
    commit:
        01) add pinctrl device framework;
        02) add gpio device framework;
        03) add clock system framework;
        04) add i2c master, device and instance (tsc2007) driver;
        05) optimize irq framework

# 2024.04.18
    author: Yang Yujun
    commit: configure .gitignore, ignore vscode and object

# 2024.04.16:
    author: Yang Yujun
    commit: change project name to "gardens"

# 2024.04.15:
    author: Yang Yujun
    commit: 
        01) fix the issue that the first thread scheduling register was overwritten;
        02) add display generic APIs
        03) optimize code

# 2024.04.13:
    author: Yang Yujun
    commit: makefile optimization, supporting selective compilation of source files

# 2024.04.07:
    author: Yang Yujun
    commit:
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

# 2023.12.21:
    author: Yang Yujun
    commit: create README.md first
    
# -----------------------------------------------------------------------
# end

