//
// Copyright (c) 2022 ZettaScale Technology
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
//

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "zenoh-pico/api/types.h"
#include "zenoh-pico/config.h"
#include "zenoh-pico/net/query.h"
#include "zenoh-pico/protocol/core.h"
#include "zenoh-pico/protocol/definitions/network.h"
#include "zenoh-pico/protocol/keyexpr.h"
#include "zenoh-pico/session/resource.h"
#include "zenoh-pico/session/utils.h"
#include "zenoh-pico/utils/logging.h"
#include "zenoh-pico/utils/pointers.h"
#include "zenoh-pico/utils/string.h"

#if Z_FEATURE_QUERYABLE == 1

#define _Z_QLEINFOS_VEC_SIZE 4  // Arbitrary initial size

#if Z_FEATURE_RX_CACHE == 1
static inline bool _z_queryable_get_from_cache(_z_session_t *zn, const _z_keyexpr_t *ke, _z_keyexpr_t *ke_val,
                                               _z_queryable_infos_svec_t *infos_val, size_t *qle_nb) {
    if (!_z_keyexpr_equals(ke, &zn->_queryable_cache.ke_in)) {
        return false;
    }
    *ke_val = _z_keyexpr_alias(zn->_queryable_cache.ke_out);
    *infos_val = _z_queryable_infos_svec_alias(&zn->_queryable_cache.infos);
    *qle_nb = zn->_queryable_cache.qle_nb;
    return true;
}

static inline void _z_queryable_update_cache(_z_session_t *zn, const _z_keyexpr_t *ke_in, const _z_keyexpr_t *ke_out,
                                             _z_queryable_infos_svec_t *infos) {
    // Clear previous data
    _z_queryable_cache_clear(&zn->_queryable_cache);
    // Register new info
    zn->_queryable_cache.ke_in = _z_keyexpr_duplicate(ke_in);
    zn->_queryable_cache.ke_out = _z_keyexpr_duplicate(ke_out);
    zn->_queryable_cache.infos = _z_queryable_infos_svec_alias(infos);
    zn->_queryable_cache.qle_nb = _z_queryable_infos_svec_len(infos);
}

void _z_queryable_cache_clear(_z_queryable_cache_t *cache) {
    _z_queryable_infos_svec_clear(&cache->infos);
    _z_keyexpr_clear(&cache->ke_in);
    _z_keyexpr_clear(&cache->ke_out);
}

#else
static inline bool _z_queryable_get_from_cache(_z_session_t *zn, const _z_keyexpr_t *ke, _z_keyexpr_t *ke_val,
                                               _z_queryable_infos_svec_t *infos_val, size_t *sub_nb) {
    _ZP_UNUSED(zn);
    _ZP_UNUSED(ke);
    _ZP_UNUSED(ke_val);
    _ZP_UNUSED(infos_val);
    _ZP_UNUSED(sub_nb);
    return false;
}

static inline void _z_queryable_update_cache(_z_session_t *zn, const _z_keyexpr_t *ke_in, const _z_keyexpr_t *ke_out,
                                             _z_queryable_infos_svec_t *infos) {
    _ZP_UNUSED(zn);
    _ZP_UNUSED(ke_in);
    _ZP_UNUSED(ke_out);
    _ZP_UNUSED(infos);
    return;
}
#endif  // Z_FEATURE_RX_CACHE == 1

bool _z_session_queryable_eq(const _z_session_queryable_t *one, const _z_session_queryable_t *two) {
    return one->_id == two->_id;
}

void _z_session_queryable_clear(_z_session_queryable_t *qle) {
    if (qle->_dropper != NULL) {
        qle->_dropper(qle->_arg);
    }
    _z_keyexpr_clear(&qle->_key);
}

