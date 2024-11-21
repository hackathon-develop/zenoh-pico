//
// Copyright (c) 2022 ZettaScale Technology
// Copyright (c) 2024 Deutsche Zentrum für Luft- und Raumfahrt e. V.
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
//   Wanja Zaeske, <wucke13+github@gmail.com>

#ifndef ZENOH_PICO_SYSTEM_THREADX_TYPES_H
#define ZENOH_PICO_SYSTEM_THREADX_TYPES_H

#include <tx_api.h>

#include "zenoh-pico/config.h"

#ifdef __cplusplus
extern "C" {
#endif

// 100 MHz
#define ZENOH_THREADX_TICKS_PER_SEC 100_000_000

#ifndef ZENOH_THREADX_TICKS_PER_SEC
#error You must specifiy how many ticks pass in one second using ZENOH_THREADX_TICKS_PER_SEC
#endif

// TODO re-visit these choices
#if Z_FEATURE_MULTI_THREAD == 1
typedef void _z_task_t; // would be TX_THREAD
typedef void z_task_attr_t; // does not exist in ThreadX
typedef void _z_mutex_t; // would be TX_MUTEX
typedef void _z_condvar_t; // does not exist in ThreadX
#endif  // Z_FEATURE_MULTI_THREAD == 1

typedef ULONG z_clock_t;
typedef ULONG z_time_t;

typedef struct {
    union {
#if Z_FEATURE_LINK_TCP == 1 || Z_FEATURE_LINK_UDP_MULTICAST == 1 || Z_FEATURE_LINK_UDP_UNICAST == 1 || \
    Z_FEATURE_RAWETH_TRANSPORT == 1
        // TODO add fd type if we want fd based connection types
        // int _fd;
#endif
    };
} _z_sys_net_socket_t;

typedef struct {
    union {
#if Z_FEATURE_LINK_TCP == 1 || Z_FEATURE_LINK_UDP_MULTICAST == 1 || Z_FEATURE_LINK_UDP_UNICAST == 1
        struct addrinfo *_iptcp;
#endif
    };
} _z_sys_net_endpoint_t;

#ifdef __cplusplus
}
#endif

#endif /* ZENOH_PICO_SYSTEM_THREADX_TYPES_H */