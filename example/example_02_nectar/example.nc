extern exit;

const terminate()
{
  let EXIT_TERMINATED = 0x100;
  exit(EXIT_TERMINATED);
}

const main()
{
  terminate();
  return 0;
}
