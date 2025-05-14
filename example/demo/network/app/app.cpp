/*
 * User Thread Instance (lwip task) Interface
 *
 * File Name:   app.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.11.21
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <platform/fwk_fcntl.h>
#include <platform/net/fwk_ether.h>
#include <platform/net/fwk_if.h>
#include <kernel/mailbox.h>

#include "../../../task.h"
#include "app.h"

using namespace bsc;
using namespace tsk;

/*!< The defines */
#define LOCAL_IP_ADDRESS            "192.168.253.206"
#define LOCAL_IP_MASK               "255.255.255.0"
#define LCOAL_GW_ADDRESS            "192.168.253.1"
#define LOCAL_IP_PORT               2560

#define REMOTE_IP_ADDRESS           "192.168.253.231"
#define REMOTE_IP_MASK              "255.255.255.0"
#define REMOTE_GW_ADDRESS           "192.168.253.1"
#define REMOTE_IP_PORT              3050

//#define REMOTE_IP_ADDRESS           LOCAL_IP_ADDRESS
//#define REMOTE_IP_MASK              LOCAL_IP_MASK
//#define REMOTE_GW_ADDRESS           LCOAL_GW_ADDRESS
//#define REMOTE_IP_PORT              LOCAL_IP_PORT

#define NETIF_NAME                  "eth0"

/*!< The globals */

/*!< API functions */
/*!
 * @brief  start up
 * @param  sptr_dctrl
 * @retval none
 * @note   none
 */
void crt_lwip_data_t::startup(void)
{
    struct fwk_sockaddr_in sgtc_local;
    struct fwk_sockaddr_in sgtc_ip, sgtc_gw, sgtc_netmask;
    kint32_t sockfd;
    kint32_t retval;

    sgtc_ip.sin_addr.s_addr = fwk_inet_addr(LOCAL_IP_ADDRESS);
    sgtc_gw.sin_addr.s_addr = fwk_inet_addr(LCOAL_GW_ADDRESS);
    sgtc_netmask.sin_addr.s_addr = fwk_inet_addr(LOCAL_IP_MASK);

    retval = net_link_up(NETIF_NAME, &sgtc_ip, &sgtc_gw, &sgtc_netmask);
    if (retval)
        return;

    sockfd = net_socket(NET_AF_INET, NR_SOCK_DGRAM, 0);
    if (sockfd < 0)
        goto fail1;

    sgtc_local.sin_port = mr_htons(LOCAL_IP_PORT);
    sgtc_local.sin_family = NET_AF_INET;
    sgtc_local.sin_addr.s_addr = fwk_inet_addr(LOCAL_IP_ADDRESS);
    memset(sgtc_local.zero, 0, sizeof(sgtc_local.zero));

    retval = socket_bind(sockfd, (struct fwk_sockaddr *)&sgtc_local, sizeof(struct fwk_sockaddr));
    if (retval)
        goto fail2;

    this->fd = sockfd;
    return;

fail2:
    virt_close(sockfd);
fail1:
    net_link_down(NETIF_NAME);
}

/*!
 * @brief  main
 * @param  args
 * @retval none
 * @note   none
 */
void crt_lwip_data_t::excute(void)
{
    struct fwk_sockaddr_in sgtc_remote;
    const kchar_t *msg = "HeavenFox OS will be all the best!";
    fwk_socklen_t addrlen;
    kssize_t len;
    crt_task_t *cprt_this = (crt_task_t *)this->args;
    struct mailbox &sgtc_mb = cprt_this->get_mailbox();
    struct mail *sptr_mail;

    if (this->fd < 0)
        return;

    sgtc_remote.sin_port = mr_htons(REMOTE_IP_PORT);
    sgtc_remote.sin_family = NET_AF_INET;
    sgtc_remote.sin_addr.s_addr = fwk_inet_addr(REMOTE_IP_ADDRESS);
    memset(sgtc_remote.zero, 0, sizeof(sgtc_remote.zero));

    len = socket_sendto(this->fd, msg, strlen(msg) + 1, 0, 
                    (struct fwk_sockaddr *)&sgtc_remote, sizeof(struct fwk_sockaddr));
    if (len <= 0)
    {
        cout << __func__ << ": send msg failed!" << endl;
        return;
    }

#if 1
    /*!< blocking */
    len = socket_recvfrom(this->fd, this->rx_buffer, 128, 0, 
                    (struct fwk_sockaddr *)&sgtc_remote, &addrlen);
    if (len <= 0)
    {
        cout << "recv msg failed!" << endl;
        return;
    }

    this->rx_buffer[len] = '\0';

    sptr_mail = mail_recv(&sgtc_mb, 0);
    if (!isValid(sptr_mail))
        goto END;

    if (sptr_mail->sptr_msg->type == NR_MAIL_TYPE_SERIAL)
    {
        kchar_t *buffer = (kchar_t *)sptr_mail->sptr_msg[0].buffer;

        if (!kstrncmp(buffer, "echo", 4))
            this->echo_cnt++;
    }

    mail_recv_finish(sptr_mail);

END:
    if (this->echo_cnt)
    {
        this->echo_cnt--;
//        cout << "recv data is: " << this->rx_buffer << endl;
    }

    cout << "recv data is: " << this->rx_buffer << endl;
#endif
}

/*!< end of file */
