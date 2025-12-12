/* ========================================

  Copyright (C) 2024-2025 Amlal El Mahrouss, licensed under the Apache 2.0 license.

======================================== */

#ifndef __NECTAR_DEFINES_H__
#define __NECTAR_DEFINES_H__

#define __ATTRIBUTE(X) __attribute__((X))

typedef __SIZE_TYPE__  size_t;
typedef __INT64_TYPE__ ssize_t;
typedef __INT32_TYPE__ int32_t;

typedef void*         ptr_type;
typedef __SIZE_TYPE__ size_type;

typedef __INT64_TYPE__ ptrdiff_t;
typedef size_t         uintptr_t;
typedef void*          voidptr_t;
typedef void*          any_t;
typedef char*          caddr_t;

#ifndef NULL
#define NULL ((voidptr_t) 0)
#endif  // !null

#define __alloca(sz) __ck_alloca(sz)

#define __deref(ptr) (*(ptr))

#ifdef __cplusplus
#define __init_decl() extern "C" {
#define __fini_decl() \
  }                   \
  ;
#else
#define __init_decl()
#define __fini_decl()
#endif

#if __has_builtin(__builtin_alloca)
#define alloca(sz) __builtin_alloca(sz)
#ifdef __alloca
#undef __alloca
#endif
#define __alloca alloca
#else
#warning !! alloca not detected !!
#endif

typedef long long          off_t;
typedef unsigned long long uoff_t;

typedef union float_cast {
  struct {
    unsigned int mantissa : 23;
    unsigned int exponent : 8;
    unsigned int sign : 1;
  };

  float f;
} __ATTRIBUTE(packed) float_cast_t;

typedef union double_cast {
  struct {
    unsigned long long int mantissa : 52;
    unsigned int           exponent : 11;
    unsigned int           sign : 1;
  };

  double f;
} __ATTRIBUTE(packed) double_cast_t;

namespace std {
struct placement_t;
struct nothrow_t;
}  // namespace std

#endif /* __NECTAR_DEFINES_H__ */
