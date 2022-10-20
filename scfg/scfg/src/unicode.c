// unicode.c

// Copyright (c) Mateusz Jandura. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <inc/scfg.h>
#include <scfgpch.h>
#include <stdlib.h>

// FUNCTION scfg_unicode_to_buffer
scfg_error_t _SCFG_CONV scfg_unicode_to_buffer(
    const wchar_t* const _Unc, const size_t _Size, scfg_buffer_t* const _Buf) {
    if (!_Buf || _Buf->_Size == 0) {
        return scfg_error_invalid_buffer;
    }

    if (_Size == 0) {
        return scfg_error_success;
    }

    uint8_t* _Ptr;
    const scfg_error_t _Err = scfg_get_associated_buffer(_Buf, &_Ptr);
    if (_Err != scfg_error_success) {
        return _Err;
    }

#pragma warning(suppress : 4996) // C4996: Use wcstombs_s() instead
    if (wcstombs((char*) _Ptr, _Unc, _Size) == (size_t) -1) {
        return scfg_error_invalid_code_point;
    } else {
        return scfg_error_success;
    }
}

// FUNCTION scfg_unicode_from_buffer
scfg_error_t _SCFG_CONV scfg_unicode_from_buffer(
    wchar_t* const _Unc, const size_t _Size, scfg_buffer_t* const _Buf) {
    if (!_Buf || !_Unc || _Size == 0) {
        return scfg_error_invalid_buffer;
    }
    
    uint8_t* _Ptr;
    const scfg_error_t _Err = scfg_get_associated_buffer(_Buf, &_Ptr);
    if (_Err != scfg_error_success) {
        return _Err;
    }

#pragma warning(suppress : 4996) // C4996: Use mbstowcs_s() instead
    if (mbstowcs(_Unc, _Ptr, _Buf->_Size) == (size_t) -1) {
        return scfg_error_invalid_code_point;
    } else {
        return scfg_error_success;
    }
}