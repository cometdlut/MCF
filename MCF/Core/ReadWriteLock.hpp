// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_READ_WRITE_LOCK_HPP__
#define __MCF_READ_WRITE_LOCK_HPP__

#include "NoCopy.hpp"
#include <memory>
#include <utility>

namespace MCF {

class ReadWriteLock : NO_COPY {
public:
	enum : std::size_t {
		MAXIMUM_CONCURRENT_READS = 16u
	};
private:
	class xDelegate;

	struct xReadUnlocker {
		void operator()(xDelegate *pDelegate) const noexcept {
			xUnlockRead(pDelegate);
		}
	};
	struct xWriteUnlocker {
		void operator()(xDelegate *pDelegate) const noexcept {
			xUnlockWrite(pDelegate);
		}
	};
private:
	static void xUnlockRead(xDelegate *pDelegate) noexcept;
	static void xUnlockWrite(xDelegate *pDelegate) noexcept;
public:
	typedef std::unique_ptr<xDelegate, xReadUnlocker> ReadLockHolder;
	typedef std::unique_ptr<xDelegate, xWriteUnlocker> WriteLockHolder;
private:
	const std::unique_ptr<xDelegate> xm_pDelegate;
public:
	ReadWriteLock(unsigned long ulSpinCount = 0x400);
	~ReadWriteLock();
public:
	ReadLockHolder GetReadLock() const noexcept;
	WriteLockHolder GetWriteLock() noexcept;
};

}

#define READ_LOCK_SCOPE(rwl)	\
	for(auto __MCF_LOCK__ = std::make_pair(static_cast<::MCF::ReadWriteLock &>(rwl).GetReadLock(), true);	\
		__MCF_LOCK__.second;	\
		__MCF_LOCK__.second = false)

#define WRITE_LOCK_SCOPE(rwl)	\
	for(auto __MCF_LOCK__ = std::make_pair(static_cast<::MCF::ReadWriteLock &>(rwl).GetWriteLock(), true);	\
		__MCF_LOCK__.second;	\
		__MCF_LOCK__.second = false)

#endif
