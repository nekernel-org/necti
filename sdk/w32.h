//
//
//	Copyright ZKA Technologies 2024
//
//	File: xpcom.h
//	Purpose: ZKA OS W32 SDK.
//
//

#pragma once

#include <sdk/xpcom.h>

typedef struct __gHANDLE* HANDLE; // API handle.

EXTERN __INT32_TYPE__ W32MessageBox(HANDLE hWnd, const __WCHAR_TYPE__* szContent, const __WCHAR_TYPE__* szTitle, __UINT32_TYPE__ iFlags);
