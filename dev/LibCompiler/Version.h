/* -------------------------------------------

	Copyright (C) 2024 Theater Quality Corp, all rights reserved

------------------------------------------- */

#pragma once

#define kDistVersion	"v1.1.0"
#define kDistVersionBCD 0x0110

#define ToString(X)	 Stringify(X)
#define Stringify(X) #X

#define kDistRelease ToString(kDistReleaseBranch)
