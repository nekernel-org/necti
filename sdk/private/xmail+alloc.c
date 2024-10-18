//
//
//	Copyright ZKA Web Services Co 2024
//
//	File: xmail+open.c
//	Purpose: ZKA OS MAIL SDK.
//
//

/**
	From: FooBar <Foo@Bar.com>
	Date: Sat, 20 Apr 2024 01:26:49 CEST
	MIME-Version: 1.0
	Content-Type: text/html; charset=us-ascii
	Subject: Welcome!
*/

#include <sdk/xmail.h>
#include <sdk/private/xhandle.h>

EXTERN XHANDLE XOpenMail(const WCHAR subect, struct XMIME* mime, struct XCONTACT* from)
{
	XHANDLE handle = XAllocObject();

	if (!XObjectMustPass(handle)))
        return NULL;

	XAttachField(handle, mime, 0);
	XAttachField(handle, from, 1);

	return handle;
}

EXTERN XRESULT XCloseMail(XHANDLE* handle)
{
	return XDestroyObject(handle);
}
