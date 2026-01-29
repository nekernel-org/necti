extern exit;

let main()
{
    let foo := 42;

    const ret_stub():
        foo := 0x10;
        exit(foo);
        return 0x0;
}
