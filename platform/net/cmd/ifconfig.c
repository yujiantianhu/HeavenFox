/*
 * Terminal Core API: Command ifconfig
 *
 * File Name:   ifconfig.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.03.09
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/net/fwk_if.h>
#include <platform/net/fwk_netif.h>
#include <platform/net/fwk_socket.h>
#include <term/term.h>

/*!< The defines */


/*!< The globals */


/*!< The functions */

/*!< API functions */
/*!
 * @brief   cmd 'ifconfig': excute function
 * @param   sprt_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_ifconfig(struct term_cmd *sprt_cmd, kint32_t argc, kchar_t **argv)
{
    struct fwk_network_if *sprt_if;
    struct fwk_net_device *sprt_ndev = mrt_nullptr;
    kchar_t ip_addr[16], netmask[16];

    switch (argc)
    {
        case 1:
            while ((sprt_ndev = next_netdevice(sprt_ndev)))
            {
                sprt_if = network_find_node(sprt_ndev->name, mrt_nullptr);

                /*!< Link Up */
                if (sprt_if)
                {
                    fwk_inet_ntoa(ip_addr, sprt_if->sgrt_ip.sin_addr.s_addr);
                    fwk_inet_ntoa(netmask, sprt_if->sgrt_netmask.sin_addr.s_addr);

                    printk("%s: <UP RUNNING> mtu %d\n", sprt_if->ifname, sprt_ndev->mtu);
                }
                else
                {
                    fwk_inet_ntoa(ip_addr, 0);
                    fwk_inet_ntoa(netmask, 0);

                    printk("%s: <DOWN SLEEPING> mtu %d\n", sprt_ndev->name, sprt_ndev->mtu);
                }

                printk("        inet: %s netmask: %s\n", ip_addr, netmask);
                printk("        ether: %x:%x:%x:%x:%x:%x\n",
                                sprt_ndev->dev_addr[0], sprt_ndev->dev_addr[1], sprt_ndev->dev_addr[2],
                                sprt_ndev->dev_addr[3], sprt_ndev->dev_addr[4], sprt_ndev->dev_addr[5]);

                printk("        Rx Packet %ld bytes %ld\n", sprt_ndev->sgrt_stats.rx_packets, sprt_ndev->sgrt_stats.rx_bytes);
                printk("        Tx Packet %ld bytes %ld\n", sprt_ndev->sgrt_stats.tx_packets, sprt_ndev->sgrt_stats.tx_bytes);
                io_putc('\n');
            }

            break;

        case 2:
            if (!strcmp(argv[1], "--help"))
                sprt_cmd->help();
            else
                goto fail;

            break;

        default: 
            goto fail;
    }

    return ER_NORMAL;

fail:
    printk("argument error, try entering \'%s --help\' to get usage\n", argv[0]);
    return -ER_FAULT;
}

/*!
 * @brief   cmd 'ifconfig': help function
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_cmd_ifconfig_help(void)
{
    printk("usage: ifconfig\n");
}

/*!
 * @brief   cmd 'ifconfig' init and add
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_add_ifconfig(void)
{
    struct term_cmd *sprt_cmd;

    sprt_cmd = term_cmd_allocate("ifconfig", GFP_KERNEL);
    if (!isValid(sprt_cmd))
        return;

    sprt_cmd->do_excute = term_cmd_ifconfig;
    sprt_cmd->help = term_cmd_ifconfig_help;

    term_cmd_add(sprt_cmd);
}
