//
//
//	Copyright ZKA Technologies 2024
//
//	File: xpdf.h
//	Purpose: ZKA OS PDF SDK.
//
//

#pragma once

#include <sdk/xpcom.h>

/// @brief PDF handle type.
typedef XHANDLE PDFHANDLE;

/// @brief Opens a new PDF handle.
EXTERN PDFHANDLE XOpenPDF(CONST WCHAR* subject, CONST WCHAR* title, CONST WCHAR* author);

/// @brief Close PDF handle and writes (optionally) to write.
EXTERN XRESULT XClosePDF(PDFHANDLE* pdf);
