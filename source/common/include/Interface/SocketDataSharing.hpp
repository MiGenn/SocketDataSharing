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
		using SocketHandle = void*;
	}
}