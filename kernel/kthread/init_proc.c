/*
 * Kernel Thread Instance (init task) Interface
 *
 * File Name:   init_proc.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.01
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/instance.h>
#include <platform/net/fwk_socket.h>
#include <platform/net/fwk_if.h>
#include <platform/net/fwk_netif.h>

/*!< The defines */
#define INIT_THREAD_STACK_SIZE                          THREAD_STACK_PAGE(1)    /*!< 1 page (4 kbytes) */

/*!< The globals */
static struct thread_attr sgtc_init_proc_attr;
static THREAD_STACK_DEFINE(g_init_proc_stack, INIT_THREAD_STACK_SIZE);

/*!< API functions */
/*!
 * @brief	application task main
 * @param  	none
 * @retval 	none
 * @note   	none
 */
__weak kint32_t main(kint32_t argc, kchar_t **argv)
{
    /*!< dummy */
    return 0;
}

/*!
 * @brief	network startup
 * @param  	none
 * @retval 	none
 * @note   	open loopback device
 */
static void net_init(void)
{
    struct fwk_sockaddr_in sgtc_ip, sgtc_gw, sgtc_netmask;

    /*!< Loopback */
    sgtc_ip.sin_addr.s_addr = fwk_inet_addr("127.0.0.1");
    sgtc_gw.sin_addr.s_addr = fwk_inet_addr("127.0.0.1");
    sgtc_netmask.sin_addr.s_addr = fwk_inet_addr("255.255.255.0");
    net_link_up("lo", &sgtc_ip, &sgtc_gw, &sgtc_netmask);
}

/*!
 * @brief	init thread entry
 * @param  	args: NULL normally
 * @retval 	none
 * @note   	create all user thread
 */
static void *init_proc_entry(void *args)
{
    thread_set_self_name(__FUNCTION__);

    print_info("%s is enter, which tid is: %d\r\n", __FUNCTION__, mr_current->tid);

    /*!< Network initial */
    net_init();

    /*!< Create application: at the end of "init_proc_entry" */
    main(0, mr_nullptr);

    for (;;)
    {  
        sleep(1);
    }

    return args;
}

/*!
 * @brief	create init thread
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t init_proc_init(void)
{
    struct thread_attr *sptr_attr = &sgtc_init_proc_attr;

	sptr_attr->detachstate = THREAD_CREATE_JOINABLE;
	sptr_attr->inheritsched	= THREAD_INHERIT_SCHED;
	sptr_attr->schedpolicy = THREAD_SCHED_FIFO;

    /*!< thread stack */
	thread_set_stack(sptr_attr, mr_nullptr, g_init_proc_stack, sizeof(g_init_proc_stack));
    /*!< lowest priority */
	thread_set_priority(sptr_attr, THREAD_PROTY_INIT);
    /*!< default time slice */
    thread_set_time_slice(sptr_attr, THREAD_TIME_DEFUALT);

    /*!< register thread */
    return kernel_thread_init_create(sptr_attr, init_proc_entry, mr_nullptr);
}

/*!< end of file */
