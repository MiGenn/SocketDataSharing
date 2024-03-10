#pragma once
#include <cstdint>

#include "IndirectIncludes/SocketDataSharingAPIDefine.hpp"

namespace SDS
{
	extern "C"
	{
		//TODO: manually assign numbers when the library is production ready.
		enum class Error : int32_t
		{
			Success,
			UnexpectedSystemError,

			NotEnoughMemory,
			PassedPointerIsNull,
			NotSupportedMachine,
			IsAlreadyInitialized,
			IsNotInitialized,

			NetworkSubsystemIsUnavailable,
			TooManyApplicationsAreUsingSystemLibrary,
			NetworkIsDown,
		};

		//Corresponding system error should be ignored unless the error is Error::UnexpectedSystemError.
		typedef void(*ErrorOccuredCallback)(Error error, int64_t correspondingSystemError, void* callbackContext);

		//The callback is set to an empty function by default. So that the library won't crash if you don't provide a callback.
		//But you should set the callback to be able to handle errors.
		//The callback will be called once with a value of Error::Success to check its validity.
		//Passing an invalid callback will throw an exception.
		SOCKETDATASHARING_API void SetErrorOccuredCallback(ErrorOccuredCallback callback, void* callbackContext);
	}
}