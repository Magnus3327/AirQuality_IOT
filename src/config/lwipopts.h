#ifndef LWIPOPTS_H
#define LWIPOPTS_H

#define NO_SYS                      0
#define SYS_LIGHTWEIGHT_PROT        1

#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0

#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    (16 * 1024)

#define MEMP_NUM_TCP_PCB            5
#define MEMP_NUM_TCP_PCB_LISTEN     2
#define MEMP_NUM_TCP_SEG            16
#define MEMP_NUM_SYS_TIMEOUT        10

#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define LWIP_DHCP                   1
#define LWIP_DNS                    1
#define LWIP_MQTT                   1

#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1

#define PBUF_POOL_SIZE              16
#define PBUF_POOL_BUFSIZE           512

#define TCP_MSS                     1460
#define TCP_SND_BUF                 (2 * TCP_MSS)
#define TCP_WND                     (4 * TCP_MSS)

#define LWIP_TIMEVAL_PRIVATE        0

#endif
