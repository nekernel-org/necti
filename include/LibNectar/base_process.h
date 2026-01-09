// Copyright 2024-2025, Amlal El Mahrouss (amlal@nekernel.org)
// Licensed under the Apache License, Version 2.0 (See accompanying
// file LICENSE or copy at http://www.apache.org/licenses/LICENSE-2.0)
// Official repository: https://github.com/nekernel-org/nectar

#ifndef NECTAR_LIBCXX_BASE_PROCESS_H
#define NECTAR_LIBCXX_BASE_PROCESS_H

#include <LibNectar/defines.h>

__init_decl()

    /// @brief CRT exit, with exit code (!!! exits all threads. !!!)
    /// @param code the exit code.
    /// @return the return > 0 for non successful.
    extern int exit_(int code);

/// @brief CRT signal handler.
/// @param code the signal code.
extern void signal_(int code);

extern void (*__atexit_cdecl_ptr)(void);
extern void (**__atexit_lst_ptr)(void);
extern size_t __atexit_lst_cnt;

__fini_decl()

inline int signal(int code) {
    signal_(code);
    return -1;
}

inline int32_t exit(const int32_t& code) {
    for (auto idx = 0UL; idx < __atexit_lst_cnt; ++idx) {
        __atexit_lst_ptr[idx]();
    }

    if (__atexit_cdecl_ptr) __atexit_cdecl_ptr();

    exit_(code);
    return -1;
}

#endif  // NECTAR_LIBCXX_BASE_PROCESS_H
