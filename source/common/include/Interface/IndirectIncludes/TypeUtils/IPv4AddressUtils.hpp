#pragma once
#include "IndirectIncludes/Types.hpp"

#include "IndirectIncludes/SocketDataSharingAPIDefine.hpp"

namespace SDS::IPv4AddressUtils
{
	SOCKETDATASHARING_API ErrorBool IsZero(const IPv4Address* address) noexcept;
	SOCKETDATASHARING_API ErrorBool IsLoopback(const IPv4Address* address) noexcept;
	SOCKETDATASHARING_API ErrorBool IsLinkLocal(const IPv4Address* address) noexcept;
	SOCKETDATASHARING_API ErrorBool IsPrivate(const IPv4Address* address) noexcept;
}