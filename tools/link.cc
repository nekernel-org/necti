/* -------------------------------------------

	Copyright ZKA Web Services Co

------------------------------------------- */

/// @file link.cxx
/// @brief ZKA Linker for AE objects.

extern "C" int ZKALinkerMain(int argc, char const* argv[]);

int main(int argc, char const* argv[])
{
	if (argc < 1)
	{
		return 1;
	}

	return ZKALinkerMain(argc, argv);
}
