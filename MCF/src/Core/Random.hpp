// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_RANDOM_HPP_
#define MCF_CORE_RANDOM_HPP_

#include <MCFCRT/ext/random.h>
#include <cstdint>

namespace MCF {

inline std::uint32_t GetRandomUint32() noexcept {
	return ::_MCFCRT_GetRandomUint32();
}
inline std::uint64_t GetRandomUint64() noexcept {
	return ::_MCFCRT_GetRandomUint64();
}
inline double GetRandomDouble() noexcept {
	return ::_MCFCRT_GetRandomDouble();
}

}

#endif
