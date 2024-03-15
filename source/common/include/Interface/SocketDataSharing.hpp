#pragma once
#include <cstdint>
#include "IndirectIncludes/TypeUtils.hpp"

#include "IndirectIncludes/SocketDataSharingAPIDefine.hpp"

//The library is not completely thread safe and uses only non-blocking sockets!
//In order to use the static version of the library you need to define SOCKETDATASHARING_STATIC.

namespace SDS
{
	extern "C"
	{
		using SocketHandle = void*;

		//BO means byte order.

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

		//This function accepts any addresses even zero ones.
		SOCKETDATASHARING_API ErrorBool IsIPv4AddressPreferred(const NetworkIPAddresses* networkIPAddressesInNetworkBO) noexcept;

		//Passing a zero address is illegal.
		//Passing 0 will assign a random port number within the inclusive range of 49152 to 65535.
		//In this case, the random number will be assigned to the portNumberInHostBO_inout.
		//If an error occured, the returned pointer is zero.
		SOCKETDATASHARING_API SocketHandle CreateIPv4TCPSocket(IPv4Address ipv4Address, uint16_t* portNumberInHostBO_inout) noexcept;

		//Passing a zero address is illegal.
		//Passing 0 will assign a random port number within the inclusive range of 49152 to 65535.
		//In this case, the random number will be assigned to the portNumberInHostBO_inout.
		//If an error occured, the returned pointer is zero.
		SOCKETDATASHARING_API SocketHandle CreateIPv4UDPSocket(IPv4Address ipv4Address, uint16_t* portNumberInHostBO_inout) noexcept;

		//Passing a zero address is illegal.
		//Passing 0 will assign a random port number within the inclusive range of 49152 to 65535.
		//In this case, the random number will be assigned to the portNumberInHostBO_inout.
		//If an error occured, the returned pointer is zero.
		SOCKETDATASHARING_API SocketHandle CreateIPv6TCPSocket(IPv6Address ipv6AddressInNetworkBO, uint16_t* portNumberInHostBO_inout) noexcept;

		//Passing a zero address is illegal.
		//Passing 0 will assign a random port number within the inclusive range of 49152 to 65535.
		//In this case, the random number will be assigned to the portNumberInHostBO_inout.
		//If an error occured, the returned pointer is zero.
		SOCKETDATASHARING_API SocketHandle CreateIPv6UDPSocket(IPv6Address ipv6AddressInNetworkBO, uint16_t* portNumberInHostBO_inout) noexcept;

		//TODO: create a function to create and connect a socket.

		//This function only works with TCP sockets.
		//Once you activate the mode, you can't deactivate it and change the queue size.
		//Set the queue size as small as possible to save the system resources. Big values are capped by the system.
		//It's recommended to pass a number within the inclusive range of 0 to 128.
		//Connections which have no place in the queue are rejected (Error::AnotherHostRejectedConnection).
		SOCKETDATASHARING_API ErrorIndicator SetSocketInListeningMode(SocketHandle socketHandle, int32_t pendingConnectionQueueSize) noexcept;

		//This function can only be used with listening sockets.
		//Call it to pop the pending connecion queue. If the queue is empty, it will set the newConnectionSocketHandle_out to null.
		SOCKETDATASHARING_API ErrorIndicator AcceptNewConnectionFor(SocketHandle listeningSocketHandle, SocketHandle* newConnectionSocketHandle_out) noexcept;

		//Peer is just another host.
		//This function returns socketAddresses in network byte order.
		SOCKETDATASHARING_API ErrorIPSocketAddress GetConnectedPeerIPSocketAddress(SocketHandle connectedSocketHandle) noexcept;

		//This function may or may not destroy the socket immediately, if it is a TCP one.
		//It depends on what you passed to the SetSocketDestructionTimeout function.
		//The socket handle will become unusable if no error occured.
		SOCKETDATASHARING_API ErrorIndicator DestroySocket(SocketHandle socketHandle) noexcept;

		//Nagle's algorithm creates delays to group small packets into one large packet.
		//The option is set to Bool::True by default.
		SOCKETDATASHARING_API ErrorIndicator SetTCPSocketNaglesAlgorithm(SocketHandle socketHandle, Bool isEnabled) noexcept;

		//This function allows you to configure how the TCP socket connection will be closed in the DestroySocket function.
		//Passing Bool::False will configure the socket to gracefully close the connection (the time parameter will be ignored).
		//Passing non-Bool::False and setting time to zero will configure the socket to abruptly close the connection.
		//Passing non-Bool::False and setting time to non-zero will configure the socket to continue to function untill
		//the timeout is expired. After that, the socket will abruptly close the connection.
		//The option is set to Bool::False by default.
		SOCKETDATASHARING_API ErrorIndicator SetSocketDestructionTimeout(SocketHandle socketHandle, Bool isEnabled, uint16_t timeInSeconds) noexcept;

		//This function only works with IPv4 UDP sockets.
		//The option is set to Bool::False by default.
		SOCKETDATASHARING_API ErrorIndicator SetSocketBroadcast(SocketHandle socketHandle, Bool isEnabled) noexcept;
	}
}