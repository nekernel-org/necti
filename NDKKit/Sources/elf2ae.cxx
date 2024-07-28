/* -------------------------------------------

    Copyright ZKA Technologies

------------------------------------------- */

#include <NDKKit/Parser.hpp>
#include <NDKKit/NFC/AE.hpp>
#include <NDKKit/NFC/PEF.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

/////////////////////////////////////////////////////////////////////////////////////////

/// @brief COFF 2 AE entrypoint, the program/module starts here.

/////////////////////////////////////////////////////////////////////////////////////////

NDK_MODULE(NewOSELFToAE) { return 0; }
