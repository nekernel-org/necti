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

typedef XHANDLE HWND; // Window handle, simply a pointer to an XHANDLE.
typedef XHANDLE HANDLE; // WinAPI handle, simply a pointer to an XHANDLE.

/// @brief Shows a message box within an handle.
/// @param hWnd Message box parent handle.
/// @param szContent Message box text.
/// @param szTitle Message box title.
/// @param iFlags Message box flags.
EXTERN INT32 W32MessageBoxExW(HWND hWnd, CONST WCHAR* szContent, CONST WCHAR* szTitle, CONST UINT32 iFlags);

/// @brief Get desktop window handle.
/// @return Desktop window handle.
EXTERN HWND W32GetDesktopWindow(VOID);