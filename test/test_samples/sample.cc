class ostream
{
  ostream()
  {
    return void;
  }

  ~ostream()
  {
    return void;
  }

  ostream& write(const char* buf, const long sz)
  {
    return *this;
  }
};

int main()
{
  ostream* f = new ostream();
  f->write("foo", 3);
  return 0;
}
