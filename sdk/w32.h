//
//
//	Copyright ZKA Technologies 2024
//
//	File: w32.h
//	Purpose: ZKA OS W32 SDK.
//
//

#pragma once

#include <sdk/xpcom.h>

typedef XHANDLE HANDLE; // API handle, simply a pointer to an XHANDLE.

/// @brief Shows a message box within an handle.
/// @param hWnd Message box parent handle.
/// @param szContent Message box text.
/// @param szTitle Message box title.
/// @param iFlags Message box flags.
EXTERN INT32 W32MessageBoxW(HANDLE hWnd, const WCHAR* szContent, const WCHAR* szTitle, UINT32 iFlags);