/*------------------ Queryable ------------------*/
static _z_session_queryable_rc_t *__z_get_session_queryable_by_id(_z_session_queryable_rc_list_t *qles,
                                                                  const _z_zint_t id) {
    _z_session_queryable_rc_t *ret = NULL;

    _z_session_queryable_rc_list_t *xs = qles;
    while (xs != NULL) {
        _z_session_queryable_rc_t *qle = _z_session_queryable_rc_list_head(xs);
        if (id == _Z_RC_IN_VAL(qle)->_id) {
            ret = qle;
            break;
        }

        xs = _z_session_queryable_rc_list_tail(xs);
    }

    return ret;
}

/**
 * This function is unsafe because it operates in potentially concurrent data.
 * Make sure that the following mutexes are locked before calling this function:
 *  - zn->_mutex_inner
 */
static _z_session_queryable_rc_t *__unsafe_z_get_session_queryable_by_id(_z_session_t *zn, const _z_zint_t id) {
    _z_session_queryable_rc_list_t *qles = zn->_local_queryable;
    return __z_get_session_queryable_by_id(qles, id);
}

/**
 * This function is unsafe because it operates in potentially concurrent data.
 * Make sure that the following mutexes are locked before calling this function:
 *  - zn->_mutex_inner
 */
static z_result_t __unsafe_z_get_session_queryable_by_key(_z_session_t *zn, const _z_keyexpr_t *key,
                                                          _z_queryable_infos_svec_t *qle_infos) {
    _z_session_queryable_rc_list_t *qles = zn->_local_queryable;

    *qle_infos = _z_queryable_infos_svec_make(_Z_QLEINFOS_VEC_SIZE);
    _z_session_queryable_rc_list_t *xs = qles;
    while (xs != NULL) {
        // Parse queryable list
        _z_session_queryable_rc_t *qle = _z_session_queryable_rc_list_head(xs);
        if (_z_keyexpr_suffix_intersects(&_Z_RC_IN_VAL(qle)->_key, key)) {
            _z_queryable_infos_t new_qle_info = {.arg = _Z_RC_IN_VAL(qle)->_arg,
                                                 .callback = _Z_RC_IN_VAL(qle)->_callback};
            _Z_RETURN_IF_ERR(_z_queryable_infos_svec_append(qle_infos, &new_qle_info, false));
        }
        xs = _z_session_queryable_rc_list_tail(xs);
    }
    return _Z_RES_OK;
}

_z_session_queryable_rc_t *_z_get_session_queryable_by_id(_z_session_t *zn, const _z_zint_t id) {
    _z_session_mutex_lock(zn);

    _z_session_queryable_rc_t *qle = __unsafe_z_get_session_queryable_by_id(zn, id);

    _z_session_mutex_unlock(zn);

    return qle;
}

_z_session_queryable_rc_t *_z_register_session_queryable(_z_session_t *zn, _z_session_queryable_t *q) {
    _Z_DEBUG(">>> Allocating queryable for (%ju:%.*s)", (uintmax_t)q->_key._id, (int)_z_string_len(&q->_key._suffix),
             _z_string_data(&q->_key._suffix));
    _z_session_queryable_rc_t *ret = NULL;

    _z_session_mutex_lock(zn);

    ret = (_z_session_queryable_rc_t *)z_malloc(sizeof(_z_session_queryable_rc_t));
    if (ret != NULL) {
        *ret = _z_session_queryable_rc_new_from_val(q);
        zn->_local_queryable = _z_session_queryable_rc_list_push(zn->_local_queryable, ret);
    }

    _z_session_mutex_unlock(zn);

    return ret;
}

