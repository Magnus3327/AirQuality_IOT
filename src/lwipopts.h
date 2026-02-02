#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Impedisce a LwIP di ridefinire struct timeval (già presente nella toolchain ARM)
#define LWIP_TIMEVAL_PRIVATE        0

// Configurazione No-OS
#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_DHCP                   1

// Disabilita esplicitamente le API che causano l'errore #error in init.c
#define LWIP_TCPIP_CORE_LOCKING     0
#define SYS_LIGHTWEIGHT_PROT        0

// Parametri di memoria
#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000
#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_ARP_QUEUE          10
#define PBUF_POOL_SIZE              24

// Protocolli
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define TCP_MSS                     1460
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            16
#define LWIP_CHKSUM_ALGORITHM       3

// Aumenta il numero di timeout di sistema disponibili
#define MEMP_NUM_SYS_TIMEOUT   12
// Assicurati che ci sia abbastanza memoria per i pacchetti MQTT
#define MEMP_NUM_TCP_PCB       10

#endif