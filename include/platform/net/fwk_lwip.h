/*
 * Hardware Abstraction Layer Net Interface
 *
 * File Name:   fwk_lwip.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.12.07
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_LWIP_H_
#define __FWK_LWIP_H_

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <platform/net/fwk_if.h>
#include <platform/net/fwk_ip.h>

#include <lwip/port/lwipopts.h>
#include <lwip/netif.h>
#include <lwip/ip.h>
#include <lwip/priv/tcp_priv.h>
#include <lwip/init.h>
#include <lwip/inet.h>
#include <lwip/etharp.h>
#include <netif/ethernet.h>
#include <lwip/udp.h>
#include <lwip/tcp.h>
#include <lwip/timeouts.h>
#include <lwip/raw.h>
#include <lwip/inet_chksum.h>

/*!< The functions */
extern kssize_t lwip_udp_raw_recvfrom(struct udp_pcb *sptr_upcb, void *buf, 
                                        kusize_t size, ip_addr_t *sptr_src, u16_t *port);
extern kssize_t lwip_udp_raw_sendto(struct udp_pcb *sptr_upcb, const ip_addr_t *sptr_dest, 
                                        u16_t dest_port, const void *buf, kusize_t size);
extern struct udp_pcb *lwip_udp_raw_bind(const ip_addr_t *sptr_ip, u16_t port);

extern kssize_t lwip_tcp_raw_recv(struct tcp_pcb *sptr_tpcb, void *buf, kusize_t size);
extern kssize_t lwip_tcp_raw_send(struct tcp_pcb *sptr_tpcb, const void *buf, kusize_t size);
extern struct tcp_pcb *lwip_tcp_raw_bind(const ip_addr_t *sptr_ip, u16_t port);
extern struct tcp_pcb *lwip_tcp_raw_listen(struct tcp_pcb *sptr_tpcb);
extern kint32_t lwip_tcp_raw_accept(struct tcp_pcb *sptr_tpcb);

extern kssize_t lwip_icmp_raw_recvfrom(struct raw_pcb *sptr_pcb, void *buf, 
                            kusize_t size, ip_addr_t *sptr_src);
extern kssize_t lwip_icmp_raw_send(struct raw_pcb *sptr_pcb, 
                        const ip_addr_t *sptr_dest, const void *buf, kusize_t size);
extern void lwip_icmp_raw_exit(struct raw_pcb *sptr_pcb);
extern struct raw_pcb *lwip_icmp_raw_init(void);

#ifdef __cplusplus
    }
#endif

#endif /*!< __FWK_LWIP_H_ */
