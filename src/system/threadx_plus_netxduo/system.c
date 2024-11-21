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

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS_IP.h"
#include "zenoh-pico/config.h"
#include "zenoh-pico/system/platform.h"


#include "tx_api.h"
#include "tx_byte_pool.h"

/*------------------ Random ------------------*/


// from FreeRTOS/FreeRTOS-Plus-TCP/source/include/FreeRTOS_IP.h
/* This xApplicationGetRandomNumber() will set *pulNumber to a random number,
 * and return pdTRUE. When the random number generator is broken, it shall return
 * pdFALSE.
 * The function is defined in 'iot_secure_sockets.c'.
 * If that module is not included in the project, the application must provide an
 * implementation of it.
 * The macro's ipconfigRAND32() and configRAND32() are not in use anymore. */

/* "xApplicationGetRandomNumber" is declared but never defined, because it may
 * be defined in a user module. */
// BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber );

// from FreeRTOS/FreeRTOS-Plus-TCP/test/build-combination/Common/main.c , types modified
uint32_t uxRand( void )
{
    const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

    /* Utility function to generate a pseudo random number. */

    ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
    return( ( int ) ( ulNextRand ) & 0x7fffUL );
}

void xApplicationGetRandomNumber( uint32_t * pulNumber )
{
    *pulNumber = ( uint32_t ) uxRand();

    return pdTRUE;
}

uint8_t z_random_u8(void) { return z_random_u32(); }

uint16_t z_random_u16(void) { return z_random_u32(); }

uint32_t z_random_u32(void) {
    uint32_t ret = 0;
    xApplicationGetRandomNumber(&ret);
    return ret;
}

uint64_t z_random_u64(void) {
    uint64_t ret = 0;
    ret |= z_random_u32();
    ret = ret << 32;
    ret |= z_random_u32();
    return ret;
}

void z_random_fill(void *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        *((uint8_t *)buf) = z_random_u8();
    }
}

/*------------------ Memory ------------------*/
// see https://embeddedartistry.com/blog/2017/02/17/implementing-malloc-with-threadx/

// ThreadX internal memory pool stucture
static TX_BYTE_POOL malloc_pool_ = {0};

// this is not zenoh standard
int z_malloc_init(uintptr_t heap_start, size_t heap_size)
{
    uint8_t r;

    r = tx_byte_pool_create(&malloc_pool_, "BMP",
            (void *)heap_start,
            heap_size);

    return (r == TX_SUCCESS) ? 0 : -1;  
}

void *z_malloc(size_t size) { 
    void * ptr = NULL;

    if(size > 0)
    {
        // TODO: is there a way to check if the byte pool is initialized/created properly
        uint8_t r = tx_byte_allocate(&malloc_pool_, &ptr, size,
            TX_WAIT_FOREVER);

        if(r != TX_SUCCESS)
        {
            ptr = NULL;
        }
    } 
    return ptr;
}

void *z_realloc(void *ptr, size_t size) {
    // realloc  not implemented in ThreadX with netXDuo port
    return NULL;
}

void z_free(void *ptr) { 
    if(ptr) {
        uint8_t r = tx_byte_release(ptr);
    }
}


#if Z_FEATURE_MULTI_THREAD == 1
// In FreeRTOS, tasks created using xTaskCreate must end with vTaskDelete.
// A task function should __not__ simply return.
typedef struct {
    void *(*fun)(void *);
    void *arg;
    EventGroupHandle_t join_event;
} z_task_arg;

static void z_task_wrapper(void *arg) {
    z_task_arg *targ = (z_task_arg *)arg;
    targ->fun(targ->arg);
    xEventGroupSetBits(targ->join_event, 1);
    vTaskDelete(NULL);
}

static z_task_attr_t z_default_task_attr = {
    .name = "",
    .priority = configMAX_PRIORITIES / 2,
    .stack_depth = 5120,
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    .static_allocation = false,
    .stack_buffer = NULL,
    .task_buffer = NULL,
#endif /* SUPPORT_STATIC_ALLOCATION */
};

