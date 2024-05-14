int main(int argc, char const* argv[])
{
	int foo = 20;
	foo -= 1;

	{
		bool bar = false;
		bar		 = true;

		bool bar2 = bar;
	}

	return foo;
}
