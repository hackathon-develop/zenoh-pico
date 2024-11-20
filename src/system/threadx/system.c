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
//

#include "tx_api.h"
#include <stddef.h>
#include <stdlib.h>

#include "zenoh-pico/system/platform_common.h"
#include "zenoh-pico/utils/result.h"

#include <unistd.h>

#include "zenoh-pico/config.h"
#include "zenoh-pico/system/platform.h"

/*------------------ Random ------------------*/
uint8_t z_random_u8(void) {
    return 1; // TODO fix this
}

uint16_t z_random_u16(void) {
    return 2;
}

uint32_t z_random_u32(void) {
    return 3;
}

uint64_t z_random_u64(void) {
    return 4;
}

void z_random_fill(void *buf, size_t len) {
  // TODO don't rely on UB
}

/*------------------ Memory ------------------*/
void *z_malloc(size_t size) { return malloc(size); }

void *z_realloc(void *ptr, size_t size) { return realloc(ptr, size); }

void z_free(void *ptr) { free(ptr); }

#if Z_FEATURE_MULTI_THREAD == 1
/*------------------ Task ------------------*/
z_result_t _z_task_init(_z_task_t *task, z_task_attr_t *attr, void *(*fun)(void *), void *arg) {
  return 0; // TODO
}

z_result_t _z_task_join(_z_task_t *task) {
  return 0; // TODO
}

z_result_t _z_task_detach(_z_task_t *task) {
  return 0; // TODO
}

z_result_t _z_task_cancel(_z_task_t *task) {
  return 0; // TODO
}

void _z_task_free(_z_task_t **task) {
  return 0; // TODO
}

/*------------------ Mutex ------------------*/
z_result_t _z_mutex_init(_z_mutex_t *m) {
  return 0; // TODO
}

z_result_t _z_mutex_drop(_z_mutex_t *m) {
  return 0; // TODO
}

z_result_t _z_mutex_lock(_z_mutex_t *m) {
  return 0; // TODO
}

z_result_t _z_mutex_try_lock(_z_mutex_t *m) {
  return 0; // TODO
}

z_result_t _z_mutex_unlock(_z_mutex_t *m) {
  return 0; // TODO
}

/*------------------ Condvar ------------------*/
z_result_t _z_condvar_init(_z_condvar_t *cv) {
  return 0; // TODO
}

z_result_t _z_condvar_drop(_z_condvar_t *cv) {
  return 0; // TODO
}

z_result_t _z_condvar_signal(_z_condvar_t *cv) {
  return 0; // TODO
}

z_result_t _z_condvar_signal_all(_z_condvar_t *cv) {
  return 0; // TODO
}

z_result_t _z_condvar_wait(_z_condvar_t *cv, _z_mutex_t *m) {
  return 0; // TODO
}
#endif  // Z_FEATURE_MULTI_THREAD == 1

/*------------------ Sleep ------------------*/
z_result_t z_sleep_us(size_t time) {

    // systick_interval_set(TX_TIMER_TICKS_PER_SECOND);
  // TODO tx sees time in ticks, how about zenoh?
  _Z_CHECK_SYS_ERR(tx_thread_sleep(time));
}

z_result_t z_sleep_ms(size_t time) {
   

  _Z_CHECK_SYS_ERR(tx_thread_sleep(time));
  
    z_time_t start = z_time_now();

    // Most sleep APIs promise to sleep at least whatever you asked them to.
    // This may compound, so this approach may make sleeps longer than expected.
    // This extra check tries to minimize the amount of extra time it might sleep.
    while (z_time_elapsed_ms(&start) < time) {
        z_result_t ret = z_sleep_us(1000);
        if (ret != _Z_RES_OK) {
            return ret;
        }
    }

    return _Z_RES_OK;
}

z_result_t z_sleep_s(size_t time) { _Z_CHECK_SYS_ERR((int)sleep((unsigned int)time)); }

/*------------------ Instant ------------------*/
z_clock_t z_clock_now(void) {
    // z_clock_t now =  {
    
    // }z_clock_t;
    // clock_gettime(CLOCK_MONOTONIC, &now);
    return 0;
}

unsigned long z_clock_elapsed_us(z_clock_t *instant) {
    // z_clock_t now;
    // clock_gettime(CLOCK_MONOTONIC, &now);

    // unsigned long elapsed =
        // (unsigned long)(1000000 * (now.tv_sec - instant->tv_sec) + (now.tv_nsec - instant->tv_nsec) / 1000);
    return 0;
}

unsigned long z_clock_elapsed_ms(z_clock_t *instant) {
    // z_clock_t now;
    // clock_gettime(CLOCK_MONOTONIC, &now);

    // unsigned long elapsed =
        // (unsigned long)(1000 * (now.tv_sec - instant->tv_sec) + (now.tv_nsec - instant->tv_nsec) / 1000000);
    return 0;
}

unsigned long z_clock_elapsed_s(z_clock_t *instant) {
    // z_clock_t now;
    // clock_gettime(CLOCK_MONOTONIC, &now);

    // unsigned long elapsed = (unsigned long)(now.tv_sec - instant->tv_sec);
    return 0;
}

/*------------------ Time ------------------*/
z_time_t z_time_now(void) {
    // gettimeofday(&now, NULL);
    return 0;
}

const char *z_time_now_as_str(char *const buf, unsigned long buflen) {
    // z_time_t tv = z_time_now();
    // struct tm ts;
    // ts = *localtime(&tv.tv_sec);
    // strftime(buf, buflen, "%Y-%m-%dT%H:%M:%SZ", &ts);
    return "0.0";
}

unsigned long z_time_elapsed_us(z_time_t *time) {
    // z_time_t now;
    // gettimeofday(&now, NULL);

    // unsigned long elapsed = (unsigned long)(1000000 * (now.tv_sec - time->tv_sec) + (now.tv_usec - time->tv_usec));
    // return elapsed;
    return 0;
}

unsigned long z_time_elapsed_ms(z_time_t *time) {
    // z_time_t now;
    // gettimeofday(&now, NULL);

    // unsigned long elapsed = (unsigned long)(1000 * (now.tv_sec - time->tv_sec) + (now.tv_usec - time->tv_usec) / 1000);
    // return elapsed;
    return 0;
}

unsigned long z_time_elapsed_s(z_time_t *time) {
    // z_time_t now;
    // gettimeofday(&now, NULL);

    // unsigned long elapsed = (unsigned long)(now.tv_sec - time->tv_sec);
    // return elapsed;
    return 0;
}

z_result_t _z_get_time_since_epoch(_z_time_since_epoch *t) {
    // z_time_t now;
    // gettimeofday(&now, NULL);
    // t->secs = (uint32_t)now.tv_sec;
    // t->nanos = (uint32_t)now.tv_usec * 1000;
    return 0;
}
