impl foo
{
    let init()
    {
        return;
    }

    let noop()
    {
        return 0x0;
    }
};

let construct_foo()
{
    let io := new;
    io := foo{};

    return io;
}

let main()
{
    let io := 0x0;
    io := construct_foo();
    
    let first_number := io->noop();
    let status := delete(io);

    return first_number;
}