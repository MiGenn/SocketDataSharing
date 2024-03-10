#pragma once
#include <cstdint>
#include <cstddef>

namespace SDS
{
	enum class ErrorIndicator : uint8_t
	{
		Error = 0
	};

	enum class ErrorBool : uint8_t
	{
		Error = 0,
		False = 1,
		True = 2
	};

	struct alignas(4) IPv4Address final
	{
		uint8_t octets[4];
	};

	struct alignas(8) IPv6Address final
	{
		uint16_t hextets[8];
	};

	//One of the IP addresses can be zero.
	struct alignas(8) NetworkIPAddresses final
	{
		//Valid values range from 1 to 64 (inclusive).
		uint8_t v4NetworkPrefixLength;

		//Valid values range from 1 to 128 (inclusive).
		uint8_t v6NetworkPrefixLength;

		//This must be ignored.
		std::byte __padding[2];

		IPv4Address v4;
		IPv6Address v6;
	};
}