#define AppMain __ImageStart
#warning TestCase #1

int bar() {
  int yyy = 100;
  return yyy;
}

int foo() {
  int arg1 = 0;
  return bar();
}

int AppMain() {
  return foo();
}
