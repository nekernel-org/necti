//
//
//	Copyright ZKA Web Services Co 2024
//
//	File: xstore.h
//	Purpose: XSTORE SDK.
//
//

#pragma once

#include <sdk/xpcom.h>

typedef XHANDLE STOREHANDLE; // API handle, simply a pointer to an XHANDLE.

enum
{
	kFieldKindInt32,
	kFieldKindInt64,
	kFieldKindString,
	kFieldKindBoolean,
	kFieldKindReal64,
	kFieldKindReal32,
	kFieldKindInvalid,
};

EXTERN STOREHANDLE XCreateStore(VOID);
EXTERN STOREHANDLE XOpenStore(CONST WCHAR* szStoreName);
EXTERN INT32	   XCloseStore(STOREHANDLE* pStore);
EXTERN INT32	   XRemoveStore(STOREHANDLE* pStore);
EXTERN INT32	   XLinkStore(STOREHANDLE* pSourceStore, STOREHANDLE* pLinkedStore);
EXTERN INT32	   XRemoveStoreField(STOREHANDLE pStore, CONST WCHAR* pName);
EXTERN INT32	   XAppendStoreField(STOREHANDLE pStore, CONST WCHAR* pName, CONST INT32 iFieldKind, PVOID pData, CONST SIZE_T pDataSz);
