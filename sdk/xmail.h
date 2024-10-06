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

typedef XHANDLE MAILHANDLE;

EXTERN MAILHANDLE XOpenMail(CONST WCHAR* subject, struct XMIME* mime, struct XCONTACT* from);

EXTERN XRESULT XSendMail(MAILHANDLE mail);

EXTERN XRESULT XCloseMail(MAILHANDLE* mail);
