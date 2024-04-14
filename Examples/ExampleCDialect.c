int main(int argc, char const* argv[]) {
  int* foo = 0x1000;

  if (foo == 57) {
    foo = 0x2000;
    *foo = 5;
    return foo;
  }

  return 57;
}
