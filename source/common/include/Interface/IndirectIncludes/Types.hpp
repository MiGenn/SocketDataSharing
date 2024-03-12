#pragma once
#include <cstdint>
#include <cstddef>

namespace SDS
{
	//Non-zero values mean no error.
	enum class ErrorIndicator : uint8_t
	{
		Error = 0
	};

	//Prefer using only the false value.
	enum class Bool : uint8_t
	{
		False = 0,
		True = 1,
	};

	enum class ErrorBool : uint8_t
	{
		Error = 0,
		False = 1,
		True = 2
	};

	struct alignas(1) ErrorSupportedProtocols final
	{
		ErrorIndicator errorIndicator;

		Bool isIPv4TCPSupported;
		Bool isIPv4UDPSupported;
		Bool isIPv6TCPSupported;
		Bool isIPv6UDPSupported;
	};

	//Zero address means no address.
	struct alignas(4) IPv4Address final
	{
		uint8_t octets[4];
	};

	//Zero address means no address.
	//If you manually create an instance of the structure, set the scopeID and the flowInfo to zero for any address.
	//Manually created link-local addresses shouldn't be used for creating a socket.
	struct alignas(8) IPv6Address final
	{
		uint16_t hextets[8];

		//This member is non-zero only if the address is link-local. 
		//It is used to distinguish the host's NICs and is always in host byte order.
		uint32_t scopeID; 

		uint32_t flowInfo; //This member is always zero and should be ignored. No one knows what to do with it.
	};

	//It's legal for one of the IP addresses to be zero.
	struct alignas(8) NetworkIPAddresses final
	{
		uint8_t v4NetworkPrefixLength; //Valid values range from 1 to 64 (inclusive).
		uint8_t v6NetworkPrefixLength; //Valid values range from 1 to 128 (inclusive).

		std::byte __padding[2]; //This must be ignored.

		IPv4Address v4;
		IPv6Address v6;
	};
}