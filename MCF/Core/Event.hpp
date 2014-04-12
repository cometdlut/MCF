// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_EVENT_HPP__
#define __MCF_EVENT_HPP__

#include "Utilities.hpp"
#include "StringObserver.hpp"
#include <memory>

namespace MCF {

class Event : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<Event> Create(bool bInitSet, const WideStringObserver &wsoName = nullptr);

public:
	bool IsSet() const noexcept;
	void Set() noexcept;
	void Clear() noexcept;

	bool WaitTimeout(unsigned long ulMilliSeconds) const noexcept;
	void Wait() const noexcept;
};

}

#endif
