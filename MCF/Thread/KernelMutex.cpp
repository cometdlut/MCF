// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "KernelMutex.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Clocks.hpp"
#include <winternl.h>
#include <ntdef.h>
#include <ntstatus.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtOpenEvent(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtCreateEvent(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes, EVENT_TYPE eEventType, BOOLEAN bInitialState) noexcept;

typedef enum tagEventInformationClass {
	EventBasicInformation,
} EVENT_INFORMATION_CLASS;

typedef struct tagEVENT_BASIC_INFORMATION {
	EVENT_TYPE eEventType;
	LONG lState;
} EVENT_BASIC_INFORMATION;

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtQueryEvent(HANDLE hEvent, EVENT_INFORMATION_CLASS eInfoClass, void *pInfo, DWORD dwInfoSize, DWORD *pdwInfoSizeRet) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtSetEvent(HANDLE hEvent, LONG *plPrevState) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtResetEvent(HANDLE hEvent, LONG *plPrevState) noexcept;

namespace MCF {

Impl_UniqueNtHandle::UniqueNtHandle KernelMutex::X_CreateEventHandle(const WideStringView &wsvName, std::uint32_t u32Flags){
	Impl_UniqueNtHandle::UniqueNtHandle hRootDirectory;
	::OBJECT_ATTRIBUTES vObjectAttributes;
	const auto uNameSize = wsvName.GetSize() * sizeof(wchar_t);
	if(uNameSize == 0){
		InitializeObjectAttributes(&vObjectAttributes, nullptr, 0, nullptr, nullptr);
	} else {
		if(uNameSize > USHRT_MAX){
			DEBUG_THROW(SystemException, ERROR_INVALID_PARAMETER, "The name for a kernel object is too long"_rcs);
		}
		::UNICODE_STRING ustrObjectName;
		ustrObjectName.Length        = (USHORT)uNameSize;
		ustrObjectName.MaximumLength = (USHORT)uNameSize;
		ustrObjectName.Buffer        = (PWSTR)wsvName.GetBegin();

		ULONG ulAttributes;
		if(u32Flags & kFailIfExists){
			ulAttributes = 0;
		} else {
			ulAttributes = OBJ_OPENIF;
		}

		hRootDirectory = Y_OpenBaseNamedObjectDirectory(u32Flags);

		InitializeObjectAttributes(&vObjectAttributes, &ustrObjectName, ulAttributes, hRootDirectory.Get(), nullptr);
	}

	HANDLE hTemp;
	bool bNameExists;
	if(u32Flags & kDontCreate){
		const auto lStatus = ::NtOpenEvent(&hTemp, EVENT_ALL_ACCESS, &vObjectAttributes);
		if(!NT_SUCCESS(lStatus)){
			DEBUG_THROW(SystemException, ::RtlNtStatusToDosError(lStatus), "NtOpenEvent"_rcs);
		}
		bNameExists = true;
	} else {
		const auto lStatus = ::NtCreateEvent(&hTemp, EVENT_ALL_ACCESS, &vObjectAttributes, SynchronizationEvent, true);
		if(!NT_SUCCESS(lStatus)){
			DEBUG_THROW(SystemException, ::RtlNtStatusToDosError(lStatus), "NtCreateEvent"_rcs);
		}
		bNameExists = lStatus == STATUS_OBJECT_NAME_EXISTS;
	}
	Impl_UniqueNtHandle::UniqueNtHandle hEvent(hTemp);

	if(bNameExists){
		EVENT_BASIC_INFORMATION vBasicInfo;
		const auto lStatus = ::NtQueryEvent(hEvent.Get(), EventBasicInformation, &vBasicInfo, sizeof(vBasicInfo), nullptr);
		if(!NT_SUCCESS(lStatus)){
			ASSERT_MSG(false, L"NtQueryEvent() 失败。");
		}
		if(vBasicInfo.eEventType != SynchronizationEvent){
			DEBUG_THROW(SystemException, ::RtlNtStatusToDosError(STATUS_OBJECT_TYPE_MISMATCH), "CreateEventHandle"_rcs);
		}
	}
	return hEvent;
}

bool KernelMutex::Try(std::uint64_t u64UntilFastMonoClock) noexcept {
	::LARGE_INTEGER liTimeout;
	liTimeout.QuadPart = 0;
	if(u64UntilFastMonoClock != 0){
		const auto u64Now = GetFastMonoClock();
		if(u64Now < u64UntilFastMonoClock){
			const auto u64DeltaMillisec = u64UntilFastMonoClock - u64Now;
			const auto n64Delta100Nanosec = static_cast<std::int64_t>(u64DeltaMillisec * 10000);
			if(static_cast<std::uint64_t>(n64Delta100Nanosec / 10000) != u64DeltaMillisec){
				liTimeout.QuadPart = INT64_MIN;
			} else {
				liTimeout.QuadPart = -n64Delta100Nanosec;
			}
		}
	}
	const auto lStatus = ::NtWaitForSingleObject(x_hEvent.Get(), false, &liTimeout);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForSingleObject() 失败。");
	}
	return lStatus != STATUS_TIMEOUT;
}
void KernelMutex::Lock() noexcept {
	const auto lStatus = ::NtWaitForSingleObject(x_hEvent.Get(), false, nullptr);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForSingleObject() 失败。");
	}
}
void KernelMutex::Unlock() noexcept {
	LONG lPrevState;
	const auto lStatus = ::NtSetEvent(x_hEvent.Get(), &lPrevState);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtSetEvent() 失败。");
	}
	ASSERT_MSG(lPrevState == 0, L"互斥锁没有被任何线程锁定。");
}

}
