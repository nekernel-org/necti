/* -------------------------------------------

  Copyright (C) 2024-2025 Amlal EL Mahrouss, all rights reserved

  File: XCOFF.h
  Purpose: XCOFF for NeKernel.

  Revision History:

  04/07/24: Added file (Amlal EL Mahrouss)

------------------------------------------- */

#ifndef _NECTI_XCOFF_H_
#define _NECTI_XCOFF_H_

#include <CompilerKit/Defines.h>

#define kXCOFF64Magic 0x01F7

#define kXCOFFRelFlg 0x0001
#define kXCOFFExecutable 0x0002
#define kXCOFFLnno 0x0004
#define kXCOFFLSyms 0x0008

namespace CompilerKit {
struct XCoffFileHeader;

/// @brief XCoff file header.
typedef struct XCoffFileHeader {
  UInt16  fMagic;
  UInt16  fTarget;
  UInt16  fNumSecs;
  UInt32  fTimeDat;
  UIntPtr fSymPtr;
  UInt32  fNumSyms;
  UInt16  fOptHdr;  // ?: Number of bytes in optional header
} XCoffFileHeader;

typedef struct XCoffFileHeader* XCoffFileHeaderPtr;
}  // namespace CompilerKit

#endif  // ifndef _NECTI_XCOFF_H_
