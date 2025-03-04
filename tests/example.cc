#define AppMain int __ImageStart
#warning TestCase #1

int bar()
{
	int yyy = 5050505055;
	return yyy;
}

int foo()
{
	return bar();
}

AppMain()
{
	return foo();
}
