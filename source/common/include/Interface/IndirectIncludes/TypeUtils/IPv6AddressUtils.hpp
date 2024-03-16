#pragma once
#include "IndirectIncludes/Types.hpp"

#include "IndirectIncludes/SocketDataSharingAPIDefine.hpp"

namespace SDS
{
	extern "C"
	{
		//BO means byte order.

		SOCKETDATASHARING_API IPv6Address ToIPv6AddressInHostBO(IPv6Address addressInNetworkBO) noexcept;
		SOCKETDATASHARING_API IPv6Address ToIPv6AddressInNetworkBO(IPv6Address addressInHostBO) noexcept;

		SOCKETDATASHARING_API ErrorBool IsIPv6AddressZero(IPv6Address address) noexcept;
		SOCKETDATASHARING_API ErrorBool IsIPv6AddressLoopback(IPv6Address address) noexcept;
		SOCKETDATASHARING_API ErrorBool IsIPv6AddressLoopbackInNetworkBO(IPv6Address addressInNetworkBO) noexcept;
		SOCKETDATASHARING_API ErrorBool IsIPv6AddressLinkLocal(IPv6Address address) noexcept;
		SOCKETDATASHARING_API ErrorBool IsIPv6AddressLinkLocalInNetworkBO(IPv6Address addressInNetworkBO) noexcept;
		SOCKETDATASHARING_API ErrorBool IsIPv6AddressPrivate(IPv6Address address) noexcept;
		SOCKETDATASHARING_API ErrorBool IsIPv6AddressPrivateInNetworBO(IPv6Address addressInNetworkBO) noexcept;
	}
}