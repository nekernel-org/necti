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

typedef struct __gHANDLE* HANDLE; // API handle.

EXTERN __INT32_TYPE__ W32MessageBoxW(HANDLE hWnd, const WCHAR* szContent, const WCHAR* szTitle, UINT32 iFlags);