static z_result_t _z_session_queryable_get_infos(_z_session_t *zn, const _z_keyexpr_t *keyexpr, _z_keyexpr_t *key,
                                                 _z_queryable_infos_svec_t *qles, size_t *qle_nb) {
    // Check cache
    if (!_z_queryable_get_from_cache(zn, keyexpr, key, qles, qle_nb)) {
        _Z_DEBUG("Resolving %d - %.*s on mapping 0x%x", keyexpr->_id, (int)_z_string_len(&keyexpr->_suffix),
                 _z_string_data(&keyexpr->_suffix), _z_keyexpr_mapping_id(keyexpr));
        _z_session_mutex_lock(zn);
        *key = __unsafe_z_get_expanded_key_from_key(zn, keyexpr, true);

        if (!_z_keyexpr_has_suffix(key)) {
            _z_session_mutex_unlock(zn);
            return _Z_ERR_KEYEXPR_UNKNOWN;
        }
        // Get queryable list
        z_result_t ret = __unsafe_z_get_session_queryable_by_key(zn, key, qles);
        _z_session_mutex_unlock(zn);
        if (ret != _Z_RES_OK) {
            return ret;
        }
        *qle_nb = _z_queryable_infos_svec_len(qles);
        // Update cache
        _z_queryable_update_cache(zn, keyexpr, key, qles);
    }
    return _Z_RES_OK;
}

static z_result_t _z_trigger_queryables_inner(_z_session_rc_t *zsrc, _z_msg_query_t *msgq, const _z_keyexpr_t *q_key,
                                              uint32_t qid) {
    _z_session_t *zn = _Z_RC_IN_VAL(zsrc);
    _z_keyexpr_t key;
    _z_queryable_infos_svec_t qles;
    size_t qle_nb;
    // Retrieve sub infos
    _Z_RETURN_IF_ERR(_z_session_queryable_get_infos(zn, q_key, &key, &qles, &qle_nb));
    // Check if there are queryables
    _Z_DEBUG("Triggering %ju queryables for key %d - %.*s", (uintmax_t)qle_nb, key._id,
             (int)_z_string_len(&key._suffix), _z_string_data(&key._suffix));
    if (qle_nb == 0) {
        _z_keyexpr_clear(&key);
        return _Z_RES_OK;
    }
    // Check anyke
    char *slice_end = _z_ptr_char_offset((char *)msgq->_parameters.start, (ptrdiff_t)msgq->_parameters.len);
    bool anyke = false;
    if (_z_slice_check(&msgq->_parameters)) {
        if (_z_strstr((char *)msgq->_parameters.start, slice_end, Z_SELECTOR_QUERY_MATCH) != NULL) {
            anyke = true;
        }
    }
    // Build the z_query
    _z_query_t query =
        _z_query_alias(&msgq->_ext_value, &key, &msgq->_parameters, zsrc, qid, &msgq->_ext_attachment, anyke);
    // Parse session_queryable svec
    for (size_t i = 0; i < qle_nb; i++) {
        _z_queryable_infos_t *qle_info = _z_queryable_infos_svec_get(&qles, i);
        qle_info->callback(&query, qle_info->arg);
    }
    // Send reply final message
    _z_query_send_reply_final(&query);
    // Clean up
    _z_keyexpr_clear(&key);
#if Z_FEATURE_RX_CACHE != 1
    _z_queryable_infos_svec_release(&qles);  // Otherwise it's released with cache
#endif
    return _Z_RES_OK;
}

z_result_t _z_trigger_queryables(_z_session_rc_t *zsrc, _z_msg_query_t *msgq, _z_keyexpr_t *q_key, uint32_t qid) {
    z_result_t ret = _z_trigger_queryables_inner(zsrc, msgq, q_key, qid);
    // Clean up
    _z_keyexpr_clear(q_key);
    _z_encoding_clear(&msgq->_ext_value.encoding);
    _z_bytes_drop(&msgq->_ext_value.payload);
    _z_bytes_drop(&msgq->_ext_attachment);
    _z_slice_clear(&msgq->_parameters);
    return ret;
}

void _z_unregister_session_queryable(_z_session_t *zn, _z_session_queryable_rc_t *qle) {
    _z_session_mutex_lock(zn);

    zn->_local_queryable =
        _z_session_queryable_rc_list_drop_filter(zn->_local_queryable, _z_session_queryable_rc_eq, qle);

    _z_session_mutex_unlock(zn);
}

void _z_flush_session_queryable(_z_session_t *zn) {
    _z_session_mutex_lock(zn);

    _z_session_queryable_rc_list_free(&zn->_local_queryable);

    _z_session_mutex_unlock(zn);
}

#endif
