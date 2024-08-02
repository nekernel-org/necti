/* -------------------------------------------

    Copyright ZKA Technologies

------------------------------------------- */

#include <NDKKit/Parser.hxx>
#include <NDKKit/NFC/AE.hxx>
#include <NDKKit/NFC/PEF.hxx>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

/////////////////////////////////////////////////////////////////////////////////////////

/// @brief COFF 2 AE entrypoint, the program/module starts here.

/////////////////////////////////////////////////////////////////////////////////////////

NDK_MODULE(NewOSELFToAE) { return 0; }
