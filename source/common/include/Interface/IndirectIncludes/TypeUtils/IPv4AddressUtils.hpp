#pragma once
#include "IndirectIncludes/Types.hpp"

#include "IndirectIncludes/SocketDataSharingAPIDefine.hpp"

namespace SDS
{
	extern "C"
	{
		SOCKETDATASHARING_API ErrorBool IsIPv4AddressZero(IPv4Address address) noexcept;
		SOCKETDATASHARING_API ErrorBool IsIPv4AddressLoopback(IPv4Address address) noexcept;
		SOCKETDATASHARING_API ErrorBool IsIPv4AddressLinkLocal(IPv4Address address) noexcept;
		SOCKETDATASHARING_API ErrorBool IsIPv4AddressPrivate(IPv4Address address) noexcept;
	}
}