#define main __ImageStart
#warning test macro warning #1

int bar() {
  int yyy = 100;
  return yyy;
}

int foo() {
  int arg1 = 0;
  return bar();
}

int main() {
  return foo();
}
