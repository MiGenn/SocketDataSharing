#pragma once
#include <cstdint>
#include "IndirectIncludes/TypeUtils.hpp"

#include "IndirectIncludes/SocketDataSharingAPIDefine.hpp"

//The library is not thread safe!
//In order to use static version of the library you need to define SOCKETDATASHARING_STATIC.

namespace SDS
{
	extern "C"
	{
		using SocketHandle = void*;

		//The second function to be called if you have called SetErrorOccuredCallback.
		//Calling this function is required if you want to use any function from this header.
		SOCKETDATASHARING_API ErrorIndicator Initialize() noexcept;

		//The last function to be called. Must be called only after the Initialize function has been called.
		//It automatically destroys all created sockets.
		SOCKETDATASHARING_API ErrorIndicator Shutdown() noexcept;

		//This function returns all supported protocols by the system and the library.
		SOCKETDATASHARING_API ErrorSupportedProtocols EnumerateSupportedProtocols() noexcept;

		//Host can be connected to multiple networks and have more than one IP address per network.
		//This function returns all IP addresses assigned to each network. Some IP addresses can be zero but only one per network.
		//If the host isn't connected to any network the returned IP address count is 0 but the data pointer isn't null.
		//The NetworkIPAddresses array pointer is null only if an error occured and you don't need to deallocate the memory.
		//The returned IPv6 addresses are in network byte order.
		//Don't store the pointer and the size because their values may be changed after another GetNetworkIPAddressesArray call.
		SOCKETDATASHARING_API NetworkIPAddresses* GetNetworkIPAddressesArray(int32_t* size_out) noexcept;

		//This function accepts any addresses even zero ones. BO means byte order.
		SOCKETDATASHARING_API ErrorBool IsIPv4AddressPreferred(const NetworkIPAddresses* networkIPAddressesInNetworkBO) noexcept;

		//Passing a zero address is illegal. TODO: create a function to create a socket and bind it.
		//Passed port number must range from 1024 to 49151 (inclusive).
		//You can also pass 0 which will assign a random port number within 49152-65535 range (inclusive).
		//In this case, the random port number will be assigned to the portNumberInHostBO_inout.
		//If an error occured, the returned pointer is zero.
		SOCKETDATASHARING_API SocketHandle CreateIPv4TCPSocket(IPv4Address ipv4Address, uint16_t* portNumberInHostBO_inout) noexcept;

		//Passing a zero address is illegal. TODO:
		//Passed port number must range from 1024 to 49151 (inclusive).
		//You can also pass 0 which will assign a random port number within 49152-65535 range (inclusive).
		//In this case, the random port number will be assigned to the portNumberInHostBO_inout.
		//If an error occured, the returned pointer is zero.
		SOCKETDATASHARING_API SocketHandle CreateIPv4UDPSocket(IPv4Address ipv4Address, uint16_t* portNumberInHostBO_inout) noexcept;

		//Passing a zero address is illegal. TODO:
		//Passed port number must range from 1024 to 49151 (inclusive).
		//You can also pass 0 which will assign a random port number within 49152-65535 range (inclusive).
		//In this case, the random port number will be assigned to the portNumberInHostBO_inout.
		//If an error occured, the returned pointer is zero.
		SOCKETDATASHARING_API SocketHandle CreateIPv6TCPSocket(IPv6Address ipv6AddressInNetworkBO, uint16_t* portNumberInHostBO_inout) noexcept;

		//Passing a zero address is illegal. TODO:
		//Passed port number must range from 1024 to 49151 (inclusive).
		//You can also pass 0 which will assign a random port number within 49152-65535 range (inclusive).
		//In this case, the random port number will be assigned to the portNumberInHostBO_inout.
		//If an error occured, the returned pointer is zero.
		SOCKETDATASHARING_API SocketHandle CreateIPv6UDPSocket(IPv6Address ipv6AddressInNetworkBO, uint16_t* portNumberInHostBO_inout) noexcept;

		//The socket handle will become unusable if no error occured.
		//TODO: add a comment regarding set socket options.
		SOCKETDATASHARING_API ErrorIndicator DestroySocket(SocketHandle socketHandle) noexcept;
	}
}