import cudaMalloc;

const main()
{
    let ptr := 0;
    let sz := 8;
    cudaMalloc(ptr, sz);
}
