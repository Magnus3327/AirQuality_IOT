#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Core behavior
#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4

// --- CORREZIONE MEMORIA ---
// Aumentiamo la memoria totale per gestire i buffer TCP/MQTT
#define MEM_SIZE                    16000 

#define PICO_CYW43_ARCH_THREADSAFE_BACKGROUND 1

// Features
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define LWIP_DNS                    1
#define LWIP_MQTT                   1 

// --- PARAMETRI TCP (Risolvono il Sanity Check Error) ---
#define TCP_MSS                     1460
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_SND_BUF                 (8 * TCP_MSS)

// Questa riga deve essere >= TCP_SND_QUEUELEN
#define MEMP_NUM_TCP_SEG            32   
// Questa riga definisce quanti pacchetti possono stare in coda
#define TCP_SND_QUEUELEN            16   

#define LWIP_CHKSUM_ALGORITHM       3

#endif