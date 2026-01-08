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

  ostream& noop(const ostream in)
  {
    return *this;
  }
};

int main()
{
  void* f = new ostream();
  f->noop(f);
  return 0;
}
