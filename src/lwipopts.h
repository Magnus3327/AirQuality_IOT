#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

/** Prevent LwIP from redefining struct timeval */
#define LWIP_TIMEVAL_PRIVATE        0

/** No-OS Configuration */
#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_DHCP                   1

/** Disable APIs causing init.c errors */
#define LWIP_TCPIP_CORE_LOCKING     0
#define SYS_LIGHTWEIGHT_PROT        0

/** Memory Parameters */
#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000
#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_ARP_QUEUE          10
#define PBUF_POOL_SIZE              24

/** Protocol Configuration */
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define TCP_MSS                     1460
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            16
#define LWIP_CHKSUM_ALGORITHM       3

/** Increase system timeouts and MQTT memory */
#define MEMP_NUM_SYS_TIMEOUT        12
#define MEMP_NUM_TCP_PCB            10

#endif