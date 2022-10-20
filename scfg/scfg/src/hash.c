// hash.c

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <blake3.h>
#include <inc/scfg.h>
#include <openssl/evp.h>
#include <openssl/types.h>
#include <openssl/whrlpool.h>
#include <scfgpch.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define _BLAKE3_HASH_SIZE    32 // 256-bit hash
#define _SHA512_HASH_SIZE    64 // 512-bit hash
#define _WHIRLPOOL_HASH_SIZE 64 // 512-bit hash

// FUNCTION _Cleanup_openssl_on_failure
inline scfg_error_t _SCFG_CONV _Cleanup_openssl_on_failure(EVP_MD_CTX* _Ctx) {
    EVP_MD_CTX_free(_Ctx); // release context
    return scfg_error_general_failure;
}

// FUNCTION _Hash_blake3
inline scfg_error_t _SCFG_CONV _Hash_blake3(
    const void* const _Data, const size_t _Size, scfg_buffer_t* const _Buf) {
    if (!_Buf) {
        return scfg_error_invalid_buffer;
    }

    if (_Buf->_Size < _BLAKE3_HASH_SIZE) {
        return scfg_error_buffer_too_small;
    }

    uint8_t* _Ptr;
    const scfg_error_t _Err = scfg_get_associated_buffer(_Buf, &_Ptr);
    if (_Err != scfg_error_success) {
        return _Err;
    }

    blake3_hasher _Hasher;
    blake3_hasher_init(&_Hasher);
    blake3_hasher_update(&_Hasher, _Data, _Size);
    blake3_hasher_finalize(&_Hasher, _Ptr, _BLAKE3_HASH_SIZE);
    return scfg_error_success;
}

// FUNCTION _Hash_file_blake3
inline scfg_error_t _SCFG_CONV _Hash_file_blake3(
    FILE* const _Stream, const size_t _Off, scfg_buffer_t* const _Buf) {
    if (!_Stream) {
        return scfg_error_invalid_stream;
    }

    if (!_Buf) {
        return scfg_error_invalid_buffer;
    }

    uint8_t* _Ptr;
    const scfg_error_t _Err = scfg_get_associated_buffer(_Buf, &_Ptr);
    if (_Err != scfg_error_success) {
        return _Err;
    }

    if (fseek(_Stream, (long) _Off, SEEK_SET) != 0) { // offset may be too large for the selected file
        return scfg_error_invalid_stream;
    }

    uint8_t _Temp_buf[1024];
    size_t _Read = 0; // read bytes
    blake3_hasher _Hasher;
    blake3_hasher_init(&_Hasher);
    for (;;) {
        _Read = fread(_Temp_buf, sizeof(uint8_t), 1024, _Stream);
        if (_Read == 0) { // no more data
            break;
        }

        blake3_hasher_update(&_Hasher, _Temp_buf, _Read);
        if (_Read < 1024) { // no more data
            break;
        }
    }

    blake3_hasher_finalize(&_Hasher, _Ptr, _BLAKE3_HASH_SIZE);
    return scfg_error_success;
}

// FUNCTION _Hash_sha512
inline scfg_error_t _SCFG_CONV _Hash_sha512(
    const void* const _Data, const size_t _Size, scfg_buffer_t* const _Buf) {
    if (!_Buf) {
        return scfg_error_invalid_buffer;
    }

    if (_Buf->_Size < _SHA512_HASH_SIZE) {
        return scfg_error_buffer_too_small;
    }

    uint8_t* _Ptr;
    const scfg_error_t _Err = scfg_get_associated_buffer(_Buf, &_Ptr);
    if (_Err != scfg_error_success) {
        return _Err;
    }

    EVP_MD_CTX* _Ctx = EVP_MD_CTX_new();
    if (!_Ctx) {
        return scfg_error_not_enough_memory;
    }

    if (EVP_DigestInit_ex(_Ctx, EVP_sha3_512(), NULL) != 1) {
        return _Cleanup_openssl_on_failure(_Ctx);
    }

    if (EVP_DigestUpdate(_Ctx, _Data, _Size) != 1) {
        return _Cleanup_openssl_on_failure(_Ctx);
    }

    int _Len = 0; // hash size (unused)
    if (EVP_DigestFinal_ex(_Ctx, _Ptr, &_Len) != 1) {
        return _Cleanup_openssl_on_failure(_Ctx);
    }

    EVP_MD_CTX_free(_Ctx);
    return scfg_error_success;
}

// FUNCTION _Hash_file_sha512
inline scfg_error_t _SCFG_CONV _Hash_file_sha512(
    FILE* const _Stream, const size_t _Off, scfg_buffer_t* const _Buf) {
    if (!_Stream) {
        return scfg_error_invalid_stream;
    }

    if (!_Buf) {
        return scfg_error_invalid_buffer;
    }

    uint8_t* _Ptr;
    const scfg_error_t _Err = scfg_get_associated_buffer(_Buf, &_Ptr);
    if (_Err != scfg_error_success) {
        return _Err;
    }

    if (fseek(_Stream, (long) _Off, SEEK_SET) != 0) { // offset may be too large for the selected file
        return scfg_error_invalid_stream;
    }

    uint8_t _Temp_buf[1024];
    size_t _Read     = 0; // read bytes
    EVP_MD_CTX* _Ctx = EVP_MD_CTX_new();
    if (!_Ctx) {
        return scfg_error_not_enough_memory;
    }

    if (EVP_DigestInit_ex(_Ctx, EVP_sha3_512(), NULL) != 1) {
        return _Cleanup_openssl_on_failure(_Ctx);
    }

    for (;;) {
        _Read = fread(_Temp_buf, sizeof(uint8_t), 1024, _Stream);
        if (_Read == 0) { // no more data
            break;
        }

        if (EVP_DigestUpdate(_Ctx, _Temp_buf, _Read) != 1) {
            return _Cleanup_openssl_on_failure(_Ctx);
        }

        if (_Read < 1024) { // no more data
            break;
        }
    }

    int _Len = 0; // hash size (unused)
    if (EVP_DigestFinal_ex(_Ctx, _Ptr, &_Len) != 1) {
        return _Cleanup_openssl_on_failure(_Ctx);
    }

    EVP_MD_CTX_free(_Ctx);
    return scfg_error_success;
}

