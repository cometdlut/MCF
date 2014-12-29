// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_USER_RECURSIVE_MUTEX_HPP_
#define MCF_THREAD_USER_RECURSIVE_MUTEX_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "UniqueLockTemplate.hpp"
#include "Mutex.hpp"
#include <cstddef>

namespace MCF {

enum RecursiveMutexResult {
	R_TRY_FAILED	= 0,
	R_STATE_CHANGED	= 1,
	R_RECURSIVE		= 2,
};

class RecursiveMutex : NONCOPYABLE {
public:
	using Result = RecursiveMutexResult;
	using UniqueLock = UniqueLockTemplate<RecursiveMutex>;

private:
	Mutex xm_vMutex;
	std::size_t xm_uRecursionCount;

public:
	explicit RecursiveMutex(std::size_t uSpinCount = 0x400);

public:
	std::size_t GetSpinCount() const noexcept {
		return xm_vMutex.GetSpinCount();
	}
	void SetSpinCount(std::size_t uSpinCount) noexcept {
		xm_vMutex.SetSpinCount(uSpinCount);
	}

	bool IsLockedByCurrentThread() const noexcept {
		return xm_vMutex.IsLockedByCurrentThread();
	}

	Result Try() noexcept;
	Result Lock() noexcept;
	Result Unlock() noexcept;

	// 要求 IsLockedByCurrentThread() != false。
	std::size_t UncheckedGetRecursionCount() const noexcept;
	std::size_t GetRecursionCount() const noexcept;

	UniqueLock TryLock() noexcept;
	UniqueLock GetLock() noexcept;
};

}

#endif