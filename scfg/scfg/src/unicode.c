// unicode.c

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <inc/scfg.h>
#include <scfgpch.h>
#include <stdlib.h>

scfg_error_t _SCFG_CONV scfg_unicode_to_utf8_required_buffer_size(
    const wchar_t* _Data, size_t _Data_size, size_t* const _Buf_size) {
    if (!_Buf_size) { // do not store any informations
        return scfg_error_success;
    }

    *_Buf_size = 0;
    while (_Data_size-- > 0) {
        if (*_Data <= 0x7F) { // 1 byte per word
            ++*_Buf_size;
        } else if (*_Data <= 0x07FF) { // 2 bytes per word
            *_Buf_size += 2;
        } else if (*_Data <= 0xFFFF) { // 3 bytes per word
            *_Buf_size += 3;
        } else if (*_Data <= 0x0010'FFFF) { // 4 bytes per word
            *_Buf_size += 4;
        } else { // word too large, see RFC 3629
            return scfg_error_code_point_too_large;
        }

        ++_Data;
    }

    return scfg_error_success;
}

scfg_error_t _SCFG_CONV scfg_utf8_to_unicode_required_buffer_size(
    const char* _Data, size_t _Data_size, size_t* const _Buf_size) {
    if (!_Buf_size) { // do not store any informations
        return scfg_error_success;
    }

    *_Buf_size = 0;
    while (_Data_size > 0) {
        if ((*_Data & 0x80) == 0) { // 1 byte per word
            --_Data_size;
            ++_Data;
        } else if ((*_Data & 0xE0) == 0xC0) { // 2 bytes per word
            _Data_size -= 2;
            _Data      += 2;
        } else if ((*_Data & 0xF0) == 0xE0) { // 3 bytes per word
            _Data_size -= 3;
            _Data      += 3;
        } else if ((*_Data & 0xF8) == 0xF0) { // 4 bytes per word
            _Data_size -= 4;
            _Data      += 4;
        } else { // word too large, see RFC 3629
            return scfg_error_code_point_too_large;
        }

        ++*_Buf_size;
    }

    return scfg_error_success;
}

scfg_error_t _SCFG_CONV scfg_unicode_to_utf8(
    const wchar_t* _Data, size_t _Size, scfg_utf8_buffer_t* const _Buf) {
    if (!_Buf || _Buf->_Size == 0) {
        return scfg_error_invalid_buffer;
    }

    size_t _Buf_size  = 0;
    scfg_error_t _Err = scfg_unicode_to_utf8_required_buffer_size(_Data, _Size, &_Buf_size);
    if (_Err != scfg_error_success) {
        return _Err;
    }

    if (_Buf->_Size < _Buf_size) {
        return scfg_error_buffer_too_small;
    }

    char* _Ptr;
    _Err = scfg_get_associated_buffer(_Buf, scfg_buffer_type_utf8, &_Ptr);
    if (_Err != scfg_error_success) {
        return _Err;
    }

    while (_Size-- > 0) {
        if (*_Data <= 0x7F) { // 1 byte per word
            *(_Ptr++) = (char)*_Data; // no leading byte
        } else if (*_Data <= 0x07FF) { // 2 bytes per word
            *(_Ptr++) = (char) (((*_Data >> 6) & 0x1F) | 0xC0); // leading byte
            *(_Ptr++) = (char) ((*_Data & 0x3F) | 0x80);
        } else if (*_Data <= 0xFFFF) { // 3 bytes per word
            *(_Ptr++) = (char) (((*_Data >> 12) & 0x0F) | 0xE0); // leading byte
            *(_Ptr++) = (char) (((*_Data >> 6) & 0x3F) | 0x80);
            *(_Ptr++) = (char) ((*_Data & 0x3F) | 0x80);
        } else if (*_Data <= 0x0010'FFFF) { // 4 bytes per word
            *(_Ptr++) = (char) (((*_Data >> 18) & 0x07) | 0xF0); // leading byte
            *(_Ptr++) = (char) (((*_Data >> 12) & 0x3F) | 0x80);
            *(_Ptr++) = (char) (((*_Data >> 6) & 0x3F) | 0x80);
            *(_Ptr++) = (char) ((*_Data & 0x3F) | 0x80);
        } else { // word too large, see RFC 3629
            return scfg_error_code_point_too_large;
        }

        ++_Data;
    }

    return scfg_error_success;
}

scfg_error_t _SCFG_CONV scfg_utf8_to_unicode(
    const char* _Data, size_t _Size, scfg_unicode_buffer_t* const _Buf) {
    if (!_Buf || _Buf->_Size == 0) {
        return scfg_error_invalid_buffer;
    }

    size_t _Buf_size  = 0;
    scfg_error_t _Err = scfg_utf8_to_unicode_required_buffer_size(_Data, _Size, &_Buf_size);
    if (_Err != scfg_error_success) {
        return _Err;
    }

    if (_Buf->_Size < _Buf_size) {
        return scfg_error_buffer_too_small;
    }

    wchar_t* _Ptr;
    _Err = scfg_get_associated_buffer(_Buf, scfg_buffer_type_unicode, &_Ptr);
    if (_Err != scfg_error_success) {
        return _Err;
    }

    while (_Size > 0) {
        if ((*_Data & 0x80) == 0) { // 1 byte per word
            *_Ptr = (wchar_t) *(_Data++);
            --_Size;
        } else if ((*_Data & 0xE0) == 0xC0) { // 2 bytes per word
            *_Ptr  = (wchar_t) ((*(_Data++) & 0x1F) << 6);
            *_Ptr |= (wchar_t) (*(_Data++) & 0x3F);
            _Size -= 2;
        } else if ((*_Data & 0xF0) == 0xE0) { // 3 bytes per word
            *_Ptr  = (wchar_t) ((*(_Data++) & 0x0F) << 12);
            *_Ptr |= (wchar_t) ((*(_Data++) & 0x3F) << 6);
            *_Ptr |= (wchar_t) (*(_Data++) & 0x3F);
            _Size -= 3;
        } else if ((*_Data & 0xF8) == 0xF0) { // 4 bytes per word
            *_Ptr  = (wchar_t) ((*(_Data++) & 0x07) << 18);
            *_Ptr |= (wchar_t) ((*(_Data++) & 0x3F) << 12);
            *_Ptr |= (wchar_t) ((*(_Data++) & 0x3F) << 6);
            *_Ptr |= (wchar_t) (*(_Data++) & 0x3F);
            _Size -= 4;
        } else { // word too large, see RFC 3629
            return scfg_error_code_point_too_large;
        }

        ++_Ptr;
    }

    return scfg_error_success;
}