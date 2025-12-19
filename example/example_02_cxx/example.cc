#define main __ImageStart
#warning test macro warning #1

int bar() {
  auto yyy = 17800;
  return yyy;
}

int foo() {
  int arg1 = bar();
  return arg1;
}

int main() {
  return foo();
}
