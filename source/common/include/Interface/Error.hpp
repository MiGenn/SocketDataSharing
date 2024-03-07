#pragma once
#include <cstdint>

#ifdef SOCKETDATASHARING_EXPORTS
	#ifdef __ANDROID__
		#define SOCKETDATASHARING_API
	#elif defined _WIN64
		#define SOCKETDATASHARING_API __declspec(dllexport)
	#else
		#error "This platform is not supported"
	#endif
#else
	#ifdef __ANDROID__
		#define SOCKETDATASHARING_API
	#elif defined _WIN64
		#define SOCKETDATASHARING_API __declspec(dllimport)
	#else
		#error "This platform is not supported"
	#endif
#endif

namespace SDS
{
	extern "C"
	{
		//TODO: manually assign numbers when the library is production ready.
		enum class Error : uintmax_t
		{
			Success,
			UnexpectedSystemError,

			NetworkSubsystemIsUnavailable,
			TooManyApplicationsAreUsingSystemLibrary,
			NetworkIsDown,
		};

		//Corresponding system error should be ignored unless the error is Error::UnexpectedSystemError.
		typedef void(*ErrorOccuredCallback)(Error error, intmax_t correspondingSystemError, void* callbackContext);

		//The callback is set to an empty function by default. So that the library won't crash if you don't provide one.
		//But you should call this function to be able to handle errors. It should be the first function you call.
		//Passing an invalid callback will throw an exception.
		SOCKETDATASHARING_API void SetErrorOccuredCallback(ErrorOccuredCallback callback, void* callbackContext);
	}
}