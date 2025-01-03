//
// Copyright (c) 2022 ZettaScale Technology
// Copyright (c) 2023 Fictionlab sp. z o.o.
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
//   Błażej Sowa, <blazej@fictionlab.pl>

#include <stdlib.h>

#include "zenoh-pico/system/platform.h"
#include "zenoh-pico/utils/pointers.h"
#include "zenoh-pico/utils/result.h"

// FreeRTOS includes
// #include "FreeRTOS.h"
// #include "FreeRTOS_IP.h"

// TODO: Cleanup
// TODO #include "threadx_plus_netxduo.h"
#define Z_FEATURE_MULTI_THREAD 1
#define Z_FEATURE_LINK_TCP 0
#define Z_FEATURE_LINK_UDP_UNICAST 1 
#define Z_FEATURE_LINK_UDP_MULTICAST 0
#define configSUPPORT_STATIC_ALLOCATION 1
#define Z_MULTI_THREAD 0
#include "tx_api.h"
#include "nx_api.h"
#include "tx_byte_pool.h"


#if Z_FEATURE_LINK_TCP == 1
#error "TCP is *so far* not supported on ThreadX with NetXDuo port of Zenoh-Pico"
#endif


#if Z_FEATURE_LINK_UDP_UNICAST == 1 || Z_FEATURE_LINK_UDP_MULTICAST == 1
/*------------------ UDP sockets ------------------*/
z_result_t _z_create_endpoint_udp(_z_sys_net_endpoint_t *ep, const char *s_address, const char *s_port) {
    z_result_t ret = _Z_RES_OK;

    // https://freertos.org/Documentation/03-Libraries/02-FreeRTOS-plus/02-FreeRTOS-plus-TCP/09-API-reference/67-FreeRTOS_getaddrinfo_a
    // https://freertos.org/Documentation/03-Libraries/02-FreeRTOS-plus/02-FreeRTOS-plus-TCP/09-API-reference/66-FreeRTOS_getaddrinfo
    // The API is used to lookup the IP-address of a host. It is an alternative to FreeRTOS_gethostbyname() with support for IPv6. 
  
    // https://github.com/eclipse-threadx/rtos-docs/blob/main/rtos-docs/netx-duo/netx-duo-dns/chapter2.md
    //  status = nx_dns_host_by_name_get(&client_dns, (UCHAR *)"www.my_example.com", &host_ip_address, 400);
    //
    // need packet pool
    // NX_PACKET_POOL          client_pool;
    // status = nx_packet_pool_create(&client_pool, "DNS Client Packet Pool", NX_DNS_PACKET_PAYLOAD, pointer, NX_DNS_PACKET_POOL_SIZE);
    // status = nx_dns_packet_pool_set(&client_dns, &client_pool);
    // then
    // status = nx_dns_host_by_name_get(&client_dns, (UCHAR *)"www.my_example.com", &host_ip_address, 400);

    /* for now the config will be injected before udp send

    if (FreeRTOS_getaddrinfo(s_address, NULL, NULL, &ep->_iptcp) < 0) {
        ret = _Z_ERR_GENERIC;
        return ret;
    }

    ep->_iptcp->ai_addr->sin_family = ep->_iptcp->ai_family;

    // Parse and check the validity of the port
    uint32_t port = strtoul(s_port, NULL, 10);
    if ((port > (uint32_t)0) && (port <= (uint32_t)65355)) {  // Port numbers should range from 1 to 65355
        ep->_iptcp->ai_addr->sin_port = FreeRTOS_htons((uint16_t)port);
        // https://www.freertos.org/Documentation/03-Libraries/02-FreeRTOS-plus/02-FreeRTOS-plus-TCP/09-API-reference/23-htons_ntohs_htonl_ntohl
        // FreeRTOS_htons and FreeRTOS_ntohs() return the value of their 16-bit parameter with the high and low bytes swapped. 
        // https://sourcevu.sysprogs.com/stm32/Libraries/netxduo/macros/htons
    } else {
        ret = _Z_ERR_GENERIC;
    }
    */ 
    printf("_z_create_endpoint_udp()\n");
    return ret;
}

void _z_free_endpoint_udp(_z_sys_net_endpoint_t *ep) { 
    /* for now the config will be injected before udp send

    FreeRTOS_freeaddrinfo(ep->_iptcp); 
    */
}
#endif // Z_FEATURE_LINK_UDP_UNICAST == 1 || Z_FEATURE_LINK_UDP_MULTICAST == 1

