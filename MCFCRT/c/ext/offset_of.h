// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_OFFSET_OF_H__
#define __MCF_CRT_OFFSET_OF_H__

#include "../../env/_crtdef.h"

#define OFFSET_OF(s, m)	\
	((__MCF_STD size_t)((__MCF_STD uintptr_t)&(((s *)(__MCF_STD uintptr_t)1)->m) - (__MCF_STD uintptr_t)1))

// 派生类指针转换成基类指针，成员指针转换成类指针。
#define DOWN_CAST(s, m, p)	\
	((s *)((__MCF_STD uintptr_t)(p) - OFFSET_OF(s, m)))

#endif
