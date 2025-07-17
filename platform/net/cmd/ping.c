/*
 * Terminal Core API: Command ping
 *
 * File Name:   ping.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.06.26
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/net/fwk_if.h>
#include <platform/net/fwk_netif.h>
#include <platform/net/fwk_socket.h>
#include <platform/net/fwk_icmp.h>
#include <platform/notifier/fwk_notifier.h>
#include <platform/base/fwk_fcntl.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <term/term.h>

/*!< The defines */
struct ping_cmd_data
{
    tid_t tid;
    kint32_t sockfd;

    struct fwk_sockaddr_in sgtc_dest;
    kbool_t tx_enable;

    struct fwk_notifier_block sgtc_nb;
};

/*!< The globals */
static kint32_t g_ping_cmd_count = -1;

/*!< The functions */
static void ping_destroy(struct ping_cmd_data *sptr_data);

/*!< API functions */
/*!
 * @brief   Notifier callback
 * @param   sptr_nb
 * @retval  errno
 * @note    none
 */
kint32_t ping_pause_action(struct fwk_notifier_block *sptr_nb, kuint32_t event, void *args)
{
    struct ping_cmd_data *sptr_data;

    sptr_data = mr_container_of(sptr_nb, struct ping_cmd_data, sgtc_nb);

    if (event == TERM_PAUSE_NOTIFIER_CANCEL)
        sptr_data->tx_enable = false;

    return event;
}

/*!
 * @brief   Packet icmp data
 * @param   buf: save icmp hdr and data
 * @retval  errno
 * @note    none
 */
static kint32_t ping_make_packet(void *buf, kuint16_t size, kuint16_t seq_num)
{
    struct fwk_icmp_hdr *sptr_hdr;
    kchar_t *data;
    kuint16_t data_len;

    if (size < NET_ICMP_HDR_LEN)
        return -ER_LACK;

    sptr_hdr = (struct fwk_icmp_hdr *)buf;
    sptr_hdr->type = NET_PROTO_ICMP_ECHO;
    sptr_hdr->code = 0;
    sptr_hdr->check_sum = 0;
    sptr_hdr->u.sgtc_echo.id = NET_ICMP_PING_ID;
    sptr_hdr->u.sgtc_echo.seq_no = seq_num;

    data = (kchar_t *)sptr_hdr + NET_ICMP_HDR_LEN;
    data_len = size - NET_ICMP_HDR_LEN;

    for (kint32_t i = 0; i < data_len; i++)
        data[i] = (kchar_t)i;

    sptr_hdr->check_sum = fwk_transport_csum(buf, size);
    return ER_NORMAL;
}

/*!
 * @brief   Ping thread
 * @param   args: struct ping_cmd_data *
 * @retval  args
 * @note    If "Ctrl+C" pressed, sleep and stop ping
 */
static void *ping_cmd_entry(void *args)
{
    struct ping_cmd_data *sptr_data;
    kuint8_t packet[NET_ICMP_HDR_LEN + 32];
    kuint16_t seq_num = 0;
    kint32_t size;
    struct fwk_sockaddr_in sgtc_src;

    sptr_data = (struct ping_cmd_data *)args;

    for (;;)
    {
        /*!< If recieve cancel notifier, self sleep */
        if (!sptr_data->tx_enable)
        {
            fwk_blocking_notifier_chain_unregister(
                                &sgtc_pause_notifier_chain, &sptr_data->sgtc_nb);

            ping_destroy(sptr_data);
            g_ping_cmd_count = -1;

            schedule_self_sleep();
        }
        else if (!ping_make_packet(packet, sizeof(packet), ++seq_num))
        {
            size = socket_sendto(sptr_data->sockfd, packet, sizeof(packet), 
                            0, (const struct fwk_sockaddr *)&sptr_data->sgtc_dest, sizeof(struct fwk_sockaddr));
            if (size < 0)
            {
                kchar_t ipaddr[16];

                printk("Ping %s icmp_seq=%u packet can't not be sent\r\n", 
                        fwk_inet_ntoa(ipaddr, &sptr_data->sgtc_dest), seq_num);
                sleep(1);

                continue;
            }

            g_ping_cmd_count++;
            sleep(1);

            /*!< Read but do nothing (socket_recvfrom will print information, and free rx buffer) */
            socket_recvfrom(sptr_data->sockfd, packet, sizeof(packet), 
                            0, (struct fwk_sockaddr *)&sgtc_src, &size);
        }
    }

    return args;
}

/*!
 * @brief   Request socket and create ping thread
 * @param   sptr_dest: destination host ip address
 * @retval  struct ping_cmd_data *
 * @note    just ping "lo" and remote host
 */
