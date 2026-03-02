// Copyright 2026, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/ne-foss-org/nectar

extern main;

//@ The main entrypoint is an external symbol defined by the user program.
//@ You may also define it as a library and then write your own main wrapper over it.
let _start()
{
    return main();
}

