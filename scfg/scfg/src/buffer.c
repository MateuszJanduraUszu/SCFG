// buffer.c

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <inc/scfg.h>
#include <scfgpch.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

inline scfg_error_t _SCFG_CONV _Init_byte_buffer(scfg_byte_buffer_t* const _Buf) {
    // Note: Use a small buffer by default.
    _Buf->_Size = 0;
    memset(_Buf->_Small, 0, _SCFG_SMALL_BUFFER_SIZE);
    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Init_utf8_buffer(scfg_utf8_buffer_t* const _Buf) {
    // Note: Use a small buffer by default.
    _Buf->_Size = 0;
    memset(_Buf->_Small, 0, _SCFG_SMALL_BUFFER_SIZE);
    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Init_unicode_buffer(scfg_unicode_buffer_t* const _Buf) {
    // Note: Use a small buffer by default.
    _Buf->_Size = 0;
    wmemset(_Buf->_Small, 0, _SCFG_SMALL_BUFFER_SIZE / sizeof(wchar_t));
    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Get_associated_byte_buffer(
    scfg_byte_buffer_t* const _Buf, uint8_t** const _Ptr) {
    if (_Buf->_Size <= _SCFG_SMALL_BUFFER_SIZE) { // use a small buffer
        *_Ptr = _Buf->_Small;
    } else { // use a large buffer
        *_Ptr = _Buf->_Large;
    }

    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Get_associated_utf8_buffer(
    scfg_utf8_buffer_t* const _Buf, char** const _Ptr) {
    if (_Buf->_Size <= _SCFG_SMALL_BUFFER_SIZE) { // use a small buffer
        *_Ptr = _Buf->_Small;
    } else { // use a large buffer
        *_Ptr = _Buf->_Large;
    }

    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Get_associated_unicode_buffer(
    scfg_unicode_buffer_t* const _Buf, wchar_t** const _Ptr) {
    if (_Buf->_Size <= _SCFG_SMALL_BUFFER_SIZE / sizeof(wchar_t)) { // use a small buffer
        *_Ptr = _Buf->_Small;
    } else { // use a large buffer
        *_Ptr = _Buf->_Large;
    }

    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Release_byte_buffer(
    scfg_byte_buffer_t* const _Buf, const scfg_allocator_t* const _Al) {
    if (_Buf->_Size <= _SCFG_SMALL_BUFFER_SIZE) { // release a small buffer
        memset(_Buf->_Small, 0, _SCFG_SMALL_BUFFER_SIZE);
    } else { // release a large buffer
        if (_Al) { // use a custom allocator
            _Al->deallocate(_Buf->_Large);
        } else { // use a default allocator
            free(_Buf->_Large);
        }

        _Buf->_Large = NULL;
        _Buf->_Size  = 0;
    }

    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Release_utf8_buffer(
    scfg_utf8_buffer_t* const _Buf, const scfg_allocator_t* const _Al) {
    if (_Buf->_Size <= _SCFG_SMALL_BUFFER_SIZE) { // release a small buffer
        memset(_Buf->_Small, 0, _SCFG_SMALL_BUFFER_SIZE);
    } else { // release a large buffer
        if (_Al) { // use a custom allocator
            _Al->deallocate(_Buf->_Large);
        } else { // use a default allocator
            free(_Buf->_Large);
        }

        _Buf->_Large = NULL;
        _Buf->_Size  = 0;
    }

    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Release_unicode_buffer(
    scfg_unicode_buffer_t* const _Buf, const scfg_allocator_t* const _Al) {
    if (_Buf->_Size <= _SCFG_SMALL_BUFFER_SIZE / sizeof(wchar_t)) { // release a small buffer
        wmemset(_Buf->_Small, 0, _SCFG_SMALL_BUFFER_SIZE / sizeof(wchar_t));
    } else { // release a large buffer
        if (_Al) { // use a custom allocator
            _Al->deallocate(_Buf->_Large);
        } else { // use a default allocator
            free(_Buf->_Large);
        }

        _Buf->_Large = NULL;
        _Buf->_Size  = 0;
    }

    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Resize_byte_buffer_using_sbo(
    scfg_byte_buffer_t* const _Buf, const size_t _New_size, const scfg_allocator_t* const _Al) {
    if (_New_size <= _SCFG_SMALL_BUFFER_SIZE) { // SBO is still available
        if (_New_size < _Buf->_Size) { // decrease the buffer, fill the rest of the buffer with zeros
            memset(_Buf->_Small + _New_size, 0, _SCFG_SMALL_BUFFER_SIZE - _New_size);
        }

        _Buf->_Size = _New_size;
    } else { // SBO is no longer available
        uint8_t* const _New_ptr = _Al ? (uint8_t*) _Al->allocate(_New_size) : (uint8_t*) malloc(_New_size);
        if (!_New_ptr) {
            return scfg_error_not_enough_memory;
        }

        _Buf->_Size = _New_size;
#pragma warning(suppress : 6385) // C6385: Reading invalid data
        memcpy(_New_ptr, _Buf->_Small, _New_size);
        memset(_Buf->_Small, 0, _SCFG_SMALL_BUFFER_SIZE);
        _Buf->_Large = _New_ptr; // assign a new buffer
    }

    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Resize_utf8_buffer_using_sbo(
    scfg_utf8_buffer_t* const _Buf, const size_t _New_size, const scfg_allocator_t* const _Al) {
    if (_New_size <= _SCFG_SMALL_BUFFER_SIZE) { // SBO is still available
        if (_New_size < _Buf->_Size) { // decrease the buffer, fill the rest of the buffer with zeros
            memset(_Buf->_Small + _New_size, 0, _SCFG_SMALL_BUFFER_SIZE - _New_size);
        }

        _Buf->_Size = _New_size;
    } else { // SBO is no longer available
        char* const _New_ptr = _Al ? (char*) _Al->allocate(_New_size) : (char*) malloc(_New_size);
        if (!_New_ptr) {
            return scfg_error_not_enough_memory;
        }

        _Buf->_Size = _New_size;
#pragma warning(suppress : 6385) // C6385: Reading invalid data
        memcpy(_New_ptr, _Buf->_Small, _New_size);
        memset(_Buf->_Small, 0, _SCFG_SMALL_BUFFER_SIZE);
        _Buf->_Large = _New_ptr; // assign a new buffer
    }

    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Resize_unicode_buffer_using_sbo(
    scfg_unicode_buffer_t* const _Buf, const size_t _New_size, const scfg_allocator_t* const _Al) {
    if (_New_size <= _SCFG_SMALL_BUFFER_SIZE / sizeof(wchar_t)) { // SBO is still available
        if (_New_size < _Buf->_Size) { // decrease the buffer, fill the rest of the buffer with zeros
            wmemset(_Buf->_Small + _New_size, 0, (_SCFG_SMALL_BUFFER_SIZE / sizeof(wchar_t)) - _New_size);
        }

        _Buf->_Size = _New_size;
    } else { // SBO is no longer available
        wchar_t* const _New_ptr = _Al ? (wchar_t*) _Al->allocate(_New_size * sizeof(wchar_t))
            : (wchar_t*) malloc(_New_size * sizeof(wchar_t));
        if (!_New_ptr) {
            return scfg_error_not_enough_memory;
        }

        _Buf->_Size = _New_size;
#pragma warning(suppress : 6385) // C6385: Reading invalid data
        wmemcpy(_New_ptr, _Buf->_Small, _New_size);
        wmemset(_Buf->_Small, 0, _SCFG_SMALL_BUFFER_SIZE / sizeof(wchar_t));
        _Buf->_Large = _New_ptr; // assign a new buffer
    }

    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Resize_byte_buffer_not_using_sbo(
    scfg_byte_buffer_t* const _Buf, const size_t _New_size, const scfg_allocator_t* const _Al) {
    if (_New_size > _SCFG_SMALL_BUFFER_SIZE) { // SBO is still not available
        uint8_t* const _New_ptr = _Al ? (uint8_t*) _Al->allocate(_New_size) : (uint8_t*) malloc(_New_size);
        if (!_New_ptr) {
            return scfg_error_not_enough_memory;
        }

        memcpy(_New_ptr, _Buf->_Large, _SCFG_MIN(_Buf->_Size, _New_size));
        if (_New_size > _Buf->_Size) { // fill the rest of the buffer with zeros
            memset(_New_ptr + _Buf->_Size, 0, _New_size - _Buf->_Size);
        }

        if (_Al) { // use a custom allocator
            _Al->deallocate(_Buf->_Large);
        } else { // use a default allocator
            free(_Buf->_Large);
        }

        _Buf->_Large = _New_ptr; // assign a new buffer
        _Buf->_Size  = _New_size;
    } else { // SBI is now available
        uint8_t _Temp_buf[_SCFG_SMALL_BUFFER_SIZE]; // intentionally uninitialized
        const size_t _Min_size = _SCFG_MIN(_Buf->_Size, _New_size);
        memcpy(_Temp_buf, _Buf->_Large, _Min_size);
        if (_Al) { // use a custom allocator
            _Al->deallocate(_Buf->_Large);
        } else { // use a default allocator
            free(_Buf->_Large);
        }

        _Buf->_Large = NULL;
        _Buf->_Size  = _New_size;
        memcpy(_Buf->_Small, _Temp_buf, _Min_size);
    }

    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Resize_utf8_buffer_not_using_sbo(
    scfg_utf8_buffer_t* const _Buf, const size_t _New_size, const scfg_allocator_t* const _Al) {
    if (_New_size > _SCFG_SMALL_BUFFER_SIZE) { // SBO is still not available
        char* const _New_ptr = _Al ? (char*) _Al->allocate(_New_size) : (char*) malloc(_New_size);
        if (!_New_ptr) {
            return scfg_error_not_enough_memory;
        }

        memcpy(_New_ptr, _Buf->_Large, _SCFG_MIN(_Buf->_Size, _New_size));
        if (_New_size > _Buf->_Size) { // fill the rest of the buffer with zeros
            memset(_New_ptr + _Buf->_Size, 0, _New_size - _Buf->_Size);
        }

        if (_Al) { // use a custom allocator
            _Al->deallocate(_Buf->_Large);
        } else { // use a default allocator
            free(_Buf->_Large);
        }

        _Buf->_Large = _New_ptr; // assign a new buffer
        _Buf->_Size  = _New_size;
    } else { // SBI is now available
        char _Temp_buf[_SCFG_SMALL_BUFFER_SIZE]; // intentionally uninitialized
        const size_t _Min_size = _SCFG_MIN(_Buf->_Size, _New_size);
        memcpy(_Temp_buf, _Buf->_Large, _Min_size);
        if (_Al) { // use a custom allocator
            _Al->deallocate(_Buf->_Large);
        } else { // use a default allocator
            free(_Buf->_Large);
        }

        _Buf->_Large = NULL;
        _Buf->_Size  = _New_size;
        memcpy(_Buf->_Small, _Temp_buf, _Min_size);
    }

    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Resize_unicode_buffer_not_using_sbo(
    scfg_unicode_buffer_t* const _Buf, const size_t _New_size, const scfg_allocator_t* const _Al) {
    if (_New_size > _SCFG_SMALL_BUFFER_SIZE / sizeof(wchar_t)) { // SBO is still not available
        wchar_t* const _New_ptr = _Al ? (wchar_t*) _Al->allocate(_New_size * sizeof(wchar_t))
            : (wchar_t*) malloc(_New_size * sizeof(wchar_t));
        if (!_New_ptr) {
            return scfg_error_not_enough_memory;
        }

        wmemcpy(_New_ptr, _Buf->_Large, _SCFG_MIN(_Buf->_Size, _New_size));
        if (_New_size > _Buf->_Size) { // fill the rest of the buffer with zeros
            wmemset(_New_ptr + _Buf->_Size, 0, _New_size - _Buf->_Size);
        }

        if (_Al) { // use a custom allocator
            _Al->deallocate(_Buf->_Large);
        } else { // use a default allocator
            free(_Buf->_Large);
        }

        _Buf->_Large = _New_ptr; // assign a new buffer
        _Buf->_Size  = _New_size;
    } else { // SBI is now available
        wchar_t _Temp_buf[_SCFG_SMALL_BUFFER_SIZE / sizeof(wchar_t)]; // intentionally uninitialized
        const size_t _Min_size = _SCFG_MIN(_Buf->_Size, _New_size);
        wmemcpy(_Temp_buf, _Buf->_Large, _Min_size);
        if (_Al) { // use a custom allocator
            _Al->deallocate(_Buf->_Large);
        } else { // use a default allocator
            free(_Buf->_Large);
        }

        _Buf->_Large = NULL;
        _Buf->_Size  = _New_size;
        wmemcpy(_Buf->_Small, _Temp_buf, _Min_size);
    }

    return scfg_error_success;
}

inline scfg_error_t _SCFG_CONV _Resize_byte_buffer(scfg_byte_buffer_t* const _Buf,
    const size_t _New_size, const scfg_allocator_t* const _Al) {
    if (_New_size == _Buf->_Size) { // nothing has changed
        return scfg_error_success;
    } else if (_New_size == 0) { // release the buffer
        return scfg_release_buffer(_Buf, scfg_buffer_type_byte, _Al);
    } else if (_Buf->_Size == 0) { // allocate or initialize a new buffer
        if (_New_size <= _SCFG_SMALL_BUFFER_SIZE) { // prepare a small buffer
            _Buf->_Size = _New_size;
            memset(_Buf->_Small, 0, _SCFG_SMALL_BUFFER_SIZE); // fill the buffer with zeros
        } else { // prepare a larger buffer
            _Buf->_Large = _Al ? (uint8_t*) _Al->allocate(_New_size) : (uint8_t*) malloc(_New_size);
            if (!_Buf->_Large) {
                return scfg_error_not_enough_memory;
            }

            _Buf->_Size = _New_size;
            memset(_Buf->_Large, 0, _New_size); // fill the buffer with zeros
        }

        return scfg_error_success;
    } else { // increase or decrease an old buffer (may change the buffer location)
        if (_Buf->_Size <= _SCFG_SMALL_BUFFER_SIZE) {
            return _Resize_byte_buffer_using_sbo(_Buf, _New_size, _Al);
        } else {
            return _Resize_byte_buffer_not_using_sbo(_Buf, _New_size, _Al);
        }
    }
}

inline scfg_error_t _SCFG_CONV _Resize_utf8_buffer(scfg_utf8_buffer_t* const _Buf,
    const size_t _New_size, const scfg_allocator_t* const _Al) {
    if (_New_size == _Buf->_Size) { // nothing has changed
        return scfg_error_success;
    } else if (_New_size == 0) { // release the buffer
        return scfg_release_buffer(_Buf, scfg_buffer_type_utf8, _Al);
    } else if (_Buf->_Size == 0) { // allocate or initialize a new buffer
        if (_New_size <= _SCFG_SMALL_BUFFER_SIZE) { // prepare a small buffer
            _Buf->_Size = _New_size;
            memset(_Buf->_Small, 0, _SCFG_SMALL_BUFFER_SIZE); // fill the buffer with zeros
        } else { // prepare a larger buffer
            _Buf->_Large = _Al ? (char*) _Al->allocate(_New_size) : (char*) malloc(_New_size);
            if (!_Buf->_Large) {
                return scfg_error_not_enough_memory;
            }

            _Buf->_Size = _New_size;
            memset(_Buf->_Large, 0, _New_size); // fill the buffer with zeros
        }

        return scfg_error_success;
    } else { // increase or decrease an old buffer (may change the buffer location)
        if (_Buf->_Size <= _SCFG_SMALL_BUFFER_SIZE) {
            return _Resize_utf8_buffer_using_sbo(_Buf, _New_size, _Al);
        } else {
            return _Resize_utf8_buffer_not_using_sbo(_Buf, _New_size, _Al);
        }
    }
}

inline scfg_error_t _SCFG_CONV _Resize_unicode_buffer(scfg_unicode_buffer_t* const _Buf,
    const size_t _New_size, const scfg_allocator_t* const _Al) {
    if (_New_size == _Buf->_Size) { // nothing has changed
        return scfg_error_success;
    } else if (_New_size == 0) { // release the buffer
        return scfg_release_buffer(_Buf, scfg_buffer_type_unicode, _Al);
    } else if (_Buf->_Size == 0) { // allocate or initialize a new buffer
        if (_New_size <= _SCFG_SMALL_BUFFER_SIZE / sizeof(wchar_t)) { // prepare a small buffer
            _Buf->_Size = _New_size;
            wmemset(
                _Buf->_Small, 0, _SCFG_SMALL_BUFFER_SIZE / sizeof(wchar_t)); // fill the buffer with zeros
        } else { // prepare a larger buffer
            _Buf->_Large = _Al ? (wchar_t*) _Al->allocate(_New_size * sizeof(wchar_t))
                : (wchar_t*) malloc(_New_size * sizeof(wchar_t));
            if (!_Buf->_Large) {
                return scfg_error_not_enough_memory;
            }

            _Buf->_Size = _New_size;
            wmemset(_Buf->_Large, 0, _New_size); // fill the buffer with zeros
        }

        return scfg_error_success;
    } else { // increase or decrease an old buffer (may change the buffer location)
        if (_Buf->_Size <= _SCFG_SMALL_BUFFER_SIZE / sizeof(wchar_t)) {
            return _Resize_unicode_buffer_using_sbo(_Buf, _New_size, _Al);
        } else {
            return _Resize_unicode_buffer_not_using_sbo(_Buf, _New_size, _Al);
        }
    }
}

scfg_error_t _SCFG_CONV scfg_initialize_buffer(void* const _Buf, const scfg_buffer_type_t _Type) {
    if (!_Buf) {
        return scfg_error_invalid_buffer;
    }

    switch (_Type) {
    case scfg_buffer_type_byte:
        return _Init_byte_buffer((scfg_byte_buffer_t*) _Buf);
    case scfg_buffer_type_utf8:
        return _Init_utf8_buffer((scfg_utf8_buffer_t*) _Buf);
    case scfg_buffer_type_unicode:
        return _Init_unicode_buffer((scfg_unicode_buffer_t*) _Buf);
    default:
        return scfg_error_unknown_buffer_type;
    }
}

scfg_error_t _SCFG_CONV scfg_get_associated_buffer(
    void* const _Buf, const scfg_buffer_type_t _Type, void** const _Ptr) {
    if (!_Buf) {
        return scfg_error_invalid_buffer;
    }

    switch (_Type) {
    case scfg_buffer_type_byte:
        return _Get_associated_byte_buffer((scfg_byte_buffer_t*) _Buf, (uint8_t**) _Ptr);
    case scfg_buffer_type_utf8:
        return _Get_associated_utf8_buffer((scfg_utf8_buffer_t*) _Buf, (char**) _Ptr);
    case scfg_buffer_type_unicode:
        return _Get_associated_unicode_buffer((scfg_unicode_buffer_t*) _Buf, (wchar_t**) _Ptr);
    default:
        return scfg_error_unknown_buffer_type;
    }
}

scfg_error_t _SCFG_CONV scfg_resize_buffer(void* const _Buf, const scfg_buffer_type_t _Type,
    const size_t _New_size, const scfg_allocator_t* const _Al) {
    if (!_Buf) {
        return scfg_error_invalid_buffer;
    }

    switch (_Type) {
    case scfg_buffer_type_byte:
        return _Resize_byte_buffer((scfg_byte_buffer_t*) _Buf, _New_size, _Al);
    case scfg_buffer_type_utf8:
        return _Resize_utf8_buffer((scfg_utf8_buffer_t*) _Buf, _New_size, _Al);
    case scfg_buffer_type_unicode:
        return _Resize_unicode_buffer((scfg_unicode_buffer_t*) _Buf, _New_size, _Al);
    default:
        return scfg_error_unknown_buffer_type;
    }
}

scfg_error_t _SCFG_CONV scfg_release_buffer(
    void* const _Buf, const scfg_buffer_type_t _Type, const scfg_allocator_t* const _Al) {
    if (!_Buf) {
        return scfg_error_invalid_buffer;
    }

    switch (_Type) {
    case scfg_buffer_type_byte:
        return _Release_byte_buffer((scfg_byte_buffer_t*) _Buf, _Al);
    case scfg_buffer_type_utf8:
        return _Release_utf8_buffer((scfg_utf8_buffer_t*) _Buf, _Al);
    case scfg_buffer_type_unicode:
        return _Release_unicode_buffer((scfg_unicode_buffer_t*) _Buf, _Al);
    default:
        return scfg_error_unknown_buffer_type;
    }
}