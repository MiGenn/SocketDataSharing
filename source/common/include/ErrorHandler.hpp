#pragma once
#include "Error.hpp"

#ifdef __ANDROID__

#elif defined _WIN64
	#include "WinAPI.hpp"
#endif

class ErrorHandler final
{
public:
	ErrorHandler() = delete;
	ErrorHandler(const ErrorHandler&) = delete;
	ErrorHandler(ErrorHandler&&) = delete;
	~ErrorHandler() = delete;

	//Error::Success and Error::UnexpectedSystemError are invalid.
	static void SignalError(SDS::Error error) noexcept;

	//This function will throw an exception if the passed callback is invalid.
	static void SetCallback(SDS::ErrorOccuredCallback callback, void* callbackContext);

	ErrorHandler& operator=(const ErrorHandler&) = delete;
	ErrorHandler& operator=(ErrorHandler&&) = delete;

	//Handle functions must be called only when an error occured.

#ifdef __ANDROID__

#elif defined _WIN64
	static void Handle_WSAStartup(int errorCode) noexcept;
	static void Handle_WSACleanup() noexcept;
	static void Handle_GetAdaptersAddresses(ULONG errorCode) noexcept;
#endif

private:
	//The callback is set to an empty function by default.
	static SDS::ErrorOccuredCallback m_callback;
	inline static void* m_callbackContext = nullptr;
};