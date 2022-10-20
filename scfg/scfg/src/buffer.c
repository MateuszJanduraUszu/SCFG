// buffer.c

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <inc/scfg.h>
#include <scfgpch.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// FUNCTION _Resize_buffer_using_sbo
inline scfg_error_t _SCFG_CONV _Resize_buffer_using_sbo(
    scfg_buffer_t* const _Buf, const size_t _New_size, const scfg_allocator_t* const _Al) {
    if (_New_size <= _SCFG_SMALL_BUFFER_SIZE) { // SBO is still available
        if (_New_size < _Buf->_Size) { // decrease the buffer, fill the rest of the buffer with zeros
            memset(_Buf->_Buf + _New_size, 0, _SCFG_SMALL_BUFFER_SIZE - _New_size);
        }

        _Buf->_Size = _New_size;
    } else { // SBO is no longer available
        uint8_t* const _New_ptr = _Al ? (uint8_t*) _Al->allocate(_New_size) : (uint8_t*) malloc(_New_size);
        if (!_New_ptr) {
            return scfg_error_not_enough_memory;
        }

        _Buf->_Size = _New_size;
#pragma warning(suppress : 6385) // C6385: Reading invalid data
        memcpy(_New_ptr, _Buf->_Buf, _New_size);
        memset(_Buf->_Buf, 0, _SCFG_SMALL_BUFFER_SIZE);
        _Buf->_Ptr = _New_ptr; // assign a new buffer
    }

    return scfg_error_success;
}

// FUNCTION _Resize_buffer_not_using_sbo
inline scfg_error_t _SCFG_CONV _Resize_buffer_not_using_sbo(
    scfg_buffer_t* const _Buf, const size_t _New_size, const scfg_allocator_t* const _Al) {
    if (_New_size > _SCFG_SMALL_BUFFER_SIZE) { // SBO is still not available
        uint8_t* const _New_ptr = _Al ? (uint8_t*) _Al->allocate(_New_size) : (uint8_t*) malloc(_New_size);
        if (!_New_ptr) {
            return scfg_error_not_enough_memory;
        }

        memcpy(_New_ptr, _Buf->_Ptr, _SCFG_MIN(_Buf->_Size, _New_size));
        if (_New_size > _Buf->_Size) { // fill the rest of the buffer with zeros
            memset(_New_ptr + _Buf->_Size, 0, _New_size - _Buf->_Size);
        }

        if (_Al) { // use a custom allocator
            _Al->deallocate(_Buf->_Ptr);
        } else { // use a default allocator
            free(_Buf->_Ptr);
        }

        _Buf->_Ptr  = _New_ptr; // assign a new buffer
        _Buf->_Size = _New_size;
    } else { // SBI is now available
        uint8_t _Temp_buf[_SCFG_SMALL_BUFFER_SIZE]; // intentionally uninitialized
        const size_t _Min_size = _SCFG_MIN(_Buf->_Size, _New_size);
        memcpy(_Temp_buf, _Buf->_Ptr, _Min_size);
        if (_Al) { // use a custom allocator
            _Al->deallocate(_Buf->_Ptr);
        } else { // use a default allocator
            free(_Buf->_Ptr);
        }

        _Buf->_Ptr  = NULL;
        _Buf->_Size = _New_size;
        memcpy(_Buf->_Buf, _Temp_buf, _Min_size);
    }

    return scfg_error_success;
}

// FUNCTION scfg_initialize_buffer
scfg_error_t _SCFG_CONV scfg_initialize_buffer(scfg_buffer_t* const _Buf) {
    if (!_Buf) {
        return scfg_error_invalid_buffer;
    }

    // Note: Use a small buffer by default.
    _Buf->_Ptr  = NULL;
    _Buf->_Size = 0;
    memset(_Buf->_Buf, 0, _SCFG_SMALL_BUFFER_SIZE);
    return scfg_error_success;
}

// FUNCTION scfg_get_associated_buffer
scfg_error_t _SCFG_CONV scfg_get_associated_buffer(scfg_buffer_t* const _Buf, uint8_t** _Ptr) {
    if (!_Buf) {
        return scfg_error_invalid_buffer;
    }

    if (_Buf->_Size <= _SCFG_SMALL_BUFFER_SIZE) { // use a small buffer
        *_Ptr = _Buf->_Buf;
    } else { // use a larger buffer
        *_Ptr = _Buf->_Ptr;
    }

    return scfg_error_success;
}

// FUNCTION scfg_resize_buffer
scfg_error_t _SCFG_CONV scfg_resize_buffer(
    scfg_buffer_t* const _Buf, const size_t _New_size, const scfg_allocator_t* const _Al) {
    if (!_Buf) {
        return scfg_error_invalid_buffer;
    }

    if (_New_size == _Buf->_Size) { // nothing has changed
        return scfg_error_success;
    } else if (_New_size == 0) { // release the buffer
        return scfg_release_buffer(_Buf, _Al);
    } else if (_Buf->_Size == 0) { // allocate or initialize a new buffer
        if (_New_size <= _SCFG_SMALL_BUFFER_SIZE) { // prepare a small buffer
            _Buf->_Size = _New_size;
            memset(_Buf->_Buf, 0, _SCFG_SMALL_BUFFER_SIZE); // fill the buffer with zeros
        } else { // prepare a larger buffer
            _Buf->_Ptr = _Al ? (uint8_t*) _Al->allocate(_New_size) : (uint8_t*) malloc(_New_size);
            if (!_Buf->_Ptr) {
                return scfg_error_not_enough_memory;
            }

            _Buf->_Size = _New_size;
            memset(_Buf->_Ptr, 0, _New_size); // fill the buffer with zeros
        }

        return scfg_error_success;
    } else { // increase or decrease an old buffer (may change the buffer location)
        if (_Buf->_Size <= _SCFG_SMALL_BUFFER_SIZE) {
            return _Resize_buffer_using_sbo(_Buf, _New_size, _Al);
        } else {
            return _Resize_buffer_not_using_sbo(_Buf, _New_size, _Al);
        }
    }
}

// FUNCTION scfg_release_buffer
scfg_error_t _SCFG_CONV scfg_release_buffer(scfg_buffer_t* const _Buf, const scfg_allocator_t* const _Al) {
    if (!_Buf) {
        return scfg_error_invalid_buffer;
    }

    if (_Buf->_Size <= _SCFG_SMALL_BUFFER_SIZE) { // release a small buffer
        memset(_Buf->_Buf, 0, _SCFG_SMALL_BUFFER_SIZE);
    } else {
        if (_Al) { // use a custom allocator
            _Al->deallocate(_Buf->_Ptr);
        } else { // use a default allocator
            free(_Buf->_Ptr);
        }
    }

    _Buf->_Ptr  = NULL;
    _Buf->_Size = 0;
    return scfg_error_success;
}