#if Z_FEATURE_LINK_UDP_UNICAST == 1
z_result_t _z_open_udp_unicast(_z_sys_net_socket_t *sock, const _z_sys_net_endpoint_t rep, uint32_t tout) {
    z_result_t ret = _Z_RES_OK;

    /* for now the config will be injected before udp send

    sock->_socket = FreeRTOS_socket(rep._iptcp->ai_family, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP);
    if (sock->_socket != FREERTOS_INVALID_SOCKET) {
        TickType_t receive_timeout = pdMS_TO_TICKS(tout);

        if (FreeRTOS_setsockopt(sock->_socket, 0, FREERTOS_SO_RCVTIMEO, &receive_timeout, 0) != 0) {
            ret = _Z_ERR_GENERIC;
        }
    } else {
        ret = _Z_ERR_GENERIC;
    }
    */ 
    printf("_z_open_udp_unicast()\n")
    return ret;
}

z_result_t _z_listen_udp_unicast(_z_sys_net_socket_t *sock, const _z_sys_net_endpoint_t rep, uint32_t tout) {
    z_result_t ret = _Z_RES_OK;
    /* for now the config will be injected before udp send
     * and listen is not implemented

    (void)sock;
    (void)rep;
    (void)tout;

    // @TODO: To be implemented
    ret = _Z_ERR_GENERIC;
    */ 

    printf("_z_listen_udp_unicast()\n");
    return ret;
}

void _z_close_udp_unicast(_z_sys_net_socket_t *sock) { 
  /* for now the config will be injected before udp send
    FreeRTOS_closesocket(sock->_socket);
  */ 
    printf("_z_close_udp_unicast()\n");
}

size_t _z_read_udp_unicast(const _z_sys_net_socket_t sock, uint8_t *ptr, size_t len) {
  /* for now the config will be injected before udp send
    struct freertos_sockaddr raddr;
    uint32_t addrlen = sizeof(struct freertos_sockaddr);

    int32_t rb = FreeRTOS_recvfrom(sock._socket, ptr, len, 0, &raddr, &addrlen);
    if (rb < 0) {
        return SIZE_MAX;
    }
*/  
    int32_t rb = 0;
    printf("_z_close_udp_unicast()\n");
    return rb;
}

size_t _z_read_exact_udp_unicast(const _z_sys_net_socket_t sock, uint8_t *ptr, size_t len) {
    size_t n = 0;
    /*
    uint8_t *pos = &ptr[0];

    do {
        size_t rb = _z_read_udp_unicast(sock, pos, len - n);
        if (rb == SIZE_MAX) {
            n = rb;
            break;
        }

        n = n + rb;
        pos = _z_ptr_u8_offset(pos, n);
    } while (n != len);
    */
    printf("_z_read_exact_udp_unicast()\n");
    return n;
}

static NX_PACKET      *packet;
static NX_UDP_SOCKET  socket;
static TX_THREAD      thread;
static NX_PACKET_POOL pool;
static NX_IP          ip;

size_t _z_send_udp_unicast(
    const _z_sys_net_socket_t sock, 
    const uint8_t *ptr, 
    size_t len,
    const _z_sys_net_endpoint_t rep) 
{
    UINT status;
    status =  nx_packet_allocate(
        &pool, 
        &packet, 
        NX_UDP_PACKET, 
        TX_WAIT_FOREVER);

    if (status != NX_SUCCESS) { break; }

   // nx_packet_data_append(packet, DEMO_DATA, sizeof(DEMO_DATA), &pool_0, TX_WAIT_FOREVER);
    nx_packet_data_append(
        packet, 
        ptr, 
        len, 
        &pool, 
        TX_WAIT_FOREVER);

    status =  nxd_udp_socket_send(&socket_0, my_packet, &ipv4_address, 0x89);
 
    if (status != NX_SUCCESS)
    {
        return 0;
    }
    return len;

    // https://www.freertos.org/Documentation/03-Libraries/02-FreeRTOS-plus/02-FreeRTOS-plus-TCP/09-API-reference/08-sendto
    // Send data to a UDP socket (see FreeRTOS_send() for the TCP equivalent). The socket must have already been created by a successful call to FreeRTOS_socket().
    // Returns: The number of bytes that were actually queue for sending, which will be 0 if an error or timeout occurred.
    /*
    return FreeRTOS_sendto(
        sock._socket, 
        ptr, 
        len, 
        0, 
        rep._iptcp->ai_addr, 
        sizeof(struct freertos_sockaddr));
    */
}
#endif

#if Z_FEATURE_LINK_UDP_MULTICAST == 1
#error "UDP Multicast is *so far* not supported on ThreadX with NetXDuo port of Zenoh-Pico"
#endif

#if Z_FEATURE_LINK_BLUETOOTH == 1
#error "Bluetooth not supported yet on FreeRTOS-Plus-TCP port of Zenoh-Pico"
#endif

#if Z_FEATURE_LINK_SERIAL == 1
#error "Serial not supported yet on FreeRTOS-Plus-TCP port of Zenoh-Pico"
#endif

#if Z_FEATURE_RAWETH_TRANSPORT == 1
#error "Raw ethernet transport not supported yet on FreeRTOS-Plus-TCP port of Zenoh-Pico"
#endif
