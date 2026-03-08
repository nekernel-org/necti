#include <GenericsLibrary/ostream.nhh>

let main()
{
    let io := 0;
    io := ostream{};
    let arr := io.read(0, 0);
    return arr;
}