/*------------------ Thread ------------------*/
z_result_t _z_task_init(_z_task_t *task, z_task_attr_t *attr, void *(*fun)(void *), void *arg) {
    z_task_arg *z_arg = (z_task_arg *)z_malloc(sizeof(z_task_arg));
    if (z_arg == NULL) {
        return -1;
    }

    z_arg->fun = fun;
    z_arg->arg = arg;
    z_arg->join_event = task->join_event = xEventGroupCreate();

    if (attr == NULL) {
        attr = &z_default_task_attr;
    }

#if (configSUPPORT_STATIC_ALLOCATION == 1)
    if (attr->static_allocation) {
        task->handle = xTaskCreateStatic(z_task_wrapper, attr->name, attr->stack_depth, z_arg, attr->priority,
                                         attr->stack_buffer, attr->task_buffer);
        if (task->handle == NULL) {
            return -1;
        }
    } else {
#endif /* SUPPORT_STATIC_ALLOCATION */
        if (xTaskCreate(z_task_wrapper, attr->name, attr->stack_depth, z_arg, attr->priority, &task->handle) !=
            pdPASS) {
            return -1;
        }
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    }
#endif /* SUPPORT_STATIC_ALLOCATION */

    return 0;
}

z_result_t _z_task_join(_z_task_t *task) {
    xEventGroupWaitBits(task->join_event, 1, pdFALSE, pdFALSE, portMAX_DELAY);
    return 0;
}

z_result_t _z_task_detach(_z_task_t *task) {
    // Not implemented
    return _Z_ERR_GENERIC;
}

z_result_t _z_task_cancel(_z_task_t *task) {
    vTaskDelete(task->handle);
    return 0;
}

void _z_task_free(_z_task_t **task) {
    z_free((*task)->join_event);
    z_free(*task);
}

/*------------------ Mutex ------------------*/
z_result_t _z_mutex_init(_z_mutex_t *m) {
    *m = xSemaphoreCreateRecursiveMutex();
    return *m == NULL ? -1 : 0;
}

z_result_t _z_mutex_drop(_z_mutex_t *m) {
    z_free(*m);
    return 0;
}

z_result_t _z_mutex_lock(_z_mutex_t *m) { return xSemaphoreTakeRecursive(*m, portMAX_DELAY) == pdTRUE ? 0 : -1; }

z_result_t _z_mutex_try_lock(_z_mutex_t *m) { return xSemaphoreTakeRecursive(*m, 0) == pdTRUE ? 0 : -1; }

z_result_t _z_mutex_unlock(_z_mutex_t *m) { return xSemaphoreGiveRecursive(*m) == pdTRUE ? 0 : -1; }

/*------------------ CondVar ------------------*/
// Condition variables not supported in FreeRTOS
z_result_t _z_condvar_init(_z_condvar_t *cv) { return -1; }
z_result_t _z_condvar_drop(_z_condvar_t *cv) { return -1; }
z_result_t _z_condvar_signal(_z_condvar_t *cv) { return -1; }
z_result_t _z_condvar_signal_all(_z_condvar_t *cv) { return -1; }
z_result_t _z_condvar_wait(_z_condvar_t *cv, _z_mutex_t *m) { return -1; }
#endif  // Z_MULTI_THREAD == 1

/*------------------ Sleep ------------------*/
z_result_t z_sleep_us(size_t time) {
    vTaskDelay(pdMS_TO_TICKS(time / 1000));
    return 0;
}

z_result_t z_sleep_ms(size_t time) {
    vTaskDelay(pdMS_TO_TICKS(time));
    return 0;
}

z_result_t z_sleep_s(size_t time) {
    vTaskDelay(pdMS_TO_TICKS(time * 1000));
    return 0;
}

/*------------------ Clock ------------------*/
z_clock_t z_clock_now(void) { return z_time_now(); }

unsigned long z_clock_elapsed_us(z_clock_t *instant) { return z_clock_elapsed_ms(instant) * 1000; }

unsigned long z_clock_elapsed_ms(z_clock_t *instant) { return z_time_elapsed_ms(instant); }

unsigned long z_clock_elapsed_s(z_clock_t *instant) { return z_clock_elapsed_ms(instant) / 1000; }

/*------------------ Time ------------------*/
z_time_t z_time_now(void) { return xTaskGetTickCount(); }

const char *z_time_now_as_str(char *const buf, unsigned long buflen) {
    snprintf(buf, buflen, "%u", z_time_now());
    return buf;
}

unsigned long z_time_elapsed_us(z_time_t *time) { return z_time_elapsed_ms(time) * 1000; }

unsigned long z_time_elapsed_ms(z_time_t *time) {
    z_time_t now = z_time_now();

    unsigned long elapsed = (now - *time) * portTICK_PERIOD_MS;
    return elapsed;
}

unsigned long z_time_elapsed_s(z_time_t *time) { return z_time_elapsed_ms(time) / 1000; }

z_result_t _z_get_time_since_epoch(_z_time_since_epoch *t) { return -1; }