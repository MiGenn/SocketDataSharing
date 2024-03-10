#ifdef SOCKETDATASHARING_EXPORTS
	#if defined(__ANDROID__) && defined(__clang__)
		#define SOCKETDATASHARING_API
	#elif defined(_WIN64) && defined(_MSC_VER)
		#ifdef SOCKETDATASHARING_STATIC
			#define SOCKETDATASHARING_API
		#else
			#define SOCKETDATASHARING_API __declspec(dllexport)
		#endif
	#else
		#error "This platform or/and compiler are not supported"
	#endif
#else
	#ifdef defined(__ANDROID__) && defined(__clang__)
		#define SOCKETDATASHARING_API
	#elif defined(_WIN64) && defined(_MSC_VER)
		#ifdef SOCKETDATASHARING_STATIC
			#define SOCKETDATASHARING_API
		#else
			#define SOCKETDATASHARING_API __declspec(dllimport)
		#endif
	#else
		#error "This platform or/and compiler are not supported"
	#endif
#endif