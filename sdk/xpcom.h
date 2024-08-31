//
//
//	Copyright ZKA Technologies 2024
//
//	File: xpcom.h
//	Purpose: ZKA OS XPCOM SDK.
//
//

#ifndef __SDK_STD_H__
#define __SDK_STD_H__

struct __gHANDLE
{
    __CHAR32_TYPE__ __UNUSED;
};

// So actualy we need to define handles.

typedef struct __gHANDLE* XHANDLE; // XPCOM handle.

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

#define PVOID void*

EXTERN __INT32_TYPE__ XMessageBox(XHANDLE hWnd, XHANDLE hContent, XHANDLE hOnSuccess, XHANDLE hOnFailure);

EXTERN __INT32_TYPE__ XDialogBoxShow(XHANDLE hDlg);
EXTERN __INT32_TYPE__ XDialogBoxDestroy(XHANDLE hDlg);

EXTERN __INT32_TYPE__ XVirtualAlloc(__SIZE_TYPE__ szPtr, __UINT32_TYPE__ iFlags, PVOID* ppOutPtr); 
EXTERN __INT32_TYPE__ XVirtualFree(PVOID* ppOutPtr); 

EXTERN XHANDLE XOpenFile(const __CHAR16_TYPE__* fileName, __UINT32_TYPE__ iDrive, __UINT32_TYPE__ iFlags);
EXTERN __INT32_TYPE__ XCloseFile(XHANDLE* ppFile);

#endif // ifndef __SDK_STD_H__
