/* -------------------------------------------

	Copyright ZKA Technologies

------------------------------------------- */

#pragma once

#define kDistVersion "v1.1.0"

#define ToPPString(X) __Str(X)
#define __Str(X) #X

#define kDistRelease ToPPString(kDistReleaseBranch)
