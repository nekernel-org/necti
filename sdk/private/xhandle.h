//
//
//	Copyright ZKA Technologies 2024
//
//	File: xhandle.h
//	Purpose: ZKA HANDLE MANAGER.
//
//

#pragma once

#ifndef __SDK_STD_H__
#error !!! Include XPCOM.H first ! !!!
#endif // !__SDK_STD_H__

/// @brief Allocate XPCOM object.
EXTERN XHANDLE XAllocObject(VOID);

/// @brief Attach method to XPCOM instance.
EXTERN XRESULT XAttachMethod(XHANDLE handle, PVOID prop, UINT32 off);

/// @brief Attach field to XPCOM instance.
EXTERN XRESULT XAttachField(XHANDLE handle, PVOID prop, UINT32 off);

/// @brief Destroy XPCOM instance.
EXTERN XRESULT XDestroyObject(XHANDLE* handle_ptr);

/// @brief Does a sanity check of the XPCOM instance.
EXTERN BOOL    XObjectMustPass(XHANDLE handle);
