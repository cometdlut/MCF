// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_CRTDEF_H_
#define __MCFCRT_ENV_CRTDEF_H_

#ifdef __cplusplus
#   include <cstddef>
#   include <cstdint>
#   include <climits>
#else
#   include <stddef.h>
#   include <stdint.h>
#   include <limits.h>
#   include <stdbool.h>
#   include <stdalign.h>
#endif

#ifdef __cplusplus
#   define __MCFCRT_EXTERN_C_BEGIN     extern "C" {
#   define __MCFCRT_EXTERN_C_END       }
#else
#   define __MCFCRT_EXTERN_C_BEGIN
#   define __MCFCRT_EXTERN_C_END
#endif

// C++ 目前还不支持 C99 的 restrict 限定符。
#ifdef __cplusplus
#   define restrict                     __restrict__
#endif

#if !defined(__cplusplus) || __cplusplus < 201103l
#   define nullptr                      ((void *)0)
#endif

#ifdef __cplusplus
#   define MCFCRT_STD                      ::std::
#   define MCFCRT_NOEXCEPT                 noexcept
#else
#   define MCFCRT_STD
#   define MCFCRT_NOEXCEPT
#endif

#endif
