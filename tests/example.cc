void foo_bar()
{
	printf("%s", "Hello");
	return;
}

int __autorelease__ main() 
{
	foo();
	return 0;
}
