extern exit;
extern malloc;

let construct_foo()
{
    let io := 0;
    io :=  malloc(4);

    return io;
}

let main()
{
    let io := 0x0;
    io := construct_foo();
    
    _ := exit(io);

    return first_number;
}