//
//
//	Copyright ZKA Technologies 2024
//
//	File: xstore.h
//	Purpose: ZKA OS STORE SDK.
//
//

#pragma once

#include <sdk/xpcom.h>

typedef XHANDLE STOREHANDLE; // API handle, simply a pointer to an XHANDLE.

enum
{
	eFieldKindInt32,
	eFieldKindInt64,
	eFieldKindString,
	eFieldKindBoolean,
	eFieldKindReal64,
	eFieldKindReal32,
	eFieldKindInvalid,
};

EXTERN STOREHANDLE XCreateStore(VOID);
EXTERN STOREHANDLE XOpenStore(CONST WCHAR* szStoreName);
EXTERN INT32	   XCloseStore(STOREHANDLE* pStore);
EXTERN INT32	   XRemoveStore(STOREHANDLE* pStore);
EXTERN INT32	   XLinkStore(STOREHANDLE* pSourceStore, STOREHANDLE* pLinkedStore);
EXTERN INT32	   XRemoveStoreField(STOREHANDLE pStore, CONST WCHAR* pName);
EXTERN INT32	   XAppendStoreField(STOREHANDLE pStore, CONST WCHAR* pName, INT32 iFieldKind, PVOID pData, SIZE_T pDataSz);
