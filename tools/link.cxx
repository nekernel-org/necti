/* -------------------------------------------

	Copyright ZKA Technologies

------------------------------------------- */

/// @file Linker.cxx
/// @brief ZKA Linker frontend for AE objects.

extern "C" int ZKALinkerMain(int argc, char const* argv[]);

int main(int argc, char const *argv[])
{
    if (argc < 1)
    {
        return 1;
    }

    return ZKALinkerMain(argc, argv);
}
