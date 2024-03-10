#pragma once
#include "IndirectIncludes/Types.hpp"

#include "IndirectIncludes/SocketDataSharingAPIDefine.hpp"

namespace SDS::IPv6AddressUtils
{
	//BO means byte order.

	SOCKETDATASHARING_API IPv6Address ToHostBO(const IPv6Address* addressInNetworkBO) noexcept;
	SOCKETDATASHARING_API IPv6Address ToNetworkBO(const IPv6Address* addressInHostBO) noexcept;

	SOCKETDATASHARING_API ErrorBool IsZero(const IPv6Address* address) noexcept;
	SOCKETDATASHARING_API ErrorBool IsLoopback(const IPv6Address* address) noexcept;
	SOCKETDATASHARING_API ErrorBool IsLoopbackInNetworkBO(const IPv6Address* addressInNetworkBO) noexcept;
	SOCKETDATASHARING_API ErrorBool IsLinkLocal(const IPv6Address* address) noexcept;
	SOCKETDATASHARING_API ErrorBool IsLinkLocalInNetworkBO(const IPv6Address* addressInNetworkBO) noexcept;
	SOCKETDATASHARING_API ErrorBool IsPrivate(const IPv6Address* address) noexcept;
	SOCKETDATASHARING_API ErrorBool IsPrivateInNetworBO(const IPv6Address* addressInNetworkBO) noexcept;
}