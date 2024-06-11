/* -------------------------------------------

    Copyright Zeta Electronics Corporation

------------------------------------------- */

#include <Comm/ParserKit.hpp>
#include <Comm/StdKit/AE.hpp>
#include <Comm/StdKit/PEF.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

/////////////////////////////////////////////////////////////////////////////////////////

/// @brief COFF 2 AE entrypoint, the program/module starts here.

/////////////////////////////////////////////////////////////////////////////////////////

MPCC_MODULE(NewOSELFToAE) { return 0; }
