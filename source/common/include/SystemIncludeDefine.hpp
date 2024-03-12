#ifdef __ANDROID__
	//TODO: include Android API
#elif defined _WIN64
	#include "WinAPI.hpp"
#else
	#error "This platform is not supported"
#endif