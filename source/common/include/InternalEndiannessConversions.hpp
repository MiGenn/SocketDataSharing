#pragma once

#ifdef __ANDROID__

#elif defined(_WIN64)
	#include "WinAPI.hpp"
#else
	#error "This platform is not supported"
#endif

//BO means byte order.

inline uint64_t HostToNetworkBO(uint64_t value) noexcept
{
#ifdef __ANDROID__

#elif defined(_WIN64)
	return htonll(value);
#endif
}

inline int64_t HostToNetworkBO(int64_t value) noexcept
{
	auto convertedValue = HostToNetworkBO(reinterpret_cast<uint64_t&>(value));
	return reinterpret_cast<int64_t&>(convertedValue);
}

inline uint32_t HostToNetworkBO(uint32_t value) noexcept
{
#ifdef __ANDROID__

#elif defined(_WIN64)
	return htonl(value);
#endif
}

inline int32_t HostToNetworkBO(int32_t value) noexcept
{
	auto convertedValue = HostToNetworkBO(reinterpret_cast<uint32_t&>(value));
	return reinterpret_cast<int32_t&>(convertedValue);
}

inline uint16_t HostToNetworkBO(uint16_t value) noexcept
{
#ifdef __ANDROID__

#elif defined(_WIN64)
	return htons(value);
#endif
}

inline int16_t HostToNetworkBO(int16_t value) noexcept
{
	auto convertedValue = HostToNetworkBO(reinterpret_cast<uint16_t&>(value));
	return reinterpret_cast<int16_t&>(convertedValue);
}

inline uint64_t NetworkToHostBO(uint64_t value) noexcept
{
#ifdef __ANDROID__

#elif defined(_WIN64)
	return ntohll(value);
#endif
}

inline int64_t NetworkToHostBO(int64_t value) noexcept
{
	auto convertedValue = NetworkToHostBO(reinterpret_cast<uint64_t&>(value));
	return reinterpret_cast<int64_t&>(convertedValue);
}

inline uint32_t NetworkToHostBO(uint32_t value) noexcept
{
#ifdef __ANDROID__

#elif defined(_WIN64)
	return ntohl(value);
#endif
}

inline int32_t NetworkToHostBO(int32_t value) noexcept
{
	auto convertedValue = NetworkToHostBO(reinterpret_cast<uint32_t&>(value));
	return reinterpret_cast<int32_t&>(convertedValue);
}

inline uint16_t NetworkToHostBO(uint16_t value) noexcept
{
#ifdef __ANDROID__

#elif defined(_WIN64)
	return ntohs(value);
#endif
}

inline int16_t NetworkToHostBO(int16_t value) noexcept
{
	auto convertedValue = NetworkToHostBO(reinterpret_cast<uint16_t&>(value));
	return reinterpret_cast<int16_t&>(convertedValue);
}