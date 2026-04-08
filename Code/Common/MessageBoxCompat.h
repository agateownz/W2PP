/*
*   Copyright (C) {2015}  {VK, Charles TheHouse}
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see [http://www.gnu.org/licenses/].
*
*   Contact at:
*/

#pragma once

// This header provides a compatibility layer for MessageBox
// On Windows, it can optionally still show MessageBox in addition to logging
// On Linux, it logs to console/file only

#include "Logger.h"
#include <Windows.h>

namespace W2PP {

// MessageBox replacement that logs and optionally shows dialog
inline int LogMessageBox(
    HWND hWnd,
    const char* lpText,
    const char* lpCaption,
    UINT uType
) {
    // Log the message
    LOG_ERROR("[MessageBox] {}: {}", lpCaption, lpText);
    
    // Still show MessageBox on Windows for now (during transition)
    // This can be disabled later when fully migrated to console
    return MessageBoxA(hWnd, lpText, lpCaption, uType);
}

// Macro to replace MessageBox calls
// Usage: Replace MessageBox(...) with W2PP_MESSAGE_BOX(...)
#define W2PP_MESSAGE_BOX(hWnd, text, caption, type) \
    W2PP::LogMessageBox(hWnd, text, caption, type)

// For code that uses MessageBoxA directly
inline int LogMessageBoxA(
    HWND hWnd,
    const char* lpText,
    const char* lpCaption,
    UINT uType
) {
    LOG_ERROR("[MessageBox] {}: {}", lpCaption, lpText);
    return MessageBoxA(hWnd, lpText, lpCaption, uType);
}

#define W2PP_MESSAGE_BOX_A(hWnd, text, caption, type) \
    W2PP::LogMessageBoxA(hWnd, text, caption, type)

} // namespace W2PP

// Global replacement macros - uncomment these to replace all MessageBox calls globally
// #define MessageBox W2PP_MESSAGE_BOX
// #define MessageBoxA W2PP_MESSAGE_BOX_A
