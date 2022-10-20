// integer.c

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <inc/scfg.h>
#include <scfgpch.h>
#include <stddef.h>
#include <stdint.h>

// FUNCTION scfg_pack_uint32
scfg_error_t _SCFG_CONV scfg_pack_uint32(const uint8_t* const _Bytes, uint32_t* const _Val) {
    if (!_Val) {
        return scfg_error_invalid_buffer;
    }

    if (!_Bytes) {
        return scfg_error_invalid_data;
    }
    
    memcpy(_Val, _Bytes, sizeof(uint32_t));
    return scfg_error_success;
}

// FUNCTION scfg_unpack_uint32
scfg_error_t _SCFG_CONV scfg_unpack_uint32(uint8_t* const _Bytes, const uint32_t _Val) {
    if (!_Bytes) {
        return scfg_error_invalid_buffer;
    }

    memcpy(_Bytes, &_Val, sizeof(uint32_t));
    return scfg_error_success;
}