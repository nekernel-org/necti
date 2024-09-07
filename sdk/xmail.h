//
//
//	Copyright ZKA Technologies 2024
//
//	File: xmail.h
//	Purpose: ZKA OS MAIL SDK.
//
//

#pragma once

#include <sdk/xpcom.h>

EXTERN XHANDLE XOpenMail(const WCHAR subect, struct XMIME* mime, struct XCONTACT* from);

EXTERN XRESULT XSendMail(XHANDLE mail);

EXTERN XRESULT XCloseMail(XHANDLE* mail);
