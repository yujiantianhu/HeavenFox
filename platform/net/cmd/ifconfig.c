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
#include <platform/base/fwk_basic.h>
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
 * @param   sptr_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_ifconfig(struct term_cmd *sptr_cmd, kint32_t argc, kchar_t **argv)
{
    struct fwk_network_if *sptr_if;
    struct fwk_net_device *sptr_ndev = mr_nullptr;
    kchar_t ip_addr[16], netmask[16];

    switch (argc)
    {
        case 1:
            while ((sptr_ndev = next_netdevice(sptr_ndev)))
            {
                sptr_if = network_find_node(sptr_ndev->name, mr_nullptr);

                /*!< Link Up */
                if (sptr_if)
                {
                    fwk_inet_ntoa(ip_addr, &sptr_if->sgtc_ip);
                    fwk_inet_ntoa(netmask, &sptr_if->sgtc_netmask);

                    printk("%s: <UP RUNNING> mtu %d\r\n", sptr_if->ifname, sptr_ndev->mtu);
                }
                else
                {
                    memset(ip_addr, 0, sizeof(ip_addr));
                    memset(netmask, 0, sizeof(netmask));

                    printk("%s: <DOWN SLEEPING> mtu %d\r\n", sptr_ndev->name, sptr_ndev->mtu);
                }

                printk("        inet: %s netmask: %s\r\n", ip_addr, netmask);
                printk("        ether: %x:%x:%x:%x:%x:%x\r\n",
                                sptr_ndev->dev_addr[0], sptr_ndev->dev_addr[1], sptr_ndev->dev_addr[2],
                                sptr_ndev->dev_addr[3], sptr_ndev->dev_addr[4], sptr_ndev->dev_addr[5]);

                printk("        Rx Packet %ld bytes %ld\r\n", sptr_ndev->sgtc_stats.rx_packets, sptr_ndev->sgtc_stats.rx_bytes);
                printk("        Tx Packet %ld bytes %ld\r\n", sptr_ndev->sgtc_stats.tx_packets, sptr_ndev->sgtc_stats.tx_bytes);
                printk("\r\n");
            }

            break;

        case 2:
            if (!kstrcmp(argv[1], "--help"))
                sptr_cmd->help();
            else
                goto fail;

            break;

        default: 
            goto fail;
    }

    return ER_NORMAL;

fail:
    printk("argument error, try entering \'%s --help\' to get usage\r\n", argv[0]);
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
    printk("usage: ifconfig\r\n");
}

/*!
 * @brief   cmd 'ifconfig' init and add
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_add_ifconfig(void)
{
    struct term_cmd *sptr_cmd;

    sptr_cmd = term_cmd_allocate("ifconfig", GFP_KERNEL);
    if (!isValid(sptr_cmd))
        return;

    sptr_cmd->do_excute = term_cmd_ifconfig;
    sptr_cmd->help = term_cmd_ifconfig_help;

    term_cmd_add(sptr_cmd);
}