static struct ping_cmd_data *ping_create(struct fwk_sockaddr_in *sptr_dest)
{
    struct ping_cmd_data *sptr_data;
    struct fwk_sockaddr_in sgtc_local;
    kchar_t task_name[32];
    kint32_t sockfd, retval = ER_NORMAL;
    struct fwk_network_if *sptr_if;

    sprintk(task_name, "ping ");
    fwk_inet_ntoa(task_name + 5, sptr_dest);

    sptr_if = network_find_node(mr_nullptr, sptr_dest);
    if (sptr_if && kstrcmp(sptr_if->ifname, "lo"))
    {
        printk("IP %s is local address, not support ping self (except \"lo\")\r\n", task_name + 5);
        return mr_nullptr;
    }

    sptr_data = kmalloc(sizeof(*sptr_data), GFP_KERNEL);
    if (!isValid(sptr_data))
        return ERR_PTR(-ER_NOMEM);

    sockfd = net_socket(NET_AF_INET, NR_SOCK_RAW, NET_IP_PROTO_ICMP);
    if (sockfd < 0) {
        retval = sockfd;
        goto fail1;
    }

    sgtc_local.sin_addr.s_addr = NET_IP_ADDR_ANY;
    sgtc_local.sin_family = NET_AF_INET;
    sgtc_local.sin_port = 0;
    memset(sgtc_local.zero, 0, sizeof(sgtc_local.zero));

    retval = socket_bind(sockfd, (struct fwk_sockaddr *)&sgtc_local, sizeof(struct fwk_sockaddr));
    if (retval)
        goto fail2;

    sptr_data->sockfd = sockfd;
    memcpy(&sptr_data->sgtc_dest, sptr_dest, sizeof(*sptr_dest));

    sptr_data->tx_enable = true;

    /*!< Create ping thread */
    retval = thread_create(&sptr_data->tid, mr_nullptr, ping_cmd_entry, sptr_data);
    if (retval < 0)
        goto fail2;

    thread_set_name(sptr_data->tid, (const kchar_t *)task_name);
    return sptr_data;

fail2:
    virt_close(sockfd);
fail1:
    kfree(sptr_data);
    return ERR_PTR(retval);
}

/*!
 * @brief   Release socket and free private data
 * @param   sptr_data
 * @retval  none
 * @note    none
 */
static void ping_destroy(struct ping_cmd_data *sptr_data)
{
    virt_close(sptr_data->sockfd);
    kfree(sptr_data);
}

/*!
 * @brief   cmd 'ping': excute function
 * @param   sptr_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_ping(struct term_cmd *sptr_cmd, kint32_t argc, kchar_t **argv)
{
    switch (argc)
    {
        case 2:
            if (!kstrcmp(argv[1], "--help"))
                sptr_cmd->help();
            else
            {
                struct ping_cmd_data *sptr_data;
                struct fwk_sockaddr_in sgtc_dest;
                kuint32_t ip_addr;

                /*!< Forbid second calling */
                if (g_ping_cmd_count != -1)
                {
                    printk("ping command is running, please cancel the previous ping task first\r\n");
                    return ER_NORMAL;
                }

                /*!< String to integer */
                ip_addr = fwk_inet_addr(argv[1]);
                if (!ip_addr)
                    goto fail;

                sgtc_dest.sin_addr.s_addr = ip_addr;
                sptr_data = ping_create(&sgtc_dest);
                if (!isValid(sptr_data))
                    return -ER_FAILD;

                g_ping_cmd_count = 0;

                /*!< Register notifier ("Ctrl + C") */
                sptr_data->sgtc_nb.data = sptr_data;
                sptr_data->sgtc_nb.notifier_call = ping_pause_action;
                sptr_data->sgtc_nb.pengding_call = mr_nullptr;
                sptr_data->sgtc_nb.expect_event = TERM_PAUSE_NOTIFIER_CANCEL;
                init_list_head(&sptr_data->sgtc_nb.sgtc_link);
                fwk_blocking_notifier_chain_register(&sgtc_pause_notifier_chain, &sptr_data->sgtc_nb);
            }

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
 * @brief   cmd 'ping': help function
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_cmd_ping_help(void)
{
    printk("usage: ping [ip address]\r\n");
}

/*!
 * @brief   cmd 'ping' init and add
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_add_ping(void)
{
    struct term_cmd *sptr_cmd;
    struct term_variable *sptr_pctrl;

    sptr_cmd = term_cmd_allocate("ping", GFP_KERNEL);
    if (!isValid(sptr_cmd))
        return;

    sptr_cmd->do_excute = term_cmd_ping;
    sptr_cmd->help = term_cmd_ping_help;

    term_cmd_add(sptr_cmd);

    /*!< Add terminal manage */
    sptr_pctrl = term_variable_allocate("g_ping_cmd_count", &g_ping_cmd_count);
    if (isValid(sptr_pctrl))
        term_variable_add(sptr_pctrl);
}