#pragma warning(push, 3)
#pragma warning(disable : 4996) // C4996: WHIRLPOOL_Init(), WHIRLPOOL_Update() and WHIRLPOOL_Final()
                                //        since OpenSSL 3.0
// FUNCTION _Hash_whirlpool
inline scfg_error_t _SCFG_CONV _Hash_whirlpool(
    const void* const _Data, const size_t _Size, scfg_buffer_t* const _Buf) {
    if (!_Buf) {
        return scfg_error_invalid_buffer;
    }

    if (_Buf->_Size < _WHIRLPOOL_HASH_SIZE) {
        return scfg_error_buffer_too_small;
    }

    uint8_t* _Ptr;
    const scfg_error_t _Err = scfg_get_associated_buffer(_Buf, &_Ptr);
    if (_Err != scfg_error_success) {
        return _Err;
    }

    WHIRLPOOL_CTX _Ctx;
    if (WHIRLPOOL_Init(&_Ctx) != 1) {
        return scfg_error_general_failure;
    }

    if (WHIRLPOOL_Update(&_Ctx, _Data, _Size) != 1) {
        return scfg_error_general_failure;
    }

    if (WHIRLPOOL_Final(_Ptr, &_Ctx) != 1) {
        return scfg_error_general_failure;
    }

    return scfg_error_success;
}

// FUNCTION _Hash_file_whirlpool
inline scfg_error_t _SCFG_CONV _Hash_file_whirlpool(
    FILE* const _Stream, const size_t _Off, scfg_buffer_t* const _Buf) {
    if (!_Stream) {
        return scfg_error_invalid_stream;
    }

    if (!_Buf) {
        return scfg_error_invalid_buffer;
    }

    uint8_t* _Ptr;
    const scfg_error_t _Err = scfg_get_associated_buffer(_Buf, &_Ptr);
    if (_Err != scfg_error_success) {
        return _Err;
    }

    if (fseek(_Stream, (long) _Off, SEEK_SET) != 0) { // offset may be too large for the selected file
        return scfg_error_invalid_stream;
    }

    uint8_t _Temp_buf[1024];
    size_t _Read = 0; // read bytes
    WHIRLPOOL_CTX _Ctx;
    if (WHIRLPOOL_Init(&_Ctx) != 1) {
        return scfg_error_general_failure;
    }

    for (;;) {
        _Read = fread(_Temp_buf, sizeof(uint8_t), 1024, _Stream);
        if (_Read == 0) { // no more data
            break;
        }

        if (WHIRLPOOL_Update(&_Ctx, _Temp_buf, _Read) != 1) {
            return scfg_error_general_failure;
        }

        if (_Read < 1024) { // no more data
            break;
        }
    }

    if (WHIRLPOOL_Final(_Ptr, &_Ctx) != 1) {
        return scfg_error_general_failure;
    }

    return scfg_error_success;
}
#pragma warning(pop)

// FUNCTION scfg_is_valid_hash_id
int _SCFG_CONV scfg_is_valid_hash_id(const scfg_hash_id_t _Id) {
    switch (_Id) {
    case scfg_hash_id_blake3:
    case scfg_hash_id_sha512:
    case scfg_hash_id_whirlpool:
        return 1;
    default:
        return 0;
    }
}

// FUNCTION scfg_hash
scfg_error_t _SCFG_CONV scfg_hash(
    const void* const _Data, const size_t _Size, const scfg_hash_id_t _Id, scfg_buffer_t* const _Buf) {
    switch (_Id) {
    case scfg_hash_id_blake3:
        return _Hash_blake3(_Data, _Size, _Buf);
    case scfg_hash_id_sha512:
        return _Hash_sha512(_Data, _Size, _Buf);
    case scfg_hash_id_whirlpool:
        return _Hash_whirlpool(_Data, _Size, _Buf);
    default:
        return scfg_error_unsupported_hash;
    }
}

// FUNCTION scfg_hash_file
scfg_error_t _SCFG_CONV scfg_hash_file(
    FILE* const _Stream, const size_t _Off, const scfg_hash_id_t _Id, scfg_buffer_t* const _Buf) {
    switch (_Id) {
    case scfg_hash_id_blake3:
        return _Hash_file_blake3(_Stream, _Off, _Buf);
    case scfg_hash_id_sha512:
        return _Hash_file_sha512(_Stream, _Off, _Buf);
    case scfg_hash_id_whirlpool:
        return _Hash_file_whirlpool(_Stream, _Off, _Buf);
    default:
        return scfg_error_unsupported_hash;
    }
}